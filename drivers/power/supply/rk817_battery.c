#include <linux/delay.h>
#include <linux/extcon.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/iio/consumer.h>
#include <linux/iio/iio.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/mfd/rk808.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/rtc.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>

enum charge_status {
	CHRG_OFF,
	DEAD_CHRG,
	TRICKLE_CHRG,
	CC_OR_CV_CHRG,
	CHARGE_FINISH,
	USB_OVER_VOL,
	BAT_TMP_ERR,
	BAT_TIM_ERR,
};

struct rk817_bat {
	struct platform_device *pdev;
	struct device *dev;
	struct i2c_client *client;

	struct power_supply *bat_ps;

	struct regmap *regmap;
	struct regmap_field *rmap_fields[ARRAY_SIZE(rk817_bat_reg_fields)];

	int design_capacity;
	int design_qmax;

	int voltage_k;
	int voltage_b;
	int res_div;
	u8 status;
};

static int rk817_get_charge_now(struct rk817_bat *battery)
{
	u32 val = 0;
	int tmp, ret, charge;

	ret = regmap_field_read(battery->rmap_fields[Q_PRES_H3], &tmp);
	if (ret)
		return ret;
	
	if (!(tmp & 0x80))
		return -EINVAL;
	
	ret = regmap_field_read(battery->rmap_fields[Q_PRES_H3], &tmp);
	if (ret)
		return ret;
	val |= tmp << 24;

	ret = regmap_field_read(battery->rmap_fields[Q_PRES_H2], &tmp);
	if (ret)
		return ret;
	val |= tmp << 16;

	ret = regmap_field_read(battery->rmap_fields[Q_PRES_L1], &tmp);
	if (ret)
		return ret;
	val |= tmp << 8;

	ret = regmap_field_read(battery->rmap_fields[Q_PRES_L0], &tmp);
	if (ret)
		return ret;
	val |= tmp; 

	charge = (val / 3600) * (172 / battery->res_div);

	printk("charge: %u, val: %u", charge, val);
	return charge;
}

static int rk817_get_reg_hl(struct rk817_bat *battery, int fieldH, int fieldL)
{
	int tmp;
	int out = 0;

	regmap_field_read(battery->rmap_fields[fieldL], &tmp);
	out |= tmp;

	regmap_field_read(battery->rmap_fields[fieldH], &tmp);
	out |= tmp << 8;

	return out;
}

static int rk817_bat_get_prop(struct power_supply *ps,
			      enum power_supply_property prop,
			      union power_supply_propval *val)
{
	struct rk817_bat *battery = power_supply_get_drvdata(ps);
	int tmp;
	int ret = 0;

	switch (prop) {
		case POWER_SUPPLY_PROP_STATUS:
			ret = regmap_field_read(battery->rmap_fields[CHG_STS], &tmp);
			if (ret)
				return ret;
			switch(tmp) {
				case CHRG_OFF:
					val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
					break;
				case DEAD_CHRG:
					val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
					break;
				case TRICKLE_CHRG:
				case CC_OR_CV_CHRG:
					val->intval = POWER_SUPPLY_STATUS_CHARGING;
					break;
				case CHARGE_FINISH:
					val->intval = POWER_SUPPLY_STATUS_FULL;
					break;
				default:
					val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
					printk("Error getting battery value, returned %d", tmp);
					return -EINVAL;

			}
			break;
		case POWER_SUPPLY_PROP_CHARGE_TYPE:
			ret = regmap_field_read(battery->rmap_fields[CHG_STS], &tmp);
			if (ret)
				return ret;
			switch(tmp) {
				case CHRG_OFF:
				case DEAD_CHRG:
					val->intval = POWER_SUPPLY_CHARGE_TYPE_NONE;
					break;
				case TRICKLE_CHRG:
					val->intval = POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
					break;
				case CC_OR_CV_CHRG:
					val->intval = POWER_SUPPLY_CHARGE_TYPE_FAST;
					break;
				default:
					val->intval = POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
					break;
			}
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_AVG:
			tmp = rk817_get_reg_hl(battery, BAT_VOL_H, BAT_VOL_L);
			val->intval = (battery->voltage_k * tmp / 1000 + battery->voltage_b) * 60 / 46;
			break;
		case POWER_SUPPLY_PROP_CHARGE_FULL:
		case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
			val->intval = battery->design_capacity * 1000;
			break;
		case POWER_SUPPLY_PROP_CHARGE_EMPTY:
			val->intval = 0;
			break;
		case POWER_SUPPLY_PROP_CHARGE_NOW:
			val->intval = rk817_get_charge_now(battery);
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static enum power_supply_property rk817_bat_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_VOLTAGE_AVG,

	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_EMPTY,
};

static const struct power_supply_desc rk817_desc = {
	.name = "rk817-battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = rk817_bat_props,
	.num_properties = ARRAY_SIZE(rk817_bat_props),
	.get_property = rk817_bat_get_prop,
};

static const struct of_device_id rk817_bat_of_match[] = {
	{ .compatible = "rk817,battery", },
	{ },
};
MODULE_DEVICE_TABLE(of, rk817_bat_of_match);


static int rk817_bat_probe(struct platform_device *pdev)
{
	struct rk817_bat *battery;
	struct device *dev = &pdev->dev;
	struct power_supply_config pscfg = {};
	int i, vcalib0, vcalib1;
	int *tmp = 0;
	int ret;

	if (!of_device_is_available(pdev->dev.of_node))
		return -ENODEV;

	battery = devm_kzalloc(&pdev->dev, sizeof(battery), GFP_KERNEL);

	if (!battery)
		return -ENOMEM;

	battery->dev = &pdev->dev;
	battery->regmap = dev_get_regmap(pdev->dev.parent, NULL);
	platform_set_drvdata(pdev, battery);

	for (i = 0; i < ARRAY_SIZE(rk817_bat_reg_fields); i++) {
		const struct reg_field *reg_fields = rk817_bat_reg_fields;

		battery->rmap_fields[i] = devm_regmap_field_alloc(dev, battery->regmap, reg_fields[i]);
		
		if (IS_ERR(battery->rmap_fields[i])) {
			dev_err(dev, "cannot allocate regmap field\n");
		
			return PTR_ERR(battery->rmap_fields[i]);
		}
	}

	/* voltage calibration data */
	vcalib0 = rk817_get_reg_hl(battery, VCALIB0_H, VCALIB0_L);
	vcalib1 = rk817_get_reg_hl(battery, VCALIB1_H, VCALIB1_L);

	battery->voltage_k = (4025 - 2300) * 1000 / ((vcalib1 - vcalib0) ? (vcalib1 - vcalib0) : 1);
	battery->voltage_b = 4025 - (battery->voltage_k * vcalib1) / 1000;

	pscfg.drv_data = battery;
	pscfg.of_node = pdev->dev.of_node;

	ret = of_property_read_u32(pdev->dev.of_node, "design_capacity", &battery->design_capacity);
	if (ret < 0)
		dev_err(dev, "Error reading design_capacity");

	ret = of_property_read_u32(pdev->dev.of_node, "design_qmax", &battery->design_qmax);
	if (ret < 0)
		dev_err(dev, "Error reading design_qmax");


	battery->bat_ps = devm_power_supply_register(&pdev->dev, &rk817_desc, &pscfg);

	return 0;
}


static struct platform_driver rk817_bat_driver = {
	.probe    = rk817_bat_probe,
	.driver   = {
		.name  = "rk817-battery",
		.of_match_table = rk817_bat_of_match,
	},
};
module_platform_driver(rk817_bat_driver);

MODULE_DESCRIPTION("Battery power supply driver for RK817 PMIC");
MODULE_AUTHOR("Maciej Matuszczyk <maccraft123mc@gmail.com>");
MODULE_LICENSE("GPL");


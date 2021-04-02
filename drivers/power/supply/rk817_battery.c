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

/*
static const struct reg_field rk817_battery_reg_fields[] = {
[ADC_SLP_RATE] = REG_FIELD(0x50, 0, 0),
[BAT_CUR_ADC_EN] = REG_FIELD(0x50, 2, 2),
[BAT_VOL_ADC_EN] = REG_FIELD(0x50, 3, 3),
[USB_VOL_ADC_EN] = REG_FIELD(0x50, 4, 4),
[TS_ADC_EN] = REG_FIELD(0x50, 5, 5),
[SYS_VOL_ADC_EN] = REG_FIELD(0x50, 6, 6),
[GG_EN] = REG_FIELD(0x50, 7, 7),*ADC_CONFIG0*

[CUR_ADC_DITH_SEL] = REG_FIELD(0x51, 1, 3),
[CUR_ADC_DIH_EN] = REG_FIELD(0x51, 4, 4),
[CUR_ADC_CHOP_EN] = REG_FIELD(0x51, 5, 5),
[CUR_ADC_CHOP_SEL] = REG_FIELD(0x51, 6, 6),
[CUR_ADC_CHOP_VREF_EN] = REG_FIELD(0x51, 7, 7), *CUR_ADC_COFG0*

[CUR_ADC_VCOM_SEL] = REG_FIELD(0x52, 0, 1),
[CUR_ADC_VCOM_BUF_INC] = REG_FIELD(0x52, 2, 2),
[CUR_ADC_VREF_BUF_INC] = REG_FIELD(0x52, 3, 3),
[CUR_ADC_BIAS_DEC] = REG_FIELD(0x52, 4, 4),
[CUR_ADC_IBIAS_SEL] = REG_FIELD(0x52, 5, 6), *CUR_ADC_COFG1*

[VOL_ADC_EXT_VREF_EN] = REG_FIELD(0x53, 0, 0),
[VOL_ADC_DITH_SEL]  = REG_FIELD(0x53, 1, 3),
[VOL_ADC_DITH_EN] = REG_FIELD(0x53, 4, 4),
[VOL_ADC_CHOP_EN] = REG_FIELD(0x53, 5, 5),
[VOL_ADC_CHOP_SEL] = REG_FIELD(0x53, 6, 6),
[VOL_ADC_CHOP_VREF_EN] = REG_FIELD(0x53, 7, 7),*VOL_ADC_COFG0*

[VOL_ADC_VCOM_SEL] = REG_FIELD(0x54, 0, 1),
[VOL_ADC_VCOM_BUF_INC] = REG_FIELD(0x54, 2, 2),
[VOL_ADC_VREF_BUF_INC] = REG_FIELD(0x54, 3, 3),
[VOL_ADC_IBIAS_SEL] = REG_FIELD(0x54, 5, 6), *VOL_ADC_COFG1*

[RLX_CUR_FILTER] = REG_FIELD(0x55, 0, 1),
[TS_FUN] = REG_FIELD(0x55, 3, 3),
[VOL_ADC_TSCUR_SEL] = REG_FIELD(0x55, 4, 5),
[VOL_CALIB_UPD] = REG_FIELD(0x55, 6, 6),
[CUR_CALIB_UPD] = REG_FIELD(0x55, 7, 7), *ADC_CONFIG1*

[CUR_OUT_MOD] = REG_FIELD(0x56, 0, 0),
[VOL_OUT_MOD] = REG_FIELD(0x56, 1, 1),
[FRAME_SMP_INTERV] = REG_FIELD(0x56, 2, 3),
[ADC_OFF_CAL_INTERV] = REG_FIELD(0x56, 4, 5),
[RLX_SPT] = REG_FIELD(0x56, 6, 7), *GG_CON*

[OCV_UPD] = REG_FIELD(0x57, 0, 0),
[RELAX_STS] = REG_FIELD(0x57, 1, 1),
[RELAX_VOL2_UPD] = REG_FIELD(0x57, 2, 2),
[RELAX_VOL1_UPD] = REG_FIELD(0x57, 3, 3),
[BAT_CON] = REG_FIELD(0x57, 4, 4),
[QMAX_UPD_SOFT] = REG_FIELD(0x57, 5, 5),
[TERM_UPD] = REG_FIELD(0x57, 6, 6),
[OCV_STS] = REG_FIELD(0x57, 7, 7), *GG_STS*

[RELAX_THRE_H] = REG_FIELD(0x58, 0, 7),
[RELAX_THRE_L] = REG_FIELD(0x59, 0, 7),

[RELAX_VOL1_H] = REG_FIELD(0x5A, 0, 7),
[RELAX_VOL1_L] = REG_FIELD(0x5B, 0, 7),
[RELAX_VOL2_H] = REG_FIELD(0x5C, 0, 7),
[RELAX_VOL2_L] = REG_FIELD(0x5D, 0, 7),

[RELAX_CUR1_H] = REG_FIELD(0x5E, 0, 7),
[RELAX_CUR1_L] = REG_FIELD(0x5F, 0, 7),
[RELAX_CUR2_H] = REG_FIELD(0x60, 0, 7),
[RELAX_CUR2_L] = REG_FIELD(0x61, 0, 7),

[OCV_THRE_VOL] = REG_FIELD(0x62, 0, 7),

[OCV_VOL_H] = REG_FIELD(0x63, 0, 7),
[OCV_VOL_L] = REG_FIELD(0x64, 0, 7),
[OCV_VOL0_H] = REG_FIELD(0x65, 0, 7),
[OCV_VOL0_L] = REG_FIELD(0x66, 0, 7),
[OCV_CUR_H] = REG_FIELD(0x67, 0, 7),
[OCV_CUR_L] = REG_FIELD(0x68, 0, 7),
[OCV_CUR0_H] = REG_FIELD(0x69, 0, 7),
[OCV_CUR0_L] = REG_FIELD(0x6A, 0, 7),
[PWRON_VOL_H] = REG_FIELD(0x6B, 0, 7),
[PWRON_VOL_L] = REG_FIELD(0x6C, 0, 7),
[PWRON_CUR_H] = REG_FIELD(0x6D, 0, 7),
[PWRON_CUR_L] = REG_FIELD(0x6E, 0, 7),
[OFF_CNT] = REG_FIELD(0x6F, 0, 7),
[Q_INIT_H3] = REG_FIELD(0x70, 0, 7),
[Q_INIT_H2] = REG_FIELD(0x71, 0, 7),
[Q_INIT_L1] = REG_FIELD(0x72, 0, 7),
[Q_INIT_L0] = REG_FIELD(0x73, 0, 7),

[Q_PRESS_H3] = REG_FIELD(0x74, 0, 7),
[Q_PRESS_H2] = REG_FIELD(0x75, 0, 7),
[Q_PRESS_L1] = REG_FIELD(0x76, 0, 7),
[Q_PRESS_L0] = REG_FIELD(0x77, 0, 7),

[BAT_VOL_H] = REG_FIELD(0x78, 0, 7),
[BAT_VOL_L] = REG_FIELD(0x79, 0, 7),

[BAT_CUR_H] = REG_FIELD(0x7A, 0, 7),
[BAT_CUR_L] = REG_FIELD(0x7B, 0, 7),

[BAT_TS_H] = REG_FIELD(0x7C, 0, 7),
[BAT_TS_L] = REG_FIELD(0x7D, 0, 7),
[USB_VOL_H] = REG_FIELD(0x7E, 0, 7),
[USB_VOL_L] = REG_FIELD(0x7F, 0, 7),

[SYS_VOL_H] = REG_FIELD(0x80, 0, 7),
[SYS_VOL_L] = REG_FIELD(0x81, 0, 7),
[Q_MAX_H3] = REG_FIELD(0x82, 0, 7),
[Q_MAX_H2] = REG_FIELD(0x83, 0, 7),
[Q_MAX_L1] = REG_FIELD(0x84, 0, 7),
[Q_MAX_L0] = REG_FIELD(0x85, 0, 7),

[Q_TERM_H3] = REG_FIELD(0x86, 0, 7),
[Q_TERM_H2] = REG_FIELD(0x87, 0, 7),
[Q_TERM_L1] = REG_FIELD(0x88, 0, 7),
[Q_TERM_L0] = REG_FIELD(0x89, 0, 7),
[Q_OCV_H3] = REG_FIELD(0x8A, 0, 7),
[Q_OCV_H2] = REG_FIELD(0x8B, 0, 7),

[Q_OCV_L1] = REG_FIELD(0x8C, 0, 7),
[Q_OCV_L0] = REG_FIELD(0x8D, 0, 7),
[OCV_CNT] = REG_FIELD(0x8E, 0, 7),
[SLEEP_CON_SAMP_CUR_H] = REG_FIELD(0x8F, 0, 7),
[SLEEP_CON_SAMP_CUR_L] = REG_FIELD(0x90, 0, 7),
[CAL_OFFSET_H] = REG_FIELD(0x91, 0, 7),
[CAL_OFFSET_L] = REG_FIELD(0x92, 0, 7),
[VCALIB0_H] = REG_FIELD(0x93, 0, 7),
[VCALIB0_L] = REG_FIELD(0x94, 0, 7),
[VCALIB1_H] = REG_FIELD(0x95, 0, 7),
[VCALIB1_L] = REG_FIELD(0x96, 0, 7),
[IOFFSET_H] = REG_FIELD(0x97, 0, 7),
[IOFFSET_L] = REG_FIELD(0x98, 0, 7),

[BAT_R0] = REG_FIELD(0x99, 0, 7),
[SOC_REG0] = REG_FIELD(0x9A, 0, 7),
[SOC_REG1] = REG_FIELD(0x9B, 0, 7),
[SOC_REG2] = REG_FIELD(0x9C, 0, 7),

[REMAIN_CAP_REG0] = REG_FIELD(0x9D, 0, 7),
[REMAIN_CAP_REG1] = REG_FIELD(0x9E, 0, 7),
[REMAIN_CAP_REG2] = REG_FIELD(0x9F, 0, 7),
[NEW_FCC_REG0] = REG_FIELD(0xA0, 0, 7),
[NEW_FCC_REG1] = REG_FIELD(0xA1, 0, 7),
[NEW_FCC_REG2] = REG_FIELD(0xA2, 0, 7),
[RESET_MODE] = REG_FIELD(0xA3, 0, 3),
[FG_INIT] = REG_FIELD(0xA5, 7, 7),

[HALT_CNT_REG] = REG_FIELD(0xA6, 0, 7),
[CALC_REST_REGL] = REG_FIELD(0xA7, 0, 7),
[CALC_REST_REGH] = REG_FIELD(0xA8, 0, 7),

[VOL_ADC_B3] = REG_FIELD(0xA9, 0, 7),
[VOL_ADC_B2] = REG_FIELD(0xAA, 0, 7),
[VOL_ADC_B1] = REG_FIELD(0xAB, 0, 7),
[VOL_ADC_B0] = REG_FIELD(0xAC, 0, 7),

[VOL_ADC_K3] = REG_FIELD(0xAD, 0, 7),
[VOL_ADC_K2] = REG_FIELD(0xAE, 0, 7),
[VOL_ADC_K1] = REG_FIELD(0xAF, 0, 7),
[VOL_ADC_K0] = REG_FIELD(0xB0, 0, 7),
[BAT_EXS] = REG_FIELD(0xEB, 7, 7),
[CHG_STS] = REG_FIELD(0xEB, 4, 6),
[BAT_OVP_STS] = REG_FIELD(0xEB, 3, 3),
[CHRG_IN_CLAMP] = REG_FIELD(0xEB, 2, 2),
[CHIP_NAME_H] = REG_FIELD(0xED, 0, 7),
[CHIP_NAME_L] = REG_FIELD(0xEE, 0, 7),
[PLUG_IN_STS] = REG_FIELD(0xF0, 6, 6),
};*/

struct rk817_bat_platform_data {
	u32 *ocv_table;
	u32 max_input_current;
	u32 max_chrg_current;
	u32 max_chrg_voltage;

	u32 design_max_voltage;
};

struct rk817_bat {
	struct platform_device *pdev;
	struct device *dev;
	struct i2c_client *client;

	struct power_supply *bat_ps;

	struct regmap *regmap;
	struct regmap_field *rmap_fields[ARRAY_SIZE(rk817_bat_reg_fields)];

	struct rk817_bat_platform_data *pdata;

	u8 status;
};

static int rk817_bat_get_prop(struct power_supply *ps,
			      enum power_supply_property psp,
			      union power_supply_propval *val)
{
	struct rk817_bat *battery = power_supply_get_drvdata(ps);
	int tmp;
	int ret = 0;

	switch(psp) {
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
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval 0;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static enum power_supply_property rk817_bat_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
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
	int i;

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

	pscfg.drv_data = battery;
	pscfg.of_node = pdev->dev.of_node;


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


/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#include "svb.h"
#include "common.h"
#include "share_drivers.h"

static svb_cfg g_svb_20 = {
	.core_curve = { CORE_CURVE_A20, CORE_CURVE_B20, CORE_CURVE_VOLT_MIN20,
			CORE_CURVE_VOLT_MAX20, 0 },
};

static svb_cfg g_svb_10 = {
	.core_curve = { CORE_CURVE_A10, CORE_CURVE_B10, CORE_CURVE_VOLT_MIN10,
			CORE_CURVE_VOLT_MAX10, 0 },
};

static svb_cfg g_svb_10_esmt = {
	.core_curve = { CORE_CURVE_A10, CORE_CURVE_B10, CORE_CURVE_VOLT_MIN10,
			CORE_CURVE_VOLT_MAX10_ESMT, 0 },
};

static inline void dwb(void) /* drain write buffer */
{
}

static inline unsigned int readl(unsigned addr)
{
	unsigned int val;
	val = (*(volatile unsigned int *)(long)(addr));
	return val;
}

static inline void writel(unsigned val, unsigned addr)
{
	dwb();
	(*(volatile unsigned *)(long)(addr)) = (val);
	dwb();
}

static void check_volt_val(unsigned int *volt_val, unsigned int curve_volt_max,
			   unsigned int curve_volt_min)
{
	if (*volt_val > curve_volt_max) {
		*volt_val = curve_volt_max;
	} else if (*volt_val < curve_volt_min) {
		*volt_val = curve_volt_min;
	}
}

static unsigned int get_svb_period(void)
{
	volatile u_svb_pwm_ctrl svb_pwm_ctrl;

	svb_pwm_ctrl.u32 = readl(PWM_CORE_SVT40_VOL_REG);

	return (svb_pwm_ctrl.bits.svb_pwm_period + 1);
}

static inline int temperature_formula(int val)
{
	return (((((val) - TEMPERATURE_A) * TEMPERATURE_B) / TEMPERATURE_C) -
		TEMPERATURE_D);
}

static inline unsigned int duty_formula(unsigned int val, unsigned int max,
					unsigned int min)
{
	unsigned int svb_period = get_svb_period();
	return ((((max) - (val)) * svb_period +
		 ((max) - (min) + DUTY_ONE) / DUTY_TWO) /
			((max) - (min)) -
		DUTY_ONE);
}

static inline int hpm_delta_formula(int val, svb_coef svb_coef_val)
{
	return ((((val) * svb_coef_val.coef_k) + svb_coef_val.coef_b) /
		DELTA_FORMULA_MULTIPLES);
}

static inline unsigned int core_max(unsigned int a, unsigned int b)
{
	return (((a) > (b)) ? (a) : (b));
}

static void svb_pwm_cfg(unsigned int reg_val, unsigned int addr)
{
	u_svb_pwm_ctrl svb_pwm_ctrl;
	unsigned int svb_period = get_svb_period();
	svb_pwm_ctrl.bits.svb_pwm_enable = 1;
	svb_pwm_ctrl.bits.svb_pwm_inv = 0;
	svb_pwm_ctrl.bits.svb_pwm_load = 1;
	svb_pwm_ctrl.bits.svb_pwm_period = (svb_period - 1);
	svb_pwm_ctrl.bits.svb_pwm_duty = reg_val & 0x3ff;
	writel(svb_pwm_ctrl.u32, addr);
}

static void svb_error(void)
{
	log_serial_puts((const s8 *)"The table is incorrect.\n");
	call_reset();
}

static unsigned int check_svb_version_id(void)
{
	volatile unsigned int version_id = 0;
	volatile u_svb_version_reg svb_version;
	unsigned int svb_version_id = 0;

	version_id = readl(OTP_BASE_REG + OTP_VERSION_ID_REG);
	svb_version.u32 = readl(SYSCTRL_REG + SVB_VERSION_ADDR);

	if ((svb_version.bits.svb_type == SVB_10) &&
	    	(version_id == OTP_10B_VERSION_ID)) {
		svb_version_id = SVB10_ID;
	} else if ((svb_version.bits.svb_type == SVB_608) &&
		(version_id == OTP_608_VERSION_ID)) {
		svb_version_id = SVB10_ID;
	} else if ((svb_version.bits.svb_type == SVB_20) &&
			((version_id == OTP_20B_VERSION_ID) ||
		    (version_id == OTP_20S_VERSION_ID) ||
		    (version_id == OTP_20G_VERSION_ID))) {
		svb_version_id = SVB20_ID;
	} else if ((svb_version.bits.svb_type == SVB_00) &&
			((version_id == OTP_00B_VERSION_ID) ||
		    (version_id == OTP_00S_VERSION_ID) ||
		    (version_id == OTP_00G_VERSION_ID))) {
		svb_version_id = SVB00_ID;
	} else {
		svb_error();
	}
	return svb_version_id;
}

static void init_temperature(void)
{
	writel(TSENSOR_CLK_CFG0, TSENSOR_CTRL_CLK_REG);

	writel(TSENSOR_CTRL0_CFG0, UBOOT_REG_TSENSOR_CTRL + TSENSOR0_CTRL0);
	writel(TSENSOR_CTRL1_CFG0, UBOOT_REG_TSENSOR_CTRL + TSENSOR0_CTRL1);
}

static void get_temperature(int *temperature)
{
	volatile int tmp = 0;

	tmp = readl(UBOOT_REG_TSENSOR_CTRL + TSENSOR_STATUS0);
	tmp = tmp & 0x3ff;
	*temperature = temperature_formula(tmp);
}

static void init_hpm(void)
{
	int i = 0;

	writel(HPM_CLK_CFG0, HPM_CTRL_CPU_SVT40_CLK_REG);
	writel(HPM_CLK_CFG1, HPM_8T_CPU_SVT40_CLK_REG);

	/* Deassert reset */
	for (i = 0; i < HPM_NUM; i++) {
		/* CORE */
		writel(HPM_CTRL_VALUE,
		       HPM_BASE_ADDR + CORE_HPM_CTRL0 + 0x10 * i);
		writel(HPM_LIMIT, HPM_BASE_ADDR + CORE_HPM_CTRL1 + 0x10 * i);
	}
}

static int core_hpm_temperature_calibration_20(int in_hpm_core_svt40,
					    int temperature)
{
	int core_hpm_delta = 0;
	svb_coef core_svb_coef[TEMPERATURE_ZONE_BUTT] = {
		{ -1707, 410190 }, { -1440, 350650 }, { -1004, 251280 },
		{ -380, 102460 },  { 192, -57158 },   { 372, -115680 }
	};

	if (temperature >= TEMPERATURE_110) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_110]);
	} else if (temperature >= TEMPERATURE_100) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_100]);
	} else if (temperature >= TEMPERATURE_85) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_85]);
	} else if (temperature >= TEMPERATURE_50) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_50]);
	} else if (temperature >= TEMPERATURE_0) {
		if (in_hpm_core_svt40 >= CORE_HPM_TEM_COM_BOUNDED) {
			core_hpm_delta = 0;
		} else {
			core_hpm_delta = hpm_delta_formula(
				in_hpm_core_svt40,
				core_svb_coef[TEMPERATURE_ZONE_50]);
		}
	} else if (temperature >= TEMPERATURE_NEG20) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_0]);
	} else {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40,
			core_svb_coef[TEMPERATURE_ZONE_NEG20]);
	}

	return core_hpm_delta;
}

static int core_hpm_temperature_calibration_10(int in_hpm_core_svt40,
					    int temperature)
{
	int core_hpm_delta = 0;
	svb_coef core_svb_coef[TEMPERATURE_ZONE_BUTT] = {
		{ -2168, 531240 }, { -1631, 402200 }, { -1057, 265380 },
		{ -303, 76110 },  { 112, -33334 },   { 298, -87228 }
	};

	if (temperature >= TEMPERATURE_110) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_110]);
	} else if (temperature >= TEMPERATURE_100) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_100]);
	} else if (temperature >= TEMPERATURE_85) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_85]);
	} else if (temperature >= TEMPERATURE_50) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_50]);
	} else if (temperature >= TEMPERATURE_0) {
			core_hpm_delta = 0;
	} else if (temperature >= TEMPERATURE_NEG20) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_0]);
	} else {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40,
			core_svb_coef[TEMPERATURE_ZONE_NEG20]);
	}

	return core_hpm_delta;
}

static int core_hpm_temperature_calibration_00(int in_hpm_core_svt40,
					    int temperature)
{
	int core_hpm_delta = 0;
	svb_coef core_svb_coef[TEMPERATURE_ZONE_BUTT] = {
		{ -1595, 371780 }, { -1295, 311000 }, { -814, 198090 },
		{ -284, 70572 },  { 164, -47200 },   { 338, -93836 }
	};

	if (temperature >= TEMPERATURE_110) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_110]);
	} else if (temperature >= TEMPERATURE_100) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_100]);
	} else if (temperature >= TEMPERATURE_85) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_85]);
	} else if (temperature >= TEMPERATURE_50) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_50]);
	} else if (temperature >= TEMPERATURE_0) {
			core_hpm_delta = 0;
	} else if (temperature >= TEMPERATURE_NEG20) {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40, core_svb_coef[TEMPERATURE_ZONE_0]);
	} else {
		core_hpm_delta = hpm_delta_formula(
			in_hpm_core_svt40,
			core_svb_coef[TEMPERATURE_ZONE_NEG20]);
	}

	return core_hpm_delta;
}

static void adjust_hpm(unsigned int *hpm_core_svt40, int temperature,
		       int *use_board_hpm)
{
	volatile unsigned int svb_version_id = 0;
	unsigned int otp_hpm_core_svt40 = 0;
	unsigned int hpm_storage = 0;
	int in_hpm_core_svt40 = 0;
	int core_hpm_delta = 0;

	svb_version_id = check_svb_version_id();

	otp_hpm_core_svt40 = (hpm_storage & 0x3ff);
	in_hpm_core_svt40 = (int)(*hpm_core_svt40);

	if (otp_hpm_core_svt40) {
		*hpm_core_svt40 = otp_hpm_core_svt40;
	} else {
		if (svb_version_id == SVB10_ID) {
			core_hpm_delta = core_hpm_temperature_calibration_10(
				in_hpm_core_svt40, temperature);
			*use_board_hpm = 1;
			in_hpm_core_svt40 -= core_hpm_delta;
			*hpm_core_svt40 = (unsigned int)in_hpm_core_svt40;
		} else if (svb_version_id == SVB20_ID) {
			core_hpm_delta = core_hpm_temperature_calibration_20(
				in_hpm_core_svt40, temperature);
			*use_board_hpm = 1;
			in_hpm_core_svt40 -= core_hpm_delta;
			*hpm_core_svt40 = (unsigned int)in_hpm_core_svt40;
		} else if (svb_version_id == SVB00_ID) {
			core_hpm_delta = core_hpm_temperature_calibration_00(
				in_hpm_core_svt40, temperature);
			*use_board_hpm = 1;
			in_hpm_core_svt40 -= core_hpm_delta;
			*hpm_core_svt40 = (unsigned int)in_hpm_core_svt40;
		} else {
			svb_error();
		}
	}
}

static unsigned int hpm_value_avg(unsigned int *val, unsigned int num)
{
	unsigned int i;
	volatile unsigned int tmp = 0;

	for (i = 0; i < num; i++) /* 4: Cycle */
		tmp += val[i];
	return ((tmp / CYCLE_NUM) / num);
}

static void start_hpm(unsigned int *hpm_core_svt40)
{
	int i = 0;
	u_hpm_reg hpm_reg;
	unsigned int core_value[HPM_RECORD_BUTT];
	memset_s(core_value, sizeof(int) * HPM_RECORD_BUTT, 0,
		 sizeof(int) * HPM_RECORD_BUTT);

	for (i = 0; i < CYCLE_NUM; i++) {
		/* core */
		hpm_reg.u32 = readl(SYSCTRL_REG + HPM_CORE_SVT40_REG0);
		core_value[HPM_RECORD1] += hpm_reg.bits.hpm1;
		core_value[HPM_RECORD0] += hpm_reg.bits.hpm0;
		hpm_reg.u32 = readl(SYSCTRL_REG + HPM_CORE_SVT40_REG1);
		core_value[HPM_RECORD3] += hpm_reg.bits.hpm1;
		core_value[HPM_RECORD2] += hpm_reg.bits.hpm0;
	}

	*hpm_core_svt40 =
		hpm_value_avg(core_value, HPM_RECORD_BUTT); /* 4 : Array size */
}

static void get_delta_v(int *core_delta_v)
{
	unsigned int value = 0;
	int flag = 0;
	volatile u_core_delta_v_reg core_delta_v_reg;

	/* core:bit 6-0,
	 * bit7 equal to 1 means negative, equal to 0 means positive,
	 * bit 6-0 is the  absolute delta_v
	 */
	core_delta_v_reg.u32 = readl(OTP_BASE_REG + OTP_DELTA_CORE_OFFSET);
	value = core_delta_v_reg.bits.core_delta_v;
	flag = (core_delta_v_reg.bits.core_flag) ? -1 : 1;
	*core_delta_v = flag * value;
}

static unsigned int calc_volt_regval(unsigned int volt_val,
				     unsigned int volt_max,
				     unsigned int volt_min)
{
	volatile unsigned int duty;

	if ((volt_val >= volt_max))
		volt_val = volt_max - 1;
	if ((volt_val <= volt_min))
		volt_val = volt_min + 1;
	duty = duty_formula(volt_val, volt_max, volt_min);

	return duty;
}

static void set_hpm_core_svt40_volt_00_20(unsigned int hpm_core_svt40_value,
				    int delta_v, const svb_curve *core_curve)
{
	unsigned int volt_val = 0;
	unsigned int reg_val = 0;
	u_storage_reg core_storage_reg;

	/* Calculate the required voltage value */
	volt_val = (core_curve->curve_b -
		    core_curve->curve_a * hpm_core_svt40_value) /
		   FORMULA_MULTIPLES;
	if (hpm_core_svt40_value < CORE_HPM_BOUND_20) {
		volt_val = core_curve->volt_max;
	}
	check_volt_val(&volt_val, core_curve->volt_max, core_curve->volt_min);

	/* Saves the core voltage */
	volt_val = (unsigned int)((int)volt_val + delta_v);
	check_volt_val(&volt_val, CORE_SVB_MAX0, CORE_SVB_MIN0);
	core_storage_reg.bits.work_v = volt_val & 0xffff;
	core_storage_reg.bits.cacl_v = volt_val & 0xffff;
	writel(core_storage_reg.u32, SYSCTRL_BASE_REG + CORE_V_STORAGE_REG);
	volt_val = volt_val - SVB_TRAINING_DELTA_VOLT_20;
	writel(volt_val, SYSCTRL_BASE_REG + CORE_TRAINING_V_STORAGE_REG);

	/* Configure the voltage */
	reg_val = calc_volt_regval(volt_val, CORE_VOLT_MAX0, CORE_VOLT_MIN0);
	svb_pwm_cfg(reg_val, PWM_CORE_SVT40_VOL_REG);
}

static void set_hpm_core_svt40_volt_10(unsigned int hpm_core_svt40_value,
				    int delta_v, const svb_curve *core_curve)
{
	unsigned int volt_val = 0;
	unsigned int reg_val = 0;
	volatile u_kgd_reg kgd_id;
	u_storage_reg core_storage_reg;

	kgd_id.u32 = readl(OTP_BASE_REG + OTP_KGD_ID_REG);
	/* Calculate the required voltage value */
	volt_val = (core_curve->curve_b -
		    core_curve->curve_a * hpm_core_svt40_value) /
		   FORMULA_MULTIPLES;

	if (kgd_id.bits.kgd_vendor_sel == KGD_A) {
		if (hpm_core_svt40_value < CORE_HPM_BOUND_10) {
			volt_val = core_curve->volt_max;
		}
	} else if (kgd_id.bits.kgd_vendor_sel == KGD_B) {
		if (hpm_core_svt40_value < CORE_HPM_BOUND_10_ESMT) {
			volt_val = core_curve->volt_max;
		}
	} else {
		svb_error();
	}

	check_volt_val(&volt_val, core_curve->volt_max, core_curve->volt_min);

	/* Saves the core voltage */
	volt_val = (unsigned int)((int)volt_val + delta_v);
	check_volt_val(&volt_val, CORE_SVB_MAX0, CORE_SVB_MIN0);
	core_storage_reg.bits.work_v = volt_val & 0xffff;
	core_storage_reg.bits.cacl_v = volt_val & 0xffff;
	writel(core_storage_reg.u32, SYSCTRL_BASE_REG + CORE_V_STORAGE_REG);
	volt_val = volt_val - SVB_TRAINING_DELTA_VOLT_10;
	writel(volt_val, SYSCTRL_BASE_REG + CORE_TRAINING_V_STORAGE_REG);

	/* Configure the voltage */
	reg_val = calc_volt_regval(volt_val, CORE_VOLT_MAX0, CORE_VOLT_MIN0);
	svb_pwm_cfg(reg_val, PWM_CORE_SVT40_VOL_REG);
}

static void set_volt(unsigned int hpm_core_svt40, const svb_cfg *svb)
{
	int core_delta_v = 0;
	volatile unsigned int svb_version_id = 0;

	svb_version_id = check_svb_version_id();

	/* Obtaining Voltage Compensation */
	get_delta_v(&core_delta_v);

	/* Calculate and configure voltage */
	if (svb_version_id == SVB10_ID) {
		set_hpm_core_svt40_volt_10(hpm_core_svt40, core_delta_v, &svb->core_curve);
	} else if (svb_version_id == SVB20_ID || svb_version_id == SVB00_ID) {
		set_hpm_core_svt40_volt_00_20(hpm_core_svt40, core_delta_v, &svb->core_curve);
	}  else {
		svb_error();
	}
}

static void save_temperature_pasensor(int temperature)
{
	u_dyn_comp_falg_storage_reg dyn_comp_falg_storage_reg;

	dyn_comp_falg_storage_reg.u32 =
		readl(SYSCTRL_REG + DYN_COMP_FALG_STORAGE_REG);

	/* Save the current temperature. */
	if (temperature >= 0) {
		dyn_comp_falg_storage_reg.bits.start_up_temp =
			(unsigned int)temperature & 0xff;
	} else {
		unsigned int tsensor_abs_val = temperature * NEG_1;
		dyn_comp_falg_storage_reg.bits.temp_negative = 1;
		dyn_comp_falg_storage_reg.bits.start_up_temp =
			((unsigned int)tsensor_abs_val & 0xff);
	}
	writel(dyn_comp_falg_storage_reg.u32,
	       SYSCTRL_REG + DYN_COMP_FALG_STORAGE_REG);
}

static void save_svb_pwm_crg(void)
{
	volatile unsigned int svb_pwm_crg = 0;

	svb_pwm_crg =
		readl(CRG_BASE_ADDR + SVB_PWM_CRG);

	/* Save the current svb_crg */

	writel(svb_pwm_crg,
	       SYSCTRL_REG + SVB_PWM_CRG_STORAGE_REG);
}

static void save_hpm(unsigned int hpm_core_svt40, int use_board_hpm)
{
	u_hpm_storage_reg hpm_storage_reg;
	hpm_storage_reg.bits.core_hpm_value = hpm_core_svt40 & 0x3ff;
	hpm_storage_reg.bits.use_board_hpm = use_board_hpm & 0x1;
	writel(hpm_storage_reg.u32, SYSCTRL_REG + HPM_STORAGE_REG);
}

static void set_svb_volt(unsigned int hpm_core_svt40)
{
	volatile unsigned int svb_version_id = 0;
	volatile u_kgd_reg kgd_id;

	kgd_id.u32 = readl(OTP_BASE_REG + OTP_KGD_ID_REG);
	svb_version_id = check_svb_version_id();
	if ((svb_version_id == SVB10_ID) && (kgd_id.bits.kgd_vendor_sel == KGD_A)) {
		set_volt(hpm_core_svt40, &g_svb_10);
	} else if ((svb_version_id == SVB10_ID) && (kgd_id.bits.kgd_vendor_sel == KGD_B)) {
		set_volt(hpm_core_svt40, &g_svb_10_esmt);
	} else if (svb_version_id == SVB20_ID || svb_version_id == SVB00_ID) {
		set_volt(hpm_core_svt40, &g_svb_20);
	} else {
		svb_error();
	}
}

void start_svb(void)
{
	unsigned int hpm_core_svt40 = 0;
	unsigned int svb_ver = 0;
	int temperature = 0;
	int use_board_hpm = 1;

	/* add SVB VER */
	svb_ver = readl(SVB_VER_REG);
	svb_ver = svb_ver & 0xffff;
	svb_ver = svb_ver | SVB_VER_VAL;
	writel(svb_ver, SVB_VER_REG);

	/* init temperature and hpm and pasensor */
	init_temperature();
	init_hpm();

	/* Waiting for 10 ms */
	mdelay(WAITING_10_MS);
	start_hpm(&hpm_core_svt40);
	get_temperature(&temperature);
	adjust_hpm(&hpm_core_svt40, temperature, &use_board_hpm);
	save_hpm(hpm_core_svt40, use_board_hpm);
	set_svb_volt(hpm_core_svt40);
	save_svb_pwm_crg();
	save_temperature_pasensor(temperature);

	mdelay(WAITING_6_MS);
}

void end_svb(void)
{
	volatile unsigned int core_volt_val = 0;
	volatile unsigned int svb_version_id = 0;

	svb_version_id = check_svb_version_id();
	if (svb_version_id == SVB10_ID) {
		core_volt_val =
			readl(SYSCTRL_REG + CORE_V_STORAGE_REG) & 0xffff;
		core_volt_val = calc_volt_regval(core_volt_val, CORE_VOLT_MAX0,
						 CORE_VOLT_MIN0);
		svb_pwm_cfg(core_volt_val, PWM_CORE_SVT40_VOL_REG);
		mdelay(WAITING_6_MS);
	} else if (svb_version_id == SVB20_ID || svb_version_id == SVB00_ID) {
		core_volt_val =
			readl(SYSCTRL_REG + CORE_V_STORAGE_REG) & 0xffff;
		core_volt_val = calc_volt_regval(core_volt_val, CORE_VOLT_MAX0,
						 CORE_VOLT_MIN0);
		svb_pwm_cfg(core_volt_val, PWM_CORE_SVT40_VOL_REG);
		mdelay(WAITING_6_MS);
	} else {
		svb_error();
	}
}

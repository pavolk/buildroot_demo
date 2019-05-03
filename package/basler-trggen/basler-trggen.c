/*
 * Basler Trigger Generator
 *
 * Copyright (C) 2016-2018 Basler AG
 *
 * The trigger generator driver is part of the Basler dart BCON for
 * LVDS Development Kit software distribution. It utilizes the Zynq-7000
 * TTC hardware to generate a periodic signal which is routed to the
 * camera interface connector and can be used to apply an external
 * trigger signal to the BCON camera. Frequency and duty cycle of this
 * signal are adjustable.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>

#include <uapi/misc/basler/trggen.h>

/* Register offsets and bit shifts */
#define TTC_CLK_CNTRL_OFFSET		0x00 /* Clock Control Reg, RW */
#define 	CLK_CNTRL_EXT_EDGE	0x06
#define		CLK_CNTRL_SRC		0x05
#define		CLK_CNTRL_PS_VAL	0x01
#define		CLK_CNTRL_PS_MASK	0x1e
#define		CLK_CNTRL_PS_EN		0x00
#define TTC_CNT_CNTRL_OFFSET		0x0C /* Counter Control Reg, RW */
#define		CNT_CNTRL_POL_WAVE	0x06
#define		CNT_CNTRL_EN_WAVE	0x05
#define		CNT_CNTRL_RST		0x04
#define		CNT_CNTRL_MATCH		0x03
#define		CNT_CNTRL_DECR		0x02
#define		CNT_CNTRL_INT		0x01
#define		CNT_CNTRL_DIS		0x00
#define TTC_COUNT_VAL_OFFSET		0x18 /* Counter Value Reg, RO */
#define TTC_INTR_VAL_OFFSET		0x24 /* Interval Count Reg, RW */
#define TTC_COUNT_MATCH_0_OFFSET	0x30 /* Counter Match #0 Reg, RW */
#define TTC_COUNT_MATCH_1_OFFSET	0x3c /* Counter Match #1 Reg, RW */
#define TTC_COUNT_MATCH_2_OFFSET	0x48 /* Counter Match #2 Reg, RW */
#define TTC_ISR_OFFSET			0x54 /* Interrupt Status Reg, RO */
#define TTC_IER_OFFSET			0x60 /* Interrupt Enable Reg, RW */
#define		IXR_EVT_OVR		0x05
#define		IXR_CNT_OVR		0x04
#define		IXR_MATCH_2		0x03
#define		IXR_MATCH_1		0x02
#define		IXR_MATCH_0		0x01
#define		IXR_INTERVAL		0x00


/*
 * struct trggen_device - device context structure
 */
struct trggen_device {
	struct device *		dev;
	void __iomem *		regbase;
	struct resource		memres;
	struct miscdevice	mdev;
	char			devname[12];
};



static const char driver_name[]	= "basler-trggen";



/*
 * The static data struct contains information that is constant
 * by nature, most important, the main time base frequency. This
 * structure can be read by user-space programs, which can then
 * perform timing calculations.
 */
static const struct trggen_staticdata sdata = {
	.clk_hz		= 111111111U,
	.scale_min	= 0U,
	.scale_max	= 16U,
	.period_min	= 1U,
	.period_max	= U16_MAX
};



/*
 * Return the current prescaler setting. Result value N ranges from 0
 * to 16. The input frequency sdata.clk_hz is divided by 2 ^^ N before
 * being fed to the counter.
 */
static unsigned int
trggen_get_prescaler(const struct trggen_device * d)
{
	u8 reg;

	reg = readb(d->regbase + TTC_CLK_CNTRL_OFFSET);
	return reg & BIT(CLK_CNTRL_PS_EN) ?
	       ((reg & CLK_CNTRL_PS_MASK) >> CLK_CNTRL_PS_VAL) + 1U :
	       0U;
}



/*
 * Set the prescaler value.
 */
static int
trggen_set_prescaler(const struct trggen_device * d, unsigned int val)
{
	u8 reg;

	if (val < sdata.scale_min || val > sdata.scale_max)
		return -ERANGE;

	reg = readb(d->regbase + TTC_CLK_CNTRL_OFFSET) &
	      ~(BIT(CLK_CNTRL_PS_EN) | CLK_CNTRL_PS_MASK);

	if (val--)
		 reg |= BIT(CLK_CNTRL_PS_EN) |
			((val << CLK_CNTRL_PS_VAL) & CLK_CNTRL_PS_MASK);

	writeb(reg, d->regbase + TTC_CLK_CNTRL_OFFSET);
	return 0;
}



/*
 * Read the current period value from the counter. Every increment
 * corresponds to one cycle of the counter input clock.
 */
static unsigned int
trggen_get_period(const struct trggen_device * d)
{

	return readw(d->regbase + TTC_INTR_VAL_OFFSET);
}



/*
 * Set up the pulse period by writing the corresponding number of
 * clock cycles to the couter. In combination with the prescaler,
 * this determines the pulse period produced at the output.
 */
static int
trggen_set_period(const struct trggen_device * d, unsigned int val)
{
	if (val < sdata.period_min || val > sdata.period_max)
		return -ERANGE;
	writew( (u16) val, d->regbase + TTC_INTR_VAL_OFFSET);
	return 0;
}



/*
 * Get the counter value where the high-to-low output transition
 * occurs.
 */
static unsigned int
trggen_get_duration(const struct trggen_device * d)
{
	return readw(d->regbase + TTC_COUNT_MATCH_0_OFFSET);
}



/*
 * Set the counter value for the desired high-to-low output transition.
 * This determines the duty cycle of the generated waveform.
 */
static int
trggen_set_duration(const struct trggen_device * d, unsigned int val)
{
	if (val < sdata.period_min || val > sdata.period_max)
		return -ERANGE;
	writew((u16) val, d->regbase + TTC_COUNT_MATCH_0_OFFSET);
	return 0;
}



/*
 * If 'start' is true, enable the output signal, reset the counter to
 * zero and restart it. If false, disable the output signal and stop
 * the counter.
 */
static int
trggen_start(const struct trggen_device * d, bool start)
{
	u8 reg;

	reg = start ?
	      BIT(CNT_CNTRL_POL_WAVE) | BIT(CNT_CNTRL_RST) |
	      BIT(CNT_CNTRL_MATCH)    | BIT(CNT_CNTRL_INT) :
	      BIT(CNT_CNTRL_POL_WAVE) | BIT(CNT_CNTRL_EN_WAVE) |
	      BIT(CNT_CNTRL_INT)      | BIT(CNT_CNTRL_DIS);
	writeb(reg, d->regbase + TTC_CNT_CNTRL_OFFSET);

	return 0;
}



/*
 * Return the counter's current 'running' state.
 */
static bool
trggen_running(const struct trggen_device * d)
{
	u8 reg;

	reg = readb(d->regbase + TTC_CNT_CNTRL_OFFSET);
	return !(reg & BIT(CNT_CNTRL_DIS));
}



/*
 * The IOCTL interface, allowing user-space programs to control the
 * trigger generator via ioctl() calls.
 */
static long
trggen_ioctl(struct file * f, unsigned int cmd, unsigned long arg)
{
	int ret = -ENOTTY;
	unsigned int val;
	struct trggen_device * trggen =
		container_of(f->private_data, struct trggen_device,
			     mdev);

	switch (cmd) {
	case TRGGEN_GET_SDATA:
		ret = copy_to_user((void *) arg, &sdata, sizeof sdata) ?
		      -ENOBUFS : 0;
		break;

	case TRGGEN_GET_SCALE:
		val = trggen_get_prescaler(trggen);
		ret = put_user(val, (unsigned int *) arg);
		break;

	case TRGGEN_SET_SCALE:
		ret = get_user(val, (unsigned int *) arg);
		if (!ret)
			ret = trggen_set_prescaler(trggen, val);
		break;

	case TRGGEN_GET_PERIOD:
		val = trggen_get_period(trggen);
		ret = put_user(val, (unsigned int *) arg);
		break;

	case TRGGEN_SET_PERIOD:
		ret = get_user(val, (unsigned int *) arg);
		if (!ret)
			ret = trggen_set_period(trggen, val);
		break;

	case TRGGEN_GET_DURATION:
		val = trggen_get_duration(trggen);
		ret = put_user(val, (unsigned int *) arg);
		break;

	case TRGGEN_SET_DURATION:
		ret = get_user(val, (unsigned int *) arg);
		if (!ret)
			ret = trggen_set_duration(trggen, val);
		break;

	case TRGGEN_START:
		ret = trggen_start(trggen, true);
		break;

	case TRGGEN_STOP:
		ret = trggen_start(trggen, false);
		break;

	case TRGGEN_IS_RUNNING:
		val = trggen_running(trggen);
		ret = put_user(val, (unsigned int *) arg);
		break;
	}

	return ret;
}



static const struct file_operations fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl		= trggen_ioctl
};



/*
 * The static data struct is exported to a sysfs attribute as a
 * formatted string that can be easily parsed by user-space
 * applications.
 */
ssize_t sdata_show(struct device *dev,
		   struct device_attribute *attr,
		   char *buf)
{
	return scnprintf(buf, PAGE_SIZE,
			 "Input clock: %u Hz, prescaler: %u..%u, "
			 "period %u..%u\n",
			 sdata.clk_hz,
			 sdata.scale_min, sdata.scale_max,
			 sdata.period_min, sdata.period_max);
}



/*
 * Export the prescaler setting to sysfs.
 */
static ssize_t
scale_show(struct device *dev, struct device_attribute *attr,
	   char *buf)
{
	struct trggen_device *trggen;
	unsigned int val;

	trggen = dev_get_drvdata(dev);
	val = trggen_get_prescaler(trggen);
	return scnprintf(buf, PAGE_SIZE, "%u\n", val);
}

static ssize_t
scale_store(struct device *dev, struct device_attribute *attr,
	    const char *buf, size_t count)
{
	struct trggen_device *trggen;
	unsigned int val;
	int res;

	trggen = dev_get_drvdata(dev);
	res = kstrtouint (buf, 10U, &val);
	if (likely(!res))
		res = trggen_set_prescaler(trggen, val);
	return unlikely(res) ? res : count;
}



/*
 * Export the pulse-period setting to sysfs.
 */
static ssize_t
period_show(struct device *dev, struct device_attribute *attr,
	    char *buf)
{
	struct trggen_device *trggen;
	unsigned int val;

	trggen = dev_get_drvdata(dev);
	val = trggen_get_period(trggen);
	return scnprintf(buf, PAGE_SIZE, "%u\n", val);
}

static ssize_t
period_store(struct device *dev, struct device_attribute *attr,
	     const char *buf, size_t count)
{
	struct trggen_device *trggen;
	unsigned int val;
	int res;

	trggen = dev_get_drvdata(dev);
	res = kstrtouint (buf, 10U, &val);
	if (likely(!res))
		res = trggen_set_period(trggen, val);
	return unlikely(res) ? res : count;
}



/*
 * Export the position of the high-to-low transition to sysfs.
 */
static ssize_t
duration_show(struct device *dev, struct device_attribute *attr,
	      char *buf)
{
	struct trggen_device *trggen;
	unsigned int val;

	trggen = dev_get_drvdata(dev);
	val = trggen_get_duration(trggen);
	return scnprintf(buf, PAGE_SIZE, "%u\n", val);
}

static ssize_t
duration_store(struct device *dev, struct device_attribute *attr,
	       const char *buf, size_t count)
{
	struct trggen_device *trggen;
	unsigned int val;
	int res;

	trggen = dev_get_drvdata(dev);
	res = kstrtouint (buf, 10U, &val);
	if (likely(!res))
		res = trggen_set_duration(trggen, val);
	return unlikely(res) ? res : count;
}



/*
 * Export the 'running' state to sysfs.
 */
static ssize_t
run_show(struct device *dev, struct device_attribute *attr,
	 char *buf)
{
	struct trggen_device *trggen;

	trggen = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%u\n",
			 trggen_running(trggen) ? 1U : 0U);
}

static ssize_t
run_store(struct device *dev, struct device_attribute *attr,
	  const char *buf, size_t count)
{
	struct trggen_device *trggen;
	unsigned int val;
	int res;

	trggen = dev_get_drvdata(dev);
	res = kstrtouint (buf, 10U, &val);
	if (likely(!res))
		res = val & ~1U ?
		      -ERANGE :
		      trggen_start(trggen, val != 0U);
	return unlikely(res) ? res : count;
}



/*
 * Export all attributes to sysfs. User-space applications can
 * control the trigger generator by reading from / writing to
 * sysfs files.
 */
static int
trggen_sysfs_setup(struct device *dev, bool start)
{	int ret = 0;
	const struct device_attribute * p;
	static const struct device_attribute attr[] = {
		__ATTR_RO(sdata),
		__ATTR_RW(scale),
		__ATTR_RW(period),
		__ATTR_RW(duration),
		__ATTR_RW(run)
	};

	if (!start) {
		p = attr + ARRAY_SIZE(attr);
		goto ex;
	}

	for (p = attr; p < attr + ARRAY_SIZE(attr); ++p) {
		ret = device_create_file(dev, p);
		if (unlikely(ret))
			goto ex;
	}

	return 0;

ex:
	while (--p >= attr)
		device_remove_file(dev, p);
	return ret;
}



/*
 * Match the driver with its device. It will claim ownership of any
 * TTC device that has the 'basler,trggen' property in its device
 * tree node in addition to the standard properties. The property's
 * value, an integer, will be used to build the device node name.
 */
static int
trggen_probe(struct platform_device *pdev)
{
	struct trggen_device *trggen;
	int ret = 0;
	u8 devidx;

	trggen = devm_kzalloc(&pdev->dev, sizeof *trggen, GFP_KERNEL);
	if (unlikely(!trggen))
		return -ENOMEM;

	ret = of_property_read_u8(pdev->dev.of_node,
				   "basler,trggen",
				   &devidx);
	if (ret)
		/* not ours */
		return -ENODEV;

	snprintf(trggen->devname, sizeof trggen->devname,
		 "trggen%hhu", devidx);
	trggen->dev = &pdev->dev;

	ret = of_address_to_resource(pdev->dev.of_node, 0,
				     &trggen->memres);
	if (unlikely(ret))
		goto err;

	ret = devm_request_resource(&pdev->dev, &iomem_resource,
				    &trggen->memres);
	if (unlikely(ret))
		goto err;

	trggen->regbase = devm_ioremap_nocache(&pdev->dev,
					       trggen->memres.start,
					       resource_size(&trggen->memres));

	trggen->mdev.fops = &fops;
	trggen->mdev.minor = MISC_DYNAMIC_MINOR;
	trggen->mdev.name = driver_name;
	trggen->mdev.nodename = trggen->devname;
	trggen->mdev.mode = S_IWUSR | S_IRUGO;

	ret = misc_register(&trggen->mdev);
	if (unlikely(ret))
		goto err;

	ret = trggen_sysfs_setup(&pdev->dev, true);
	if (unlikely(ret))
		goto err1;

	/* Initial device setup - stop everything. */
	writeb(BIT(CNT_CNTRL_EN_WAVE) | BIT(CNT_CNTRL_DIS) |
	       BIT(CNT_CNTRL_INT),
	       trggen->regbase + TTC_CNT_CNTRL_OFFSET);
	writeb(0U, trggen->regbase + TTC_CLK_CNTRL_OFFSET);
	writew(sdata.period_max, trggen->regbase + TTC_INTR_VAL_OFFSET);
	writew(sdata.period_max / 2U,
	       trggen->regbase + TTC_COUNT_MATCH_0_OFFSET);
	writew(0U, trggen->regbase + TTC_COUNT_MATCH_1_OFFSET);
	writew(0U, trggen->regbase + TTC_COUNT_MATCH_2_OFFSET);

	platform_set_drvdata(pdev, trggen);
	dev_set_drvdata(&pdev->dev, trggen);
	dev_info(trggen->dev, "Found unit %hhu\n", devidx);
	return 0;

err1:
	misc_deregister(&trggen->mdev);
err:
	dev_err(trggen->dev, "Probe failed: %d\n", ret);
	return ret;
}


static int
trggen_remove(struct platform_device *pdev)
{
	struct trggen_device *trggen;
	trggen = platform_get_drvdata(pdev);
	trggen_start(trggen, false);
	trggen_sysfs_setup(&pdev->dev, false);
	misc_deregister(&trggen->mdev);
	return 0;
}



/*
 * The driver utilizes the TTC (triple timer counter) hardware unit
 * present on the Zynq-7000 SoC.
 */
static const struct of_device_id trggen_of_id_table[] = {
	{ .compatible = "cdns,ttc" },
	{}
};
MODULE_DEVICE_TABLE(of, trggen_of_id_table);

static struct platform_driver trggen_driver = {
	.driver = {
		.name		= driver_name,
		.of_match_table	= trggen_of_id_table
	},
	.probe		= trggen_probe,
	.remove		= trggen_remove
};

module_platform_driver(trggen_driver);

MODULE_AUTHOR("Thomas Koeller <thomas.koeller@baslerweb.com>");
MODULE_DESCRIPTION("Basler BCON DevKit Trigger Generator Driver");
MODULE_LICENSE("GPL v2");

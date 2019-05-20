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


#ifndef	_UAPI_MISC_BASLER_TRGGEN_H
#define	_UAPI_MISC_BASLER_TRGGEN_H

#include <linux/ioctl.h>

/*
 * Immutable driver data:
 *
 * clk_hz: time base frequency in Hz, before prescaling
 * scale_min, scale_max: prescaler min/max allowed values
 * period_min, period_max: pulse period min/max allowed values
 */
struct trggen_staticdata {
	unsigned int	clk_hz;
	unsigned int	scale_min, scale_max;
	unsigned int	period_min, period_max;
};

/* IOCTL function codes */
#define TRGGEN_GET_SDATA	_IOR('T', 0, struct trggen_staticdata)
#define TRGGEN_GET_SCALE	_IOR('T', 1, unsigned int)
#define TRGGEN_SET_SCALE	_IOW('T', 2, unsigned int)
#define TRGGEN_GET_PERIOD	_IOR('T', 3, unsigned int)
#define TRGGEN_SET_PERIOD	_IOW('T', 4, unsigned int)
#define TRGGEN_GET_DURATION	_IOR('T', 5, unsigned int)
#define TRGGEN_SET_DURATION	_IOW('T', 6, unsigned int)
#define TRGGEN_START		_IO ('T', 7)
#define TRGGEN_STOP		_IO ('T', 8)
#define TRGGEN_IS_RUNNING	_IOR('T', 9, unsigned int)

#endif	/* _UAPI_MISC_BASLER_TRGGEN_H */

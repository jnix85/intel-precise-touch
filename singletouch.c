// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/kernel.h>

#include "context.h"
#include "protocol/enums.h"
#include "protocol/touch.h"

// HACK: Workaround for DKMS build without BUS_MEI patch
#ifndef BUS_MEI
#define BUS_MEI 0x44
#endif

void ipts_singletouch_parse_report(struct ipts_context *ipts,
		struct ipts_touch_data *data)
{
	u8 touch = data->data[1];
	u16 x = *(u16 *)&data->data[2];
	u16 y = *(u16 *)&data->data[4];

	input_report_key(ipts->touch, BTN_TOUCH, touch);
	input_report_abs(ipts->touch, ABS_X, x);
	input_report_abs(ipts->touch, ABS_Y, y);

	input_sync(ipts->touch);
}

int ipts_singletouch_init(struct ipts_context *ipts)
{
	int ret;

	if (ipts->mode != IPTS_SENSOR_MODE_SINGLETOUCH)
		return 0;

	ipts->touch = devm_input_allocate_device(ipts->dev);
	if (!ipts->touch)
		return -ENOMEM;

	input_set_capability(ipts->touch, EV_KEY, BTN_TOUCH);
	input_set_abs_params(ipts->touch, ABS_X, 0, 32767, 0, 0);
	input_abs_set_res(ipts->touch, ABS_X, 112);
	input_set_abs_params(ipts->touch, ABS_Y, 0, 32767, 0, 0);
	input_abs_set_res(ipts->touch, ABS_Y, 199);

	ipts->touch->id.bustype = BUS_MEI;
	ipts->touch->id.vendor = ipts->device_info.vendor_id;
	ipts->touch->id.product = ipts->device_info.device_id;
	ipts->touch->id.version = ipts->device_info.fw_rev;

	ipts->touch->phys = "heci3";
	ipts->touch->name = "Intel Precise Touchscreen";

	ret = input_register_device(ipts->touch);
	if (ret) {
		dev_err(ipts->dev, "Failed to register touch device: %d\n",
				ret);
		return ret;
	}

	return 0;
}

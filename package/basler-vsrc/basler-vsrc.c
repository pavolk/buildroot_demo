/*
 * Basler Video Source
 *
 * Copyright (C) 2016-2018 Basler AG
 *
 * The video source driver is part of the Basler dart BCON for
 * LVDS Development Kit software distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#include <media/v4l2-subdev.h>
#include <media/media-entity.h>


/*
 * struct bvsrc_device - Basler video source device structure
 */
struct bvsrc_device {
	struct media_pad pad;
	struct device *dev;
	struct v4l2_subdev subdev;
	struct v4l2_mbus_framefmt format;
	struct v4l2_mbus_framefmt default_format;
};

#define to_vsrc(sd) container_of(sd, struct bvsrc_device, subdev)



/* -----------------------------------------------------------------------------
 * V4L2 Subdevice Pad Operations
 */

static struct v4l2_mbus_framefmt *
__bvsrc_get_pad_format(struct bvsrc_device *bvsrc,
		       struct v4l2_subdev_pad_config *cfg,
		       unsigned int pad, u32 which)
{
	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		return v4l2_subdev_get_try_format(&bvsrc->subdev, cfg, pad);
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		return &bvsrc->format;
	default:
		return NULL;
	}
}

static int
bvsrc_get_format(struct v4l2_subdev *subdev,
		 struct v4l2_subdev_pad_config *cfg,
		 struct v4l2_subdev_format *fmt)
{
	struct bvsrc_device *bvsrc = to_vsrc(subdev);

	fmt->format = *__bvsrc_get_pad_format(bvsrc, cfg, fmt->pad, fmt->which);

	return 0;
}

static int
bvsrc_set_format(struct v4l2_subdev *subdev,
		 struct v4l2_subdev_pad_config *cfg,
		 struct v4l2_subdev_format *fmt)
{
	struct bvsrc_device *bvsrc = to_vsrc(subdev);
	bvsrc->format = fmt->format;
	return 0;
}

/* -----------------------------------------------------------------------------
 * V4L2 Subdevice Operations
 */

static const struct v4l2_subdev_pad_ops bvsrc_pad_ops = {
	.get_fmt	= bvsrc_get_format,
	.set_fmt	= bvsrc_set_format
};

static const struct v4l2_subdev_ops bvsrc_ops = {
	.pad		= &bvsrc_pad_ops
};

static const struct media_entity_operations bvsrc_media_ops = {
	.link_validate = v4l2_subdev_link_validate
};

/* -----------------------------------------------------------------------------
 * Platform Device Driver
 */

static int
bvsrc_parse_of(struct bvsrc_device *bvsrc)
{
	const struct device_node *node = bvsrc->dev->of_node;
	const struct device_node *ports;
	struct device_node *port;
	unsigned int nports = 0;

	ports = of_get_child_by_name(node, "ports");
	if (unlikely(ports == NULL))
		ports = node;

	for_each_child_of_node(ports, port) {
		if (likely(port->name) && !of_node_cmp(port->name, "port")) {
			if (likely(++nports == 1)) {
				struct device_node *endpoint;
				endpoint = of_get_next_child(port, NULL);
				if (likely(endpoint))
					of_node_put(endpoint);
			} else {
				dev_err(bvsrc->dev, "multiple ports\n");
				return -EINVAL;
			}
		}
	}

	return 0;
}


static int
bvsrc_probe(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev;
	struct bvsrc_device *bvsrc;
	int ret;


	bvsrc = devm_kzalloc(&pdev->dev, sizeof *bvsrc, GFP_KERNEL);
	if (unlikely(!bvsrc))
		return -ENOMEM;

	bvsrc->dev = &pdev->dev;

	dev_info(bvsrc->dev, "probe called on basler video source\n");

	ret = bvsrc_parse_of(bvsrc);
	if (unlikely(ret < 0))
		return ret;

	/* Initialize the default format */
	bvsrc->default_format.code = V4L2_PIX_FMT_GREY;
	bvsrc->default_format.field = V4L2_FIELD_NONE;
	bvsrc->default_format.colorspace = V4L2_COLORSPACE_SRGB;
	bvsrc->format = bvsrc->default_format;

	/* Initialize V4L2 subdevice and media entity */
	subdev = &bvsrc->subdev;
	v4l2_subdev_init(subdev, &bvsrc_ops);
	subdev->dev = &pdev->dev;
	subdev->flags = V4L2_SUBDEV_FL_HAS_DEVNODE;
	v4l2_set_subdevdata(subdev, pdev);
	strlcpy(subdev->name, dev_name(&pdev->dev), sizeof subdev->name);
	v4l2_set_subdevdata(subdev, bvsrc);

	subdev->entity.ops = &bvsrc_media_ops;
	subdev->entity.name = "vidsrc";
	subdev->entity.function = MEDIA_ENT_F_V4L2_SUBDEV_UNKNOWN;
	bvsrc->pad.flags = MEDIA_PAD_FL_SOURCE;
	ret = media_entity_pads_init(&subdev->entity, 1U, &bvsrc->pad);
	if (unlikely(ret < 0))
		goto error;

	platform_set_drvdata(pdev, bvsrc);

	ret = v4l2_async_register_subdev(subdev);
	if (unlikely(ret < 0)) {
		dev_err(&pdev->dev, "failed to register subdev\n");
		goto error;
	}

	return 0;

error:
	media_entity_cleanup(&subdev->entity);
	return ret;
}

static int
bvsrc_remove(struct platform_device *pdev)
{
	struct bvsrc_device *bvsrc = platform_get_drvdata(pdev);
	struct v4l2_subdev *subdev = &bvsrc->subdev;

	v4l2_async_unregister_subdev(subdev);
	media_entity_cleanup(&subdev->entity);
	return 0;
}


static const struct of_device_id bvsrc_of_id_table[] = {
	{ .compatible = "basler,v-src-0.1" },
	{}
};
MODULE_DEVICE_TABLE(of, bvsrc_of_id_table);

static struct platform_driver bvsrc_driver = {
	.driver = {
		.name       = "basler-vsrc",
		.of_match_table = bvsrc_of_id_table,
	},
	.probe          = bvsrc_probe,
	.remove         = bvsrc_remove,
};

module_platform_driver(bvsrc_driver);

MODULE_AUTHOR("Thomas Koeller <thomas.koeller@baslerweb.com>");
MODULE_DESCRIPTION("Basler Video Source Driver");
MODULE_LICENSE("GPL v2");

// SPDX-License-Identifier: ISC
/*
 * Copyright (c) 2014 Broadcom Corporation
 */
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_irq.h>

#include <defs.h>
#include "debug.h"
#include "core.h"
#include "common.h"
#include "of.h"

void brcmf_of_probe(struct device *dev, enum brcmf_bus_type bus_type,
		    struct brcmf_mp_device *settings)
{
	struct brcmfmac_sdio_pd *sdio = &settings->bus.sdio;
	struct device_node *root, *np = dev->of_node;
	struct property *prop;
	const char *cp = NULL;
	int irq;
	u32 irqf;
	u32 val32;
	u16 val16;
	const char * domain;

	/* Set board-type to the first string of the machine compatible prop */
	root = of_find_node_by_path("/");
	if (root) {
		prop = of_find_property(root, "compatible", NULL);
		cp = of_prop_next_string(prop, NULL);
		of_node_put(root);
	}

	if (!np || bus_type != BRCMF_BUSTYPE_SDIO ||
	    !of_device_is_compatible(np, "brcm,bcm4329-fmac"))
		return;

	// Laird - Get regdomain/country code string if it exists
	if (of_property_read_string(np, "laird,regdomain", &domain) == 0)
		strlcpy(settings->regdomain, domain, BRCMF_REGDOMAIN_LEN);

	if (of_property_read_u32(np, "brcm,drive-strength", &val32) == 0)
		sdio->drive_strength = val32;

	if (of_property_read_bool(np, "brcm,use_board_type"))
		settings->board_type = cp;

	sdio->broken_sg_support = of_property_read_bool(np,
			"brcm,broken_sg_support");
	if (of_property_read_u16(np, "brcm,sd_head_align", &val16) == 0)
		sdio->sd_head_align = val16;
	if (of_property_read_u16(np, "brcm,sd_sgentry_align", &val16) == 0)
		sdio->sd_sgentry_align = val16;

	/* make sure there are interrupts defined in the node */
	if (!of_find_property(np, "interrupts", NULL))
		return;

	irq = irq_of_parse_and_map(np, 0);
	if (!irq) {
		brcmf_err("interrupt could not be mapped\n");
		return;
	}
	irqf = irqd_get_trigger_type(irq_get_irq_data(irq));

	sdio->oob_irq_supported = true;
	sdio->oob_irq_nr = irq;
	sdio->oob_irq_flags = irqf;
}

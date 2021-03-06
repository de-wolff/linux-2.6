/*
 * arch/arm/mach-tegra/board-smba1002-bt.c
 *
 * Copyright (C) 2011 Eduardo Jos� Tagle <ejtagle@tutopia.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/console.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/memblock.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/io.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/nand.h>
#include <mach/iomap.h>

#include "board.h"
#include "board-smba1002.h"
#include "clock.h"
#include "gpio-names.h"
#include "devices.h"

static struct resource smba_bcm4329_rfkill_resources[] = {
	{
		.name   = "bcm4329_nreset_gpio",
		.start  = SMBA1002_BT_RESET,
		.end    = SMBA1002_BT_RESET,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device smba_bcm4329_rfkill_device = {
	.name = "bcm4329_rfkill",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(smba_bcm4329_rfkill_resources),
	.resource       = smba_bcm4329_rfkill_resources,
};

void __init smba_bt_rfkill(void)
{
	/*Add Clock Resource*/
	int res = clk_add_alias("bcm4329_32k_clk", smba_bcm4329_rfkill_device.name, \
				"blink", NULL);
	printk("Initializing BT RFKILL, clk_add_alias: %d \n", res);
	res = platform_device_register(&smba_bcm4329_rfkill_device);
	if (res)
		printk("Error on BT RFKILL reg, result: %d \n", res);
	return;
}

void __init smba_setup_bluesleep(void)
{
	struct platform_device *pdev = NULL;
	struct resource *res;

	pdev = platform_device_alloc("bluesleep", 0);
	if (!pdev) {
		pr_err("unable to allocate platform device for bluesleep");
		return;
	}

	res = kzalloc(sizeof(struct resource) * 2, GFP_KERNEL);
	if (!res) {
		pr_err("unable to allocate resource for bluesleep\n");
		goto err_free_dev;
	}

	res[0].name   = "gpio_host_wake";
	res[0].start  = SMBA1002_BT_IRQ;
	res[0].end    = SMBA1002_BT_IRQ;
	res[0].flags  = IORESOURCE_IO;

	res[1].name   = "host_wake";
	res[1].start  = gpio_to_irq(SMBA1002_BT_IRQ);
	res[1].end    = gpio_to_irq(SMBA1002_BT_IRQ);
	res[1].flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWEDGE;

	if (platform_device_add_resources(pdev, res, 2)) {
		pr_err("unable to add resources to bluesleep device\n");
		goto err_free_res;
	}

	if (platform_device_add(pdev)) {
		pr_err("unable to add bluesleep device\n");
		goto err_free_res;
	}

	tegra_gpio_enable(SMBA1002_BT_IRQ);

	kfree(res);

	return;

err_free_res:
	kfree(res);
err_free_dev:
	platform_device_put(pdev);
	return;
}

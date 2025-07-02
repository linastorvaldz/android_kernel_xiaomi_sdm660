// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include "version.h"
#include "device.h"
#include "noise.h"
#include "queueing.h"
#include "ratelimiter.h"
#include "netlink.h"

#include <uapi/linux/wireguard.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/genetlink.h>
#include <net/rtnetlink.h>

<<<<<<< HEAD
static int __init wg_mod_init(void)
=======
static int __init mod_init(void)
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
{
	int ret;

	ret = wg_allowedips_slab_init();
	if (ret < 0)
		goto err_allowedips;

#ifdef DEBUG
	ret = -ENOTRECOVERABLE;
	if (!wg_allowedips_selftest() || !wg_packet_counter_selftest() ||
	    !wg_ratelimiter_selftest())
<<<<<<< HEAD
		goto err_peer;
#endif
	wg_noise_init();

	ret = wg_peer_init();
	if (ret < 0)
		goto err_peer;

=======
		goto err_device;
#endif
	wg_noise_init();

>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
	ret = wg_device_init();
	if (ret < 0)
		goto err_device;

	ret = wg_genetlink_init();
	if (ret < 0)
		goto err_netlink;

	pr_info("WireGuard " WIREGUARD_VERSION " loaded. See www.wireguard.com for information.\n");
	pr_info("Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.\n");

	return 0;

err_netlink:
	wg_device_uninit();
err_device:
<<<<<<< HEAD
	wg_peer_uninit();
err_peer:
=======
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
	wg_allowedips_slab_uninit();
err_allowedips:
	return ret;
}

<<<<<<< HEAD
static void __exit wg_mod_exit(void)
{
	wg_genetlink_uninit();
	wg_device_uninit();
	wg_peer_uninit();
	wg_allowedips_slab_uninit();
}

module_init(wg_mod_init);
module_exit(wg_mod_exit);
=======
static void __exit mod_exit(void)
{
	wg_genetlink_uninit();
	wg_device_uninit();
	wg_allowedips_slab_uninit();
}

module_init(mod_init);
module_exit(mod_exit);
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("WireGuard secure network tunnel");
MODULE_AUTHOR("Jason A. Donenfeld <Jason@zx2c4.com>");
MODULE_VERSION(WIREGUARD_VERSION);
MODULE_ALIAS_RTNL_LINK(KBUILD_MODNAME);
MODULE_ALIAS_GENL_FAMILY(WG_GENL_NAME);

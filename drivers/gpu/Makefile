# SPDX-License-Identifier: GPL-2.0-only
# drm/tegra depends on host1x, so if both drivers are built-in care must be
# taken to initialize them in the correct order. Link order is the only way
# to ensure this currently.
obj-$(CONFIG_TEGRA_HOST1X)	+= host1x/
obj-y			+= drm/ vga/
obj-$(CONFIG_IMX_IPUV3_CORE)	+= ipu-v3/
<<<<<<< HEAD
obj-$(CONFIG_QCOM_KGSL) += msm/
=======
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
obj-$(CONFIG_TRACE_GPU_MEM)		+= trace/

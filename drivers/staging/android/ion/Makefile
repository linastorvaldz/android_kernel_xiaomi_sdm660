# SPDX-License-Identifier: GPL-2.0
<<<<<<< HEAD
obj-$(CONFIG_ION) +=	ion.o ion_heap.o \
			ion_page_pool.o ion_system_heap.o \
			ion_carveout_heap.o ion_chunk_heap.o \
			ion_system_secure_heap.o ion_cma_heap.o \
			ion_secure_util.o ion_cma_secure_heap.o msm/

ifdef CONFIG_COMPAT
obj-$(CONFIG_ION_LEGACY) += compat_ion.o
endif
=======
obj-$(CONFIG_ION) +=	ion-alloc.o
CFLAGS_ion.o = -I$(src)
ion-alloc-objs += ion.o ion-ioctl.o ion_heap.o
ion-alloc-$(CONFIG_ION_SYSTEM_HEAP) += ion_system_heap.o ion_page_pool.o
ion-alloc-$(CONFIG_ION_CARVEOUT_HEAP) += ion_carveout_heap.o
ion-alloc-$(CONFIG_ION_CHUNK_HEAP) += ion_chunk_heap.o
ion-alloc-$(CONFIG_ION_CMA_HEAP) += ion_cma_heap.o
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)

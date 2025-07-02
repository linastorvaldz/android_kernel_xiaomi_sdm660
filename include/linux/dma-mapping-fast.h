/* SPDX-License-Identifier: GPL-2.0-only */
/*
<<<<<<< HEAD
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
=======
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
 */

#ifndef __LINUX_DMA_MAPPING_FAST_H
#define __LINUX_DMA_MAPPING_FAST_H

#include <linux/iommu.h>
#include <linux/io-pgtable-fast.h>

struct dma_iommu_mapping;
struct io_pgtable_ops;
struct iova_domain;

struct dma_fast_smmu_mapping {
	struct device		*dev;
	struct iommu_domain	*domain;
	struct iova_domain	*iovad;

	dma_addr_t	 base;
	size_t		 size;
	size_t		 num_4k_pages;

	unsigned int	bitmap_size;
<<<<<<< HEAD
	/* bitmap has 1s marked only valid mappings */
	unsigned long	*bitmap;
	/* clean_bitmap has 1s marked for both valid and stale tlb mappings */
	unsigned long	*clean_bitmap;
	unsigned long	next_start;
=======
	unsigned long	*bitmap;
	unsigned long	next_start;
	unsigned long	upcoming_stale_bit;
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
	bool		have_stale_tlbs;

	dma_addr_t	pgtbl_dma_handle;
	struct io_pgtable_ops *pgtbl_ops;

	spinlock_t	lock;
	struct notifier_block notifier;
};

#ifdef CONFIG_IOMMU_IO_PGTABLE_FAST
int fast_smmu_init_mapping(struct device *dev,
			    struct dma_iommu_mapping *mapping);
void fast_smmu_put_dma_cookie(struct iommu_domain *domain);
#else
static inline int fast_smmu_init_mapping(struct device *dev,
					  struct dma_iommu_mapping *mapping)
{
	return -ENODEV;
}

static inline void fast_smmu_put_dma_cookie(struct iommu_domain *domain) {}
#endif

#endif /* __LINUX_DMA_MAPPING_FAST_H */

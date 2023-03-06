/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 */

#ifndef _DMA_BUF_REF_H
#define _DMA_BUF_REF_H

struct dma_buf;
struct seq_file;

#ifdef CONFIG_DEBUG_DMA_BUF_REF
void dma_buf_ref_init(struct dma_buf *b);
void dma_buf_ref_destroy(struct dma_buf *b);
void dma_buf_ref_mod(struct dma_buf *b, int nr);
int dma_buf_ref_show(struct seq_file *s, struct dma_buf *dmabuf);

#else
static inline void dma_buf_ref_init(struct dma_buf *b) {}
static inline void dma_buf_ref_destroy(struct dma_buf *b) {}
static inline void dma_buf_ref_mod(struct dma_buf *b, int nr) {}
static inline int dma_buf_ref_show(struct seq_file *s, struct dma_buf *dmabuf)
{
	return -ENOMEM;
}
struct msm_dma_buf *msm_dma_buf_create(struct dma_buf *dma_buf) { return ERR_PTR(-ENOMEM); }
void msm_dma_buf_destroy(struct dma_buf *dma_buf) {}
#endif


#endif /* _DMA_BUF_REF_H */

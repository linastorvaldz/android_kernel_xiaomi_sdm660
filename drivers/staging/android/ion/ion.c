// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011 Google, Inc.
 * Copyright (c) 2011-2018, The Linux Foundation. All rights reserved.
 * Copyright (C) 2019-2021 Sultan Alsawaf <sultan@kerneltoast.com>.
 */

#include <linux/miscdevice.h>
<<<<<<< HEAD
=======
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/rbtree.h>
#include <linux/sched/task.h>
#include <linux/seq_file.h>
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "ion_secure_util.h"
#include "ion_system_secure_heap.h"
#include "compat_ion.h"

<<<<<<< HEAD
#ifdef CONFIG_ION_LEGACY
#include "ion_legacy.h"
#endif

struct ion_dma_buf_attachment {
	struct ion_dma_buf_attachment *next;
	struct device *dev;
	struct sg_table table;
	struct list_head list;
	bool dma_mapped;
};
=======
#define CREATE_TRACE_POINTS
#include "ion_trace.h"
#include "ion.h"

static struct ion_device *internal_dev;
static int heap_id;
static atomic_long_t total_heap_bytes;
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)

static long ion_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static const struct file_operations ion_fops = {
	.unlocked_ioctl = ion_ioctl,
#ifdef CONFIG_COMPAT
#ifdef CONFIG_ION_LEGACY
	.compat_ioctl = compat_ion_ioctl
#else
	.compat_ioctl = ion_ioctl
#endif
#endif
};

static struct ion_device ion_dev = {
	.heaps = PLIST_HEAD_INIT(ion_dev.heaps),
	.dev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "ion",
		.fops = &ion_fops
	}
};

static void ion_buffer_free_work(struct work_struct *work)
{
	struct ion_buffer *buffer = container_of(work, typeof(*buffer), free);
	struct ion_dma_buf_attachment *a, *next;
	struct ion_heap *heap = buffer->heap;

	msm_dma_buf_freed(&buffer->iommu_data);
	for (a = buffer->attachments; a; a = next) {
		next = a->next;
		sg_free_table(&a->table);
		kfree(a);
	}
	if (buffer->kmap_refcount)
		heap->ops->unmap_kernel(heap, buffer);
	heap->ops->free(buffer);
	kfree(buffer);
}

<<<<<<< HEAD
static struct ion_buffer *ion_buffer_create(struct ion_heap *heap, size_t len,
					    unsigned int flags)
=======
static void track_buffer_created(struct ion_buffer *buffer)
{
	long total = atomic_long_add_return(buffer->size, &total_heap_bytes);

	trace_ion_stat(buffer->sg_table, buffer->size, total);
}

static void track_buffer_destroyed(struct ion_buffer *buffer)
{
	long total = atomic_long_sub_return(buffer->size, &total_heap_bytes);

	trace_ion_stat(buffer->sg_table, -buffer->size, total);
}

/* this function should only be called while dev->lock is held */
static struct ion_buffer *ion_buffer_create(struct ion_heap *heap,
					    struct ion_device *dev,
					    unsigned long len,
					    unsigned long flags)
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
{
	struct ion_buffer *buffer;
	int ret;

	buffer = kmalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	*buffer = (typeof(*buffer)){
		.flags = flags,
		.heap = heap,
		.size = len,
		.kmap_lock = __MUTEX_INITIALIZER(buffer->kmap_lock),
		.free = __WORK_INITIALIZER(buffer->free, ion_buffer_free_work),
		.map_freelist = LIST_HEAD_INIT(buffer->map_freelist),
		.freelist_lock = __SPIN_LOCK_INITIALIZER(buffer->freelist_lock),
		.iommu_data = {
			.map_list = LIST_HEAD_INIT(buffer->iommu_data.map_list),
			.lock = __MUTEX_INITIALIZER(buffer->iommu_data.lock)
		}
	};

	ret = heap->ops->allocate(heap, buffer, len, flags);
	if (ret) {
		if (ret == -EINTR || !(heap->flags & ION_HEAP_FLAG_DEFER_FREE))
			goto free_buffer;

		drain_workqueue(heap->wq);
		if (heap->ops->allocate(heap, buffer, len, flags))
			goto free_buffer;
	}

<<<<<<< HEAD
=======
	if (!buffer->sg_table) {
		WARN_ONCE(1, "This heap needs to set the sgtable");
		ret = -EINVAL;
		goto err1;
	}

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	mutex_lock(&dev->buffer_lock);
	ion_buffer_add(dev, buffer);
	mutex_unlock(&dev->buffer_lock);
	track_buffer_created(buffer);
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
	return buffer;

free_buffer:
	kfree(buffer);
	return ERR_PTR(ret);
}

<<<<<<< HEAD
=======
void ion_buffer_destroy(struct ion_buffer *buffer)
{
	if (buffer->kmap_cnt > 0) {
		pr_warn_once("%s: buffer still mapped in the kernel\n",
			     __func__);
		buffer->heap->ops->unmap_kernel(buffer->heap, buffer);
	}
	buffer->heap->ops->free(buffer);
	kfree(buffer);
}

static void _ion_buffer_destroy(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;
	struct ion_device *dev = buffer->dev;

	mutex_lock(&dev->buffer_lock);
	rb_erase(&buffer->node, &dev->buffers);
	mutex_unlock(&dev->buffer_lock);
	track_buffer_destroyed(buffer);

	if (heap->flags & ION_HEAP_FLAG_DEFER_FREE)
		ion_heap_freelist_add(heap, buffer);
	else
		ion_buffer_destroy(buffer);
}

static void *ion_buffer_kmap_get(struct ion_buffer *buffer)
{
	void *vaddr;

	if (buffer->kmap_cnt) {
		if (buffer->kmap_cnt == INT_MAX)
			return ERR_PTR(-EOVERFLOW);

		buffer->kmap_cnt++;
		return buffer->vaddr;
	}
	vaddr = buffer->heap->ops->map_kernel(buffer->heap, buffer);
	if (WARN_ONCE(!vaddr,
		      "heap->ops->map_kernel should return ERR_PTR on error"))
		return ERR_PTR(-EINVAL);
	if (IS_ERR(vaddr))
		return vaddr;
	buffer->vaddr = vaddr;
	buffer->kmap_cnt++;
	return vaddr;
}

static void ion_buffer_kmap_put(struct ion_buffer *buffer)
{
	buffer->kmap_cnt--;
	if (!buffer->kmap_cnt) {
		buffer->heap->ops->unmap_kernel(buffer->heap, buffer);
		buffer->vaddr = NULL;
	}
}

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	struct sg_table *new_table;
	int ret, i;
	struct scatterlist *sg, *new_sg;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(new_table, table->nents, GFP_KERNEL);
	if (ret) {
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sg(table->sgl, sg, table->nents, i) {
		memcpy(new_sg, sg, sizeof(*sg));
		new_sg->dma_address = 0;
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static void free_duped_table(struct sg_table *table)
{
	sg_free_table(table);
	kfree(table);
}

struct ion_dma_buf_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
};

static int ion_dma_buf_attach(struct dma_buf *dmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct ion_dma_buf_attachment *a;
	struct sg_table *table;
	struct ion_buffer *buffer = dmabuf->priv;

	a = kzalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	table = dup_sg_table(buffer->sg_table);
	if (IS_ERR(table)) {
		kfree(a);
		return -ENOMEM;
	}

	a->table = table;
	a->dev = attachment->dev;
	INIT_LIST_HEAD(&a->list);

	attachment->priv = a;

	mutex_lock(&buffer->lock);
	list_add(&a->list, &buffer->attachments);
	mutex_unlock(&buffer->lock);

	return 0;
}

static void ion_dma_buf_detatch(struct dma_buf *dmabuf,
				struct dma_buf_attachment *attachment)
{
	struct ion_dma_buf_attachment *a = attachment->priv;
	struct ion_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	list_del(&a->list);
	mutex_unlock(&buffer->lock);
	free_duped_table(a->table);

	kfree(a);
}

>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
static struct sg_table *ion_map_dma_buf(struct dma_buf_attachment *attachment,
					enum dma_data_direction dir)
{
	struct dma_buf *dmabuf = attachment->dmabuf;
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_dma_buf_attachment *a = attachment->priv;
	int count, map_attrs = attachment->dma_map_attrs;

	if (!(buffer->flags & ION_FLAG_CACHED) ||
	    !hlos_accessible_buffer(buffer))
		map_attrs |= DMA_ATTR_SKIP_CPU_SYNC;

	if (map_attrs & DMA_ATTR_DELAYED_UNMAP)
		count = msm_dma_map_sg_attrs(attachment->dev, a->table.sgl,
					     a->table.nents, dir, dmabuf,
					     map_attrs);
	else
		count = dma_map_sg_attrs(attachment->dev, a->table.sgl,
					 a->table.nents, dir, map_attrs);
	if (!count)
		return ERR_PTR(-ENOMEM);

	a->dma_mapped = true;
	return &a->table;
}

static void ion_unmap_dma_buf(struct dma_buf_attachment *attachment,
			      struct sg_table *table,
			      enum dma_data_direction dir)
{
	struct dma_buf *dmabuf = attachment->dmabuf;
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_dma_buf_attachment *a = attachment->priv;
	int map_attrs = attachment->dma_map_attrs;

	if (!(buffer->flags & ION_FLAG_CACHED) ||
	    !hlos_accessible_buffer(buffer))
		map_attrs |= DMA_ATTR_SKIP_CPU_SYNC;

	if (map_attrs & DMA_ATTR_DELAYED_UNMAP)
		msm_dma_unmap_sg_attrs(attachment->dev, table->sgl,
				       table->nents, dir, dmabuf, map_attrs);
	else
		dma_unmap_sg_attrs(attachment->dev, table->sgl, table->nents,
				   dir, map_attrs);
	a->dma_mapped = false;
}

static int ion_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_heap *heap = buffer->heap;

	if (!buffer->heap->ops->map_user)
		return -EINVAL;

	if (!(buffer->flags & ION_FLAG_CACHED))
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	return heap->ops->map_user(heap, buffer, vma);
}

static void ion_dma_buf_release(struct dma_buf *dmabuf)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_heap *heap = buffer->heap;

	if (heap->flags & ION_HEAP_FLAG_DEFER_FREE)
		queue_work(heap->wq, &buffer->free);
	else
		ion_buffer_free_work(&buffer->free);
}

static void *ion_dma_buf_vmap(struct dma_buf *dmabuf)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_heap *heap = buffer->heap;
	void *vaddr;

	if (!heap->ops->map_kernel)
		return ERR_PTR(-ENODEV);

	mutex_lock(&buffer->kmap_lock);
	if (buffer->kmap_refcount) {
		vaddr = buffer->vaddr;
		buffer->kmap_refcount++;
	} else {
		vaddr = heap->ops->map_kernel(heap, buffer);
		if (IS_ERR_OR_NULL(vaddr)) {
			vaddr = ERR_PTR(-EINVAL);
		} else {
			buffer->vaddr = vaddr;
			buffer->kmap_refcount++;
		}
	}
	mutex_unlock(&buffer->kmap_lock);

	return vaddr;
}

static void ion_dma_buf_vunmap(struct dma_buf *dmabuf, void *vaddr)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_heap *heap = buffer->heap;

	mutex_lock(&buffer->kmap_lock);
	if (!--buffer->kmap_refcount)
		heap->ops->unmap_kernel(heap, buffer);
	mutex_unlock(&buffer->kmap_lock);
}

static void *ion_dma_buf_kmap(struct dma_buf *dmabuf, unsigned long offset)
{
<<<<<<< HEAD
	void *vaddr;

	vaddr = ion_dma_buf_vmap(dmabuf);
=======
	struct ion_buffer *buffer = dmabuf->priv;
	void *vaddr;

	if (!buffer->heap->ops->map_kernel) {
		pr_err("%s: map kernel is not implemented by this heap.\n",
		       __func__);
		return ERR_PTR(-ENOTTY);
	}
	mutex_lock(&buffer->lock);
	vaddr = ion_buffer_kmap_get(buffer);
	mutex_unlock(&buffer->lock);

>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
	if (IS_ERR(vaddr))
		return vaddr;

	return vaddr + offset * PAGE_SIZE;
}

static void ion_dma_buf_kunmap(struct dma_buf *dmabuf, unsigned long offset,
			       void *ptr)
{
<<<<<<< HEAD
	ion_dma_buf_vunmap(dmabuf, NULL);
=======
	struct ion_buffer *buffer = dmabuf->priv;

	if (buffer->heap->ops->map_kernel) {
		mutex_lock(&buffer->lock);
		ion_buffer_kmap_put(buffer);
		mutex_unlock(&buffer->lock);
	}

>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
}

static int ion_dup_sg_table(struct sg_table *dst, struct sg_table *src)
{
<<<<<<< HEAD
	unsigned int nents = src->nents;
	struct scatterlist *d, *s;

	if (sg_alloc_table(dst, nents, GFP_KERNEL))
		return -ENOMEM;

	for (d = dst->sgl, s = src->sgl;
	     nents > SG_MAX_SINGLE_ALLOC; nents -= SG_MAX_SINGLE_ALLOC - 1,
	     d = sg_chain_ptr(&d[SG_MAX_SINGLE_ALLOC - 1]),
	     s = sg_chain_ptr(&s[SG_MAX_SINGLE_ALLOC - 1]))
		memcpy(d, s, (SG_MAX_SINGLE_ALLOC - 1) * sizeof(*d));

	if (nents)
		memcpy(d, s, nents * sizeof(*d));
=======
	struct ion_buffer *buffer = dmabuf->priv;
	struct ion_dma_buf_attachment *a;

	mutex_lock(&buffer->lock);
	list_for_each_entry(a, &buffer->attachments, list) {
		dma_sync_sg_for_cpu(a->dev, a->table->sgl, a->table->nents,
				    direction);
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int ion_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
				      enum dma_data_direction direction)
{
	struct ion_buffer *buffer = dmabuf->priv;
	struct ion_dma_buf_attachment *a;

	mutex_lock(&buffer->lock);
	list_for_each_entry(a, &buffer->attachments, list) {
		dma_sync_sg_for_device(a->dev, a->table->sgl, a->table->nents,
				       direction);
	}
	mutex_unlock(&buffer->lock);
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)

	return 0;
}

static int ion_dma_buf_attach(struct dma_buf *dmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_dma_buf_attachment *a;

	spin_lock(&buffer->freelist_lock);
	list_for_each_entry(a, &buffer->map_freelist, list) {
		if (a->dev == attachment->dev) {
			list_del(&a->list);
			spin_unlock(&buffer->freelist_lock);
			attachment->priv = a;
			return 0;
		}
	}
	spin_unlock(&buffer->freelist_lock);

	a = kmalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	if (ion_dup_sg_table(&a->table, buffer->sg_table)) {
		kfree(a);
		return -ENOMEM;
	}

	a->dev = attachment->dev;
	a->dma_mapped = false;
	attachment->priv = a;
	a->next = buffer->attachments;
	buffer->attachments = a;

	return 0;
}

static void ion_dma_buf_detach(struct dma_buf *dmabuf,
			       struct dma_buf_attachment *attachment)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_dma_buf_attachment *a = attachment->priv;

	spin_lock(&buffer->freelist_lock);
	list_add(&a->list, &buffer->map_freelist);
	spin_unlock(&buffer->freelist_lock);
}

static int ion_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
					enum dma_data_direction dir)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_dma_buf_attachment *a;

	if (!hlos_accessible_buffer(buffer))
		return -EPERM;

	if (!(buffer->flags & ION_FLAG_CACHED))
		return 0;

	for (a = buffer->attachments; a; a = a->next) {
		if (a->dma_mapped)
			dma_sync_sg_for_cpu(a->dev, a->table.sgl,
					    a->table.nents, dir);
	}

	return 0;
}

static int ion_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
				      enum dma_data_direction dir)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_dma_buf_attachment *a;

	if (!hlos_accessible_buffer(buffer))
		return -EPERM;

	if (!(buffer->flags & ION_FLAG_CACHED))
		return 0;

	for (a = buffer->attachments; a; a = a->next) {
		if (a->dma_mapped)
			dma_sync_sg_for_device(a->dev, a->table.sgl,
					       a->table.nents, dir);
	}

	return 0;
}

static void ion_sgl_sync_range(struct device *dev, struct scatterlist *sgl,
			       unsigned int nents, unsigned long offset,
			       unsigned long len, enum dma_data_direction dir,
			       bool for_cpu)
{
	dma_addr_t sg_dma_addr = sg_dma_address(sgl);
	unsigned long total = 0;
	struct scatterlist *sg;
	int i;

	for_each_sg(sgl, sg, nents, i) {
		unsigned long sg_offset, sg_left, size;

		total += sg->length;
		if (total <= offset) {
			sg_dma_addr += sg->length;
			continue;
		}

		sg_left = total - offset;
		sg_offset = sg->length - sg_left;
		size = min(len, sg_left);
		if (for_cpu)
			dma_sync_single_range_for_cpu(dev, sg_dma_addr,
						      sg_offset, size, dir);
		else
			dma_sync_single_range_for_device(dev, sg_dma_addr,
							 sg_offset, size, dir);
		len -= size;
		if (!len)
			break;

		offset += size;
		sg_dma_addr += sg->length;
	}
}

static int ion_dma_buf_cpu_access_partial(struct dma_buf *dmabuf,
					  enum dma_data_direction dir,
					  unsigned int offset, unsigned int len,
					  bool start)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);
	struct ion_dma_buf_attachment *a;
	int ret = 0;

	for (a = buffer->attachments; a; a = a->next) {
		if (!a->dma_mapped)
			continue;

		if (a->table.nents > 1 && sg_next(a->table.sgl)->dma_length) {
			ret = -EINVAL;
			continue;
		}

		ion_sgl_sync_range(a->dev, a->table.sgl, a->table.nents, offset,
				   len, dir, start);
	}

	return ret;
}

static int ion_dma_buf_begin_cpu_access_partial(struct dma_buf *dmabuf,
						enum dma_data_direction dir,
						unsigned int offset,
						unsigned int len)
{
	return ion_dma_buf_cpu_access_partial(dmabuf, dir, offset, len, true);
}

static int ion_dma_buf_end_cpu_access_partial(struct dma_buf *dmabuf,
					      enum dma_data_direction dir,
					      unsigned int offset,
					      unsigned int len)
{
	return ion_dma_buf_cpu_access_partial(dmabuf, dir, offset, len, false);
}

static int ion_dma_buf_get_flags(struct dma_buf *dmabuf, unsigned long *flags)
{
	struct ion_buffer *buffer = container_of(dmabuf->priv, typeof(*buffer),
						 iommu_data);

	*flags = buffer->flags;
	return 0;
}

static const struct dma_buf_ops ion_dma_buf_ops = {
	.map_dma_buf = ion_map_dma_buf,
	.unmap_dma_buf = ion_unmap_dma_buf,
	.mmap = ion_mmap,
	.release = ion_dma_buf_release,
	.attach = ion_dma_buf_attach,
	.detach = ion_dma_buf_detach,
	.begin_cpu_access = ion_dma_buf_begin_cpu_access,
	.end_cpu_access = ion_dma_buf_end_cpu_access,
	.begin_cpu_access_partial = ion_dma_buf_begin_cpu_access_partial,
	.end_cpu_access_partial = ion_dma_buf_end_cpu_access_partial,
	.map = ion_dma_buf_kmap,
	.unmap = ion_dma_buf_kunmap,
	.vmap = ion_dma_buf_vmap,
	.vunmap = ion_dma_buf_vunmap,
	.get_flags = ion_dma_buf_get_flags
};

struct dma_buf *ion_alloc_dmabuf(size_t len, unsigned int heap_id_mask,
				 unsigned int flags)
{
	struct ion_device *idev = &ion_dev;
	struct dma_buf_export_info exp_info;
	struct ion_buffer *buffer = NULL;
	struct dma_buf *dmabuf;
	struct ion_heap *heap;

	len = PAGE_ALIGN(len);
	if (!len)
		return ERR_PTR(-EINVAL);

	plist_for_each_entry(heap, &idev->heaps, node) {
		if (BIT(heap->id) & heap_id_mask) {
			buffer = ion_buffer_create(heap, len, flags);
			if (!IS_ERR(buffer) || PTR_ERR(buffer) == -EINTR)
				break;
		}
	}

	if (!buffer)
		return ERR_PTR(-ENODEV);

	if (IS_ERR(buffer))
		return ERR_CAST(buffer);

	exp_info = (typeof(exp_info)){
		.ops = &ion_dma_buf_ops,
		.size = buffer->size,
		.flags = O_RDWR,
		.priv = &buffer->iommu_data
	};

	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf))
		ion_buffer_free_work(&buffer->free);

	return dmabuf;
}

static int ion_alloc_fd(struct ion_allocation_data *a)
{
	struct dma_buf *dmabuf;
	int fd;

	dmabuf = ion_alloc_dmabuf(a->len, a->heap_id_mask, a->flags);
	if (IS_ERR(dmabuf))
		return PTR_ERR(dmabuf);

	fd = dma_buf_fd(dmabuf, O_CLOEXEC);
	if (fd < 0)
		dma_buf_put(dmabuf);

	return fd;
}

static int ion_old_alloc_fd(struct ion_old_allocation_data *a)
{
	struct dma_buf *dmabuf;
	int fd;

	dmabuf = ion_alloc_dmabuf(a->len, a->heap_id_mask, a->flags);
	if (IS_ERR(dmabuf))
		return PTR_ERR(dmabuf);

	fd = dma_buf_fd(dmabuf, O_CLOEXEC);
	if (fd < 0)
		dma_buf_put(dmabuf);

	return fd;
}

void ion_add_heap(struct ion_device *idev, struct ion_heap *heap)
{
	struct ion_heap_data *hdata = &idev->heap_data[idev->heap_count];

	if (heap->flags & ION_HEAP_FLAG_DEFER_FREE) {
		heap->wq = alloc_workqueue("%s", WQ_UNBOUND | WQ_MEM_RECLAIM, 0,
					   heap->name);
		BUG_ON(!heap->wq);
		workqueue_set_max_active(heap->wq, 1);
	}

	if (heap->ops->shrink)
		ion_heap_init_shrinker(heap);

	plist_node_init(&heap->node, -heap->id);
	plist_add(&heap->node, &idev->heaps);

	strlcpy(hdata->name, heap->name, sizeof(hdata->name));
	hdata->type = heap->type;
	hdata->heap_id = heap->id;
	idev->heap_count++;
}

static int ion_walk_heaps(int heap_id, int type, void *data,
			  int (*f)(struct ion_heap *heap, void *data))
{
	struct ion_device *idev = &ion_dev;
	struct ion_heap *heap;
	int ret = 0;

	plist_for_each_entry(heap, &idev->heaps, node) {
		if (heap->type == type && ION_HEAP(heap->id) == heap_id) {
			ret = f(heap, data);
			break;
		}
	}

	return ret;
}

static int ion_query_heaps(struct ion_heap_query *query)
{
	struct ion_device *idev = &ion_dev;

	if (!query->cnt)
		return -EINVAL;

	if (copy_to_user(u64_to_user_ptr(query->heaps), idev->heap_data,
			 min(query->cnt, idev->heap_count) *
			 sizeof(*idev->heap_data)))
		return -EFAULT;

	return 0;
}

static long ion_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct ion_device *idev = &ion_dev;
	union {
		struct ion_allocation_data allocation;
		struct ion_prefetch_data prefetch;
		struct ion_heap_query query;
#ifdef CONFIG_ION_LEGACY
		struct ion_fd_data fd;
		struct ion_old_allocation_data old_allocation;
		struct ion_handle_data handle;
#endif
	} data;
	int fd, *output;

	switch (cmd) {
	case ION_IOC_ALLOC:
		if (copy_from_user(&data, (void __user *)arg,
				   sizeof(struct ion_allocation_data)))
			return -EFAULT;

		fd = ion_alloc_fd(&data.allocation);
		if (fd < 0)
			return fd;

		output = &fd;
		arg += offsetof(struct ion_allocation_data, fd);
		break;
	case ION_IOC_HEAP_QUERY:
		/* The data used in ion_heap_query ends at `heaps` */
		if (copy_from_user(&data, (void __user *)arg,
				   offsetof(struct ion_heap_query, heaps) +
				   sizeof(data.query.heaps)))
			return -EFAULT;

		if (data.query.heaps)
			return ion_query_heaps(&data.query);

		output = &idev->heap_count;
		/* `arg` already points to the ion_heap_query member we want */
		break;
	case ION_IOC_PREFETCH:
		/* The data used in ion_prefetch_data begins at `regions` */
		if (copy_from_user(&data.prefetch.regions,
				   (void __user *)arg +
				   offsetof(struct ion_prefetch_data, regions),
				   sizeof(struct ion_prefetch_data) -
				   offsetof(struct ion_prefetch_data, regions)))
			return -EFAULT;

		return ion_walk_heaps(data.prefetch.heap_id,
				      ION_HEAP_TYPE_SYSTEM_SECURE,
				      &data.prefetch,
				      ion_system_secure_heap_prefetch);
	case ION_IOC_DRAIN:
		/* The data used in ion_prefetch_data begins at `regions` */
		if (copy_from_user(&data.prefetch.regions,
				   (void __user *)arg +
				   offsetof(struct ion_prefetch_data, regions),
				   sizeof(struct ion_prefetch_data) -
				   offsetof(struct ion_prefetch_data, regions)))
			return -EFAULT;

		return ion_walk_heaps(data.prefetch.heap_id,
				      ION_HEAP_TYPE_SYSTEM_SECURE,
				      &data.prefetch,
				      ion_system_secure_heap_drain);
#ifdef CONFIG_ION_LEGACY
	case ION_OLD_IOC_ALLOC:
		/* The data used in ion_old_allocation_data ends at `flags` */
		if (copy_from_user(&data, (void __user *)arg,
				   offsetof(struct ion_old_allocation_data, flags) +
				   sizeof(data.old_allocation.flags)))
			return -EFAULT;

		fd = ion_old_alloc_fd(&data.old_allocation);
		if (fd < 0)
			return fd;

		output = &fd;
		arg += offsetof(struct ion_old_allocation_data, handle);
		break;
	case ION_IOC_FREE:
		/* Only copy the `handle` field */
		if (copy_from_user(&data.handle.handle,
				   (void __user *)arg +
				   offsetof(struct ion_handle_data, handle),
				   sizeof(data.handle.handle)))
			return -EFAULT;

		/*
		 * libion passes 0 as the handle to check for this ioctl's
		 * existence and expects -ENOTTY on kernel 4.12+ as an indicator
		 * of having a new ION ABI. We want to use new ION as much as
		 * possible, so pretend that this ioctl doesn't exist when
		 * libion checks for it.
		 */
		if (!data.handle.handle)
			return -ENOTTY;

		return 0;
	case ION_IOC_SHARE:
	case ION_IOC_MAP:
		/* Only copy the `handle` field */
		if (copy_from_user(&data.fd.handle,
				   (void __user *)arg +
				   offsetof(struct ion_fd_data, handle),
				   sizeof(data.fd.handle)))
			return -EFAULT;

		output = &data.fd.handle;
		arg += offsetof(struct ion_fd_data, fd);
		break;
	case ION_IOC_IMPORT:
		/* Only copy the `fd` field */
		if (copy_from_user(&data.fd.fd,
				   (void __user *)arg +
				   offsetof(struct ion_fd_data, fd),
				   sizeof(data.fd.fd)))
			return -EFAULT;

		output = &data.fd.fd;
		arg += offsetof(struct ion_fd_data, handle);
		break;
#endif
	default:
		return -ENOTTY;
	}

	if (copy_to_user((void __user *)arg, output, sizeof(*output)))
		return -EFAULT;

	return 0;
}

struct ion_device *ion_device_create(struct ion_heap_data *heap_data)
{
	struct ion_device *idev = &ion_dev;
	int ret;

<<<<<<< HEAD
	ret = misc_register(&idev->dev);
	if (ret)
		return ERR_PTR(ret);

	idev->heap_data = heap_data;
	return idev;
}
=======
	if (!heap->ops->allocate || !heap->ops->free)
		pr_err("%s: can not add heap with invalid ops struct.\n",
		       __func__);

	spin_lock_init(&heap->free_lock);
	heap->free_list_size = 0;

	if (heap->flags & ION_HEAP_FLAG_DEFER_FREE)
		ion_heap_init_deferred_free(heap);

	if ((heap->flags & ION_HEAP_FLAG_DEFER_FREE) || heap->ops->shrink) {
		ret = ion_heap_init_shrinker(heap);
		if (ret)
			pr_err("%s: Failed to register shrinker\n", __func__);
	}

	heap->dev = dev;
	down_write(&dev->lock);
	heap->id = heap_id++;
	/*
	 * use negative heap->id to reverse the priority -- when traversing
	 * the list later attempt higher id numbers first
	 */
	plist_node_init(&heap->node, -heap->id);
	plist_add(&heap->node, &dev->heaps);

	if (heap->shrinker.count_objects && heap->shrinker.scan_objects) {
		char debug_name[64];

		snprintf(debug_name, 64, "%s_shrink", heap->name);
		debugfs_create_file(debug_name, 0644, dev->debug_root,
				    heap, &debug_shrink_fops);
	}

	dev->heap_cnt++;
	up_write(&dev->lock);
}
EXPORT_SYMBOL(ion_device_add_heap);

static ssize_t
total_heaps_kb_show(struct kobject *kobj, struct kobj_attribute *attr,
		    char *buf)
{
	u64 size_in_bytes = atomic_long_read(&total_heap_bytes);

	return sprintf(buf, "%llu\n", div_u64(size_in_bytes, 1024));
}

static ssize_t
total_pools_kb_show(struct kobject *kobj, struct kobj_attribute *attr,
		    char *buf)
{
	u64 size_in_bytes = ion_page_pool_nr_pages() * PAGE_SIZE;

	return sprintf(buf, "%llu\n", div_u64(size_in_bytes, 1024));
}

static struct kobj_attribute total_heaps_kb_attr =
	__ATTR_RO(total_heaps_kb);

static struct kobj_attribute total_pools_kb_attr =
	__ATTR_RO(total_pools_kb);

static struct attribute *ion_device_attrs[] = {
	&total_heaps_kb_attr.attr,
	&total_pools_kb_attr.attr,
	NULL,
};

ATTRIBUTE_GROUPS(ion_device);

static int ion_init_sysfs(void)
{
	struct kobject *ion_kobj;
	int ret;

	ion_kobj = kobject_create_and_add("ion", kernel_kobj);
	if (!ion_kobj)
		return -ENOMEM;

	ret = sysfs_create_groups(ion_kobj, ion_device_groups);
	if (ret) {
		kobject_put(ion_kobj);
		return ret;
	}

	return 0;
}

static int ion_device_create(void)
{
	struct ion_device *idev;
	int ret;

	idev = kzalloc(sizeof(*idev), GFP_KERNEL);
	if (!idev)
		return -ENOMEM;

	idev->dev.minor = MISC_DYNAMIC_MINOR;
	idev->dev.name = "ion";
	idev->dev.fops = &ion_fops;
	idev->dev.parent = NULL;
	ret = misc_register(&idev->dev);
	if (ret) {
		pr_err("ion: failed to register misc device.\n");
		goto err_reg;
	}

	ret = ion_init_sysfs();
	if (ret) {
		pr_err("ion: failed to add sysfs attributes.\n");
		goto err_sysfs;
	}

	idev->debug_root = debugfs_create_dir("ion", NULL);
	idev->buffers = RB_ROOT;
	mutex_init(&idev->buffer_lock);
	init_rwsem(&idev->lock);
	plist_head_init(&idev->heaps);
	internal_dev = idev;
	return 0;

err_sysfs:
	misc_deregister(&idev->dev);
err_reg:
	kfree(idev);
	return ret;
}

#ifdef CONFIG_ION_MODULE
int ion_module_init(void)
{
	int ret;

	ret = ion_device_create();
#ifdef CONFIG_ION_SYSTEM_HEAP
	if (ret)
		return ret;

	ret = ion_system_heap_create();
	if (ret)
		return ret;

	ret = ion_system_contig_heap_create();
#endif
#ifdef CONFIG_ION_CMA_HEAP
	if (ret)
		return ret;

	ret = ion_add_cma_heaps();
#endif
	return ret;
}

module_init(ion_module_init);
#else
subsys_initcall(ion_device_create);
#endif

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Ion memory allocator");
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)

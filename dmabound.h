#ifndef	_DMABOUND_H
#define	_DMABOUND_H
/*
 *	dmabound.h
 */

/*  Functions in dmabound.c  */
void	*malloc_dma_page_aligned_block (size_t size);
void	 free_dma_page_aligned_block (void *block);

#endif	/* _DMABOUND_H */

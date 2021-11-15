/*
 *	dmabound.c	$Id: dmabound.c 1.1 1997/05/08 21:30:54 Graham Rel $
 *
 *	$Log: dmabound.c $
 *	Revision 1.1  1997/05/08 21:30:54  Graham
 *	Added missing dmabound.h.  Use TEST.  Added RCS control.
 *
 */
#ifdef	RCS
static const char	rcs_id[] = "$Id: dmabound.c 1.1 1997/05/08 21:30:54 Graham Rel $";
#endif

#include <stdlib.h>

#include "global.h"
#include "dmabound.h"

typedef struct block_header_struct
{
	char		*ptr;
	unsigned long	sig;
} Block_Header;

#define	BLOCK_HEADER_SIG	0xfacecabeL

#define LINEAR_ADDRESS(p)	((((unsigned long) FP_SEG(p)) << 4) + (unsigned long) FP_OFF(p))
#define CROSSES_DMA_PAGE(p,l)	(((LINEAR_ADDRESS(p) + (l) - 1) & 0xffff) < (LINEAR_ADDRESS(p) & 0xffff))

/*
 *  Returns ptr to a buffer that will not cross a 64K DMA Page boundary
 *  Block must be free'd by free_dma_page_aligned_block
 *
 */
void *
malloc_dma_page_aligned_block (size_t size)
{
#ifdef	__DJGPP__

	return (malloc (size));	

#else	/* !__DJGPP__ */

	char	*block, *p;
	Block_Header	*bhp;

	block = malloc (size + sizeof (Block_Header));
	if (block == NULL)
		return (NULL);

	p = block + sizeof (Block_Header);	/*  our housekeeping overhead  */

	DPRINTF (("malloc: %u bytes block: %p p: %p %04x:%04x linear: %08lx end: %08lx %d\n", size, block, p, FP_SEG(p), FP_OFF(p), LINEAR_ADDRESS(p), LINEAR_ADDRESS(p+size-1), CROSSES_DMA_PAGE(p,size)));
	/*
 	 * If it crosses a 64K boundary then (end & 0xffff) < (start & 0xffff)
	 */
	if (CROSSES_DMA_PAGE (p, size))
	{
		free (block);

		if (size > ((65535U - sizeof(Block_Header)) / 2))
			return (NULL);

		block = malloc ((2*size) + sizeof (Block_Header));
		if (block == NULL)
			return (NULL);

		p = block + sizeof (Block_Header);

		DPRINTF (("re-malloc %u block: %p p: %p %04x:%04x linear: %08lx end: %08lx %d\n", size, block, p, FP_SEG(p), FP_OFF(p), LINEAR_ADDRESS(p), LINEAR_ADDRESS(p+size-1), CROSSES_DMA_PAGE(p,size)));

		if (CROSSES_DMA_PAGE (p, size))
			p += size;	/*  Can't fail for size < 64K  */
	}

	bhp = (Block_Header *) p;
	bhp[-1].ptr = block;
	bhp[-1].sig = BLOCK_HEADER_SIG;

	DPRINTF (("malloc: %p %p %p %lx %d\n", block, p, bhp[-1].ptr, bhp[-1].sig, CROSSES_DMA_PAGE(p,size)));

	return (p);

#endif	/* !__DJGPP__ */
}

void
free_dma_page_aligned_block (void *block)
{
#ifdef	__DJGPP__

	free (block);

#else	/* !__DJGPP__ */

	Block_Header	*bhp;

	if (block == NULL)
		return;

	bhp = (Block_Header *) block;
	DPRINTF (("free: %p %p %lx\n", block, bhp[-1].ptr, bhp[-1].sig));
	if (bhp[-1].sig != BLOCK_HEADER_SIG)
		return;	/*  Bad mem block  */
	free (bhp[-1].ptr);

#endif	/* !__DJGPP__ */
}

#ifdef	TEST
#include <stdio.h>

void
_ExceptInit (void)
{
}

int
main (void)
{
	char	*p[10];

	int	i;

	for (i = 0; i < 10; i++)
		p[i] = malloc_dma_page_aligned_block ((i+1)*1024);

	for (i = 0; i < 10; i++)
		free_dma_page_aligned_block(p[i]);

	p[0] = malloc(1024);
	free_dma_page_aligned_block(p[0]);
	free (p[0]);

	return (0);
}
#endif	/* TEST */

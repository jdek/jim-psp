/*
 *  Copyright (C) 2003-2005  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  $Id: memory_mips.c,v 1.2 2005/11/30 16:23:09 debug Exp $
 *
 *  MIPS-specific memory routines. Included from cpu_mips.c.
 */

#include <sys/types.h>
#include <sys/mman.h>


/*
 *  insert_into_tiny_cache():
 *
 *  If the tiny cache is enabled (USE_TINY_CACHE), then this routine inserts
 *  a vaddr to paddr translation first in the instruction (or data) tiny
 *  translation cache.
 */
static void insert_into_tiny_cache(struct cpu *cpu, int instr, int writeflag,
	uint64_t vaddr, uint64_t paddr)
{
#ifdef USE_TINY_CACHE
	int wf = 1 + (writeflag == MEM_WRITE);

	if (cpu->machine->bintrans_enable)
		return;

	paddr &= ~0xfff;
	vaddr >>= 12;

	if (instr) {
		/*  Code:  */
		memmove(&cpu->cd.mips.translation_cache_instr[1],
		    &cpu->cd.mips.translation_cache_instr[0],
		    sizeof(struct translation_cache_entry) *
		    (N_TRANSLATION_CACHE_INSTR - 1));

		cpu->cd.mips.translation_cache_instr[0].wf = wf;
		cpu->cd.mips.translation_cache_instr[0].vaddr_pfn = vaddr;
		cpu->cd.mips.translation_cache_instr[0].paddr = paddr;
	} else {
		/*  Data:  */
		memmove(&cpu->cd.mips.translation_cache_data[1],
		    &cpu->cd.mips.translation_cache_data[0],
		    sizeof(struct translation_cache_entry) *
		    (N_TRANSLATION_CACHE_DATA - 1));

		cpu->cd.mips.translation_cache_data[0].wf = wf;
		cpu->cd.mips.translation_cache_data[0].vaddr_pfn = vaddr;
		cpu->cd.mips.translation_cache_data[0].paddr = paddr;
	}
#endif
}


/*
 *  memory_cache_R3000():
 *
 *  R2000/R3000 specific cache handling.
 *
 *  Return value is 1 if a jump to do_return_ok is supposed to happen directly
 *  after this routine is finished, 0 otherwise.
 */
int memory_cache_R3000(struct cpu *cpu, int cache, uint64_t paddr,
	int writeflag, size_t len, unsigned char *data)
{
#ifdef ENABLE_CACHE_EMULATION
	struct r3000_cache_line *rp;
	int cache_line;
	uint32_t tag_mask;
	unsigned char *memblock;
	struct memory *mem = cpu->mem;
	int offset;
#endif
	unsigned int i;
	int cache_isolated = 0, addr, hit, which_cache = cache;


	if (len > 4 || cache == CACHE_NONE)
		return 0;


#ifdef ENABLE_CACHE_EMULATION
	if (cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & MIPS1_SWAP_CACHES)
		which_cache ^= 1;

	tag_mask = 0xffffffff & ~cpu->cd.mips.cache_mask[which_cache];
	cache_line = (paddr & cpu->cd.mips.cache_mask[which_cache])
	    / cpu->cd.mips.cache_linesize[which_cache];
	rp = (struct r3000_cache_line *) cpu->cd.mips.cache_tags[which_cache];

	/*  Is this a cache hit or miss?  */
	hit = (rp[cache_line].tag_valid & R3000_TAG_VALID) &&
	    (rp[cache_line].tag_paddr == (paddr & tag_mask));

#ifdef ENABLE_INSTRUCTION_DELAYS
	if (!hit)
		cpu->cd.mips.instruction_delay +=
		    cpu->cd.mips.cpu_type.instrs_per_cycle
		    * cpu->cd.mips.cache_miss_penalty[which_cache];
#endif

	/*
	 *  The cache miss bit is only set on cache reads, and only to the
	 *  data cache. (?)
	 *
	 *  (TODO: is this correct? I don't remember where I got this from.)
	 */
	if (cache == CACHE_DATA && writeflag==MEM_READ) {
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~MIPS1_CACHE_MISS;
		if (!hit)
			cpu->cd.mips.coproc[0]->reg[COP0_STATUS] |=
			    MIPS1_CACHE_MISS;
	}

	/*
	 *  Is the Data cache isolated?  Then don't access main memory:
	 */
	if (cache == CACHE_DATA &&
	    cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & MIPS1_ISOL_CACHES)
		cache_isolated = 1;

	addr = paddr & cpu->cd.mips.cache_mask[which_cache];

	/*
	 *  If there was a miss and the cache is not isolated, then flush
	 *  the old cacheline back to main memory, and read in the new
	 *  cacheline.
	 *
	 *  Then access the cache.
	 */
/*
	fatal("L1 CACHE isolated=%i hit=%i write=%i cache=%i cacheline=%i"
	    " paddr=%08x => addr in"
	    " cache = 0x%lx\n", cache_isolated, hit, writeflag,
	    which_cache, cache_line, (int)paddr,
	    addr);
*/
	if (!hit && !cache_isolated) {
		unsigned char *dst, *src;
		uint64_t old_cached_paddr = rp[cache_line].tag_paddr
		    + cache_line * cpu->cd.mips.cache_linesize[which_cache];

		/*  Flush the old cacheline to main memory:  */
		if ((rp[cache_line].tag_valid & R3000_TAG_VALID) &&
		    (rp[cache_line].tag_valid & R3000_TAG_DIRTY)) {
/*			fatal("  FLUSHING old tag=0%08x "
			    "old_cached_paddr=0x%08x\n",
			    rp[cache_line].tag_paddr,
			    old_cached_paddr);
*/
			memblock = memory_paddr_to_hostaddr(
			    mem, old_cached_paddr, MEM_WRITE);
			offset = old_cached_paddr
			    & ((1 << BITS_PER_MEMBLOCK) - 1)
			    & ~cpu->cd.mips.cache_mask[which_cache];

			src = cpu->cd.mips.cache[which_cache];
			dst = memblock + (offset &
			    ~cpu->cd.mips.cache_mask[which_cache]);

			src += cache_line *
			    cpu->cd.mips.cache_linesize[which_cache];
			dst += cache_line *
			    cpu->cd.mips.cache_linesize[which_cache];

			if (memblock == NULL) {
				fatal("BUG in memory.c! Hm.\n");
			} else {
				memcpy(dst, src,
				    cpu->cd.mips.cache_linesize[which_cache]);
			}
			/*  offset is the offset within
			 *  the memblock:
			 *  printf("read: offset = 0x%x\n", offset);
			 */
		}

		/*  Copy from main memory into the cache:  */
		memblock = memory_paddr_to_hostaddr(mem, paddr, writeflag);
		offset = paddr & ((1 << BITS_PER_MEMBLOCK) - 1)
		    & ~cpu->cd.mips.cache_mask[which_cache];
		/*  offset is offset within the memblock:
		 *  printf("write: offset = 0x%x\n", offset);
		 */

/*		fatal("  FETCHING new paddr=0%08x\n", paddr);
*/
		dst = cpu->cd.mips.cache[which_cache];

		if (memblock == NULL) {
			if (writeflag == MEM_READ)
			memset(dst, 0,
			    cpu->cd.mips.cache_linesize[which_cache]);
		} else {
			src = memblock + (offset &
			    ~cpu->cd.mips.cache_mask[which_cache]);

			src += cache_line *
			    cpu->cd.mips.cache_linesize[which_cache];
			dst += cache_line *
			    cpu->cd.mips.cache_linesize[which_cache];
			memcpy(dst, src,
			    cpu->cd.mips.cache_linesize[which_cache]);
		}

		rp[cache_line].tag_paddr = paddr & tag_mask;
		rp[cache_line].tag_valid = R3000_TAG_VALID;
	}

	if (cache_isolated && writeflag == MEM_WRITE) {
		rp[cache_line].tag_valid = 0;
	}

	if (writeflag==MEM_READ) {
		for (i=0; i<len; i++)
			data[i] = cpu->cd.mips.cache[which_cache][(addr+i) &
			    cpu->cd.mips.cache_mask[which_cache]];
	} else {
		for (i=0; i<len; i++) {
			if (cpu->cd.mips.cache[which_cache][(addr+i) &
			    cpu->cd.mips.cache_mask[which_cache]] != data[i]) {
				rp[cache_line].tag_valid |= R3000_TAG_DIRTY;
			}
			cpu->cd.mips.cache[which_cache][(addr+i) &
			    cpu->cd.mips.cache_mask[which_cache]] = data[i];
		}
	}

	/*  Run instructions from the right host page:  */
	if (cache == CACHE_INSTRUCTION) {
		memblock = memory_paddr_to_hostaddr(mem, paddr, writeflag);
		if (memblock != NULL) {
			cpu->cd.mips.pc_last_host_4k_page = memblock +
			    (paddr & ((1 << BITS_PER_MEMBLOCK) - 1) & ~0xfff);
		}
	}

	/*  Write-through! (Write to main memory as well.)  */
	if (writeflag == MEM_READ || cache_isolated)
		return 1;

#else

	/*
	 *  R2000/R3000 without correct cache emulation:
	 *
	 *  TODO: This is just enough to trick NetBSD/pmax and Ultrix into
	 *  being able to detect the cache sizes and think that the caches
	 *  are actually working, but they are not.
	 */

	if (cache != CACHE_DATA)
		return 0;

	/*  Is this a cache hit or miss?  */
	hit = (cpu->cd.mips.cache_last_paddr[which_cache]
		& ~cpu->cd.mips.cache_mask[which_cache])
	    == (paddr & ~(cpu->cd.mips.cache_mask[which_cache]));

#ifdef ENABLE_INSTRUCTION_DELAYS
	if (!hit)
		cpu->cd.mips.instruction_delay +=
		    cpu->cd.mips.cpu_type.instrs_per_cycle
		    * cpu->cd.mips.cache_miss_penalty[which_cache];
#endif

	/*
	 *  The cache miss bit is only set on cache reads, and only to the
	 *  data cache. (?)
	 *
	 *  (TODO: is this correct? I don't remember where I got this from.)
	 */
	if (cache == CACHE_DATA && writeflag==MEM_READ) {
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~MIPS1_CACHE_MISS;
		if (!hit)
			cpu->cd.mips.coproc[0]->reg[COP0_STATUS] |=
			    MIPS1_CACHE_MISS;
	}

	/*
	 *  Is the Data cache isolated?  Then don't access main memory:
	 */
	if (cache == CACHE_DATA &&
	    cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & MIPS1_ISOL_CACHES)
		cache_isolated = 1;

	addr = paddr & cpu->cd.mips.cache_mask[which_cache];

	/*  Data cache isolated?  Then don't access main memory:  */
	if (cache_isolated) {
		/*  debug("ISOLATED write=%i cache=%i vaddr=%016llx "
		    "paddr=%016llx => addr in cache = 0x%lx\n",
		    writeflag, cache, (long long)vaddr,
		    (long long)paddr, addr);  */

		if (writeflag==MEM_READ) {
			for (i=0; i<len; i++)
				data[i] = cpu->cd.mips.cache[cache][(addr+i) &
				    cpu->cd.mips.cache_mask[cache]];
		} else {
			for (i=0; i<len; i++)
				cpu->cd.mips.cache[cache][(addr+i) &
				    cpu->cd.mips.cache_mask[cache]] = data[i];
		}
		return 1;
	} else {
		/*  Reload caches if necessary:  */

		/*  No!  Not when not emulating caches fully. (TODO?)  */
		cpu->cd.mips.cache_last_paddr[cache] = paddr;
	}
#endif

	return 0;
}

#if 0	/* psp hack */

#define TRANSLATE_ADDRESS	translate_address_mmu3k
#define	V2P_MMU3K
#include "memory_mips_v2p.c"
#undef TRANSLATE_ADDRESS
#undef V2P_MMU3K

#define TRANSLATE_ADDRESS	translate_address_mmu8k
#define	V2P_MMU8K
#include "memory_mips_v2p.c"
#undef TRANSLATE_ADDRESS
#undef V2P_MMU8K

#define TRANSLATE_ADDRESS	translate_address_mmu10k
#define	V2P_MMU10K
#include "memory_mips_v2p.c"
#undef TRANSLATE_ADDRESS
#undef V2P_MMU10K

/*  Almost generic  :-)  */
#define TRANSLATE_ADDRESS	translate_address_mmu4100
#define	V2P_MMU4100
#include "memory_mips_v2p.c"
#undef TRANSLATE_ADDRESS
#undef V2P_MMU4100

#define TRANSLATE_ADDRESS	translate_address_generic
#include "memory_mips_v2p.c"

#endif

#define TRANSLATE_ADDRESS	translate_address_allegrex
#include "memory_mips_v2p.c"


#ifndef EXPERIMENTAL_NEWMIPS

#define MEMORY_RW	mips_memory_rw
#define MEM_MIPS
#include "../memory_rw.c"
#undef MEM_MIPS
#undef MEMORY_RW

#endif

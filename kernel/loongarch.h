#include <larchintrin.h>

#define  CSR_CRMD_IE_SHIFT		    2
#define  CSR_CRMD_IE			        ( 0x1 << CSR_CRMD_IE_SHIFT )

#define  EXT_INT_EN_SHIFT         48

#define LOONGARCH_IOCSR_EXTIOI_EN_BASE		    0x1600
#define LOONGARCH_IOCSR_EXTIOI_ISR_BASE		    0x1800
#define LOONGARCH_IOCSR_EXTIOI_MAP_BASE       0x14c0
#define LOONGARCH_IOCSR_EXTIOI_ROUTE_BASE	    0x1c00
#define LOONGARCH_IOCSR_EXRIOI_NODETYPE_BASE  0x14a0

// read and write tp, the thread pointer, which holds
// this core's hartid (core number), the index into cpus[].

static inline uint64
r_sp()
{
  uint64 x;
  asm volatile("addi.d %0, $sp, 0" : "=r" (x) );
  return x;
}

static inline uint64
r_tp()
{
  uint64 x;
  asm volatile("addi.d %0, $tp, 0" : "=r" (x) );
  return x;
}

static inline uint32
r_csr_crmd()
{
  uint32 x;
  asm volatile("csrrd %0, 0x0" : "=r" (x) );
  return x;
}

static inline void 
w_csr_crmd(uint32 x)
{
  asm volatile("csrwr %0, 0x0" : : "r" (x));
}

#define PRMD_PPLV (3U << 0)  // Previous Privilege
#define PRMD_PIE  (1U << 2)  // Previous Int_enable

static inline uint32
r_csr_prmd()
{
  uint32 x;
  asm volatile("csrrd %0, 0x1" : "=r" (x) );
  return x;
}

static inline void 
w_csr_prmd(uint32 x)
{
  asm volatile("csrwr %0, 0x1" : : "r" (x));
}

static inline uint64
r_csr_era()
{
  uint64 x;
  asm volatile("csrrd %0, 0x6" : "=r" (x) );
  return x;
}

static inline void 
w_csr_era(uint64 x)
{
  asm volatile("csrwr %0, 0x6" : : "r" (x));
}

#define CSR_ESTAT_ECODE  (0x3fU << 16)

static inline uint32
r_csr_estat()
{
  uint32 x;
  asm volatile("csrrd %0, 0x5" : "=r" (x) );
  return x;
}

#define CSR_ECFG_VS_SHIFT  16 
#define CSR_ECFG_LIE_TI_SHIFT  11
#define HWI_VEC  0x3fcU
#define TI_VEC  (0x1 << CSR_ECFG_LIE_TI_SHIFT)

static inline uint32
r_csr_ecfg()
{
  uint32 x;
  asm volatile("csrrd %0, 0x4" : "=r" (x) );
  return x;
}

static inline void
w_csr_ecfg(uint32 x)
{
  asm volatile("csrwr %0, 0x4" : : "r" (x) );
}

#define CSR_TICLR_CLR  (0x1 << 0)

static inline uint32
r_csr_ticlr()
{
  uint32 x;
  asm volatile("csrrd %0, 0x44" : "=r" (x) );
  return x;
}

static inline void
w_csr_ticlr(uint32 x)
{
  asm volatile("csrwr %0, 0x44" : : "r" (x) );
}

static inline uint64
r_csr_eentry()
{
  uint64 x;
  asm volatile("csrrd %0, 0xc" : "=r" (x) );
  return x;
}

static inline uint64
r_csr_tlbrelo0()
{
  uint64 x;
  asm volatile("csrrd %0, 0x8c" : "=r" (x) );
  return x;
}

static inline uint64
r_csr_tlbrelo1()
{
  uint64 x;
  asm volatile("csrrd %0, 0x8d" : "=r" (x) );
  return x;
}

static inline void
w_csr_eentry(uint64 x)
{
  asm volatile("csrwr %0, 0xc" : : "r" (x) );
}

static inline void
w_csr_tlbrentry(uint64 x)
{
  asm volatile("csrwr %0, 0x88" : : "r" (x) );
}

static inline void
w_csr_merrentry(uint64 x)
{
  asm volatile("csrwr %0, 0x93" : : "r" (x) );
}

static inline void
w_csr_stlbps(uint32 x)
{
  asm volatile("csrwr %0, 0x1e" : : "r" (x) );
}

static inline void
w_csr_asid(uint32 x)
{
  asm volatile("csrwr %0, 0x18" : : "r" (x) );
}

#define CSR_TCFG_EN            (1U << 0)
#define CSR_TCFG_PER           (1U << 1)

static inline void
w_csr_tcfg(uint64 x)
{
  asm volatile("csrwr %0, 0x41" : : "r" (x) );
}

static inline void
w_csr_tlbrehi(uint64 x)
{
  asm volatile("csrwr %0, 0x8e" : : "r" (x) );
}

static inline uint64
r_csr_pgdl()
{
  uint64 x;
  asm volatile("csrrd %0, 0x19" : "=r" (x) );
  return x;
}

static inline void
w_csr_pgdl(uint64 x)
{
  asm volatile("csrwr %0, 0x19" : : "r" (x) );
}

static inline void
w_csr_pgdh(uint64 x)
{
  asm volatile("csrwr %0, 0x1a" : : "r" (x) );
}

#define PTBASE  12U
#define PTWIDTH  9U
#define DIR1BASE  21U 
#define DIR1WIDTH  9U
#define DIR2BASE  30U
#define DIR2WIDTH  9U 
#define PTEWIDTH  0U
#define DIR3BASE  39U
#define DIR3WIDTH  9U
#define DIR4WIDTH  0U

static inline void
w_csr_pwcl(uint32 x)
{
  asm volatile("csrwr %0, 0x1c" : : "r" (x) );
}

static inline void
w_csr_pwch(uint32 x)
{
  asm volatile("csrwr %0, 0x1d" : : "r" (x) );
}

static inline uint32
r_csr_badi()
{
  uint32 x;
  asm volatile("csrrd %0, 0x8" : "=r" (x) );
  return x;
}

/* IOCSR */
static inline uint32 iocsr_readl(uint32 reg)
{
	return __iocsrrd_w(reg);
}

static inline uint64 iocsr_readq(uint32 reg)
{
	return __iocsrrd_d(reg);
}

static inline void iocsr_writel(uint32 val, uint32 reg)
{
	__iocsrwr_w(val, reg);
}

static inline void iocsr_writeq(uint64 val, uint32 reg)
{
	__iocsrwr_d(val, reg);
}

static inline int
intr_get()
{
  uint32 x = r_csr_crmd();
  return (x & CSR_CRMD_IE) != 0;
}

// enable device interrupts
static inline void
intr_on()
{
  w_csr_crmd(r_csr_crmd() | CSR_CRMD_IE);
}

// disable device interrupts
static inline void
intr_off()
{
  w_csr_crmd(r_csr_crmd() & ~CSR_CRMD_IE);
}

#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page

#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

#define PTE_V (1L << 0) // valid
#define PTE_D (1L << 1) // dirty
#define PTE_PLV (3L << 2) //privilege level
#define PTE_MAT (1L << 4) //memory access type
#define PTE_P (1L << 7) // physical page exists
#define PTE_W (1L << 8) // writeable
#define PTE_NX (1UL << 62) //non executable
#define PTE_NR (1L << 61) //non readable
#define PTE_RPLV (1UL << 63) //restricted privilege level enable

#define PAMASK          0xFFFFFFFFFUL << PGSHIFT
#define PTE2PA(pte) (pte & PAMASK)
// shift a physical address to the right place for a PTE.
#define PA2PTE(pa) (((uint64)pa) & PAMASK)
#define PTE_FLAGS(pte) ((pte) & 0xE0000000000001FFUL)

// extract the three 9-bit page table indices from a virtual address.
#define PXMASK          0x1FF // 9 bits
#define PXSHIFT(level)  (PGSHIFT+(9*(level)))
#define PX(level, va) ((((uint64) (va)) >> PXSHIFT(level)) & PXMASK)

#define MAXVA (1L << (9 + 12 - 1)) //Lower half virtual address

typedef uint64 pte_t;
typedef uint64 *pagetable_t;
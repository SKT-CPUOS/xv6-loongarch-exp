#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "loongarch.h"
#include "defs.h"

//
// the loongarch 7A1000 I/O Interrupt Controller Registers.
//

void
apic_init(void)
{
  *(volatile uint64*)(LS7A_INT_MASK_REG) = ~(0x1UL << UART0_IRQ);

  *(volatile uint64*)(LS7A_INT_EDGE_REG) = (0x1UL << UART0_IRQ);

  *(volatile uint8*)(LS7A_INT_HTMSI_VEC_REG + UART0_IRQ) = UART0_IRQ;

  *(volatile uint64*)(LS7A_INT_POL_REG) = 0x0UL;

}

// tell the apic we've served this IRQ.
void
apic_complete(uint64 irq)
{
  *(volatile uint64*)(LS7A_INT_CLEAR_REG) = (irq);
}

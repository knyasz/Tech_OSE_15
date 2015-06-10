#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here
int e1000_pci_attach(struct pci_func *pcif){
	pci_func_enable(pcif);
	e1000_bar0 = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);
	cprintf("Should be 0x80080783 : 0x%08x \n",e1000_bar0[BYTE_TO_WORD(E1000_STATUS)]);
	return 0;
}

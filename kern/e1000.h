#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>


volatile  uint32_t * e1000_bar0;

#define BYTE_TO_WORD(byte) ( (byte) >> 2 )

#define E1000_VENDOR_ID 			0x8086
#define E1000_DEV_ID_82540EM		0x100E

#define E1000_STATUS				0x00008  /* Device Status - RO */


int e1000_pci_attach(struct pci_func *pcif);

#endif	// JOS_KERN_E1000_H

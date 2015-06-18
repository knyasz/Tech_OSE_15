#include <kern/e1000.h>
#include <kern/pmap.h>

#include <inc/stdio.h>
#include <inc/assert.h>
#include <inc/string.h>

// LAB 6: Your driver code here

// Transmit descriptors
tx_desctiptor tx_descriptors[E1000_NUM_OF_TX_DESCRIPTORS] __attribute__ ((aligned (16)));
// Transmit buffers
tx_packet_buffer tx_packet_buffers[E1000_NUM_OF_TX_DESCRIPTORS];

int e1000_pci_attach(struct pci_func *pcif){

	//Enable PCI device - Ex3
	pci_func_enable(pcif);

	//Memory map I/O for PCI device - Ex4
	p_e1000_MMIO = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);
	cprintf("Should be 0x80080783 : 0x%08x \n", p_e1000_MMIO[E1000_STATUS]);
	assert(p_e1000_MMIO[E1000_STATUS] == 0x80080783);

	//Transmit Initialization
	e1000_tx_init();


	return 0;
}


void e1000_tx_init(){
	/*****************************************************
	 Transmit Initialization 14.5
	 *****************************************************/
	/*
	 * Allocate a region of memory for the transmit descriptor list.
	 * Software should insure this memory is aligned on a paragraph (16-byte)
	 * boundary. - static array tx_descriptors[].
	 * Initialize tx_descriptors[]
	 */
	//Initialize TX descriptors - Ex5
	memset(	tx_descriptors,
			0,
			sizeof(tx_desctiptor) * E1000_NUM_OF_TX_DESCRIPTORS);

	//Initialize TX buffers - Ex5
	memset(	tx_packet_buffers,
			0,
			sizeof(tx_packet_buffer) * E1000_NUM_OF_TX_DESCRIPTORS);

	//Connect descriptors to buffers - Ex5
	int i;
	for (i = 0; i < E1000_NUM_OF_TX_DESCRIPTORS; i++) {
		tx_descriptors[i].addr = PADDR(tx_packet_buffers[i].buffer);
		/*
		 * Need feedback from HW that the packet is sent:
		 * If you set the RS bit in the command field of a transmit descriptor,
		 * then, when the card has transmitted the packet in that descriptor,
		 * the card will set the DD bit in the status field of the descriptor.
		 */
		tx_descriptors[i].cmd |= E1000_TXD_CMD_RS;
		/*
		 * If a descriptor's DD bit is set,
		 * you know it's safe to recycle that descriptor
		 * and use it to transmit another packet.
		 */
		tx_descriptors[i].status |= E1000_TXD_STAT_DD;
	}

	/*
	 * Program the Transmit Descriptor Base Address (TDBAL/TDBAH) register(s)
	 * with the address of the region.
	 * TDBAL is used for 32-bit addresses and both TDBAL and TDBAH
	 * are used for 64-bit addresses.
	 */
	p_e1000_MMIO[E1000_TDBAL] = PADDR(tx_descriptors);
	p_e1000_MMIO[E1000_TDBAH] = 0;

	/*
	 * Set the Transmit Descriptor Length (TDLEN) register
	 * to the size (in bytes) of the descriptor ring.
	 * This register must be 128-byte aligned.
	 */
	p_e1000_MMIO[E1000_TDLEN] = sizeof(tx_desctiptor) *
									E1000_NUM_OF_TX_DESCRIPTORS;

	/*
	 * The Transmit Descriptor Head and Tail (TDH/TDT)
	 * registers are initialized (by hardware) to 0b
	 * after a power-on or a software initiated Ethernet controller reset.
	 * Software should write 0b to both these registers to ensure this.
	 */
	p_e1000_MMIO[E1000_TDH] = 0;
	p_e1000_MMIO[E1000_TDT] = 0;

	/* Initialize the Transmit Control Register (TCTL) for desired operation
	 * to include the following:
	 */

	 // Set the Enable (TCTL.EN) bit to 1b for normal operation.
	p_e1000_MMIO[E1000_TCTL] |= E1000_TCTL_EN;

	// Set the Pad Short Packets (TCTL.PSP) bit to 1b.
	p_e1000_MMIO[E1000_TCTL] |= E1000_TCTL_PSP;

	// Configure the Collision Threshold (TCTL.CT) to the desired value.
	// Ethernet standard is 10h.
	// This setting only has meaning in half duplex mode
	p_e1000_MMIO[E1000_TCTL] &= ~E1000_TCTL_CT;
	p_e1000_MMIO[E1000_TCTL] |= E1000_TCTL_CT_VALUE;

	// Configure the Collision Distance (TCTL.COLD) to its expected value.
	// For full duplex	operation, this value should be set to 40h.
	// For gigabit half duplex, this value should be set to	200h.
	// For 10/100 half duplex, this value should be set to 40h.
	p_e1000_MMIO[E1000_TCTL] &= ~E1000_TCTL_COLD;
	p_e1000_MMIO[E1000_TCTL] |= E1000_TCTL_COLD_VALUE;

	// Program the Transmit IPG (TIPG) register
	// with the  decimal values from Table 13-77. TIPG Register Bit Description.
	// to get the minimum legal Inter Packet Gap
	p_e1000_MMIO[E1000_TIPG] = 0;
	p_e1000_MMIO[E1000_TIPG] |= E1000_TIPG_IPGT_VALUE;
	p_e1000_MMIO[E1000_TIPG] |= E1000_TIPG_IPGR1_VALUE;
	p_e1000_MMIO[E1000_TIPG] |= E1000_TIPG_IPGR2_VALUE;
}

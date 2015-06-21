#include <kern/e1000.h>
#include <kern/pmap.h>

#include <inc/error.h>

#include <inc/stdio.h>
#include <inc/assert.h>
#include <inc/string.h>

// LAB 6: Your driver code here
static void e1000_mac_address_EEPROM_update();
// Transmit descriptors
tx_desctiptor tx_descriptors[E1000_NUM_OF_TX_DESCRIPTORS] __attribute__ ((aligned (16)));
// Transmit buffers
tx_packet_buffer tx_packet_buffers[E1000_NUM_OF_TX_DESCRIPTORS];

int e1000_pci_attach(struct pci_func *pcif){

	//Enable PCI device - Ex3
	pci_func_enable(pcif);

	//Memory map I/O for PCI device - Ex4
	p_e1000_MMIO = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);
	cprintf("E1000 device status Should be 0x80080783 : 0x%08x \n",
			p_e1000_MMIO[E1000_STATUS]);
	assert(p_e1000_MMIO[E1000_STATUS] == 0x80080783);


	e1000_tx_init(); //Transmit Initialization
	e1000_rx_init(); //Receive Initialization
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

int e1000_transmit_packet(char* data_to_transmit, int data_size_bytes){

	if(data_size_bytes > TX_PACKET_SIZE){
		return - (E_PKT_TOO_LONG);
	}


	uint32_t TDT_index = p_e1000_MMIO[E1000_TDT];

//	if( ! (tx_descriptors[TDT_index].cmd & E1000_TXD_CMD_RS) ){
//		panic (" descriptor at index %d has not "
//				"desc.cmd:RS set by e1000_tx_init()", TDT_index );
//	}

	if( ! tx_descriptors[TDT_index].status & E1000_TXD_STAT_DD){
		return - (E_TX_FULL);
	}

	//BAAAD - use virtual addresses
//	memmove(tx_descriptors[TDT_index].addr,
//			buffer_to_transmit,
//			buffer_size_bytes );

	memmove(tx_packet_buffers[TDT_index].buffer,
			data_to_transmit,
			data_size_bytes);
	tx_descriptors[TDT_index].length = data_size_bytes;

	// Unset status:DD bit -
	// the descriptor can't be reused until the data is sent
	// the status:DD bill be set back by HW after fetching the data
	tx_descriptors[TDT_index].status &= ~ E1000_TXD_STAT_DD;

	// Ensure cmd:RS is set to make future status:DD data valid
	tx_descriptors[TDT_index].cmd |= E1000_TXD_CMD_RS;

	//For multi-descriptor packets,
	//packet status is provided in the final descriptor of the packet (EOP set).
	//If EOP is not set for a descriptor,
	//only the Address, Length, and DD bits are valid.
	//Here - each packet will be held in one descriptor only,
	//Thus every descriptor is an end of packet.
	tx_descriptors[TDT_index].cmd |=  E1000_TXD_CMD_EOP;

	//Advance the Queue tail (TDT)
	p_e1000_MMIO[E1000_TDT] = (++TDT_index)%E1000_NUM_OF_TX_DESCRIPTORS;

	return 0;
}

// Receive descriptors
rx_desctiptor rx_descriptors[E1000_NUM_OF_RX_DESCRIPTORS] __attribute__ ((aligned (16)));
// Receive buffers
rx_packet_buffer rx_packet_buffers[E1000_NUM_OF_RX_DESCRIPTORS];
//
void e1000_rx_init(){
	/*****************************************************
	 Receive Initialization 14.4
	 *****************************************************/
/*
 * Program the Receive Address Register(s) (RAL/RAH)
 * with the desired Ethernet addresses.
 * RAL[0]/RAH[0] should always be used to store
 * the Individual Ethernet MAC address of the Ethernet controller.
 *
 * QEMU's default MAC address of 52:54:00:12:34:56
 *
 * MAC addresses are written from lowest-order byte to highest-order byte,
 * so 52:54:00:12 are the low-order 32 bits of the MAC address
 * and 34:56 are the high-order 16 bits.
 *
 */
	e1000_mac_address_EEPROM_update();


/*
 * Initialize the MTA (Multicast Table Array) to 0b.
 *  Per software, entries can be added to this table as desired.
 */
	p_e1000_MMIO[E1000_MTA] = 0;

/*
 * Program the Interrupt Mask Set/Read (IMS) register to enable any interrupt
 *  the software driver wants to be notified of when the event occurs.
 *   Suggested bits include RXT, RXO, RXDMT, RXSEQ, and LSC. There is no
 *   immediate reason to enable the transmit interrupts
 */

	//  For now, don't configure the card to use interrupts;

/*
 * Allocate a region of memory for the receive descriptor list.
 *  Software should insure this memory is aligned on a paragraph
 *  (16-byte) boundary.
 *   Program the Receive Descriptor Base Address (RDBAL/RDBAH) register(s)
 *    with the address of the region. RDBAL is used for 32-bit addresses
 *    and both RDBAL and RDBAH are used for 64-bit addresses
 */
	p_e1000_MMIO[E1000_RDBAL] = PADDR(rx_descriptors);
	p_e1000_MMIO[E1000_RDBAH] = 0;

/*
 * Set the Receive Descriptor Length (RDLEN) register to the size (in bytes)
 *  of the descriptor ring.
 */
	p_e1000_MMIO[E1000_RDLEN] = sizeof(rx_desctiptor) *
								E1000_NUM_OF_RX_DESCRIPTORS;

/*
 * The Receive Descriptor Head and Tail registers are initialized (by hardware)
 *  to 0b after a power-on or a software-initiated Ethernet controller reset.
 *  Receive buffers of appropriate size should be allocated and pointers
 *  to these buffers should be stored in the receive descriptor ring.
 *  Software initializes the Receive Descriptor Head (RDH) register and
 *  Receive Descriptor Tail (RDT) with the appropriate head and tail addresses.
 *  Head should point to the first valid receive descriptor in the descriptor
 *  ring and tail should point to one descriptor beyond the last valid
 *  descriptor in the descriptor ring.
 */
	//Initialize RX descriptors - Ex10
	memset(	rx_descriptors,
			0,
			sizeof(rx_desctiptor) * E1000_NUM_OF_RX_DESCRIPTORS);

	//Initialize RX buffers - Ex10
	memset(	rx_packet_buffers,
			0,
			sizeof(rx_packet_buffer) * E1000_NUM_OF_RX_DESCRIPTORS);

	//Connect descriptors to buffers - Ex10
	int i;
	for (i = 0; i < E1000_NUM_OF_RX_DESCRIPTORS; i++) {
		rx_descriptors[i].addr = PADDR(rx_packet_buffers[i].buffer);
		rx_descriptors[i].status |= E1000_RXD_STAT_DD;
		rx_descriptors[i].status |=  E1000_RXD_STAT_EOP;
	}

	//Initialize the Head and Tail, Tail points to one descriptor beyond the
	//last valid descriptor in the descriptor ring
	p_e1000_MMIO[E1000_RDH] = 0;
	p_e1000_MMIO[E1000_RDT] = E1000_NUM_OF_RX_DESCRIPTORS - 1;

/*
 * Program the Receive Control (RCTL) register with appropriate values
 * for desired operation to include the following:
 *
 * Set the receiver Enable (RCTL.EN) bit to 1b for normal operation.
 *  However, it is best to leave the Ethernet controller receive logic disabled
 *   (RCTL.EN = 0b) until after the receive descriptor ring has been
 *    initialized and software is ready to process received packets.
 */
	p_e1000_MMIO[E1000_RCTL] |= E1000_RCTL_EN;
	p_e1000_MMIO[E1000_RCTL] &= ~E1000_RCTL_LPE;
//Loopback Mode (RCTL.LBM) should be set to 00b for normal operation
	p_e1000_MMIO[E1000_RCTL] &= ~E1000_RCTL_LBM;

//Configure the Receive Descriptor Minimum Threshold Size (RCTL.RDMTS)
//bits to the desired value, The threshold is set to 1/2 the descriptors list
//when reaching the threshold the hardware would send an interrupt:
// RCR.RXMT0
	p_e1000_MMIO[E1000_RCTL] &= ~E1000_RCTL_RDMTS;
	p_e1000_MMIO[E1000_RCTL] |= E1000_RCTL_RDMTS_TRESHOLD_HALF;

// Configure the Multicast Offset (RCTL.MO) bits to the desired value
	p_e1000_MMIO[E1000_RCTL] &= ~E1000_RCTL_MO;

//Set the Broadcast Accept Mode (RCTL.BAM) bit to 1b allowing the hardware
//to accept broadcast packets
	p_e1000_MMIO[E1000_RCTL] |= E1000_RCTL_BAM;

/*
 * Configure the Receive Buffer Size (RCTL.BSIZE) bits to reflect the size
 * of the receive buffers software provides to hardware. Also configure
 * the Buffer Extension Size (RCTL.BSEX) bits if receive buffer
 * needs to be larger than 2048 bytes.
 */
	p_e1000_MMIO[E1000_RCTL] &= ~E1000_RCTL_BSEX;
	p_e1000_MMIO[E1000_RCTL] &= ~E1000_RCTL_BSIZE;

/*
 * Set the Strip Ethernet CRC (RCTL.SECRC) bit if the desire is for hardware
 *  to strip the CRC prior to DMA-ing the receive packet to host memory.
 */
	p_e1000_MMIO[E1000_RCTL] |= E1000_RCTL_SECRC;

/*
 *	Make sure you got the byte ordering right and didn't forget to set the
 *	 "Address Valid" bit in RAH
 */
	p_e1000_MMIO[E1000_RAH] |= E1000_RAH_AV;
}


/*
 * Fill the p_data_buffer of p_data_length length
 * with received data from net,
 * Set the minimal value between the given p_data_length
 * and data length which was received to p_data_length
 *
 * Panic on JUMBO packets
 *
 * return :
 * 		E_RX_EMPTY error while no data to receive
 * 		0 on OK
 */
int e1000_receive_packet(char* p_data_buffer, uint32_t*  p_data_length){
	uint32_t length = *p_data_length;
	uint32_t RDT_index = p_e1000_MMIO[E1000_RDT];

	if( ! (rx_descriptors[RDT_index].status & E1000_RXD_STAT_DD) ){
		return - (E_RX_EMPTY);
	}
	if( ! (rx_descriptors[RDT_index].status & E1000_RXD_STAT_EOP) ){
		panic ("No Jumbo packets "
				"- E1000_RXD_STAT_EOP is unset at RDT = %d",RDT_index);
	}
	// We take the minimal length between the sent packet length,
	// and the allocated memory - user gave us
	length =  *p_data_length < rx_descriptors[RDT_index].length ?
			 *p_data_length : rx_descriptors[RDT_index].length;

	memmove(p_data_buffer,rx_packet_buffers[RDT_index].buffer,length);
	* p_data_length = rx_descriptors[RDT_index].length;

	rx_descriptors[RDT_index].status &= ~ E1000_RXD_STAT_DD;

	p_e1000_MMIO[E1000_RDT] = (++RDT_index)%E1000_NUM_OF_RX_DESCRIPTORS;

	return 0;
}

static void e1000_mac_address_EEPROM_update(){

	p_e1000_MMIO[E1000_EERD] = E1000_EERD_FIRST_PART_MAC; //0x0;
	p_e1000_MMIO[E1000_EERD] |= E1000_EERD_START;
	while (!(p_e1000_MMIO[E1000_EERD] & E1000_EERD_DONE));
	p_e1000_MMIO[E1000_RAL] = WORD_HIGH_TO_LOW(p_e1000_MMIO[E1000_EERD]);

	p_e1000_MMIO[E1000_EERD] = E1000_EERD_SECOND_PART_MAC;		//0x1 << 8;
	p_e1000_MMIO[E1000_EERD] |= E1000_EERD_START;
	while (!(p_e1000_MMIO[E1000_EERD] & E1000_EERD_DONE));
	p_e1000_MMIO[E1000_RAL] |= p_e1000_MMIO[E1000_EERD] & MASK_HIGH_HALF_OF_WORD;

	p_e1000_MMIO[E1000_EERD] = E1000_EERD_THIRD_PART_MAC; //0x2 << 8;
	p_e1000_MMIO[E1000_EERD] |= E1000_EERD_START;
	while (!(p_e1000_MMIO[E1000_EERD] & E1000_EERD_DONE));
	p_e1000_MMIO[E1000_RAH] = WORD_HIGH_TO_LOW(p_e1000_MMIO[E1000_EERD]);

}

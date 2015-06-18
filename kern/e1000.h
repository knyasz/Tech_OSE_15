#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>


volatile  uint32_t * p_e1000_MMIO;

/*
 * Our p_e1000_MMIO array has each cell of 32 bits,
 * but e1000 registers use the offset in bytes,
 * thus to use our array we need to translate bytes indexes
 * to 32-bit words indexes of the array.
 */
#define BYTE_TO_WORD(byte) ( (byte) >> 2 )

#define E1000_VENDOR_ID 			0x8086
#define E1000_DEV_ID_82540EM		0x100E


// We will support the maximum number of descriptors allowed for lab 6 - 64
#define E1000_NUM_OF_TX_DESCRIPTORS 		64
#define TX_PACKET_SIZE				1518 // Bytes - uint_8t

/* Device Status - RO */
#define E1000_STATUS				BYTE_TO_WORD(0x00008)
/* TX Descriptor Base Address Low - RW */
#define E1000_TDBAL					BYTE_TO_WORD(0x03800)
/* TX Descriptor Base Address High - RW */
#define E1000_TDBAH					BYTE_TO_WORD(0x03804)
/* TX Descriptor Length - RW */
#define E1000_TDLEN					BYTE_TO_WORD(0x03808)
/* TX Descriptor Head - RW */
#define E1000_TDH					BYTE_TO_WORD(0x03810)
/* TX Descriptor Tail - RW */
#define E1000_TDT					BYTE_TO_WORD(0x03818)
/* TX Control - RW */
#define E1000_TCTL					BYTE_TO_WORD(0x00400)

/* Transmit Control */
#define E1000_TCTL_EN				0x00000002	/* enable tx */
#define E1000_TCTL_PSP				0x00000008	/* pad short packets */
#define E1000_TCTL_CT				0x00000ff0	/* collision threshold */
#define E1000_TCTL_CT_VALUE			0x00000100	/* collision threshold value */
#define E1000_TCTL_COLD				0x003ff000	/* collision distance */
//For full duplex	operation, this value should be set to 40h.
#define E1000_TCTL_COLD_VALUE		0x00040000	/* collision distance value*/

/* TX Inter-packet gap -RW */
#define E1000_TIPG					BYTE_TO_WORD(0x00410)
// The value that should be programmed into IPGT is 10
#define E1000_TIPG_IPGT_VALUE		10
// The value that should be programmed into IPGR1 is eight.
#define E1000_TIPG_IPGR1_VALUE		8 << 10
// the value that should be programmed into IPGR2 is six
#define E1000_TIPG_IPGR2_VALUE		6 << 20

/* Descriptor Status */
#define E1000_TXD_STAT_DD    0x00000001 /* Descriptor Done */
#define E1000_TXD_CMD_RS     0x08000000 /* Report Status */



/*
 * Legacy transmit descriptor - Table 3-9
 */
struct tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
} __attribute__((packed));// minimum required memory will be used
typedef struct tx_desc tx_desctiptor;

/*
 * The maximum size of an Ethernet packet is 1518 bytes,
 * which bounds how big the buffer needs to be.
 */
struct tx_packet_buffer
{
	uint8_t buffer[TX_PACKET_SIZE];
} __attribute__((packed));// minimum required memory will be used
typedef struct tx_packet_buffer tx_packet_buffer;


int e1000_pci_attach(struct pci_func *pcif);
void e1000_tx_init();

#endif	// JOS_KERN_E1000_H

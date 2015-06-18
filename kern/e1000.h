#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

volatile  uint32_t * p_e1000_MMIO;
int e1000_pci_attach(struct pci_func *pcif);

void e1000_tx_init();
void e1000_rx_init();

int e1000_transmit_packet(	char* 	data_to_transmit,
							int 	data_size_bytes);

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
//#define E1000_TIPG_IPGT_VALUE		10
#define E1000_TIPG_IPGT_VALUE		0x0000000a
// The value that should be programmed into IPGR1 is eight.
//#define E1000_TIPG_IPGR1_VALUE		8 << 10
#define E1000_TIPG_IPGR1_VALUE		0x00001000
// the value that should be programmed into IPGR2 is six
//#define E1000_TIPG_IPGR2_VALUE		6 << 20
#define E1000_TIPG_IPGR2_VALUE		0x00600000

/*
 * Transmit descriptor status:
 * Software can determine if a packet has been sent by setting the RS bit
 * in the transmit descriptor command field.
 * Checking the transmit descriptor DD bit in memory
 * eliminates a potential race condition.
 * */
// Report Status - set to make DD bit valid
#define E1000_TXD_CMD_RS     0x08000000
// Descriptor Done - when set - the buffer can be filled by SW to be sent
#define E1000_TXD_STAT_DD    0x00000001
#define E1000_TXD_CMD_EOP    0x01000000 /* End of Packet */


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


struct tx_packet_buffer
{
	uint8_t buffer[TX_PACKET_SIZE];
	uint16_t length;
} __attribute__((packed));// minimum required memory will be used
typedef struct tx_packet_buffer tx_packet_buffer;


/*************Receive Definitions****************/
#define E1000_NUM_OF_RX_DESCRIPTORS 		128
/*
 * If you make your receive packet buffers large enough and disable long packets,
 * you won't have to worry about packets spanning multiple receive buffers.
 */
#define RX_PACKET_SIZE				2048 // Bytes - uint_8t
// if packet size is changed, bsex and bsize should be changed accordingly
#define E1000_RCTL_BSEX				0x02000000	/* Buffer size extension */
#define E1000_RCTL_BSIZE			0x00030000	/* rx buffer size 256 */


/* Receive Address (RW Array) - can't initialize with single command */
#define E1000_RA					BYTE_TO_WORD(0x05400)
/* Receive Address Low (RW Array) */
#define E1000_RAL					BYTE_TO_WORD(0x05400)
/* Receive Address High (RW Array) */
#define E1000_RAH					BYTE_TO_WORD(0x05404)
/* RX Descriptor Base Address Low - RW */
#define E1000_RDBAL    				BYTE_TO_WORD(0x02800)
/* RX Descriptor Base Address High - RW */
#define E1000_RDBAH    				BYTE_TO_WORD(0x02804)
/* RX Descriptor Length - RW */
#define E1000_RDLEN    				BYTE_TO_WORD(0x02808)
/* RX Descriptor Head - RW */
#define E1000_RDH					BYTE_TO_WORD(0x02810)
/* RX Descriptor Tail - RW */
#define E1000_RDT 					BYTE_TO_WORD(0x02818)
/* Multicast Table Array - RW Array */
#define E1000_MTA					BYTE_TO_WORD(0x05200)
/* RX Control - RW */
#define E1000_RCTL					BYTE_TO_WORD(0x00100)


/* Receive Control */
#define E1000_RCTL_EN				0x00000002	/* enable */
/* Loop back mode are the 6th and 7th bits */
#define E1000_RCTL_LBM				0x000000C0	/* loopback mode */
#define E1000_RCTL_RDMTS			0x00000300	/* rx desc min threshold */
#define E1000_RCTL_MO				0x00003000	/* multicast offset*/
#define E1000_RCTL_BAM 				0x00008000	/* broadcast enable */
#define E1000_RCTL_SECRC			0x04000000	/* Strip Ethernet CRC */
#define E1000_RCTL_LPE				0x00000020	/* long packet enable */

#define E1000_RAH_AV				0x80000000	/* Receive descriptor valid */

/* rx descriptor min threshold 1/2 of the descriptors lengh*/
#define E1000_RCTL_RDMTS_TRESHOLD_HALF	0x00000000

#define E1000_RXD_STAT_DD			0x01		/* Descriptor Done */
#define E1000_RXD_STAT_EOP			0x02		/* End of Packet */

/*
 * Receive descriptor
 */
struct rx_desc
{
	uint64_t addr;
	uint16_t length;
	uint16_t chksum;
	uint8_t status;
	uint8_t errors;
	uint16_t special;
} __attribute__((packed));// minimum required memory will be used
typedef struct rx_desc rx_desctiptor;


struct rx_packet_buffer
{
	uint8_t buffer[RX_PACKET_SIZE];
	uint16_t length;
} __attribute__((packed));// minimum required memory will be used
typedef struct rx_packet_buffer rx_packet_buffer;

#endif	// JOS_KERN_E1000_H

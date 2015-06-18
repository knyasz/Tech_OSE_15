#include "ns.h"

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:

#define RX_PACKET_SIZE				2048 // Bytes


	// 	- read a packet from the device driver
	char buffer[RX_PACKET_SIZE];
	uint32_t length = RX_PACKET_SIZE;

	int result=0;
	while ( ( result=sys_net_try_receive(buffer,&length) )== E_RX_EMPTY){
		sys_yeld();
	}
	if (result<0){
		panic ("net/input : net_try_receive result is %e.",result);
	}
	//	- send it to the network server

	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.

	// So need to receive from net to some buffer
	int perm = PTE_U | PTE_P | PTE_W;
	// Allocate a page of memory and map it at 'va' with permission
	// 'perm' in the address space of 'envid'.
	// The page's contents are set to 0.
	// If a page is already mapped at 'va', that page is unmapped as a
	// side effect.
	while(sys_page_alloc(0,&nsipcbuf,perm) < 0){
		sys_yeld();
	}

	nsipcbuf.pkt.jp_len = length;
	memmove(nsipcbuf.pkt.jp_data, buffer, length);

	while (sys_ipc_try_send(ns_envid, NSREQ_INPUT, &nsipcbuf, perm) < 0);

}

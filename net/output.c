#include "ns.h"

#include "inc/error.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";
	// LAB 6: Your code here:
	int result = 0;
	while(true){
	// 	- read a packet from the network server
		if ((result = sys_ipc_recv(&nsipcbuf)) < 0){
			panic("net/output receiving error %e.",result);
		}
		// Ensure valid sender (ns_envid) and request type (NSREQ_OUTPUT)
		if (	(thisenv->env_ipc_from != ns_envid)
				||
				(thisenv->env_ipc_value != NSREQ_OUTPUT)){
			continue ;
		}

		do{
			result = sys_net_try_send(nsipcbuf.pkt.jp_data,
									  nsipcbuf.pkt.jp_len);
			if (result == E_PKT_TOO_LONG){
				panic("net/output sending packet error: %e.",result);
			}

		}while(result != 0);

	}

	//	- send the packet to the device driver
}

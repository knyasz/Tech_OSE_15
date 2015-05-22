
#include <inc/lib.h>

void
exit(void)
{
	/*
	 * temporarily comment out the call to close_all() in lib/exit.c;
	 * this function calls subroutines that you will implement later in the lab,
	 *  and therefore will panic if called.
	 */
	//close_all();
	sys_env_destroy(0);
}


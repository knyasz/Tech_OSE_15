// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display all current outstanding stack frames", mon_backtrace},
	{ "showmappingsPD",
		"Display Page Directory mappings for a particular range of virtual addresses"
			"\tUsage:"
			"showmappings <virtual address from> <virtual address to>\n",
			mon_showmappingsPD },
	{ "showmappings",
		"Display Physical mappings for a particular range of virtual addresses"
			"\tUsage:"
			"showmappings <virtual address from> <virtual address to>\n",
			mon_showmappings },
	{ "permission",
			"explicitly set/clear/toggle one of the following pte permissions:\n"
			"\tUsage: "
			"permission <virtual address> <--set|--clear|--toggle> <flag>\n"
			"\tflag is one of the following:\n"
			"\tp 	present\n"
			"\tw	writable\n"
			"\tu	user\n"
			"\tpwt	wrtite-through\n"
			"\tpcd	cache-disabled\n"
			"\ta	accessed\n"
			"\td	dirty\n"
			"\tps	page-size\n"
			"\tg	global" ,
			mon_permissionsManage},
	{ "dump", "dump contents of physical or virtual memory"
			"\tUsage: "
			"dump <--physical|--virtual> <from hexa address> <to hexa address>",
			mon_dump},
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

struct Flag {
	const char *name;
	const uint32_t value;
};
static const struct Flag flags[] = {
	{"p",	PTE_P},
	{"w",	PTE_W},
	{"u",	PTE_U},
	{"pwt",	PTE_PWT},
	{"pcd",	PTE_PCD},
	{"a",	PTE_A},
	{"d",	PTE_D},
	{"ps",	PTE_PS},
	{"g",	PTE_G}
};
#define FLAG_LENGTH 4;
#define NFLAGS (sizeof(flags)/sizeof(flags[0]))
/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}


#define EIP(ebp) (uint32_t)((ebp)+1)
#define ARG(ebp) (uint32_t)((ebp)+2)
int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	return 0;
}

int
mon_dump(int argc, char** argv, struct Trapframe *tf)
{
	if (argc != 4) {
		cprintf("You've entered %d arguments instead of 3\n", argc - 1);
		return 1;
	}
	int isPhysical = -1;
	if (strcmp(argv[1],"--physical")==0) {
		isPhysical = 1;
	} else if (strcmp(argv[1],"--virtual")==0) {
		isPhysical = 0;
	} else {
		cprintf("invalid argument 1\n");
		return 1;
	}
	if (isStringHexValid(argv[2])||isStringHexValid(argv[2])){
		cprintf("one of the addresses is incorrect\n");
		return 1;
	}
	uintptr_t* fromAddr = (uintptr_t*)strtol(argv[2], NULL, 16);
	fromAddr = ROUNDDOWN(fromAddr,4);
	uintptr_t* toAddr = (uintptr_t*)strtol(argv[3], NULL, 16);
	toAddr = ROUNDDOWN(toAddr,4);
	if(fromAddr>toAddr){
		uintptr_t* tmp = fromAddr;
		fromAddr = toAddr;
		toAddr=tmp;
	}
	if(isPhysical){
		fromAddr = KADDR((physaddr_t)fromAddr);
		toAddr = KADDR((physaddr_t)toAddr);
	}

	uintptr_t* vAddress = fromAddr;
	while(vAddress<=toAddr){
		pte_t* pPTe = pgdir_walk(kern_pgdir,vAddress,(int)false);
		if(pPTe){
			if(*pPTe & PTE_P){
				cprintf("\t 0x%08x keeps ====> 0x%08x\n",
						(uintptr_t)vAddress,*vAddress);
				++vAddress;
				if (vAddress==0){
					break;
				}
				continue;
			}
		}
		cprintf("\t 0x%08x is not mapped yet\n",(uintptr_t)vAddress);
		++vAddress;
		vAddress = (uintptr_t*)ROUNDUP((uintptr_t)vAddress,PGSIZE);
		if (vAddress==0){
			break;
		}

	}
	cprintf("\n");
	return 0;
}

/*****************************************************************************/

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}

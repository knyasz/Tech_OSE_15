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
#include <kern/pmap.h>		// Lab2: Challenge

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
	uint32_t * ebp = (uint32_t*)read_ebp();// ebp typically carries the address
	cprintf("Stack backtrace:\n");
	while(ebp){
		uint32_t* eip = (uint32_t*)EIP(ebp);
		uint32_t* arg = (uint32_t*)ARG(ebp);
		cprintf(" ebp %08x eip %08x args %08x %08x %08x %08x %08x\n",
				ebp,*eip,arg[0],arg[1],arg[2],arg[3],arg[4]);
		struct Eipdebuginfo info;
		debuginfo_eip(*eip,&info);
		cprintf("	%s:%d: %.*s+%d\n",
				info.eip_file,info.eip_line,
				info.eip_fn_namelen,info.eip_fn_name,
				(*eip) - info.eip_fn_addr);
		ebp=(uint32_t*)*ebp;
	}
	return 0;
}

/******************************************************************************/

static int
isStringHexValid(char *string) {
	if (string == NULL)
		return 1;
	if (!(string[0] == '0' && (string[1] =='x' || string[1] == 'X')))
		return 1;
	int i;
	for (i=2;string[i]!='\0';++i) {
		if (string[i] >= '0' && string[i] <= '9')
			continue;
		else if (string[i] >= 'A' && string[i] <= 'F')
			continue;
		else if (string[i] >= 'a' && string[i] <= 'f')
			continue;
		else
			return 1;
	}

	return 0;
}

int printPageDirectoryByVirtualAddress(uint32_t address) {
	physaddr_t pde = (physaddr_t) kern_pgdir[PDX(address)];
	cprintf(
			"Virtual Address: 0x%08x, Page Directory Index %d,"
			" P=%01d, W=%01d, U=%01d\n",
			address, PDX(address),
			(pde & PTE_P)? 1 : 0,
			(pde & PTE_W)? 1 : 0,
			(pde & PTE_U)? 1 : 0);

	return 0;
}

int mon_showmappingsPD(int argc, char **argv, struct Trapframe *tf) {
	if (argc != 3){
		cprintf("You've entered %d arguments instead of 2\n", argc - 1);
		return 1;
	}
	uint32_t start = strtol(argv[1], NULL, 16);
	uint32_t end = strtol(argv[2], NULL, 16);
	for (; start <= end; start += NPTENTRIES * PGSIZE) {
		printPageDirectoryByVirtualAddress(start);
	}
	return 0;
}

int mon_showmappings(int argc, char **argv, struct Trapframe *tf) {
	if (argc != 3){
		cprintf("You've entered %d arguments instead of 2\n", argc - 1);
		return 1;
	}
	if (isStringHexValid(argv[1])!=0 || isStringHexValid(argv[2])) {
		cprintf("invalid hexa address %s or %s. \n",argv[1],argv[2]);
		return 1;
	}
	uint32_t start = strtol(argv[1], NULL, 16) & 0xfffff000;
	uint32_t end = strtol(argv[2], NULL, 16) & 0xfffff000;
	uint32_t iterator = start;
	cprintf("Show Mappings from: 0x%08x to:0x%08x\n", start, end);
	for (; iterator <= end; iterator += PGSIZE) {
		pte_t* pte = pgdir_walk(kern_pgdir, (const void*) iterator, 0);
		if(pte == NULL) {
			cprintf("0x%08x : NULL\n",iterator);
			continue;
		}
		cprintf("Vaddress: 0x%08x, Paddress: 0x%08x, P=%01d, W=%01d, U=%01d \n",
				iterator, PTE_ADDR(*pte),
				(*pte & PTE_P)? 1 : 0,
				(*pte & PTE_W)? 1 : 0,
				(*pte & PTE_U)? 1 : 0);
	}
	return 0;
}
int
mon_permissionsManage(int argc, char**argv, struct Trapframe *tf)
{
	if (argc != 4) {
		cprintf("You've entered %d arguments instead of 3\n", argc - 1);
		return 1;
	}
	if (isStringHexValid(argv[1])!=0) {
		cprintf("invalid hexa address %s. \n",argv[1]);
		return 1;
	}
	uintptr_t addr = strtol(argv[1], NULL, 16);
	uint32_t flag=0;
//	char sFlag[FLAG_LENGTH];
	char sFlag[4];
	strtolowercpy(sFlag,argv[3]);
	int i;
	for (i=0;i<NFLAGS;++i) {
		if (strcmp(flags[i].name,sFlag)==0) {
			flag=flags[i].value;
			break;
		}
	}
	if (i==NFLAGS) {
		cprintf("Flag %s is not supported yet.\n",argv[3]);
		return 1;
	}
	pte_t* pte = pgdir_walk(kern_pgdir,(void*)addr,(int)false);
	if (!pte) {
		cprintf("Page, which includes the address %s "
				"has not been mapped yet.\n",argv[1]);
		return 1;
	}
	if (strcmp(argv[2],"--set") == 0) {
		*pte |= flag;
	} else if (strcmp(argv[2],"--clear") == 0) {
		*pte &= ~flag;
	} else if (strcmp(argv[2],"--toggle") == 0) {
		*pte^=flag;
	} else {
		cprintf("The option %s is not supported yet.\n",argv[2]);
		return 1;
	}
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
	for(;vAddress<=toAddr;++vAddress){
		pte_t* pPTe = pgdir_walk(kern_pgdir,vAddress,(int)false);
		if(pPTe){
			if(*pPTe & PTE_P){
				cprintf("\t 0x%08x keeps ====> 0x%08x\n",(uintptr_t)vAddress,*vAddress);
				continue;
			}
		}
		cprintf("\t 0x%08x is not mapped yet\n",(uintptr_t)vAddress);
	}




























//	uint32_t *iter;
//	pte_t * pte;
//	pte = pgdir_walk(kern_pgdir,from,0);
//	for (iter = (uint32_t*)from; iter < (uint32_t*)MIN((ROUNDUP((char*)from,PGSIZE)),to); ++iter) {
//		if (pte && *pte) {
//			assert(*pte & PTE_P);
//			cprintf("  %08x",*iter);
//		} else {
//			cprintf("  UNMAPPED");
//		}
//	}
//	from = ROUNDUP((char*)from,PGSIZE);
//	for (; from <to;from+=PGSIZE) {
//		pte = pgdir_walk(kern_pgdir,from,0);
//		for (iter = (uint32_t*)from; iter < (uint32_t*)MIN((((char*)from)+PGSIZE),to); ++iter) {
//			if (pte && *pte) {
//				assert(*pte&PTE_P);
//				cprintf("  %08x",*iter);
//			} else {
//				cprintf("  UNMAPPED");
//			}
//		}
//	}
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


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}

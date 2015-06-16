/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>
#include <kern/time.h>

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.

	// LAB 3: Your code here.
	user_mem_assert(curenv, s, len, 0);
	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console without blocking.
// Returns the character, or 0 if there is no input waiting.
static int
sys_cgetc(void)
{
	return cons_getc();
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	/* to run lab4 tests*/
	if (e == curenv)
		cprintf("[%08x] exiting gracefully\n", curenv->env_id);
	else
		cprintf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	/**********************/
	env_destroy(e);
	return 0;
}

// Deschedule current environment and pick a different one to run.
static void
sys_yield(void)
{
	sched_yield();
}

// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
//	-E_NO_MEM on memory exhaustion.
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.

	// LAB 4: Your code here.
	int r;
	struct Env *e;
	if ((r=env_alloc(&e, thiscpu->cpu_env->env_id)) < 0) {
		cprintf("sys_exofork %e\n", r);
		return r;
	}
	e->env_tf = thiscpu->cpu_env->env_tf;
	e->env_tf.tf_regs.reg_eax = 0;
	e->env_status = ENV_NOT_RUNNABLE;

	return e->env_id;
}

// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
	// Hint: Use the 'envid2env' function from kern/env.c to translate an
	// envid to a struct Env.
	// You should set envid2env's third argument to 1, which will
	// check whether the current environment has permission to set
	// envid's status.

	// LAB 4: Your code here.
	struct Env *e;
	if (status != ENV_RUNNABLE
	      	&& status != ENV_NOT_RUNNABLE) 
			return -E_INVAL;

	if (envid2env(envid, &e, 1))
		return -E_BAD_ENV;

	e->env_status = status;
	return 0;
}

// Set envid's trap frame to 'tf'.
// tf is modified to make sure that user environments always run at code
// protection level 3 (CPL 3) with interrupts enabled.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	// LAB 5: Your code here.
	// Remember to check whether the user has supplied us with a good
	// address!
	// Are we able to access the given trapframe
	struct Env *e;
	int  r = envid2env(envid,&e,true);//only me or my father can change the env
	if(r<0){
		return -E_BAD_ENV;
	}
	r=user_mem_check(curenv, tf, sizeof(struct Trapframe), PTE_U);
	if(r<0){
		cprintf("kern/syscall.c:sys_env_set_trapframe\n");
		return -E_BAD_ENV;
	}
	//Maybe "=" could work too ?
	memmove(&(e->env_tf), tf, sizeof(struct Trapframe));
	return 0;
}

// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	struct Env *e;
	// LAB 4: Your code here.
	if (envid2env(envid, &e, 1) != 0)
		return -E_BAD_ENV;

	if (!func)
		return -E_INVAL;

	e->env_pgfault_upcall = func;
	
	return 0;
}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.  See PTE_SYSCALL in inc/mmu.h.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you
	//   allocated!

	// LAB 4: Your code here.
	struct Env *e;
	struct PageInfo *page;
	if ((uint32_t) va >= UTOP
	    || (uint32_t) va % PGSIZE!=0)
		return -E_INVAL;

	if ((perm & PTE_U) == 0 ||
	    (perm & PTE_P) == 0 ||
	    (perm & ~PTE_SYSCALL) != 0)
		return -E_INVAL;

	if (envid2env(envid, &e, 1) != 0)
		return -E_BAD_ENV;

	if ((page = page_alloc(ALLOC_ZERO)) == NULL)
		return -E_NO_MEM;

	if (page_insert(e->env_pgdir, page, va, perm) != 0 ) {
		page_free(page);
		return -E_NO_MEM;
	}
	return 0;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva,
	     envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.
	struct Env *se, *de;
	pte_t *septe;
	struct PageInfo *page;

	if (envid2env(srcenvid, &se, 1)
		|| envid2env(dstenvid, &de, 1))
		return -E_BAD_ENV;
	// case 1
	if ((uint32_t) srcva >= UTOP
		|| (uint32_t) dstva >= UTOP
	  	|| (uint32_t) srcva % PGSIZE != 0
	  	|| (uint32_t) dstva % PGSIZE != 0) {
		cprintf("sys_page_map: invalid boundary or page-aligned\n");
		return -E_INVAL;
	}
	// case 2
	if ( (perm & PTE_U) == 0
		|| (perm & PTE_P) == 0
		|| (perm & ~PTE_SYSCALL) != 0) {
		cprintf("sys_page_map: invalid perm\n");
		return -E_INVAL;
	}

	if (!(page = page_lookup(se->env_pgdir, srcva, &septe))) {
		cprintf("sys_page_map: page not found\n");
		return -E_INVAL;
	}

	if ((perm & PTE_W) != 0
		&& (*septe & PTE_W) == 0) {
		cprintf("sys_page_map: invalid PTE_W\n");
		return -E_INVAL;
	}
	
	if (page_insert(de->env_pgdir, page, dstva, perm))
		return -E_NO_MEM;

	return 0;
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().

	// LAB 4: Your code here.
	struct Env *e;
	if ((uint32_t) va >= UTOP
		|| (uint32_t) va % PGSIZE != 0)
		return -E_INVAL;
	if (envid2env(envid, &e, 1))
		return -E_BAD_ENV;

	page_remove(e->env_pgdir, va);
	return 0;
}

// Try to send 'value' to the target env 'envid'.
// If srcva < UTOP, then also send page currently mapped at 'srcva',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target is not blocked, waiting for an IPC.
//
// The send also can fail for the other reasons listed below.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again, returning 0
// from the paused sys_ipc_recv system call.  (Hint: does the
// sys_ipc_recv function ever actually return?)
//
// If the sender wants to send a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc only happens when no errors occur.
//
// Returns 0 on success, < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in the
//		current environment's address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	// LAB 4: Your code here.
  int r;
  struct Env * dstenv;
  pte_t * pte;
  struct PageInfo *pp;
  // Check parameters
  // Check envid and env status
  r = envid2env(envid, &dstenv, 0);
  if (r < 0) 
    return -E_BAD_ENV;
  if (!dstenv->env_ipc_recving)
    return -E_IPC_NOT_RECV;
  
  // Check srcva and perm
  if ((uintptr_t)srcva < UTOP) {
    if (PGOFF(srcva) != 0)
      return  -E_INVAL;

    if ((perm & (PTE_U | PTE_P)) != (PTE_U | PTE_P)) 
      return -E_INVAL;

    if (((perm | PTE_SYSCALL) != PTE_SYSCALL))
      return -E_INVAL;

    // Check physical page exist
    pp = page_lookup(curenv->env_pgdir, srcva, &pte);
    if ((uintptr_t)srcva < UTOP && !pp)
      return -E_INVAL;

    // Check perm write conflict
    if ((perm & PTE_W) && !(*pte & PTE_W))
      return -E_INVAL;

    // Send mapping 
    if (dstenv->env_ipc_dstva) {
      // Do page map
      r = page_insert(dstenv->env_pgdir, pp, dstenv->env_ipc_dstva, perm);
      if (r < 0)
        return -E_NO_MEM;
      // Make page perm
      dstenv->env_ipc_perm = perm;
    }
  }

  // If srcva >= UTOP, no mapping transfered and no errors.

  dstenv->env_ipc_recving = false;
  dstenv->env_ipc_value = value;
  dstenv->env_ipc_from = curenv->env_id;
  dstenv->env_status = ENV_RUNNABLE;

  return 0;
}

// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	// LAB 4: Your code here.
  // check dstva
  if ((uintptr_t)dstva < UTOP && (PGOFF(dstva) != 0))
    return -E_INVAL;

  // Record this env want to receive
  curenv->env_ipc_recving = true;
  curenv->env_ipc_dstva = dstva;
  
  // Block this env, and giveup CPU
  curenv->env_status = ENV_NOT_RUNNABLE;

	return 0;
}

// Return the current time.
static int
sys_time_msec(void)
{
	// LAB 6: Your code here.
	return time_msec();
//	panic("sys_time_msec not implemented");
}

// Dispatches to the correct kernel function, passing the arguments.
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.
  
	uint32_t ret = 0;
	switch (syscallno) {
		default :
			ret = -E_INVAL;
			break;

		case SYS_cputs :
			sys_cputs((char *)a1, (size_t)a2);
			break;

		case SYS_cgetc :
			ret = (uint32_t)sys_cgetc();
			break;

		case SYS_getenvid :
			ret = (uint32_t)sys_getenvid();
			break;

		case SYS_env_destroy :
			ret = (uint32_t)sys_env_destroy((envid_t)a1);
			break;

		case SYS_yield :
			sys_yield();
			break;

		case SYS_page_alloc :
			ret = (uint32_t)sys_page_alloc((envid_t)a1, (void *)a2, (int)a3);
			break;

		case SYS_page_map :
		ret = (uint32_t)sys_page_map((envid_t)a1, (void *)a2,
								(envid_t)a3, (void *)a4, (int)a5);
		break;

		case SYS_page_unmap :
		ret = (uint32_t)sys_page_unmap((envid_t)a1, (void *)a2);
		break;

		case SYS_exofork :
			ret = (uint32_t)sys_exofork();
			break;

		case SYS_env_set_status :
			ret = (uint32_t)sys_env_set_status((envid_t)a1, (int)a2);
			break;

		case SYS_env_set_pgfault_upcall :
			ret = (uint32_t)sys_env_set_pgfault_upcall((envid_t)a1, (void*)a2);
			break;

		case SYS_ipc_try_send :
			ret = (uint32_t)sys_ipc_try_send((envid_t)a1,
												(uint32_t)a2,
												(void *)a3,
												(unsigned)a4);
			break;

		case SYS_ipc_recv :
			ret = (uint32_t)sys_ipc_recv((void *)a1);
			break;
		case SYS_env_set_trapframe :
			ret = (uint32_t)sys_env_set_trapframe((envid_t)a1,(struct Trapframe *) a2);
			break;

		case SYS_time_msec :
			ret = (uint32_t)sys_time_msec();
			break;

//		case SYS_net_try_send :
//			ret = (uint32_t)sys_net_try_send(char* a1, uint32_t a2);

	}

	//panic("syscall not implemented");
  return ret;

}


#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *create_pcb, void (*entry)(void *), void *arg)
{
  Area stack = {create_pcb->stack, create_pcb->stack + STACK_SIZE};
  create_pcb->cp = kcontext(stack, entry, arg);
}
char *hello_arg_test1="kcontext 1 !!!";
char *hello_arg_test2="kcontext 2 !!!";
extern void naive_uload(PCB *pcb, const char *filename);
extern void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
static char *args_menu[] = {"/bin/menu", "sss","ok",NULL};
void init_proc() {
   context_uload(&pcb[0], "/bin/hello", args_menu,NULL);

  context_kload(&pcb[1], hello_fun,NULL);

  switch_boot_pcb();
  Log("Initializing processes...");
}

Context *schedule(Context *prev)
{
  /* 操作系统中以PCB为单位 到了中断时是以context为单位的 */
  // save the context pointer
current->cp = prev;
// always select pcb[0] as the new process
current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
// then return the new context
return current->cp;
}
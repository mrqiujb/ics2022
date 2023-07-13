#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename)
{
  Log("loader:%s",filename);
  Elf_Ehdr elf;
  int fd = fs_open(filename);
  assert(fd != -1);
  fs_lseek(fd, 0, SEEK_SET);
  fs_read(fd, &elf, sizeof(Elf_Ehdr));
  Elf_Phdr Phdr;
  assert(*(uint32_t *)elf.e_ident == 0x464c457f);
  for (int i = 0; i < elf.e_phnum; i++)
  {
    fs_lseek(fd, elf.e_phoff + i * elf.e_phentsize, SEEK_SET);
    fs_read(fd, &Phdr, sizeof(Phdr));
    if (Phdr.p_type == 1)
    {
      fs_lseek(fd, Phdr.p_offset, SEEK_SET);
      fs_read(fd, (void *)Phdr.p_vaddr, Phdr.p_filesz);
      for (unsigned int i = Phdr.p_filesz; i < Phdr.p_memsz; i++)
      {
        ((char *)Phdr.p_vaddr)[i] = 0;
      }
    }
  }
  return elf.e_entry;
  /* 加载第一个文件 */
  /*Elf_Ehdr ehdr;
  ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  assert((*(uint32_t *)ehdr.e_ident == 0x464c457f));

  Elf_Phdr phdr[ehdr.e_phnum];
  ramdisk_read(phdr, ehdr.e_phoff, sizeof(Elf_Phdr)*ehdr.e_phnum);
  for (int i = 0; i < ehdr.e_phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      ramdisk_read((void*)phdr[i].p_vaddr, phdr[i].p_offset, phdr[i].p_memsz);
      memset((void*)(phdr[i].p_vaddr+phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
    }
  }
  return ehdr.e_entry;*/
}

void naive_uload(PCB *pcb, const char *filename)
{
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[])
{
  uintptr_t entry = loader(pcb, filename);
  pcb->cp = ucontext(&pcb->as, heap, (void *)entry);
  pcb->cp->GPRx = (uintptr_t)heap.end;
  Log("entry:%p gprx:%p cp:%p",entry,pcb->cp->GPRx,pcb->cp);
}

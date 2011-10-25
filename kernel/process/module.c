#include <string.h>
#include "module.h"
#include "elf.h"

/* The module descriptors. */
static ModuleInfo_t modules[MODULES_MAX];

/* The number of modules in the system. */
static unsigned int modules_count;

/* Parses the module's ELF. */
static int ParseModule (ModuleInfo_t *mod);

/* Returns the stripped name of the module. Returns the index of the first character. */
static char *ParseName (char *mbname);

int ModuleInit (module_t *mods, unsigned int mods_count)
{
  /* make sure our list is valid */
  if (mods_count && !mods) {
    return 0;
  }
  /* parse the list */
  modules_count = mods_count;
  for (unsigned int m = 0; m < modules_count; m++) {
    modules[m].name = ParseName((char *)mods[m].string);
    modules[m].module_start = mods[m].mod_start;
    modules[m].module_end = mods[m].mod_end;
  }
  /* success! */
  return 1;
}

ModuleInfo_t *ModuleFind (char *name)
{
  /* make sure we have a valid name */
  if (!name) {
    return 0;
  }
  for (unsigned int m = 0; m < modules_count; m++) {
    ModuleInfo_t *mod = &modules[m];
    if (strcmp(name, mod->name) == 0) {
      return ParseModule(mod) ? mod : 0;
    }
  }
  return 0;
}

static int ParseModule (ModuleInfo_t *mod)
{
  /* make sure we're trying to parse a valid module */
  if (!mod) {
    return 0;
  }
  /* check if we've already parsed */
  if (mod->code_start) {
    return 1;
  }

  Elf32_Ehdr *eh = (Elf32_Ehdr *)mod->module_start;

  /* make sure we can use the elf */
  if (eh->e_ident[EI_MAG0] != ELFMAG0 ||
      eh->e_ident[EI_MAG1] != ELFMAG1 ||
      eh->e_ident[EI_MAG2] != ELFMAG2 ||
      eh->e_ident[EI_MAG3] != ELFMAG3 ||
      eh->e_ident[EI_CLASS] != ELFCLASS32 ||
      eh->e_ident[EI_DATA] != ELFDATA2LSB ||
      eh->e_ident[EI_VERSION] != EV_CURRENT ||
      eh->e_version != EV_CURRENT ||
      eh->e_type != ET_EXEC ||
      eh->e_machine != EM_386) {
    return 0;
  }

  /* note the entry point */
  mod->code_entry = eh->e_entry;

  /* parse the program headers */
  for (int h = 0; h < eh->e_phnum; h++) {
    Elf32_Phdr *ph = (Elf32_Phdr *)(mod->module_start + eh->e_phoff + h*eh->e_phentsize);
    if (ph->p_type != PT_LOAD) {
      continue;
    }
    /* check for data segment */
    if ((ph->p_flags & PF_R) && (ph->p_flags & PF_W)) {
      mod->data_start = mod->module_start + ph->p_offset;
      mod->data_init_size = ph->p_filesz;
      mod->data_size = ph->p_memsz;
    }
    /* check for executable segment */
    else if ((ph->p_flags & PF_R) && (ph->p_flags & PF_X)) {
      mod->code_start = mod->module_start + ph->p_offset;
      mod->code_size = ph->p_filesz;
    }
  }
  
  return 1;
}

static char *ParseName (char *name)
{
  /* make sure the name is valid */
  if (!name) {
    return 0;
  }
  /* find the last slash */
  char *slash_pos = 0;
  while (*name) {
    if (*name == '/') {
      slash_pos = name + 1; /* strip out the slash */
    }
    name++;
  }
  return slash_pos;
}


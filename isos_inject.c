#include "argp_parser.c"

#include "isos_inject.h"
#include <argp.h>
#include <bfd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define ARCHITECTURE_SIZE 64

/**
 * Open the binary and check that it is an ELF, executable of architecture
 * 64-bit
 */
void check_binary_with_libbfd(struct arguments *arguments) {
  /* Initializing BFD library*/
  bfd_init();

  /* Opening binary with libbfd */
  bfd *bfd_file = bfd_openr(arguments->elf_file_to_analyze, NULL);
  if (bfd_file == NULL) {
    errx(EXIT_FAILURE, "Error while opening the file %s with libbfd",
         arguments->elf_file_to_analyze);
  }

  /* Check that the binary is an ELF, executable of architecture 64-bit */
  if (bfd_check_format(bfd_file, bfd_object) &&
      ((bfd_get_file_flags(bfd_file) & (unsigned int)EXEC_P) != 0) &&
      bfd_get_arch_size(bfd_file) == ARCHITECTURE_SIZE) {

    fprintf(stdout,
            "Success: %s is an excutable ELF of architecture %d bit. \n",
            arguments->elf_file_to_analyze, ARCHITECTURE_SIZE);
  } else {

    bfd_close(bfd_file); /* Close binary */
    errx(EXIT_FAILURE,
         "Error: %s must be an excutable ELF of architecture %d bit. \n",
         arguments->elf_file_to_analyze, ARCHITECTURE_SIZE);
  }

  /* Close binary */
  bfd_close(bfd_file);
}

static struct argp argp = {.options = options,
                           .parser = parse_opt,
                           .args_doc = NULL,
                           .doc = doc,
                           .help_filter = NULL};

int main(int argc, char **argv) {

  struct arguments arguments;
  arguments.argCount = 0;
  arguments.modify_entry_function_address = false;

  /* Parse the arguments */
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  /* Check that it is an ELF, executable of architecture 64-bit */
  check_binary_with_libbfd(&arguments);

  /* Open the file */
  int fd = open(arguments.elf_file_to_analyze, O_RDONLY);
  if (fd == -1) {
    errx(EXIT_FAILURE, "Error while opening the file %s .",
         arguments.elf_file_to_analyze);
  }

  /* To get informations on the binary like his size */
  struct stat fstat_structure;
  if (fstat(fd, &fstat_structure) == -1) {
    close(fd);
    perror("fstat");
    exit(EXIT_FAILURE);
  }

  /* Mapping between a process address space and the binary. Binary can now be
   * accessed like an array. */
  char *addr = mmap(0, fstat_structure.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    close(fd);
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  /* Initializing executable header structure */
  Elf64_Ehdr executable_header;
  memcpy(&executable_header, addr, sizeof(Elf64_Ehdr));
  fprintf(stdout, "Number of program headers: %d. \n",
          executable_header.e_phnum);

  int index_pt_note = -1; /* first program header of type PT_NOTE index */
  for (int i = 0; i < executable_header.e_phnum; i++) {

    /* Initializing of program header number i */
    Elf64_Phdr program_header;
    memcpy(&program_header,
           addr + executable_header.e_phoff + i * sizeof(Elf64_Phdr),
           sizeof(Elf64_Phdr));

    if (program_header.p_type == PT_NOTE) {
      index_pt_note = i;
      break;
    }
  }

  if (index_pt_note == -1) {
    close(fd);
    errx(EXIT_FAILURE, "Any program header of type PT_NOTE found in %s\n",
         arguments.elf_file_to_analyze);

  } else {
    fprintf(stdout, "Index of first program header of type PT_NOTE: %d\n",
            index_pt_note);
  }

  close(fd);

  return EXIT_SUCCESS;
}
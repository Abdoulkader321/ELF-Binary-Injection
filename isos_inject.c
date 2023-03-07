#include "argp_parser.c"
#include <argp.h>
#include <bfd.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>

#define ARCHITECTURE_SIZE 64

static struct argp argp = {.options = options,
                           .parser = parse_opt,
                           .args_doc = NULL,
                           .doc = doc,
                           .help_filter = NULL};

int main(int argc, char **argv) {

  struct arguments arguments;
  arguments.argCount = 0;
  arguments.modify_entry_function_address = false;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  /* Initializing BFD library*/
  bfd_init();

  /* Opening binary with libbfd */
  bfd *bfd_file = bfd_openr(arguments.elf_file_to_analyze, NULL);
  if (bfd_file == NULL) {
    fprintf(stderr, "Error while opening the file %s with libbfd",
            arguments.elf_file_to_analyze);
    exit(EXIT_FAILURE);
  }

  /* Check binary format and architecture */
  if (bfd_check_format(bfd_file, bfd_object) &&
      bfd_get_arch_size(bfd_file) == ARCHITECTURE_SIZE) {

    fprintf(stdout, "%s is of ELF format and of architecture %d bit. \n",
            arguments.elf_file_to_analyze, ARCHITECTURE_SIZE);
  } else {
    fprintf(stderr, "%s must be of ELF format and of architecture %d bit. \n",
            arguments.elf_file_to_analyze, ARCHITECTURE_SIZE);
    exit(EXIT_FAILURE);
  }

  /* Close binary*/
  bfd_close(bfd_file);

  return EXIT_SUCCESS;
}
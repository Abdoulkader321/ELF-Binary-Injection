#include <argp.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define EXPECTED_NUMBER_OF_ARGS 4 /* The last boolean argument is optionnal*/

const char *argp_program_version = "isos_inject 1.0";

char doc[] =
    "Example: --path-to-elf=./date --path-to-code=./binary_to_inject "
    "--new-section-name='abcd' --base-address='123' --modify-entry-function";

struct argp_option options[] = {
    {"path-to-elf", 'e', "PATH", 0, "Path to ELF file to analyze", 0},
    {"path-to-code", 'c', "PATH", 0,
     "Path to binary file that contains the machine code to inject", 0},
    {"new-section-name", 'n', "NAME", 0, "Name of the newly created section",
     0},
    {"base-address", 'a', "ADDRESS", 0, "Base address of the injected code", 0},
    {"modify-entry-function", 'm', 0, 0,
     "Modify the address of the entry function(default: False)", 0},
    {0, 0, 0, 0, 0, 0}};

/* This structure is used by main to communicate with parse_opt. */
struct arguments {
  char *elf_file_to_analyze; /* Argument for --path-to-elf */
  char *code_to_inject;      /* Argument for --path-to-code */
  char *new_section_name;
  unsigned int injected_code_base_address; /* Argument for --base-address */
  bool modify_entry_function_address;      /* Argument for
                                                  --modify-entry-function */
  int argCount; /* To count the number of provided arguments */
};

/* Parser */
error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  char *endPtr;

  switch (key) {

  case 'e':
    arguments->elf_file_to_analyze = arg;
    arguments->argCount++;
    break;

  case 'c':
    arguments->code_to_inject = arg;
    arguments->argCount++;

    break;

  case 'n':
    arguments->new_section_name = arg;
    arguments->argCount++;
    break;

  case 'a':

    arguments->injected_code_base_address = strtol(arg, &endPtr, 10);
    arguments->argCount++;

    if (strcmp(endPtr, "")) {
      errx(EXIT_FAILURE, "A valid base-address must be provided");
    }

    break;

  case 'm':
    arguments->modify_entry_function_address = true;
    break;

  case ARGP_KEY_END:
    if (arguments->argCount != EXPECTED_NUMBER_OF_ARGS) {
      errx(EXIT_FAILURE, "Error: All the required arguments must be "
                         "provided.\nTry ./isos_inject --help");
    }
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

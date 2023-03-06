#include "argp_parser.c"
#include <argp.h>
#include <stdbool.h>
#include <stdlib.h>

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

  return EXIT_SUCCESS;
}
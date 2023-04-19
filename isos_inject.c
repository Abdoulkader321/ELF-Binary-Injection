#include "argp_parser.c"

#include "isos_inject.h"
#include <argp.h>
#include <bfd.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define ARCHITECTURE_SIZE 64
#define ALIGNMENT 4096
#define ADDR_ALIGN 16
#define SIZE_OF_NOTE_ABI_TAG 13
#define HIJACK_ADDRESS_GOT_ENTRY 0x10038 /* Localtime is overwrited */

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

int get_index_of_pt_note_program_header(int fd) {
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

  return index_pt_note;
}

/**
 * Sort section headers regarding to their adress.
 * Return True if it has been sorted, else False(it was already sorted)
 */
bool sort_section_headers(Elf64_Shdr *section_headers,
                          int section_headers_number,
                          int *index_of_note_abi_tag) {

  if ((section_headers[*index_of_note_abi_tag - 1].sh_addr <
       section_headers[*index_of_note_abi_tag].sh_addr) &&
      (section_headers[*index_of_note_abi_tag].sh_addr <
       section_headers[*index_of_note_abi_tag + 1].sh_addr)) {
    /* Already sorted, no need to move */
    return false;
  }

  if (section_headers[*index_of_note_abi_tag].sh_addr <
      section_headers[*index_of_note_abi_tag - 1].sh_addr) {

    // Move to left

    /* Search the index where to move it*/
    int i;
    for (i = 0; i < section_headers_number; i++) {

      if (section_headers[i].sh_addr >
          section_headers[*index_of_note_abi_tag].sh_addr) {
        break;
      }
    }

    Elf64_Shdr tmp = section_headers[*index_of_note_abi_tag];

    /* Shift to left all other section headers */
    for (int k = *index_of_note_abi_tag; k > i; k--) {
      section_headers[k] = section_headers[k - 1];
    }

    section_headers[i] = tmp;
    *index_of_note_abi_tag = i;
  } else {
    // Move to right

    /* Search the index where to move it, do not touch the 2 last section
     * headers*/
    int i;
    for (i = 0; i < section_headers_number - 2; i++) {

      if (section_headers[i].sh_addr >
          section_headers[*index_of_note_abi_tag].sh_addr) {
        break;
      }
    }

    Elf64_Shdr tmp = section_headers[*index_of_note_abi_tag];

    /* Shift to right all other section headers */
    int k;
    for (k = *index_of_note_abi_tag; k < i - 1; k++) {
      section_headers[k] = section_headers[k + 1];
    }

    section_headers[k] = tmp;

    /* Update the sh_link field to avoir readelf warnings */
    for (i = 0; i < section_headers_number; i++) {
      if (section_headers[i].sh_link != 0) {
        section_headers[i].sh_link--;
      }
    }

    *index_of_note_abi_tag = k;
  }

  return true;
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
  int fd = open(arguments.elf_file_to_analyze, O_RDWR);
  if (fd == -1) {
    errx(EXIT_FAILURE, "Error while opening the file %s .",
         arguments.elf_file_to_analyze);
  }

  /*--   CHALLENGE 2   --*/

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

  /*--   CHALLENGE3   --*/

  int inject_file_fd = open(arguments.code_to_inject, O_RDONLY);
  if (inject_file_fd == -1) {
    errx(EXIT_FAILURE, "Error while opening the file %s .",
         arguments.code_to_inject);
  }

  /* To get informations on the inject_code like his size */
  struct stat fstat_inject;
  if (fstat(inject_file_fd, &fstat_inject) == -1) {
    close(fd);
    perror("fstat");
    exit(EXIT_FAILURE);
  }

  int end_position_elf = lseek(fd, 0, SEEK_END); /* At the end of the binary */

  /* Buffer containing the injection code bytes */
  char *buffer = malloc(fstat_inject.st_size * sizeof(char));

  /* Code injection at the end of the binary */
  read(inject_file_fd, buffer, fstat_inject.st_size);
  write(fd, buffer, fstat_inject.st_size);
  free(buffer);

  /* Computing base address so that the difference with the offset become
    zero modulo 4096.*/
  arguments.injected_code_base_address +=
      (end_position_elf - arguments.injected_code_base_address) % ALIGNMENT;

  printf("Computed base address: %d\n", arguments.injected_code_base_address);

  /*--   CHALLENGE4   --*/

  Elf64_Shdr *section_headers =
      (Elf64_Shdr *)malloc(executable_header.e_shnum * sizeof(Elf64_Shdr));
  if (section_headers == NULL) {
    errx(EXIT_FAILURE, "Error while calling malloc ");
  }

  memcpy(section_headers, addr + executable_header.e_shoff,
         executable_header.e_shnum * sizeof(Elf64_Shdr));

  int index_of_shstrtab = executable_header.e_shstrndx;
  printf("Index of section header '.shstrtab' %d \n", index_of_shstrtab);

  int index_of_note_abi_tag = -1;

  for (int i = 0; i < executable_header.e_shnum; i++) {
    char *sh_name =
        (char *)(addr + section_headers[index_of_shstrtab].sh_offset +
                 section_headers[i].sh_name); /* Section header name */

    if (strcmp(".note.ABI-tag", sh_name) == 0) {
      printf("Index of section header '.note.ABI-tag' %d \n", i);
      index_of_note_abi_tag = i;

      section_headers[i].sh_type = SHT_PROGBITS;
      section_headers[i].sh_addr = arguments.injected_code_base_address;
      section_headers[i].sh_offset = end_position_elf;
      section_headers[i].sh_size = fstat_inject.st_size;
      section_headers[i].sh_addralign = ADDR_ALIGN;
      section_headers[i].sh_flags |= SHF_EXECINSTR;
      
      break;
    }
  }

  if (index_of_note_abi_tag == -1) {
    /* Section header called '.note.ABI-tag' not found */

    free(section_headers);
    close(fd);
    close(inject_file_fd);

    errx(EXIT_FAILURE, " '.note.ABI-tag' not found in section headers\n");
  }

  lseek(fd,
        executable_header.e_shoff +
            (index_of_note_abi_tag * sizeof(Elf64_Shdr)),
        SEEK_SET);
  write(fd, &section_headers[index_of_note_abi_tag], sizeof(Elf64_Shdr));

  /* Challenge 5 */

  /* Reorder section headers by section address if it's not ordered*/
  if (sort_section_headers(section_headers, executable_header.e_shnum,
                           &index_of_note_abi_tag)) {

    lseek(fd, executable_header.e_shoff, SEEK_SET);
    write(fd, section_headers, executable_header.e_shnum * sizeof(Elf64_Shdr));
  }

  /* Set the name of the injected section*/
  lseek(fd,
        section_headers[index_of_shstrtab].sh_offset +
            section_headers[index_of_note_abi_tag].sh_name,
        SEEK_SET);

  arguments.new_section_name[strlen(arguments.new_section_name)] = '\0';
  write(fd, arguments.new_section_name, SIZE_OF_NOTE_ABI_TAG);

  /* Challenge 6 */

  Elf64_Phdr pt_note_ph;
  memcpy(&pt_note_ph,
         addr + executable_header.e_phoff + index_pt_note * sizeof(Elf64_Phdr),
         sizeof(Elf64_Phdr));

  pt_note_ph.p_type = PT_LOAD;
  pt_note_ph.p_flags |= PF_X;
  pt_note_ph.p_align = 0x1000;

  pt_note_ph.p_offset = section_headers[index_of_note_abi_tag].sh_offset;
  pt_note_ph.p_paddr = section_headers[index_of_note_abi_tag].sh_addr;
  pt_note_ph.p_vaddr = section_headers[index_of_note_abi_tag].sh_addr;
  pt_note_ph.p_filesz = fstat_inject.st_size;
  pt_note_ph.p_memsz = section_headers[index_of_note_abi_tag].sh_size;

  lseek(fd, executable_header.e_phoff + index_pt_note * sizeof(Elf64_Phdr),
        SEEK_SET);

  write(fd, &pt_note_ph, sizeof(Elf64_Phdr));

  /* Challenge 7 */

  if (arguments.modify_entry_function_address) {

    printf("-- Entry point changed -- \n");
    executable_header.e_entry = arguments.injected_code_base_address;
    lseek(fd, 0, SEEK_SET);
    write(fd, &executable_header, sizeof(Elf64_Ehdr));

  } else {

    printf("-- .got.plt overwrited -- \n");
    lseek(fd, HIJACK_ADDRESS_GOT_ENTRY, SEEK_SET);
    write(fd, &arguments.injected_code_base_address,
          sizeof(arguments.injected_code_base_address));
  }

  free(section_headers);

  close(fd);
  close(inject_file_fd);

  return EXIT_SUCCESS;
}
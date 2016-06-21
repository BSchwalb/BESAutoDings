#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include "headerstuff.h"
#include "rlutil.h"


char* prog_name = NULL;
int display = -1;


void cleanup();
void handle_sigint(int sig);
void print_help();



int main(int argc, char* argv[]) {
  (void)signal(SIGINT, handle_sigint);
  char incoming[PIPE_BUF] = "";
  char c;
    
  prog_name = (char*)malloc((strlen(argv[0]) + 1) * sizeof(char));
  strcpy(prog_name, argv[0]);
    
  while ((c = getopt(argc, argv, "h")) != -1) {
    switch (c) {
      case 'h':
        print_help(argv[0]);
        cleanup();
        return EXIT_SUCCESS;
        break;
      default:
        fprintf(stderr, "Error: %s No correct input was made.", argv[0]);
        clear_eol();
        cleanup();
        resetColor();
        return EXIT_FAILURE;
    }
  }
    
  display = open(PIPE_DISPLAY, O_RDONLY);
  if (display == -1) {
    fprintf(stderr, "Error %s: Can't open fifo. Is the gridserver running?", prog_name);
    clear_eol();
    cleanup();
    resetColor();
    return EXIT_FAILURE;
  }
  printf("\nWelcome!\n");
    
    
  while (1) {
    int status = read(display, incoming, PIPE_BUF);
    if (status > 0) {
      printf("%s", incoming);
    }
      
    else if (status == -1) {
      fprintf(stderr, "Error %s: Can't read from the fifo", prog_name);
      clear_eol();
      cleanup();
      resetColor();
      return EXIT_FAILURE;
    }
  }
  cleanup();
  return EXIT_SUCCESS;
}


void cleanup() {
  clear_eol();
  printf("Info %s: Exiting...", prog_name);
  clear_eol();
  printf("Info %s: Closing the fifo", prog_name);
  clear_eol();
  close(display);
  printf("Info %s: Freeing memory", prog_name);
  clear_eol();
  free(prog_name);
}


void handle_sigint(int sig) {
  printf("\n");
  cleanup();
  exit(sig);
}


void print_help() {
  printf("Usage %s:", prog_name);
  clear_eol();
  printf("1) Start the gridserver.");
  clear_eol();
  printf("2) Start the griddisplay.");
  clear_eol();
}

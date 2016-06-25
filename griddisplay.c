#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include "headerstuff.h"


char* prog_name = NULL;
int display = -1;


void cleanup();
void sigIntHandler(int sig);


int main(int argc, char* argv[]) {
  (void)signal(SIGINT, sigIntHandler);
  char incoming[PIPE_BUF] = "";
  //char c;
    
  prog_name = (char*)malloc((strlen(argv[0]) + 1) * sizeof(char));
  strcpy(prog_name, argv[0]);
        
  display = open(PIPE_DISPLAY, O_RDONLY);
  if (display == -1) {
    fprintf(stderr, "Error %s: Gridserver not running?", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
  printf("\n GRIDDISPLAY !\n");
    
    
  while (1) {
    int status = read(display, incoming, PIPE_BUF);
    if (status > 0) {
      printf("%s", incoming);
    }
      
    else if (status == -1) {
      fprintf(stderr, "Error %s: Can't read from the fifo", prog_name);
      clear_eol();
      cleanup();
      return EXIT_FAILURE;
    }
  }
  cleanup();
  return EXIT_SUCCESS;
}


void cleanup() {
  clear_eol();
  printf("Info %s: Dying...", prog_name);
  clear_eol();
  close(display);
  free(prog_name);
}


void sigIntHandler(int sig) {
  printf("\n");
  cleanup();
  exit(sig);
}

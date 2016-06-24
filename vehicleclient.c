#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include "headerstuff.h"

char* prog_name = NULL;
char client_id = 0;
int msgid = -1;

void sigIntHandler(int sig);
void sigTermHandler(int sig);
void cleanup();
char* dirToStr(char id);

int main(int argc, char const* argv[]) {
    
  (void)signal(SIGINT, sigIntHandler);
  (void)signal(SIGTERM, sigTermHandler);
    
  char command = '0';
  long my_pid = (long)getpid();
  navigation msg;
  position init_pos;
    
  prog_name = (char*)malloc((strlen(argv[0]) + 1) * sizeof(char));
  strcpy(prog_name, argv[0]);

    // invalid # of arguments
  if (argc != 2) {
    fprintf(stderr, "Usage: %s A-Z", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  client_id = toupper(argv[1][0]);
    
    // not a letter
  if (!isalpha(client_id)) {
      
    fprintf(stderr, "Error: %s No. Don't.", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  msg.msgRecip = SERVER;
  msg.msgSender = my_pid;
    
  msg.clientId = client_id;
  msg.command = 'i';
    
  msgid = msgget(KEY, PERM);
    
  if (msgid == -1) {
    fprintf(stderr, "Error %s: Message Queue ded.", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
      
    fprintf(stderr, "Error %s: Initializing failed", prog_name);
    return EXIT_FAILURE;
  }
    
  if (msgrcv(msgid, &init_pos, sizeof(init_pos), my_pid, 0) == -1) {
      
    fprintf(stderr, "Error %s: No initial position.",
            prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  if (init_pos.status == REG_FULL) {
    fprintf(stderr, "Error %s: Grid full.", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  } else if (init_pos.status == REG_DOUBLE) {
    fprintf(stderr, "%s: The ID %c already exists", prog_name, client_id);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
    
  printf("ID: %c\n", client_id);
  printf("Pos (x/y): %d/%d", init_pos.x, init_pos.y);
    
    
  while (true) {

    printf(" Commands: N/E/S/W, T (Terminate) \n");
      
    scanf(" %c", &command);
    command = toupper(command);
    
    if (command == 'N' || command == 'E' || command == 'S' || command == 'W' || command == 'T') {
      msg.command = command;
        
      if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
        fprintf(stderr, "Error %s: command not sent", prog_name);
        clear_eol();
        cleanup();
        return EXIT_FAILURE;
      }
        
      if (command != 'T') {
          printf("%c navigated %s\n", msg.clientId, dirToStr(msg.command));
      }
    } else {
      printf("No such command.\n");
    }
  }
  cleanup();
  return EXIT_SUCCESS;
}


void cleanup() {
  clear_eol();
  printf("Info %s: Dying...", prog_name);
  clear_eol();
  free(prog_name);
}

void sigIntHandler(int sig) {
  printf("\nInfo %s: SIGINT received", prog_name);
  clear_eol();
    
  if (msgid != -1) {
      
    printf("Info %s: Server ded", prog_name);
    clear_eol();
    navigation msg;
    msg.msgRecip = SERVER;
    msg.msgSender = (long)getpid();
    msg.clientId = client_id;
    msg.command = 'T';
      
    if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
      fprintf(stderr, "Error %s: command not sent", prog_name);
      clear_eol();
      exit(EXIT_FAILURE);
    }
  } else {
    cleanup();
    exit(sig);
  }
}


void sigTermHandler(int sig) {
  printf("\nInfo %s: SIGTERM received", prog_name);
  clear_eol();
  printf("Info %s: %c ded.", prog_name, client_id);
  clear_eol();
  cleanup();
  exit(sig);
}


char* dirToStr(char id) {
  switch (id) {
    case 'N':
      return "north";
      break;
    case 'E':
      return "east";
      break;
    case 'S':
      return "south";
      break;
    case 'W':
      return "west";
      break;
    default:
      return "somewhere";
      break;
  }
}

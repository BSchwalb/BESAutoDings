#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include "headerstuff.h"

char* prog_name = NULL;
char client_id = 0;
int msgid = -1;

void handle_sigint(int sig);
void handle_sigterm(int sig);
void cleanup();
char* dirIDtoStr(char id);

int main(int argc, char const* argv[]) {
    
  (void)signal(SIGINT, handle_sigint);
  (void)signal(SIGTERM, handle_sigterm);
    
  char command = '0';
  long my_pid = (long)getpid();
  navigation msg;
  position init_pos;
    
  prog_name = (char*)malloc((strlen(argv[0]) + 1) * sizeof(char));
  strcpy(prog_name, argv[0]);

    // invalid # of arguments
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <ID CHAR>", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  client_id = toupper(argv[1][0]);
    
    // not a letter
  if (!isalpha(client_id)) {
      
    fprintf(stderr, "Error: %s You have to use a letter as ID", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  msg.msg_to = SERVER;
  msg.msg_from = my_pid;
    
  msg.client_id = client_id;
  msg.command = 'i';
    
  msgid = msgget(KEY, PERM);
    
  if (msgid == -1) {
    fprintf(stderr, "Error %s: Can't access message queue", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
      
    fprintf(stderr, "Error %s: Can't send discovery message", prog_name);
    return EXIT_FAILURE;
  }
    
  if (msgrcv(msgid, &init_pos, sizeof(init_pos), my_pid, 0) == -1) {
      
    fprintf(stderr, "Error %s: Can't receive initial position from message queue",
            prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  if (init_pos.status == REG_FULL) {
    fprintf(stderr, "Error %s: The grid is full.", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  else if (init_pos.status == REG_DOUBLE) {
      
    fprintf(stderr, "%s: The ID %c is allready in use.", prog_name, client_id);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
    
  printf("ID: %c\n", client_id);
  printf("Pos (x/y): %d/%d", init_pos.x, init_pos.y);
    
    
  while (true) {

    printf(" Commands:\n");
    printf("- N North\n");
    printf("- E East\n");
    printf("- S South\n");
    printf("- W West\n");
    printf("- T Terminate\n");

      
    scanf(" %c", &command);
    command = toupper(command);
    if (command == 'N' || command == 'E' || command == 'S' || command == 'W' || command == 'T') {
        
      msg.command = command;
        
      if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
        fprintf(stderr, "Error %s: Can't send command", prog_name);
        clear_eol();
        cleanup();
        return EXIT_FAILURE;
      }
        
      if (command != 'T') {
          printf("%c navigated %s\n", msg.client_id, dirIDtoStr(msg.command));
      }
    } else {
      printf("Command not found.\n");
    }
  }
  cleanup();
  return EXIT_SUCCESS;
}


void cleanup() {
  clear_eol();
  printf("Info %s: Exiting...", prog_name);
  clear_eol();
  printf("Info %s: Freeing memory", prog_name);
  clear_eol();
  free(prog_name);
}

void handle_sigint(int sig) {
  printf("\nInfo %s: SIGINT received", prog_name);
  clear_eol();
    
  if (msgid != -1) {
      
    printf("Info %s: Sending terminate command to server", prog_name);
    clear_eol();
    navigation msg;
    msg.msg_to = SERVER;
    msg.msg_from = (long)getpid();
    msg.client_id = client_id;
    msg.command = 'T';
      
    if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
      fprintf(stderr, "Error %s: Can't send command", prog_name);
      clear_eol();
      exit(EXIT_FAILURE);
    }
  } else {
    cleanup();
    exit(sig);
  }
}


void handle_sigterm(int sig) {
  printf("\nInfo %s: SIGTERM received", prog_name);
  clear_eol();
  printf("Info %s: Vehicle %c has been eliminated.", prog_name, client_id);
  clear_eol();
  cleanup();
  exit(sig);
}


char* dirIDtoStr(char id) {
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

#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "headerstuff.h"
#include <stdio.h>


char* grid = NULL;
char* prog_name = NULL;

int msgid = -1;
long clients[26] = {};
int display = -1;


void sigHandler();

void cleanup();
bool isOnBoard(char id, long clients[]);
int dirCheck(char dir, int width);
int move(char id, char dir, char grid[], int width, int size);

int main(int argc, char* argv[]) {
    
  (void)signal(SIGINT, sigHandler);
  (void)signal(SIGQUIT, sigHandler);
  (void)signal(SIGTERM, sigHandler);
  (void)signal(SIGHUP, sigHandler);
    
  int size = 0;
  int height = 0;
  int width = 0;
    
  char output[PIPE_BUF] = "";
  navigation msg;
    
  prog_name = (char*)malloc((strlen(argv[0]) + 1) * sizeof(char));
    
  strcpy(prog_name, argv[0]);
    
  if (argc != 5) {
    height = 10;
    width = 10;
    printf("Warning: %s Not like this. 10x10 it is.", prog_name);
    clear_eol();
    printf("Like this: './gridserver -x 10 -y 10'");
    clear_eol();
      
  } else {
      
    char c;
    while ((c = getopt(argc, argv, "x:y:")) != -1)
      switch (c) {
        case 'x':
          width = atoi(optarg);
          break;
        case 'y':
          height = atoi(optarg);
          break;
        default:
          fprintf(stderr, "Error: %s No correct input.", prog_name);
          cleanup();
          return EXIT_FAILURE;
      }
  }
    
  if (width <= 0 || height <= 0) {
    fprintf(stderr, "Error %s: No, wtf...", prog_name);
    cleanup();
    return EXIT_FAILURE;
  }
    
  size = (width + 2) * (height + 2);
  grid = (char*)malloc((size) * sizeof(char));

    
  for (int i = 0; i < size; ++i) {
    if (i < width + 2 || i > (size - (width + 2))) {
      grid[i] = '#';
    } else if ((i % (width + 2) == (width + 1)) || (i % (width + 2) == 0)) {
      grid[i] = '#';
    } else {
      grid[i] = ' ';
    }
  }

  struct stat st;
  if (stat(PIPE_DISPLAY, &st) != 0 && mkfifo(PIPE_DISPLAY, PERM) == -1) {
      fprintf(stderr, "Error %s: Can't create FIFO.", prog_name);
      clear_eol();
      cleanup();
      return EXIT_FAILURE;
  }
    
  display = open(PIPE_DISPLAY, O_RDWR);
    
  if (display == -1) {
    fprintf(stderr, "Error %s: Can't open FIFO.", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  if ((msgid = msgget(KEY, PERM | IPC_CREAT | IPC_EXCL)) == -1) {
      
    fprintf(stderr, "Error %s: Can't create MQ.", prog_name);
    clear_eol();
    cleanup();
    return EXIT_FAILURE;
  }
    
  printf("\n GRIDSERVER LAN!\n");
    
  while (1) {
    if (msgrcv(msgid, &msg, sizeof(msg), SERVER, 0) == -1) {
        
      fprintf(stderr, "Error %s: Can't receive from MQ", prog_name);
      clear_eol();
      cleanup();
      return EXIT_FAILURE;
    }

    printf("MSG received: Client ID: %c cmd: %c\n", msg.clientId, msg.command);
    if (msg.command == 'i') {
      printf("New\n");
      position init_pos;
      init_pos.msgRecip = msg.msgSender;
      init_pos.msgSender = SERVER;
      init_pos.clientId = msg.clientId;
      init_pos.x = 0;
      init_pos.y = 0;

        init_pos.status = (isOnBoard(msg.clientId, clients) ? REG_DOUBLE : 0);
      if (init_pos.status != REG_DOUBLE) {
        for (int i = 0; i < size; ++i) {
          if (grid[i] == ' ') {
            grid[i] = msg.clientId;
            clients[msg.clientId - 'A'] = (long)msg.msgSender;
            init_pos.x = i % (width + 2) - 1;
            init_pos.y = i / (width + 2) - 1;
            init_pos.status = REG_OK;
            printf("%c registration ok\n", msg.clientId);
            break;
          }
        }

      if (init_pos.status == 0) {
          init_pos.status = REG_FULL;
        }
      }

        if (msgsnd(msgid, &init_pos, sizeof(init_pos), 0) == -1) {
        fprintf(stderr, "Error %s: nota legita postione! *sounding italiano*... BAGUETTE", prog_name);
        clear_eol();
        cleanup();
        exit(EXIT_FAILURE);
      }
    }

      else if (isOnBoard(msg.clientId, clients) && msg.command == 'T') {
      for (int i = 0; i < size; ++i) {
        if (grid[i] == msg.clientId) {
          printf("%c is ded by signiori\n", msg.clientId);
          kill(clients[msg.clientId - 'A'], SIGTERM);
          clients[msg.clientId - 'A'] = 0;
          grid[i] = ' ';
        }
      }
    }

      else if (isOnBoard(msg.clientId, clients)) {
      printf("itsy bitsy griddy \n");
      if (move(msg.clientId, msg.command, grid, width, size) == 0) {
        for (int i = 0; i < size; ++i) {
          if (grid[i] == msg.clientId) {
            printf("Women driving...! (jk...) %c and %c, U ded!\n", grid[i],
                   grid[i + dirCheck(msg.command, width)]);
            kill(clients[grid[i] - 'A'], SIGTERM);
            clients[grid[i] - 'A'] = 0;
            kill(clients[grid[i + dirCheck(msg.command, width)] - 'A'], SIGTERM);
            clients[grid[i + dirCheck(msg.command, width)] - 'A'] = 0;
            grid[i] = ' ';
            grid[i + dirCheck(msg.command, width)] = ' ';
            break;
          }
        }

      } else if (move(msg.clientId, msg.command, grid, width, size) == 2) {
        for (int i = 0; i < size; ++i) {
          if (grid[i] == msg.clientId) {
            printf("%c You fell off of the world...!\n", msg.clientId);
            kill(clients[grid[i] - 'A'], SIGTERM);
            clients[grid[i] - 'A'] = 0;
            grid[i] = ' ';
            break;
          }
        }
      }

      else if (move(msg.clientId, msg.command, grid, width, size) == 1) {
        for (int i = 0; i < size; ++i) {
          if (grid[i] == msg.clientId) {
            printf("%c walky walky to %c \n", msg.clientId, msg.command);
            grid[i + dirCheck(msg.command, width)] = msg.clientId;
            grid[i] = ' ';
            break;
          }
        }
      }
    }

      
    printf("( . Y . )( . Y . )( . Y . )( . Y . )( . Y . )( . Y . )( . Y . )\n");
    output[0] = '\n';
    int size_count = 1;
    for (int y = 0; y < height + 2; ++y) {
      for (int x = 0; x < width + 2; ++x) {
        output[size_count] = grid[y * (width + 2) + x];
        ++size_count;
      }
      output[size_count] = '\n';
      ++size_count;
    }
    output[size_count] = '\n';
    output[size_count + 1] = '\0';
    if (write(display, output, strlen(output)) == -1) {
      fprintf(stderr, "Error %s: Display seems to be chinese... \n", prog_name);
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
  printf("Info %s: RELEASE THE GOKU...", prog_name);
  clear_eol();
  if (msgid != -1) {
    printf("Info %s: MQ DED...", prog_name);
    clear_eol();
    msgctl(msgid, IPC_RMID, (struct msqid_ds*)0);
  }
  printf("Info %s: Clients DED...", prog_name);
  clear_eol();
  for (int i = 0; i < 26; ++i) {
    if (clients[i] != 0) {
      kill(clients[i], SIGTERM);
    }
  }
  printf("Info %s: FIFO DED...", prog_name);
  clear_eol();
  close(display);
  remove(PIPE_DISPLAY);
  printf("Info %s: Memory DED (well actually not ded, but you get the point)...", prog_name);
  clear_eol();
  free(grid);
  free(prog_name);
}


void sigHandler() {
  printf("\n");
  cleanup();
  exit(EXIT_SUCCESS);
}


bool isOnBoard(char id, long clients[]) {
  if (clients[id - 'A'] != 0) {
    return true;
  }
  return false;
}


int dirCheck(char dir, int width) {
  if (dir == 'N') {
    return (-(width + 2));
  } else if (dir == 'E') {
    return 1;
  } else if (dir == 'S') {
    return (width + 2);
  } else if (dir == 'W') {
    return -1;
  } else {
    return 0;
  }
}

int move(char id, char dir, char grid[], int width, int size) {
  for (int i = 0; i < size; ++i) {
    if (grid[i] == id) {
      if (grid[i + dirCheck(dir, width)] == '#') {
        return 2;
      }
      if (grid[i + dirCheck(dir, width)] == ' ') {
        return 1;
      } else {
        return 0;
      }
    }
  }
  return 3;
}

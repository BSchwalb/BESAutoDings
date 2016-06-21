#ifndef headerstuff_H
#define headerstuff_H
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include "rlutil.h"



#define KEY 1234 // original 4224
#define PIPE_DISPLAY "fourtytwo"
#define PERM 0660
#define SERVER 42L
#define REG_DOUBLE -2
#define REG_FULL -1
#define REG_OK 1



void clear_eol() {
  printf("\x1B[K\n");
}


typedef struct 
  long msg_to;
  long msg_from;
  char client_id;
  char command;
} navigation;


typedef struct {
  long msg_to;
  long msg_from;
  char client_id;
  int x;
  int y;
  int status;
} position;

#endif

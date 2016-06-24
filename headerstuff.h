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



#define KEY 1234
#define PIPE_DISPLAY "asdfwer"
#define PERM 0660
#define SERVER 42L
#define REG_DOUBLE -2
#define REG_FULL -1
#define REG_OK 1



void clear_eol() {
  printf("\x1B[K\n");
}


typedef struct {
  long msgRecip;
  long msgSender;
  char clientId;
  char command;
} navigation;


typedef struct {
  long msgRecip;
  long msgSender;
  char clientId;
  int x;
  int y;
  int status;
} position;

#endif

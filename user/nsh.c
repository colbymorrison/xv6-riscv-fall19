#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXTOKEN 100

// Command types
#define EXECCMD 5

// Tokens
enum { ARG, EOF };

// Command structs - inspired by sh.c

// Global vars
char inputBuf[MAXTOKEN*10];
static char argvbuf[MAXTOKEN*10];
static char *argvPt[MAXTOKEN/10];

int bufIdx = 0;
int argvIdx = 0;
char *argvCur = argvbuf;

int tokentype;
int commandType;
char token[MAXTOKEN];

// Execute a command (child)
//void executecmd(){
 // printf("Execute!");
  //struct execcmd *execCmd;
 //switch(cmd->type){
    //case EXECCMD:
      //execCmd = (struct execcmd *)cmd;
      //printf("executing with argv: %s\n", execCmd->argv);
      //exec(execCmd->argv[0], execCmd->argv); 
      //break;
  //}
//}

// Lexer - inspired by K&R section 5.12
int isalphanum(char c){
  int ascii = (int) c;
  return (ascii >= 49 && ascii <= 57) || (ascii >= 65 && ascii <= 90) || (ascii >= 61 && ascii <= 122);
}


void getToken(){
  memset(token, 0, sizeof(token));  
  char *p = token;
  char cur = inputBuf[bufIdx];

  while(cur == ' ' || cur == '\t'){
    cur = inputBuf[++bufIdx];
  }

  if(isalphanum(cur)){
    for(*p++ = cur; isalphanum(cur = inputBuf[++bufIdx]); ){
      *p++ = cur;
    }
    *p = '\0';
    tokentype = ARG;
  }
  else if(cur == '\n'){
    tokentype = EOF;
  }
}

// Recursive-descent parsing
void parseExec();

void printargv(){
  for(int i = 0; i < sizeof(argvbuf); i++){
    printf("argvbuf[%d] = %d ", i, argvbuf[i]);
    if(i == sizeof(argvbuf)-1){
      printf("\n");
    }
  }
}

void parseCmd(){
  getToken();
  switch(tokentype){
    case ARG:
      parseExec();
      break;
    case EOF:
      parseExec();
      break;
  }
}

void parseExec(){
  switch(tokentype){
    case ARG:
      strcpy(argvCur, token);
      argvPt[argvIdx] = argvCur;
      argvCur+=strlen(token) + 1;
      argvIdx++;
      getToken();
      parseExec();
      break;
    case EOF:
      commandType = EXECCMD;
      break;
  }
}

int main(int argc, char **argv){
  int fd;
  // Ensure we have stdin, stdout, stderr
  while((fd = open("shell", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  printf("@ ");
  // Execute command
  while(gets(inputBuf, sizeof(inputBuf))){
    memset(argvbuf, 0, sizeof(argvbuf));
    // cd ?
    // Child executes command
    if(!fork()){
      parseCmd();
      switch(commandType){
        case EXECCMD:
          exec(argvPt[0], argvPt); 
          printf("Exec fail\n");
          break;
      }
    }
    wait(0);
    printf("@ ");
  }
  exit(0);
}

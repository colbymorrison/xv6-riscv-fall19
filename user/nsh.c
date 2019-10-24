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
char inputBuf[512];
int argIdx = 0;
int bufIdx = 0;

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

void printargv(char **argv){
  for(int i = 0; i < 20; i++){
    printf("argv[%d] = %s ", i, argv[i]);
    if(i == 19){
      printf("\n");
    }
  }
}

void parseCmd(char **argv){
  getToken();
  switch(tokentype){
    case ARG:
      parseExec(argv);
      break;
    case EOF:
      parseExec(argv);
      break;
  }
}

void parseExec(char **argv){
  printf("Current token type : %d\n", tokentype);
  printf("Current token value : %s\n", token);
  switch(tokentype){
    case ARG:
      printf("Setting argv[%d] to %s\n", argIdx, token);
      argv[argIdx] =  malloc(strlen(token));
      memmove(argv[argIdx], token, strlen(token));
      printargv(argv);
      argIdx++;
      getToken();
      parseExec(argv);
      break;
    case EOF:
      printargv(argv);
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
      char *argvExec[20];
  while(gets(inputBuf, sizeof(inputBuf))){
    // cd ?
    printf("Got input buf: %s", inputBuf); 
    // Child executes command
    if(!fork()){
      parseCmd(argvExec);
      printf("Execute! Type %d\n", commandType);
      switch(commandType){
        case EXECCMD:
          printargv(argvExec);
          exec(argvExec[0], argvExec); 
          break;
      }
    }
    memset(argvExec, 0, sizeof(argvExec));   
    wait(0);
    printf("@ ");
  }
  exit(0);
}

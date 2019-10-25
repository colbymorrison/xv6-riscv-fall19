#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXTOKEN 100

// Command types
#define EXECCMD 5

// Tokens
enum { ARG, EOF, EXIT };

// Command structs - inspired by sh.c
struct command{
  int type;
  char *argv[MAXTOKEN];
};

// Global vars
char inputBuf[MAXTOKEN*10];

int bufIdx = 0;
int argvIdx = 0;

// Lexer - inspired by K&R section 5.12
int isalphanum(char c){
  int ascii = (int) c;
  return (ascii >= 49 && ascii <= 57) || (ascii >= 65 && ascii <= 90) || (ascii >= 61 && ascii <= 122);
}


int getToken(char *token){
  memset(token, 0, sizeof(token));  
  char cur = inputBuf[bufIdx];

  if(bufIdx == sizeof(inputBuf)-1){
    fprintf(2,"nsh: Token too long\n"); 
  }

  while(cur == ' ' || cur == '\t'){
    cur = inputBuf[++bufIdx];
  }

  if(isalphanum(cur) || cur == '.'){
    for(*token++ = cur; isalphanum(cur = inputBuf[++bufIdx]); ){
      *token++ = cur;
    }
    *token = '\0';
    return ARG;
  }
  else if(cur == '\n'){
    return EOF;
  }
  // Error case
  else{
    fprintf(2, "nsh: Invalid syntax\n");
    exit(1);
  }
}

// Recursive-descent parsing
void parseExec();

//void printargv(){
  //for(int i = 0; i < sizeof(argvbuf); i++){
    //printf("argvbuf[%d] = %d ", i, argvbuf[i]);
    //if(i == sizeof(argvbuf)-1){
      //printf("\n");
    //}
  //}
//}
//
void parseCmd(struct command *cmd, char *argvCur){
  char token[MAXTOKEN];
  int tokentype = getToken(token);
  switch(tokentype){
    case ARG:
      parseExec(cmd, argvCur, token, tokentype);
      break;
    case EOF:
      break;
  }
}

void parseExec(struct command *execCmd, char *argvCur, char *token, int tokenType){
  //fprintf(2, "Current token type : %d\n", tokenType);
  //fprintf(2, "Current token value : %s\n", token);
  switch(tokenType){
    case ARG:
      strcpy(argvCur, token);
      execCmd->argv[argvIdx++] = argvCur;
      argvCur+=strlen(token) + 1;
      parseExec(execCmd, argvCur, token, getToken(token));
      break;
    case EOF:
      execCmd -> type = EXECCMD;
      break;
  }
}

// Execute a command (child)
void runcmd(){
  struct command cmd;
  char argvbuf[MAXTOKEN * 10];

  parseCmd(&cmd, argvbuf);

  switch(cmd.type){
    case EXECCMD:
      exec(cmd.argv[0], cmd.argv); 
      fprintf(2, "nsh: Exec %s failed\n", cmd.argv[0]);
      break;
    default:
      fprintf(2, "nsh: Exec failed\n");
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
  while(gets(inputBuf, sizeof(inputBuf))){
    if(!inputBuf[0]){
        break;
    }
    // cd ?
    // Child executes command
    if(!fork()){
      runcmd();
      exit(0);
    }
    wait(0);
    printf("@ ");
    // Reset input buffer
    memset(inputBuf, 0, sizeof(inputBuf));
  }
  exit(0);
}

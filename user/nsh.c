#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXTOKEN 50 
#define MAXCOMMANDS 20
#define NULL 0

enum commandTypes {EXECCMD, REDRCMD, PIPECMD};

enum tokenTypes { ARG=3, EOF, REDR, EXIT };

// Command structs - inspired by sh.c
struct execcmd{
  char *argv[MAXTOKEN];
};

struct redrcmd{
  char file[MAXTOKEN];
  int lr;
};

struct command{
  int type;
  union{
    struct execcmd execCmd;
    struct redrcmd redrCmd;
  } cmd;
};

// Global vars
char inputBuf[MAXTOKEN*10];
char argvBuf[MAXTOKEN*10];
struct command commands[MAXCOMMANDS];
int bufIdx = 0;
int cmdIdx = 0;
int argvIdx = 0; // Index into current execCmd struct argv (not global argvbuf)

// Lexer - inspired by K&R section 5.12
int isalphanum(char c){
  int ascii = (int) c;
  return (ascii >= 48 && ascii <= 57) || (ascii >= 65 && ascii <= 90) || (ascii >= 97 && ascii <= 122) || ascii == 46;
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

  if(isalphanum(cur)){
    for(*token++ = cur; isalphanum(cur = inputBuf[++bufIdx]); ){
      *token++ = cur;
    }
    *token = '\0';
    return ARG;
  }
  else if(cur == '<' || cur == '>'){
    *token = cur;
    bufIdx++;
    return REDR;
  }
  else if(cur == '\n'){
    return EOF;
  }
  // Error case
  else{
    fprintf(2, "nsh1: Invalid syntax. Cur: %c / %d \n", cur, cur);
    exit(1);
  }
}

// Recursive-descent parsing
void parseExec(struct execcmd *execCmd, char *argvCur, char *token, int tokenType);
void parseRedr(char *token, int tokenType);

void printargv(char **argvbuf){
  for(int i = 0; i < sizeof(argvbuf); i++){
    printf("argvbuf[%d] = %s ", i, argvbuf[i]);
    if(i == sizeof(argvbuf)-1){
      printf("\n");
    }
  }
}

void insertCmd(struct command *cmd){
  struct command* commandsCur = &commands[cmdIdx++];
  memmove(commandsCur, cmd, sizeof(struct command));
}


void parseCmd(char *argvCur){
  struct execcmd execCmd;
  char token[MAXTOKEN];
  int tokentype = getToken(token);
  switch(tokentype){
    case ARG:
        parseExec(&execCmd, argvCur, token, tokentype);
        break;
    case EOF:
        break;
    default:
        fprintf(2, "nsh: Invalid syntax\n");
        exit(1);
  }
}

void parseExec(struct execcmd *execCmd, char *argvCur, char *token, int tokenType){
  fprintf(2, "Current token type : %d\n", tokenType);
  fprintf(2, "Current token value : %s\n", token);
  struct command cmd;

  switch(tokenType){
    case ARG:
      strcpy(argvCur, token);
      execCmd->argv[argvIdx++] = argvCur;
      argvCur+=strlen(token) + 1;
      parseExec(execCmd, argvCur, token, getToken(token));
      break;
    case REDR:
      cmd.type = EXECCMD;
      cmd.cmd.execCmd = *execCmd;
      insertCmd(&cmd);
      parseRedr(token, tokenType);
      break;
    case EOF:
      cmd.type = EXECCMD;
      cmd.cmd.execCmd = *execCmd;
      insertCmd(&cmd);
      break;
    default:
      fprintf(2, "nsh: Invalid syntax\n");
      exit(1);
  }
}

void parseRedr(char *token, int tokenType){
  struct redrcmd redrCmd;
  struct command cmd;
  switch(tokenType){
    case REDR:
      fprintf(2, "Current token type : %d\n", tokenType);
      fprintf(2, "Current token value : %s\n", token);
      if(*token == '>'){
        redrCmd.lr = 1;
      }
      else{
        redrCmd.lr = 0;
      }
      if((tokenType = getToken(token)) != ARG){
        fprintf(2, "nsh: Invalid redirect syntax\n");
        exit(1);
      }
      fprintf(2, "Current token type1 : %d\n", tokenType);
      fprintf(2, "Current token value : %s\n", token);
      strcpy(redrCmd.file, token);
      cmd.type = REDRCMD;
      cmd.cmd.redrCmd = redrCmd;
      insertCmd(&cmd);
      parseRedr(token, getToken(token));
    case EOF:
      break;
  }

}


// Execute a command (child), inspired by sh.c
void execute(){
  struct execcmd execCmd;
  struct redrcmd redrCmd;

  //printf("Exec\n");
  for(int i = cmdIdx-1; i >= 0; i --){
    struct command curCmd = commands[i];
    //printf("Type %d\n", curCmd.type);
    switch(curCmd.type){
      case EXECCMD:
        execCmd = curCmd.cmd.execCmd;
        //printf("Execcmd: argv[0]: %p\n", execCmd.argv[0]);
        exec(execCmd.argv[0], execCmd.argv); 
        fprintf(2, "nsh: Exec %s failed\n", execCmd.argv[0]);
        break;
      case REDRCMD:
        redrCmd = curCmd.cmd.redrCmd;
        //printf("Redr command: file %s, type %d\n", redrCmd.file, redrCmd.lr);
        close(redrCmd.lr);
        if(open(redrCmd.file, O_RDWR|O_CREATE) < 0){ // TODO Just write?
          fprintf(2, "nsh: Error opening file %s", redrCmd.file);
          exit(1);
        }
        //printf("Type %d\n", curCmd.type);
        break;
      default:
        fprintf(2, "nsh: Exec failed\n");
        exit(1);
        break;
    }
  }
}

// Run user-inputted command
void runCmd(){
  char *argvCur = argvBuf;
  parseCmd(argvCur);
  execute();
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
      runCmd();
      exit(0);
    }
    wait(0);
    printf("@ ");
    // Reset input buffer and argvbuffer
    memset(inputBuf, 0, sizeof(inputBuf));
    memset(argvBuf, 0, sizeof(argvBuf));
  }
  exit(0);
}

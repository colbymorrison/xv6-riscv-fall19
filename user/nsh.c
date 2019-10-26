#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXTOKEN 50 
#define NULL 0

enum commandTypes {EXECMD, REDRCMD, PIPECMD};

enum tokenTypes { ARG=3, EOF, REDR, EXIT };

// Command structs - inspired by sh.c
struct command{
  int type;
  struct command *nextCmd;
};

struct execcmd{
  int type;
  struct command *nextCmd;
  char *argv[MAXTOKEN];
};

struct redrcmd{
  int type;
  struct command *nextCmd;
  char file[MAXTOKEN];
  int lr;
};

// Global vars
char inputBuf[MAXTOKEN*10];
int bufIdx = 0;
int argvIdx = 0;

// Lexer - inspired by K&R section 5.12
int isalphanum(char c){
  int ascii = (int) c;
  return (ascii >= 48 && ascii <= 57) || (ascii >= 65 && ascii <= 90) || (ascii >= 97 && ascii <= 122);
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
void parseExec(struct execcmd *execCmd, char *argvCur, struct command *cmdList, char *token, int tokenType);
void parseRedr(struct redrcmd *redrCmd, struct command *cmdList, char *token, int tokenType);

void printargv(char **argvbuf){
  for(int i = 0; i < sizeof(argvbuf); i++){
    printf("argvbuf[%d] = %s ", i, argvbuf[i]);
    if(i == sizeof(argvbuf)-1){
      printf("\n");
    }
  }
}

int sizeofCmd(struct command *cmd){
  switch(cmd->type){
    case(EXECMD):
      return sizeof(struct execcmd);
    case(REDRCMD):
      return sizeof(struct redrcmd);
    default:
      return 0;
  }
}

void insertCmdList(struct command *cmdList, struct command *currentCmd){
  int sizeList = sizeofCmd(cmdList);
  int sizeCur = sizeofCmd(currentCmd);

  printf("sizeList: %d\n sizeCur: %d\n", sizeList, sizeCur);
  printf("Prev cmd type: %d Inserting command type: %d\n", cmdList->type, currentCmd->type);

  // Not first elt in list
  if(sizeList){
    cmdList->nextCmd = cmdList + sizeList;
    cmdList += sizeList;
  }

  memmove(cmdList, currentCmd, sizeCur);
  printf("Memmoe\n");
}


int parseCmd(struct command *cmdList, char *argvCur){
  struct execcmd execCmd = {EXECMD, NULL};
  char token[MAXTOKEN];
  int tokentype = getToken(token);
  switch(tokentype){
    case ARG:
        parseExec(&execCmd, argvCur, cmdList, token, tokentype);
        break;
    case EOF:
        break;
    default:
        fprintf(2, "nsh: Invalid syntax\n");
        exit(1);
  }
}

void parseExec(struct execcmd *execCmd, char *argvCur, struct command *cmdList, char *token, int tokenType){
  fprintf(2, "Current token type : %d\n", tokenType);
  fprintf(2, "Current token value : %s\n", token);
  struct redrcmd redrCmd;

  switch(tokenType){
    case ARG:
      strcpy(argvCur, token);
      execCmd->argv[argvIdx++] = argvCur;
      argvCur+=strlen(token) + 1;
      parseExec(execCmd, argvCur, cmdList, token, getToken(token));
      break;
    case REDR:
      insertCmdList(cmdList, (struct command *)execCmd);

      redrCmd = (struct redrcmd){REDRCMD, NULL};
      parseRedr(&redrCmd, cmdList, token, tokenType);
      break;
    case EOF:
      insertCmdList(cmdList, (struct command *)execCmd);
      break;
    default:
      fprintf(2, "nsh: Invalid syntax\n");
      exit(1);
  }
}

void parseRedr(struct redrcmd *redrCmd, struct command *cmdList, char *token, int tokenType){
  if(*token == '>'){
    redrCmd -> lr = 1;
  }
  else{
    redrCmd -> lr = 0;
  }
  if((tokenType = getToken(token)) != ARG){
    fprintf(2, "nsh: Invalid redirect syntax\n");
    exit(1);
  }
  fprintf(2, "Current token type : %d\n", tokenType);
  fprintf(2, "Current token value : %s\n", token);
  strcpy(redrCmd->file, token);
  redrCmd -> type = REDRCMD;
  insertCmdList(cmdList, (struct command *)redrCmd);
}


// Execute a command (child), inspired by sh.c
void execute(struct command *cmdList){
  struct command *cmd = cmdList;
  struct execcmd *execCmd;
  struct redrcmd *redrCmd;

  printf("Exec\n");
  while(cmd){
    printf("Type %d\n", cmd->type);
    switch(cmd->type){
      case EXECMD:
        execCmd = (struct execcmd *)cmd;
        printf("Execcmd: argv[0]: %s\n", execCmd->argv[0]);
        exec(execCmd->argv[0], execCmd->argv); 
        fprintf(2, "nsh: Exec %s failed\n", execCmd->argv[0]);
        break;
      case REDRCMD:
        redrCmd = (struct redrcmd *)cmd;
        printf("Redr command: file %s, type %d\n", redrCmd->file, redrCmd->lr);
        close(redrCmd->lr);
        if(open(redrCmd -> file, O_RDWR|O_CREATE) < 0){ // TODO Just write?
          fprintf(2, "nsh: Error opening file %s", redrCmd->file);
          exit(1);
        }
        cmd = redrCmd->nextCmd;
        printf("Type %d\n", cmd->type);
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
  char argvbuf[30];
  // Linked list of commands
  struct command cmdList[100];
  printf("List total size: %d\n", sizeof(cmdList));
  struct command *cmdListP = cmdList;
  // Trigger default case, 1st elt
  cmdListP->type = -1;

  parseCmd(cmdListP, argvbuf);
  execute(cmdList);
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
    // Reset input buffer
    memset(inputBuf, 0, sizeof(inputBuf));
  }
  exit(0);
}

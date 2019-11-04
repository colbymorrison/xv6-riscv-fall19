#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXTOKEN 20 
#define MAXCOMMANDS 40
#define NULL 0

enum commandTypes {EXECCMD, REDRCMD, PIPECMD};

enum tokenTypes { ARG=3, EOF, REDR, PIPE, EXIT };

// Command structs - inspired by sh.c
struct execcmd{
  char *argv[MAXTOKEN];
};

struct redrcmd{
  char file[MAXTOKEN];
  int lr;
};

struct pipecmd{
  int brk;
};

struct command{
  int type;
  union{
    struct execcmd execCmd;
    struct redrcmd redrCmd;
    struct pipecmd pipeCmd;
  } cmd;
};

// Global vars
char inputBuf[MAXTOKEN*10];
char argvBuf[MAXTOKEN*10];
struct command commands[MAXCOMMANDS];
struct execcmd execCmd;
char *argvCur = argvBuf;
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
  else if(cur == '|'){
    bufIdx++;
    return PIPE;
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
void parseExec(struct execcmd *execCmd);
void parseRedr(char *token, int tokentype);
void parsePipe(int tokentype);

void printargv(char **argvbuf){
  for(int i = 0; i < sizeof(argvbuf); i++){
    //printf("argvbuf[%d] = %s ", i, argvbuf[i]);
    if(i == sizeof(argvbuf)-1){
      //printf("\n");
    }
  }
}

void insertCmd(struct command *cmd){
  //printf("Inserting command of type %d at index %d\n", cmd->type, cmdIdx);
  //fprintf(2, "argv[0]: %s\n", cmd->cmd.execCmd.argv[0]);
  struct command* commandsCur = &commands[cmdIdx++];
  memmove(commandsCur, cmd, sizeof(struct command));
}


void parseCmd(){
  char token[MAXTOKEN];
  int tokentype = getToken(token);
  switch(tokentype){
    case ARG:
        fprintf(2, "Current token type : %d\n", tokentype);
        fprintf(2, "Current token value : %s\n", token);
        strcpy(argvCur, token);
        execCmd.argv[argvIdx++] = argvCur;
        argvCur+=strlen(token) + 1;
        parseExec(&execCmd);
        break;
    case EOF:
        break;
    default:
        fprintf(2, "nsh: Invalid syntax\n");
        exit(1);
  }
}

void parseExec(struct execcmd *execCmd){
  struct command cmd;
  
  char token[MAXTOKEN];
  int tokenType = getToken(token);
  fprintf(2, "Current token type : %d\n", tokenType);
  fprintf(2, "Current token value : %s\n", token);
  //printargv(execCmd->argv);
  switch(tokenType){
    case ARG:
      strcpy(argvCur, token);
      execCmd->argv[argvIdx++] = argvCur;
      argvCur+=strlen(token) + 1;
      //printargv(execCmd->argv);
      parseExec(execCmd);
      break;
    case REDR:
      cmd.type = EXECCMD;
      cmd.cmd.execCmd = *execCmd;
      insertCmd(&cmd);
      //printf("parserder\n");
      //printf("token: %s, type: %d\n", token, tokenType);
      parseRedr(token, tokenType);
      break;
    case PIPE:
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
      fprintf(2, "nshb: Invalid syntax\n");
      exit(1);
  }
}

void parseRedr(char *token, int tokentype){
  //printf("parseredr22\n");
  struct redrcmd redrCmd;
  struct command cmd;

  fprintf(2, "Current token type : %d\n", tokentype);
  fprintf(2, "Current token value : %s\n", token);
  switch(tokentype){
    case REDR:
      if(*token == '>'){
        redrCmd.lr = 1;
      }
      else{
        redrCmd.lr = 0;
      }
      if((tokentype = getToken(token)) != ARG){
        fprintf(2, "nsh: Invalid redirect syntax\n");
        exit(1);
      }
      fprintf(2, "Current token type1 : %d\n", tokentype);
      fprintf(2, "Current token value : %s\n", token);
      strcpy(redrCmd.file, token);
      cmd.type = REDRCMD;
      cmd.cmd.redrCmd = redrCmd;
      insertCmd(&cmd);
      parseRedr(token, getToken(token));
      break;
    case PIPE:
      parsePipe(tokentype);   
      break;
    case EOF:
      break;
    default:
        fprintf(2, "nsha: Invalid syntax\n");
        exit(1);
  }

}

void parsePipe(int tokentype){
  struct pipecmd pipeCmd;
  struct command cmd;

  switch(tokentype){
    case PIPE:
      argvIdx = 0;
      pipeCmd.brk = cmdIdx;
      memset(&execCmd, 0, sizeof(struct execcmd));
      parseExec(&execCmd);
      cmd.type = PIPECMD;
      cmd.cmd.pipeCmd = pipeCmd;
      insertCmd(&cmd);
      break;
    default:
        fprintf(2, "nsh: Invalid syntax\n");
        exit(1);
  }
}

void executeExecRedr(int high, int low);

void execute(){
  cmdIdx--;
  struct command last = commands[cmdIdx];

  if(last.type != PIPECMD){
    executeExecRedr(cmdIdx, 0);
  }
  else{
    struct pipecmd pipeCmd = last.cmd.pipeCmd;
    //printf("Exec a pipe. Break %d\n", pipeCmd.brk);

    int pipefd[2];
    int child1;
    int child2;

    if(pipe(pipefd) < 0){
      printf("Pipe error!");
      exit(1);
    }

    if((child1 = fork()) < 0){
      printf("Fork error!");
      exit(1);
    }

    if(!child1){
      close(1);
      dup(pipefd[1]);
      close(pipefd[0]);
      close(pipefd[1]);
      executeExecRedr(pipeCmd.brk-1, 0);
    }


    if((child2 = fork()) < 0){
      printf("Fork error!");
      exit(1);
    }

    if(!child2){
      close(0);
      dup(pipefd[0]);
      close(pipefd[0]);
      close(pipefd[1]);
      executeExecRedr(cmdIdx-1, pipeCmd.brk);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    wait(&child1);
    wait(&child2);
  }
}



// Execute a command (child), inspired by sh.c
void executeExecRedr(int high, int low){
  struct execcmd execCmd;
  struct redrcmd redrCmd;

  //printf("Exec high: %d, low: %d\n", high, low);
  for(int i = high; i >= low; i --){
    struct command curCmd = commands[i];
    //printf("Type %d\n", curCmd.type);
    switch(curCmd.type){
      case EXECCMD:
        execCmd = curCmd.cmd.execCmd;
        fprintf(2, "Execcmd: argv[0]: %s\n", execCmd.argv[0]);
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
      parseCmd(argvCur);
      execute();
      exit(0);
    }
    wait(0);
    printf("@ ");
    // Reset input buffer and argvbuffer
    memset(inputBuf, 0, sizeof(inputBuf));
    memset(argvBuf, 0, sizeof(argvBuf));
    memset(&execCmd, 0, sizeof(struct execcmd));
  }
  exit(0);
}

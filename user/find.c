#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int substring(char *needle, char *haystack){

}

void searchDir(char *path, char *search){
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    exit(1);
  }
  switch (st.type){
    case T_FILE:
      if(substring(path, search)){
        printf("%s", path);
      }

     case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
          printf("find: path too long\n");
          break;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
          if(de.inum == 0)
            continue;
          memmove(p, de.name, DIRSIZ);
          p[DIRSIZ] = 0;
          if(stat(buf, &st) < 0){
            printf("find: cannot stat %s\n", buf);
            continue;
          }
        }
        searchDir(path, search);
        break;
      }
  }

  int main(int argc, char **argv){
    if(argc < 3){
      printf("Be better");
      exit(0);
    }
    searchDir(argv[1], argv[2]);
    exit(0);
  }


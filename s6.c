#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>


int main(int argc, char *argv[])
{

  
  char *file=argv[1];
  int fd;
  fd=open(file, O_RDONLY);
  if(fd == -1) {
    perror("error open");
  }

  int fout;
  fout=creat("statistica.txt", S_IWUSR);
  if(fout == -1) {
    perror("error open");
  }

  

  int latimea;
  lseek(fd,18,SEEK_SET);
  if(read(fd,&latimea,sizeof(int))!=sizeof(int)) {
    perror("error read");
    exit(1);
  }

  int inaltimea;
  lseek(fd,22,SEEK_SET);
  if(read(fd,&inaltimea,sizeof(int))!=sizeof(int)) {
    perror("error read");
    exit(1);
  }

  int dimoct;
  lseek(fd,2,SEEK_SET);
  if(read(fd,&dimoct,sizeof(int))!=sizeof(int)) {
    perror("error read");
    exit(1);
  }
  
    struct stat info;

    if((stat(argv[1],&info)) == -1) 
    {
        perror("error read");
        exit(1);
    }

    struct tm *lastm;

    lastm = localtime(&info.st_mtime);
    char buffer[256];

     sprintf(buffer,"nume fisier: %s\ninaltime: %d\nlatime: %d\ndimensiune: %ld\n",argv[1],inaltimea,latimea,info.st_size);
     write(fout,buffer,strlen(buffer));

    sprintf(buffer,"identificatorul utiliziatorului: %d\ntimpul ultimei modificari: %d.%d.%d\ncontorul de legaturi: %d\n",info.st_uid,lastm->tm_mday,(lastm->tm_mon+1),(lastm->tm_year+1900),info.st_nlink);
    write(fout,buffer,strlen(buffer));

    sprintf(buffer,"drepturi de acces user: ");

    if(info.st_mode & S_IRUSR) strcat(buffer,"R");
    else strcat(buffer,"-");

  
    if(info.st_mode & S_IWUSR) strcat(buffer,"W");
    else strcat(buffer,"-");

  
    if(info.st_mode & S_IXUSR) strcat(buffer,"X");
    else strcat(buffer,"-");

    strcat(buffer,"\n");
    write(fout,buffer,strlen(buffer));

    sprintf(buffer,"drepturi de acces grup: ");

     if(info.st_mode & S_IRGRP) strcat(buffer,"R");
    else strcat(buffer,"-");

  
    if(info.st_mode & S_IWGRP) strcat(buffer,"W");
    else strcat(buffer,"-");

  
    if(info.st_mode & S_IXGRP) strcat(buffer,"X");
    else strcat(buffer,"-");

    strcat(buffer,"\n");
    write(fout,buffer,strlen(buffer));

    sprintf(buffer,"drepturi de acces altii: ");

     if(info.st_mode & S_IROTH) strcat(buffer,"R");
    else strcat(buffer,"-");

  
    if(info.st_mode & S_IWOTH) strcat(buffer,"W");
    else strcat(buffer,"-");

  
    if(info.st_mode & S_IXOTH) strcat(buffer,"X");
    else strcat(buffer,"-");

    strcat(buffer,"\n");
    write(fout,buffer,strlen(buffer));


    if((close(fd)) == -1) {
    perror("error close");
    exit(1);
  }

    if((close(fout)) == -1) {
    perror("error close");
    exit(1);
  }

    
  
  return 0;
}

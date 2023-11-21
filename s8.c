#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

void process_file(const char *file, int fout) {
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("error open");
        return;
    }

    int width, height;
    if (lseek(fd, 18, SEEK_SET) == -1 ||
        read(fd, &width, sizeof(int)) != sizeof(int) ||
        read(fd, &height, sizeof(int)) != sizeof(int)) {
        perror("error read");
        close(fd);
        return;
    }
    close(fd);

    struct stat info;
    if (stat(file, &info) == -1) {
        perror("error stat");
        return;
    }

    char buffer[512];
    if (strstr(file, ".bmp") != NULL) {
        sprintf(buffer, "nume fisier: %s\ninaltime: %d\nlatime: %d\ndimensiune: %ld\n", file, height, width, info.st_size);
    } else {
        sprintf(buffer, "nume fisier: %s\ndimensiune: %ld\n", file, info.st_size);
    }
    write(fout, buffer, strlen(buffer));

    struct tm *lastm;
    lastm = localtime(&info.st_mtime);
    sprintf(buffer, "identificatorul utilizatorului: %d\ntimpul ultimei modificari: %d.%d.%d\ncontorul de legaturi: %lu\n", info.st_uid, lastm->tm_mday, (lastm->tm_mon + 1), (lastm->tm_year + 1900), info.st_nlink);
    write(fout, buffer, strlen(buffer));

    sprintf(buffer, "drepturi de acces user: ");
    if(info.st_mode & S_IRUSR) strcat(buffer,"R");
    else strcat(buffer,"-");
  
    if(info.st_mode & S_IWUSR) strcat(buffer,"W");
    else strcat(buffer,"-");
  
    if(info.st_mode & S_IXUSR) strcat(buffer,"X");
    else strcat(buffer,"-");
    strcat(buffer,"\n");
    write(fout, buffer, strlen(buffer));

    sprintf(buffer, "drepturi de acces grup: ");
    if(info.st_mode & S_IRGRP) strcat(buffer,"R");
    else strcat(buffer,"-");
  
    if(info.st_mode & S_IWGRP) strcat(buffer,"W");
    else strcat(buffer,"-");
  
    if(info.st_mode & S_IXGRP) strcat(buffer,"X");
    else strcat(buffer,"-");
    strcat(buffer,"\n");
    write(fout, buffer, strlen(buffer));

    sprintf(buffer, "drepturi de acces altii: ");
    if(info.st_mode & S_IROTH) strcat(buffer,"R");
    else strcat(buffer,"-");
  
    if(info.st_mode & S_IWOTH) strcat(buffer,"W");
    else strcat(buffer,"-");
  
    if(info.st_mode & S_IXOTH) strcat(buffer,"X");
    else strcat(buffer,"-");
    strcat(buffer,"\n");
    write(fout, buffer, strlen(buffer));

    close(fd);
}

void process_symlink(const char *link, const char *file, int fout) {
    struct stat link_stat, target_stat;
    if (lstat(link, &link_stat) == -1 || stat(file, &target_stat) == -1) {
        perror("error stat");
        return;
    }

    char buffer[256];
    sprintf(buffer, "nume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier: %ld\ndrepturi de acces user legatura: ", link, link_stat.st_size, target_stat.st_size);
    if (link_stat.st_mode & S_IRUSR) strcat(buffer, "R");
    else strcat(buffer, "-");

    if (link_stat.st_mode & S_IWUSR) strcat(buffer, "W");
    else strcat(buffer, "-");

    if (link_stat.st_mode & S_IXUSR) strcat(buffer, "X");
    else strcat(buffer, "-");
    strcat(buffer, "\n");
    write(fout, buffer, strlen(buffer));
}

void process_directory(const char *dir, const char *out_dir, int fout) {
    DIR *dp = opendir(dir);
    if (!dp) {
        perror("error opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, entry->d_name);

            struct stat file_stat;
            if (lstat(file_path, &file_stat) == -1) {
                perror("error stat");
                continue;
            }

            if (S_ISREG(file_stat.st_mode)) {
                if (strstr(entry->d_name, ".bmp") != NULL) {
                    process_file(file_path, fout);

                    pid_t pid_bmp = fork();
                    if (pid_bmp < 0) {
                        perror("error fork");
                        return;
                    } else if (pid_bmp == 0) {
                        int fd = open(file_path, O_RDWR);
                        if (fd == -1) {
                            perror("error open");
                            return;
                        }

                        int width, height;
                        if (lseek(fd, 18, SEEK_SET) == -1 ||
                            read(fd, &width, sizeof(int)) != sizeof(int) ||
                            read(fd, &height, sizeof(int)) != sizeof(int)) {
                            perror("error read");
                            close(fd);
                            return;
                        }

                        if (lseek(fd, 54, SEEK_SET) == -1) {
                            perror("error lseek");
                            close(fd);
                            return;
                        }

                        for (int i = 0; i < height; ++i) {
                            for (int j = 0; j < width; ++j) {
                                unsigned char pixel[3];
                                if (read(fd, pixel, 3) != 3) {
                                    perror("error read");
                                    close(fd);
                                    return;
                                }

                                unsigned char pixel_gri = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);

                                if (lseek(fd, -3, SEEK_CUR) == -1) {
                                    perror("error lseek");
                                    close(fd);
                                    return;
                                }

                                if (write(fd, &pixel_gri, sizeof(unsigned char)) != sizeof(unsigned char)) {
                                    perror("error write");
                                    close(fd);
                                    return;
                                }
                            }
                        }
                        close(fd);
                        return;
                    } else {
                        int status;
                        waitpid(pid_bmp, &status, 0);
                        printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", pid_bmp, status);
                    }
                } else {
                    process_file(file_path, fout);
                }
            } else if (S_ISLNK(file_stat.st_mode)) {
                char target_path[512];
                long long len = readlink(file_path, target_path, sizeof(target_path) - 1);
                if (len != -1) {
                    target_path[len] = '\0';
                    process_symlink(file_path, target_path, fout);
                }

            } else if (S_ISDIR(file_stat.st_mode)) {
                char buffer[1024];
                sprintf(buffer, "nume director: %s\nidentificatorul utilizatorului: %d\n", file_path, file_stat.st_uid);
                write(fout, buffer, strlen(buffer));

                pid_t pid_dir = fork();
                if (pid_dir < 0) {
                    perror("error fork");
                    return;
                } else if (pid_dir == 0) {
                    process_directory(file_path, out_dir, fout);
                    return;
                } else {
                    int status;
                    waitpid(pid_dir, &status, 0);
                    printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", pid_dir, status);
                }
            }
        }
    }
    closedir(dp);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <director_intrare> <director_iesire>\n", argv[0]);
        return 1;
    }

    int fout = creat("statistica.txt", S_IWUSR | S_IRUSR); // Deschide fișierul cu permisiuni de scriere și citire pentru utilizator
    if (fout == -1) {
        perror("error open");
        return 1;
    }

    struct stat dir_stat;
    if (stat(argv[2], &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode)) {
        printf("Argumentul nu este un director valid.\n");
        close(fout);
        return 1;
    }

    process_directory(argv[1], argv[2], fout);

    if (close(fout) == -1) {
        perror("error close");
        return 1;
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

static char time_buffer[32] = "";
static char output_buffer[512] = "";
static char output_path[128] = "";

struct BMPHeader {
    __attribute__((unused)) uint16_t signature;
    __attribute__((unused)) uint32_t fileSize;
    __attribute__((unused)) uint32_t reserved;
    uint32_t dataOffset;
}__attribute__((packed));

struct BMPInfoHeader {
    __attribute__((unused)) uint32_t size;
    uint32_t width;
    uint32_t height;
    __attribute__((unused)) uint16_t planes;
    uint16_t bitCount;
    __attribute__((unused)) uint32_t compression;
    __attribute__((unused)) uint32_t imageSize;
    __attribute__((unused)) uint32_t xPixelsPerM;
    __attribute__((unused)) uint32_t yPixelsPerM;
    __attribute__((unused)) uint32_t colorsUsed;
    __attribute__((unused)) uint32_t colorsImportant;
}__attribute__((packed));

struct BMPFile {
    struct BMPHeader header;
    struct BMPInfoHeader info;
    uint8_t *data;
};

void convertBmpToGrayscale(const char *path) {
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("Eroare la deschiderea fisierului");
        exit(EXIT_FAILURE);
    }

    struct BMPFile bmpFile;
    if ((read(fd, &bmpFile.header, sizeof(bmpFile.header)) < 0) ||
        (read(fd, &bmpFile.info, sizeof(bmpFile.info)) < 0)) {
        perror("Eroare la citirea antetului bitmap-ului");
        exit(EXIT_FAILURE);
    }

    if (bmpFile.info.bitCount <= 8) {
        fprintf(stderr, "Format bitmap neacceptat\n");
        exit(EXIT_FAILURE);
    }
    u_int32_t imageSize = bmpFile.info.width * bmpFile.info.height * 3;

    bmpFile.data = (uint8_t *) malloc(imageSize);
    if (bmpFile.data == NULL) {
        perror("Eroare de alocare dinamică a memoriei");
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, bmpFile.header.dataOffset, SEEK_SET) < 0) {
        perror("Eroare la citirea datelor pixelilor bitmap-ulu");
        free(bmpFile.data);
        exit(EXIT_FAILURE);
    }

    if (read(fd, bmpFile.data, imageSize) < 0) {
        perror("Eroare la conversia bitmap-ului la tonuri de gri");
        free(bmpFile.data);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < imageSize; i = i + 3) {
        uint8_t *pixel = &bmpFile.data[i];
        uint8_t gray = (uint8_t) (0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
        pixel[0] = pixel[1] = pixel[2] = gray;
    }

    if (lseek(fd, bmpFile.header.dataOffset, SEEK_SET) < 0) {
        perror("Eroare la căutarea în fișier");
        free(bmpFile.data);
        exit(EXIT_FAILURE);
    }

    if (write(fd, bmpFile.data, imageSize) < 0) {
        perror("Eroare la scriere");
        free(bmpFile.data);
        exit(EXIT_FAILURE);
    }
    free(bmpFile.data);

    close(fd);
}

void writeToOutputFile() {
    int out_fd = creat(output_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (out_fd < 0) {
        perror("Eroare la crearea fisierului");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(output_buffer);
    long ret_val = write(out_fd, output_buffer, len);
    if (ret_val < 0) {
        perror("Eroare la scriere");
        exit(EXIT_FAILURE);
    } else if (ret_val != len) {
        fprintf(stderr, "Eroare la scrierere");
        exit(EXIT_FAILURE);
    }

    close(out_fd);
}

void writeRegularFileStats(const char *name, struct stat *stp) {

    snprintf(output_buffer, 512,
             "nume fisier: %s\n"
             "dimensiune: %lld\n"
             "identificatorul utilizatorului: %d\n"
             "timpul ultimei modificari: %s\n"
             "contorul de legaturi: %hu\n"
             "drepturi de acces user: %c%c%c\n"
             "drepturi de acces grup: %c%c%c\n"
             "drepturi de acces altii: %c%c%c\n",
             name, stp->st_size, stp->st_uid, time_buffer, stp->st_nlink,
             (stp->st_mode & S_IRUSR) ? 'R' : '-', (stp->st_mode & S_IWUSR) ? 'W' : '-',
             (stp->st_mode & S_IXUSR) ? 'X' : '-',
             (stp->st_mode & S_IRGRP) ? 'R' : '-', (stp->st_mode & S_IWGRP) ? 'W' : '-',
             (stp->st_mode & S_IXGRP) ? 'X' : '-',
             (stp->st_mode & S_IROTH) ? 'R' : '-', (stp->st_mode & S_IWOTH) ? 'W' : '-',
             (stp->st_mode & S_IXOTH) ? 'X' : '-');

    writeToOutputFile();
}

void write_bmp_file_stats(const char *name, const char *path, struct stat *stp) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Eroare la deschiderea fisierului");
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, sizeof(struct BMPHeader), SEEK_SET) < 0) {
        perror("Error seeking through file");
        exit(EXIT_FAILURE);
    }

    struct BMPInfoHeader info;
    if (read(fd, &info, sizeof(info)) < 0) {
        perror("Eroare la citirea antetului bitmap-ului");
        exit(EXIT_FAILURE);
    }

    close(fd);

    snprintf(output_buffer, 512,
             "nume poza: %s\n"
             "inaltime: %d\n"
             "lungime: %d\n"
             "dimensiune: %lld\n"
             "identificatorul utilizatorului: %d\n"
             "timpul ultimei modificari: %s\n"
             "contorul de legaturi: %hu\n"
             "drepturi de acces user: %c%c%c\n"
             "drepturi de acces grup: %c%c%c\n"
             "drepturi de acces altii: %c%c%c\n",
             name, info.height, info.width, stp->st_size, stp->st_uid, time_buffer, stp->st_nlink,
             (stp->st_mode & S_IRUSR) ? 'R' : '-', (stp->st_mode & S_IWUSR) ? 'W' : '-',
             (stp->st_mode & S_IXUSR) ? 'X' : '-',
             (stp->st_mode & S_IRGRP) ? 'R' : '-', (stp->st_mode & S_IWGRP) ? 'W' : '-',
             (stp->st_mode & S_IXGRP) ? 'X' : '-',
             (stp->st_mode & S_IROTH) ? 'R' : '-', (stp->st_mode & S_IWOTH) ? 'W' : '-',
             (stp->st_mode & S_IXOTH) ? 'X' : '-');

    writeToOutputFile();
}

void writeLinkStats(const char *name, const char *path, struct stat *stp) {
    struct stat tgf;
    if (stat(path, &tgf) < 0) {
        perror("Error reading target file stats");
        exit(EXIT_FAILURE);
    }

    snprintf(output_buffer, 512,
             "nume legatura: %s\n"
             "dimensiune: %lld\n"
             "dimensiune fisier tinta:%lld\n"
             "timpul ultimei modificari: %s\n"
             "identificatorul utilizatorului: %d\n"
             "drepturi de acces user: %c%c%c\n"
             "drepturi de acces grup: %c%c%c\n"
             "drepturi de acces altii: %c%c%c\n",
             name, stp->st_size, tgf.st_size, time_buffer, stp->st_uid,
             (stp->st_mode & S_IRUSR) ? 'R' : '-', (stp->st_mode & S_IWUSR) ? 'W' : '-',
             (stp->st_mode & S_IXUSR) ? 'X' : '-',
             (stp->st_mode & S_IRGRP) ? 'R' : '-', (stp->st_mode & S_IWGRP) ? 'W' : '-',
             (stp->st_mode & S_IXGRP) ? 'X' : '-',
             (stp->st_mode & S_IROTH) ? 'R' : '-', (stp->st_mode & S_IWOTH) ? 'W' : '-',
             (stp->st_mode & S_IXOTH) ? 'X' : '-');

    writeToOutputFile();
}

void writeDirStats(const char *name, struct stat *stp) {
    snprintf(output_buffer, 512,
             "nume director: %s\n"
             "identificatorul utilizatorului: %d\n"
             "dimensiune: %lld\n"
             "timpul ultimei modificari: %s"
             "contorul de legaturi: %hu\n"
             "drepturi de acces user: %c%c%c\n"
             "drepturi de acces grup: %c%c%c\n"
             "drepturi de acces altii: %c%c%c\n",
             name, stp->st_uid, stp->st_size, time_buffer, stp->st_nlink,
             (stp->st_mode & S_IRUSR) ? 'R' : '-', (stp->st_mode & S_IWUSR) ? 'W' : '-',
             (stp->st_mode & S_IXUSR) ? 'X' : '-',
             (stp->st_mode & S_IRGRP) ? 'R' : '-', (stp->st_mode & S_IWGRP) ? 'W' : '-',
             (stp->st_mode & S_IXGRP) ? 'X' : '-',
             (stp->st_mode & S_IROTH) ? 'R' : '-', (stp->st_mode & S_IWOTH) ? 'W' : '-',
             (stp->st_mode & S_IXOTH) ? 'X' : '-');

    writeToOutputFile();
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Wrong number of arguments.\n");
        fprintf(stderr, "Usage: %s <input_directory> <output_directory> <c>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DIR *input_directory = opendir(argv[1]);
    if (input_directory == NULL) {
        perror("Error opening input directory");
        exit(EXIT_FAILURE);
    }

    int pid;
    long num_correct_sentences = 0;

    struct dirent *dr;
    dr = readdir(input_directory);

    char file_path[128] = "";
    struct stat fst;
    for (; dr != NULL; dr = readdir(input_directory)) {
        memset(file_path, 0, 128);
        strncat(file_path, argv[1], 128 - 1);
        strncat(file_path, "/", 128 - strlen(file_path) - 1);
        strncat(file_path, dr->d_name, 128 - strlen(file_path) - 1);
        size_t path_len = strlen(file_path);

        if (lstat(file_path, &fst) < 0) {
            perror("Error reading file stats");
            exit(EXIT_FAILURE);
        }

        time_t t = fst.st_mtime;
        struct tm *lt = localtime(&t);
        strftime(time_buffer, 32, "%c", lt);

        memset(output_path, 0, 128);
        strncat(output_path, argv[2], 128 - 1);
        strncat(output_path, "/", 128 - strlen(output_path) - 1);
        strncat(output_path, dr->d_name, 128 - strlen(output_path) - 1);
        strncat(output_path, "_", 128 - strlen(output_path) - 1);
        strncat(output_path, "stats.txt", 128 - strlen(output_path) - 1);

        int p1fd[2], p2fd[2];
        if ((pipe(p1fd) < 0) || (pipe(p2fd) < 0)) {
            perror("Eroare la crearea pipes");
            exit(EXIT_FAILURE);
        }

        if ((pid = fork()) < 0) {
            perror("Eroare la crearea procesului");
            exit(EXIT_FAILURE);
        }

        if (dr->d_name[0] != '.') {
            if (S_ISDIR(fst.st_mode)) {
                if (pid == 0) {
                    writeDirStats(dr->d_name, &fst);
                    exit(EXIT_SUCCESS);
                }
            } else if (S_ISLNK(fst.st_mode)) {
                if (pid == 0) {
                    writeLinkStats(dr->d_name, file_path, &fst);
                    exit(EXIT_SUCCESS);
                }
            } else if (file_path[path_len - 4] == '.' && file_path[path_len - 3] == 'b' &&
                       file_path[path_len - 2] == 'm' && file_path[path_len - 1] == 'p') {
                if (pid == 0) {
                    write_bmp_file_stats(dr->d_name, file_path, &fst);
                    exit(EXIT_SUCCESS);
                } else {
                    if ((pid = fork()) < 0) {
                        perror("Eroare la crearea procesului");
                        exit(EXIT_FAILURE);
                    }
                    if (pid == 0) {
                        convertBmpToGrayscale(file_path);
                        exit(EXIT_SUCCESS);
                    }
                }
            } else if (S_ISREG(fst.st_mode)) {
                if (pid == 0) {
                    writeRegularFileStats(dr->d_name, &fst);
                    close(p2fd[0]);
                    close(p2fd[1]);
                    close(p1fd[0]);
                    if (dup2(p1fd[1], 1) < 0) {
                        perror("Error redirecting stdout");
                        exit(EXIT_FAILURE);
                    }
                    execlp("cat", "cat", file_path, NULL);
                    exit(EXIT_FAILURE);
                } else {
                    if ((pid = fork()) < 0) {
                        perror("Eroare la crearea procesului");
                        exit(EXIT_FAILURE);
                    }
                    if (pid == 0) {
                        close(p1fd[1]);
                        if (dup2(p1fd[0], 0) < 0) {
                            perror("Error redirecting stdin");
                            exit(EXIT_FAILURE);
                        }
                        close(p2fd[0]);
                        if (dup2(p2fd[1], 1) < 0) {
                            perror("Error redirecting stdout");
                            exit(EXIT_FAILURE);
                        }
                        execlp("/bin/sh", "/bin/sh", "./count_sentences.sh", argv[3], NULL);
                        exit(EXIT_FAILURE);
                    }
                    close(p1fd[0]);
                    close(p1fd[1]);
                    close(p2fd[1]);
                    char sen_count[16] = "";
                    if (read(p2fd[0], sen_count, sizeof(sen_count)) < 0) {
                        perror("Error reading from pipe");
                        exit(EXIT_FAILURE);
                    }
                    num_correct_sentences += strtol(sen_count, NULL, 10);
                }
            }
        }
        close(p2fd[0]);
    }

    int status = 0;
    while ((pid = wait(&status)) > 0) {
        printf("Procesul cu PID-ul %d a fost încheiat cu codul %d\n", pid, status);
    }
    closedir(input_directory);

    printf("Au fost identificate in total %ld propozitii corecte care contin caracterul %c\n", num_correct_sentences,
           argv[3][0]);

    return 0;
}

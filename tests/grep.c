#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024

char *toLowerCase(char *str);

size_t read_line(FILE *file, char **line, size_t *line_size) {
    char buf[MAX_LINE_LENGTH];
    size_t len = 0;

    while (fgets(buf, sizeof(buf), file)) {
        size_t buf_len = strlen(buf);
        size_t new_len = len + buf_len;
        *line = realloc(*line, new_len + 1);
        if (*line == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        memcpy(*line + len, buf, buf_len);
        len = new_len;
        if (buf[buf_len - 1] == '\n') {
            break;
        }
    }
    if (len == 0 && feof(file)) {
        return 0;
    }
    (*line)[len] = '\0';
    *line_size = len;
    return len;
}

void searchInFile(const char *filePath, const char *pattern, bool ignoreCase) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("fopen failed");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    int lineNumber = 0;

    if (ignoreCase) {
        while (read_line(file, &line, &len) > 0) {
            char *line = toLowerCase(line);
            printf("searching in line: %s\n", line);
            lineNumber++;
            if (strstr(line, pattern) != NULL) {
                printf("%s - %d: %s", filePath, lineNumber, line);
                fflush(stdout);
            }
        }

        free(line);
        fclose(file);
        return;
    }
    
    while (read_line(file, &line, &len) > 0) {
        lineNumber++;
        if (strstr(line, pattern) != NULL) {
            printf("%s - %d: %s", filePath, lineNumber, line);
            fflush(stdout);
        }
    }

    free(line);
    fclose(file);
}

void traverseDirectory(const char *basePath, const char *pattern, bool ignoreCase) {
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir) {
        perror("opendir failed");
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        char path[1024];
        struct stat statbuf;

        // Skip "." and ".."
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);
        if (stat(path, &statbuf) == -1) {
            perror("stat failed");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            //printf("Entering directory: %s\n", path); // Debug print
            traverseDirectory(path, pattern, ignoreCase);
        } else if (S_ISREG(statbuf.st_mode)) {
            //printf("Searching file: %s\n", path); // Debug print
            searchInFile(path, pattern, ignoreCase);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <directory> <pattern>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *directory = argv[1];
    char *pattern;
    if(argc > 2 && strcasecmp(argv[3], "-i") == 0) {
        pattern = toLowerCase(argv[2]);
        traverseDirectory(directory, pattern, 1);
    }
    else {
        pattern = argv[2];
        traverseDirectory(directory, pattern, 0);
    }
    printf("Searching for: %s in directory: %s\n", pattern, directory);
    fflush(stdout);
    

    return EXIT_SUCCESS;
}

char *toLowerCase(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + 32;
        }
    }
    return str;
}

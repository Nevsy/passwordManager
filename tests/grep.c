#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024

char *toLowerCase(const char *str);

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

int searchInFile(const char *filePath, const char *pattern, bool ignoreCase) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    int lineNumber = 0;

    int totalInFile = 0;

    while (read_line(file, &line, &len) > 0) {
        lineNumber++;
        char *searchLine = ignoreCase ? toLowerCase(line) : strdup(line);
        char *searchPattern = ignoreCase ? toLowerCase(strdup(pattern)) : strdup(pattern);

        if (strstr(searchLine, searchPattern) != NULL) {
            printf("%s\n", filePath);
            printf("%d: %s\n", lineNumber, line);
            fflush(stdout);

            totalInFile++;
        }

        free(searchLine);
        free(searchPattern);
    }

    free(line);
    fclose(file);

    return totalInFile;
}

void traverseDirectory(const char *basePath, const char *pattern, bool ignoreCase) {
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir) {
        perror("opendir failed");
        return;
    }

    int totalMatches = 0;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == -1) {
            perror("stat failed");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            traverseDirectory(path, pattern, ignoreCase);
        } else if (S_ISREG(statbuf.st_mode)) {
            totalMatches += searchInFile(path, pattern, ignoreCase);
        }
    }

    closedir(dir);

    printf("Found %d matches\n", totalMatches);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <pattern> <directory> [-i]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *pattern = argv[1];
    const char *directory = argv[2];
    bool ignoreCase = false;

    if (argc > 3 && strcmp(argv[3], "-i") == 0) {
        ignoreCase = true;
    }

    printf("Searching for: %s in directory: %s (Case %ssensitive)\n", 
           pattern, directory, ignoreCase ? "in" : "");
    fflush(stdout);

    traverseDirectory(directory, pattern, ignoreCase);

    return EXIT_SUCCESS;
}

char *toLowerCase(const char *str) {
    char *result = strdup(str);
    for (int i = 0; result[i]; i++) {
        result[i] = tolower((unsigned char)result[i]);
    }
    return result;
}

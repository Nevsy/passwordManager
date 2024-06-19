#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <sys/stat.h>

void create_directory(const char* path) {
    if (CreateDirectory(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        printf("Directory created or already exists: %s\n", path);
    } else {
        printf("Error creating directory: %s\n", path);
    }
}

int main(int argc, char *argv[]){
    char executable_path[MAX_PATH];
    GetModuleFileName(NULL, executable_path, sizeof(executable_path));
    char *last_backslash = strrchr(executable_path, '\\');
    if (last_backslash != NULL) {
        *last_backslash = '\0';
    }

    char passwords_folder[MAX_PATH];
    snprintf(passwords_folder, sizeof(passwords_folder), "%s\\passwords", executable_path);
    printf("%s", passwords_folder);

    struct stat st;
    if (stat(passwords_folder, &st) == 0) {
        printf("folder already exists\n");
    }
    else {
        create_directory(passwords_folder);
    }
}
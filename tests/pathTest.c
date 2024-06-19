// #include <stdio.h>
// #include <stdlib.h>

// int main(int argc, char **argv)
// {
//     const char* s = getenv("PATH");

//     // If the environment variable doesn't exist, it returns NULL
//     printf("%s\n", (s != NULL) ? s : "getenv returned NULL");

//     printf("%s", argv[0]);
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define ENV_VAR_NAME "PASSWORDS_DIR"

void set_default_passwords_folder() {
    printf("setting default\n");
    char executable_path[MAX_PATH];
    GetModuleFileName(NULL, executable_path, sizeof(executable_path));
    char *last_backslash = strrchr(executable_path, '\\');
    if (last_backslash != NULL) {
        *last_backslash = '\0';
    }

    char passwords_folder[MAX_PATH];
    snprintf(passwords_folder, sizeof(passwords_folder), "%s\\passwords", executable_path);
    SetEnvironmentVariable(ENV_VAR_NAME, passwords_folder);
}

void get_passwords_folder(char *buffer, size_t buffer_size) {
    DWORD size = GetEnvironmentVariable(ENV_VAR_NAME, buffer, buffer_size);
    if (size == 0) {
        // Environment variable not set, setting it to default location
        set_default_passwords_folder();
        size = GetEnvironmentVariable(ENV_VAR_NAME, buffer, buffer_size);
        if (size == 0) {
            // Handle error
            fprintf(stderr, "Error getting passwords folder path\n");
            exit(EXIT_FAILURE);
        }
    }
}

int main() {
    char passwords_folder[MAX_PATH];
    printf("Passwords folder: %s\n", passwords_folder);
    get_passwords_folder(passwords_folder, sizeof(passwords_folder));
    printf("Passwords folder: %s\n", passwords_folder);


    // You can now use passwords_folder to read/write passwords
    // ...

    return 0;
}

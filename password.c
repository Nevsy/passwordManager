#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <sys/stat.h> // Check if file extists
#include <openssl/ssl.h>
#include <curses.h>

#define MAXNODES 2048
#define MAXNAME 256
#define MAX_PASSWORD_LENGTH 10000

void showPassword(int argc, char *argv[]);
int addPassword(int argc, char *argv[], int random);

char *randomPassword(int len);
void CopyToClipboard(const char* str);

int main(int argc, char *argv[]) {
    int success;
    if (argc > 1 && argc < 4) {
        if (!strcasecmp(argv[1], "show")) {
            showPassword(argc, argv);
        } else if (!strcasecmp(argv[1], "gen") || !strcasecmp(argv[1], "generate")) {
            success = addPassword(argc, argv, 1);
        } else if (!strcasecmp(argv[1], "add")) {
            success = addPassword(argc, argv, 0);
        } else if (!strcasecmp(argv[1], "-h") || !strcasecmp(argv[1], "--help")) {
            printf("Choose an option\n");
            printf("-- show {{name of service}} (optional -a) show a password (and additional metadata)\n");
            printf("-- add {{name of the service}} (metadata)\n");
            return 0;
        } else {
            printf("1st argument invalid\n");
            return 0;
        }
    } else {
        printf("1 Argument expected");
        return 1;
    }

    if(success != 0) {
        printf("Error when adding password\n");
    }
    return 0;
}

int addPassword(int argc, char *argv[], int random) {
    if (argc < 3) {
        printf("Service name is required\n");
        return 1;
    }

    if (argv[2] == NULL) {
        printf("argv 3 is null\n");
        return 1;
    }

    char *key = (char *)malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    if (random) {
        int length;
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srand((time_t)ts.tv_nsec);
        length = 20 + rand() % 10;  // length gets values between 20 and 30
        key = randomPassword(length);
    } else {
        printf("Enter password: ");
        fflush(stdout);
        int ch;
        int i = 0;
        while ((ch = getch()) != '\n' && i < MAX_PASSWORD_LENGTH) {
            key[i++] = ch;
        }
        key[i] = '\0'; // Null-terminate the string

        addstr("Password entered: ");
        addstr(key); // Display a confirmation message
        refresh();
        getch(); // Wait for a key press


        printf("Confirm password: ");
        fflush(stdout);
        char *confirmPassword = (char *)malloc(MAX_PASSWORD_LENGTH * sizeof(char));
        // Get user input without echoing
        while ((ch = getch()) != '\n' && i < MAX_PASSWORD_LENGTH) {
            confirmPassword[i++] = ch;
        }
        confirmPassword[i] = '\0'; // Null-terminate the string

        addstr("Password entered: ");
        addstr(confirmPassword); // Display a confirmation message
        refresh();
        getch(); // Wait for a key press
        endwin(); // Clean up and exit ncurses

        if(strncmp(confirmPassword, key, MAX_PASSWORD_LENGTH)) {
            printf("Passwords do not match\n");
            return 1;
        }

        key[strcspn(key, "\n")] = 0;  // remove newline character
        if (strlen(key) > MAX_PASSWORD_LENGTH) {
            printf("Password too long\n");
            return 1;
        }
    }

    char filePath[MAXNAME];
    snprintf(filePath, MAXNAME, "C:\\Users\\svenroyer\\OneDrive - Kolanden\\Documenten\\Home\\Code\\LowLevel\\passwordCLA\\passwords\\%s.txt", argv[2]);

    struct stat st;
    if (stat(filePath, &st) == 0) {
        printf("File already exists\n");
        return 1;
    }

    FILE *thisPasswordFile = fopen(filePath, "a+");
    if (thisPasswordFile == NULL) {
        printf("Error when opening file!\n");
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(thisPasswordFile, "%s\n", key);

    free(key);
    fclose(thisPasswordFile);
    return 0;
}

void showPassword(int argc, char *argv[]) {
    // implement showPassword function
}

char *randomPassword(int len) {
    char characters[33] = "~`!@#$\%^&*()_-+={[}]|\\:;\"'<,>.?/";
    char majuscule[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char miniscule[27] = "abcdefghijklmnopqrstuvwxyz";

    char allChars[200];
    allChars[0] = '\0';
    strcat(allChars, characters);
    strcat(allChars, majuscule);
    strcat(allChars, miniscule);

    int charsLen = strlen(allChars);

    char *password = (char *)malloc((len + 1) * sizeof(char)); // Allocate memory for password
    if (password == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }
    password[0] = '\0';

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    srand(ts.tv_nsec);

    for (int i = 0; i < len; i++) {
        char randomChar = allChars[rand() % charsLen];
        sprintf(password + i, "%c", randomChar);
    }

    printf("%s\n", password);
    //CopyToClipboard(password); currently not copying to clipboard
    return password;
}

void CopyToClipboard(const char* str) {
    const size_t len = strlen(str) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hMem != NULL) {
        memcpy(GlobalLock(hMem), str, len);
        GlobalUnlock(hMem);
        OpenClipboard(NULL);
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
    }
}


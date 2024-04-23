#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <sys/stat.h> // Check if file extists
#include <openssl/ssl.h>
#include "curses.h"

#define MAX_NAME_LENGTH 256
#define MAX_LINE_LENGTH 1025

#define MAXNODES 2048
#define MAXNAME 256
#define MAX_PASSWORD_LENGTH 1024

int showPassword(int argc, char *argv[]);
int addPassword(int argc, char *argv[], int random);

char *randomPassword(int len);
void CopyToClipboard(const char* str);

int main(int argc, char *argv[]) {
    // MARK: CLA
    int addPasswordSuccess = 0;
    int showPasswordSuccess = 0;
    if (argc > 1 && argc < 4) {
        if (!strcasecmp(argv[1], "show")) {
            showPasswordSuccess = showPassword(argc, argv);
        } else if (!strcasecmp(argv[1], "gen") || !strcasecmp(argv[1], "generate")) {
            addPasswordSuccess = addPassword(argc, argv, 1);
        } else if (!strcasecmp(argv[1], "add")) {
            addPasswordSuccess = addPassword(argc, argv, 0);
        } else if (!strcasecmp(argv[1], "-h") || !strcasecmp(argv[1], "--help")) {
            printf("Choose an option:\n");
            printf("--> password.exe show <name of service> (optional -a) \n\t--> show a password (and additional metadata)\n");
            printf("--> password.exe add <name of the service> (metadata) \n\t--> add a password (and add metadata)\n");
            printf("--> password.exe gen / generate <name of the service> (metadata) \n\t--> generate a password and add it (and add metadata)\n");
            return 0;
        } else {
            printf("1st argument invalid\n");
            return 0;
        }
    } else {
        printf("1 Argument expected");
        return 1;
    }

    if(addPasswordSuccess != 0) {
        printf("Error when adding password\n");
    }
    if(showPasswordSuccess != 0) {
        printf("Error when showing password\n");
    }
    return 0;
}

// MARK: Add
int addPassword(int argc, char *argv[], int random) {
    if (argc < 3) {
        printf("Service name is required\n");
        return 1;
    }

    if (argv[2] == NULL) {
        printf("argv 3 is null\n");
        return 1;
    }

    char argumentToLowerCase[MAXNAME];
    for (int i = 0; i < strlen(argv[2]); i++) {
        argumentToLowerCase[i] = tolower(argv[2][i]);
    }
    argumentToLowerCase[strlen(argv[2])] = '\0';

    char filePath[MAXNAME];
    snprintf(filePath, MAXNAME, "./passwords/%s.txt", argumentToLowerCase);

    struct stat st;
    if (stat(filePath, &st) == 0) {
        printf("File already exists\n");
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
        
        // TODO: Only copy password if -c if called
        CopyToClipboard(key); // Copy password to clipboard
    } else { 
        // MARK: User input
        initscr();
        addstr("Enter password: ");
        refresh();
        // do not show user input in terminal
        noecho(); 
        int i = 0;
        int ch;
        while ((ch = getch()) != '\n' && i < MAX_PASSWORD_LENGTH) {
            key[i++] = ch;
        }
        key[i] = '\0'; // Null-terminate the string

        addstr("\n");

        char *confirmPassword = (char *)malloc(MAX_PASSWORD_LENGTH * sizeof(char));
        addstr("Confirm password: ");
        refresh();
        i = 0;
        while ((ch = getch()) != '\n' && i < MAX_PASSWORD_LENGTH) {
            confirmPassword[i++] = ch;
        }
        confirmPassword[i] = '\0'; // Null-terminate the string

        //addstr("Password entered: "); // Would be kinda nonsence outputting the password but it's possible
        //addstr(confirmPassword); // Display a confirmation message
        //refresh();
        //getch(); // Wait for a key press
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

        printf("Password added successfully\n");
    }

    // MARK: Write to file
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

// MARK: Show
int showPassword(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s show <filename>\n", argv[0]);
        return 1;
    }

    char argumentToLowerCase[MAXNAME];
    for (int i = 0; i < strlen(argv[2]); i++) {
        argumentToLowerCase[i] = tolower(argv[2][i]);
    }
    argumentToLowerCase[strlen(argv[2])] = '\0';

    //Construct the search path
    char searchPath[MAX_NAME_LENGTH + 12];
    sprintf(searchPath, ".\\passwords\\%s.txt", argumentToLowerCase);
    // Search for the file
    struct stat st;
    if (stat(searchPath, &st) != 0) {
        printf("File not found\n");
        return 1;
    }
    else {
        FILE *file = fopen(searchPath, "r");
        if (file == NULL) {
            printf("Error opening file\n");
            return 1;
        }
        char line[MAX_LINE_LENGTH];
        if (fgets(line, MAX_LINE_LENGTH, file) == NULL) { // Read a single line if it's not empty
            printf("File is empty\n");
            fclose(file);
            return 1;
        }
        printf("%s", line);
        fclose(file);
    }

    return 0;
}

// MARK: Random
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

// MARK: Copy Clip
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

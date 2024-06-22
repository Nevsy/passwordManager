/***************************************/
/*          PASSWORD MANAGER           */
/***************************************/

/*
* TODO:
    - Grep
    - General: Move all 'path' searching to functions
    - Add: password strength indicator
    - Gen: Cryptographically secure random number
    - Gen: which characters
    - ...
    - Encryption?
    - Speed dial?
    - History?
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <sys/stat.h> // Check if file extists
#include <getopt.h> // Command line arguments

#include "curses.h" // Hidden input
// -L. -lpdcurses

// constants
#define MAX_NAME_LENGTH 256
#define MAX_LINE_LENGTH 1024

#define MAXNAME 256
#define MAX_PASSWORD_LENGTH 1023 // 1024 - 1 for \0

typedef struct Flags {
    bool m; // add metadata / show metadata
    bool metaDataAdd;
    bool c; // copy to clipboard
    bool h; // help
    bool a; // add lines to metadata
    bool r; // replace lines in metadata
    char metadata[MAX_LINE_LENGTH];
}Flags;

struct Flags flags = {0, 0, 0, 0, 0, 0, ""};

// main functions
int showPassword(int argc, char *argv[]);
int addPassword(int argc, char *argv[], int random);
int editPassword(int argc, char *argv[]);
int deletePassword(int argc, char *argv[]);
int lsDir(int argc, char *argv[]);

// text/file manipulation
int writeToFile(FILE *file, char *key);
void deleteTextExceptFirstLine(char *filePath);
size_t read_line(FILE *file, char **line, size_t *line_size);

// Get file data
char *findFile(int argc, char *argv[]);
void create_directory(const char* path);
char *getLocationOfExecution(void);
int initPasswordDirectory(const char *passwordsFolder);

// helper functions
char *randomPassword(int len);
char *toLowerCase(char *str);
void CopyToClipboard(const char* str);


// CLA reader
bool *checkFlags(int argc, char *argv[], Flags *flags);

int main(int argc, char *argv[]) {
    // MARK: CLA
    
    int addPasswordSuccess = 0;
    int showPasswordSuccess = 0;
    int editPasswordSuccess = 0;
    int deletePasswordSuccess = 0;
    
    if (argc > 1 && argc < 7) {
        bool *arrayDo = checkFlags(argc, argv, &flags);

        if (arrayDo[0]) {
            char *path = findFile(argc, argv);
            printf("%s", path);
            showPasswordSuccess = showPassword(argc, argv);
        } else if (arrayDo[1]) {
            addPasswordSuccess = addPassword(argc, argv, 1);
        } else if (arrayDo[2]) {
            addPasswordSuccess = addPassword(argc, argv, 0);
        } else if (arrayDo[3]){
            editPasswordSuccess = editPassword(argc, argv);
        } else if (arrayDo[4]) {
            deletePasswordSuccess = deletePassword(argc, argv);
        } else if (arrayDo[5]) {
            lsDir(argc, argv);
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
    if(editPasswordSuccess != 0){
        printf("Error when editing password\n");
    }
    if(deletePasswordSuccess != 0){
        printf("Error when deleting password\n");
    }
    return 0;
}

// MARK: check flags
bool *checkFlags(int argc, char *argv[], Flags *flags){
    int argcCopy = argc;
    char **argvCopy = malloc(argc * sizeof(char *));
    for (int i = 0; i < argc; i++) {
        argvCopy[i] = strdup(argv[i]);
    }

    int c;
    static struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    bool isFirstArgShow = (argcCopy > 1 && strcasecmp(argvCopy[1], "show") == 0);
    bool isFirstArgEdit = (argcCopy > 1 && strcasecmp(argvCopy[1], "edit") == 0);
    bool isFirstArgGen = (argcCopy > 1 && strcasecmp(argvCopy[1], "gen") == 0 || strcasecmp(argvCopy[1], "generate") == 0);
    bool isFirstArgAdd = (argcCopy > 1 && strcasecmp(argvCopy[1], "add") == 0);
    bool isFirstArgDelete = (argcCopy > 1 && strcasecmp(argvCopy[1], "delete") == 0 || strcasecmp(argvCopy[1], "del") == 0);
    bool isFirstArgLs = (argcCopy > 1 && strcasecmp(argvCopy[1], "ls") == 0);

    bool *arrayDo = malloc(6 * sizeof(bool));

    for (int i = 0; i < 5; i++) {
        arrayDo[i] = false;
    }

    if(isFirstArgShow) arrayDo[0] = true;
    if(isFirstArgGen) arrayDo[1] = true;
    if(isFirstArgAdd) arrayDo[2] = true;
    if(isFirstArgEdit) arrayDo[3] = true;
    if(isFirstArgDelete) arrayDo[4] = true;
    if(isFirstArgLs) arrayDo[5] = true;

    while ((c = getopt_long(argcCopy, argvCopy, "ciarm::h", long_options, NULL)) != -1) {
        switch (c) {
            case 'c':
                if(!isFirstArgShow && !isFirstArgGen && !isFirstArgAdd) {
                    printf("Found -c flag without the add or generate command\n");
                    printf("Use: ./password add <name> [-c] [-m] <data in double quotes>");
                    printf("Use: ./password gen | generate <name> [-c] [-m] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                }
                //printf("Found -c flag\n");
                flags->c = true;
                break;
           case 'm':
                if (optind < argcCopy && argvCopy[optind][0] != '-') {
                    if(isFirstArgShow || isFirstArgDelete) {
                        printf("Found -m flag with argument: %s\n", argvCopy[optind]);
                        printf("Use: ./password show <name> [-m]");
                        fflush(stdout);
                        exit(1);
                    }
                    //printf("Found -m flag with argument: %s\n", argvCopy[optind]);
                    strncpy(flags->metadata, argvCopy[optind], MAX_LINE_LENGTH);
                    flags->metaDataAdd = true;
                    optind++;
                } else {
                    if(!isFirstArgShow && !isFirstArgDelete) {
                        printf("Found -m flag without argument\n");
                        printf("Use: ./password show <name> [-m] <metadata>");
                        fflush(stdout);
                        exit(1);
                    }
                    flags->metadata[0] = '\0';
                }
                flags->m = true;
                break;
            case 'h':
                //printf("Found --help flag\n");
                printf("Choose an option:\n");
                printf("--> password.exe show <name> [-m | -c] \n\t--> show a password (and metadata, copy to clipboard and don't show in terminal)\n");
                printf("--> password.exe add <name> [-m] <metadata> [-c] \n\t--> add a password (and add metadata)\n");
                printf("--> password.exe gen | generate <name> <metadata> \n\t--> generate a password and add it (and add metadata)\n");
                printf("--> password.exe edit <name> [-a | -r] <metadata> \n\t--> edit a password (a: add inline, r: replace with, auto: open vim)\n");
                printf("--> password.exe delete <name> [-m] \n\t--> delete a password, or only the metadata\n");
                printf("--> password.exe ls \n\t--> list all passwords\n");
                fflush(stdout);
                exit(0);
                flags->h = true;
                break;
            case 'a':
                if(!isFirstArgEdit) {
                    printf("Found -a flag without the edit command\n");
                    printf("Use: ./password edit <name> [-a | -r] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                }

                if (optind < argcCopy && argvCopy[optind][0] != '-') {
                    //printf("Found -a flag with argument: %s\n", argvCopy[optind]);
                    strncpy(flags->metadata, argvCopy[optind], MAX_LINE_LENGTH);
                    flags->metaDataAdd = true;
                    optind++;
                } else {
                    printf("Found -a flag without argument\n");
                    printf("Use: ./password edit <file> [-a | -r] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                    flags->metadata[0] = '\0';
                }
                
                flags->a = true;
                break;
            case 'r':
                if(!isFirstArgEdit) {
                    printf("Found -r flag without the edit command\n");
                    printf("Use: ./password edit <name> [-a | -r] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                }

                if (optind < argcCopy && argvCopy[optind][0] != '-') {
                    //printf("Found -r flag with argument: %s\n", argvCopy[optind]);
                    strncpy(flags->metadata, argvCopy[optind], MAX_LINE_LENGTH);
                    flags->metaDataAdd = true;
                    optind++;
                } else {
                    printf("Found -r flag without argument\n");
                    printf("Use: ./password edit <name> [-a | -r] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                    flags->metadata[0] = '\0';
                }

                flags->r = true;
                break;
            case '?':
                printf("Unknown option: %c\n", optopt);
                perror("Error when reading flags\n");
                exit(EXIT_FAILURE);
                break;
        }
    }

    for (int i = 0; i < argcCopy; i++) {
        free(argvCopy[i]);
    }
    free(argvCopy);

    return arrayDo;
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

    if(flags.m && !flags.metaDataAdd){
        printf("Use: ./password <name> [-m] <data in double quotes>\n");
        fflush(stdout);
        return 1;
    }

    char *argumentToLowerCase = toLowerCase(argv[2]);
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
        // struct timespec ts;
        // clock_gettime(CLOCK_MONOTONIC, &ts);
        // srand((time_t)ts.tv_nsec);
        
        LARGE_INTEGER frequency, counter;
        if (!QueryPerformanceFrequency(&frequency)) {
            printf("QueryPerformanceFrequency failed.\n");
            exit(1);
        }
        if (!QueryPerformanceCounter(&counter)) {
            printf("QueryPerformanceFrequency failed.\n");
            exit(1);
        }

        // Use the high-resolution counter value as the seed for the random number generator
        srand((unsigned int)(counter.QuadPart % UINT_MAX));
        length = 20 + rand() % 10;  // length gets values between 20 and 30
        key = randomPassword(length);
    } else { 
        // MARK: ADD - Input
        initscr();
        addstr("Enter password: ");
        refresh();
        // Do not show user input in terminal
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

        // Would be kinda nonsence outputting the password but it's possible
        //addstr("Password entered: "); 
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
    }

    // MARK: ADD - Write
    FILE *thisPasswordFile = fopen(filePath, "a+");
    if (thisPasswordFile == NULL) {
        printf("Error when opening file!\n");
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    writeToFile(thisPasswordFile, key);

    printf("Password added successfully\n");
    fflush(stdout);

    free(key);
    fclose(thisPasswordFile);
    return 0;
}

// MARK: Show
int showPassword(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s show <name>\n", argv[0]);
        fflush(stdout);
        return 1;
    }

    //Construct the search path
    char *argumentToLowerCase = toLowerCase(argv[2]);
    char searchPath[MAX_NAME_LENGTH + 12];
    sprintf(searchPath, "./passwords/%s.txt", argumentToLowerCase);
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

        if(flags.c){
            if(!strncasecmp(line, "Google", 6)){
                printf("Google");
            }
            else{
                CopyToClipboard(line);
            }
        }
        else{
            printf("%s", line);
        }

        if(flags.m){
            while(fgets(line, MAX_LINE_LENGTH, file) != NULL){
                printf("%s", line);
            }
        }

        fclose(file);
    }

    return 0;
}

// MARK: Edit
int editPassword(int argc, char *argv[]){
    char *argumentToLowerCase = toLowerCase(argv[2]);
    char searchPath[MAX_NAME_LENGTH + 12];
    sprintf(searchPath, "./passwords/%s.txt", argumentToLowerCase);

    struct stat st;
    FILE *file;
    if (stat(searchPath, &st) != 0) { // if file doesn't exist
        fprintf(stderr, "File not found\n");
        exit(EXIT_FAILURE);
    }
    
    if (!flags.a && !flags.r) {
        const char *command = "vim"; // or "vi"
        char cmd[100];
        snprintf(cmd, sizeof(cmd), "%s %s", command, searchPath);

        // Call Vi/Vim using system()
        if (system(cmd) == -1) {
            perror("Error executing Vi/Vim");
            return EXIT_FAILURE;
        }
    }
    else {
        file = fopen(searchPath, "a");
        if (file == NULL) {
            fprintf(stderr, "Error opening the file\n");
            exit(EXIT_FAILURE);
        }

        if(flags.r){
            deleteTextExceptFirstLine(searchPath);
        }
        else if(flags.a){
            // do nothing
        }

        fprintf(file, "%s", flags.metadata);
        fprintf(file, "%s", "\n");
    }

    fclose(file);

    printf("File '%s' edited successfully.\n", argv[2]);

    return 0;
}

// MARK: Delete
int deletePassword(int argc, char *argv[]) {
    char *argumentToLowerCase = toLowerCase(argv[2]);
    char searchPath[MAX_NAME_LENGTH + 12];
    sprintf(searchPath, "./passwords/%s.txt", argumentToLowerCase);

    if(flags.m){
        deleteTextExceptFirstLine(searchPath);

        printf("Metadata of '%s' deleted successfully.\n", argv[2]);
    }
    else {
        if (remove(searchPath) != 0) {
            perror("Error deleting file");
            return EXIT_FAILURE;
        }

        printf("File '%s' deleted successfully.\n", argv[2]);
    }

    return 0;
}

// MARK: List
int lsDir(int argc, char *argv[]) {
    {
        // creating an isolated scope for dirent.h to not bloat
        #include <dirent.h>

        DIR *dir;
        struct dirent *entry;

        // Open the directory
        dir = opendir("./passwords");
        if (dir == NULL) {
            perror("Error opening directory");
            return EXIT_FAILURE;
        }

        // Traverse the directory
        while ((entry = readdir(dir)) != NULL) {
            if(entry->d_name[0] != '.'){
                // removing the extension
                char *dot = strchr(entry->d_name, '.');
                if (dot != NULL) {
                    *dot = '\0';
                }
                printf("%s\n", entry->d_name);
            }
        }

        // Close the directory
        closedir(dir);
    } // dirent.h is no longer in scope here

    return EXIT_SUCCESS;
}

// MARK: Write
int writeToFile(FILE *file, char *key){
    // Check if -m flag is present AND metadata has been provided, already done in the flags function
    if(flags.m){
        fprintf(file, "%s\n", key);
        fprintf(file, "%s\n", flags.metadata);
    }
    else {
        fprintf(file, "%s\n", key);
    }

    // Copy password to clipboard
    if(flags.c){
        CopyToClipboard(key);
    }
    else{
        printf("%s", key);
        printf("\n");
    }
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

    // char *password = (char *)malloc((len + 1) * sizeof(char)); // Allocate memory for password
    // if (password == NULL) {
    //     printf("Memory allocation failed.\n");
    //     return NULL;
    // }
    // password[0] = '\0';

    // struct timespec ts;
    // clock_gettime(CLOCK_REALTIME, &ts);
    // srand(ts.tv_nsec);

    LARGE_INTEGER frequency, counter;
    if (!QueryPerformanceFrequency(&frequency)) {
        printf("QueryPerformanceFrequency failed.\n");
        free(password);
        return NULL;
    }
    if (!QueryPerformanceCounter(&counter)) {
        printf("QueryPerformanceCounter failed.\n");
        free(password);
        return NULL;
    }

    // Use the high-resolution counter value as the seed for the random number generator
    srand((unsigned int)(counter.QuadPart % UINT_MAX));

    for (int i = 0; i < len; i++) {
        char randomChar = allChars[rand() % charsLen];
        sprintf(password + i, "%c", randomChar);
    }

    return password;
}

// MARK: Delete w/o 1st line
void deleteTextExceptFirstLine(char *filename) {
    FILE *file;
    char *buffer = NULL;
    size_t bufferSize = 0;
    ssize_t bytesRead;
    size_t firstLineEnd;

    // Open the file in read mode
    file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Error opening file.\n");
        exit(EXIT_FAILURE);
    }

    // Read the entire file into the buffer
    bytesRead = read_line(file, &buffer, &bufferSize);

    if (bytesRead == -1) {
        fprintf(stderr, "Error reading file.\n");
        fclose(file);
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Find the end of the first line
    firstLineEnd = strcspn(buffer, "\n");

    // Close the file
    fclose(file);

    // Open the file in write mode to truncate it
    file = fopen(filename, "w");

    if (file == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Write the first line back to the file
    fwrite(buffer, 1, firstLineEnd + 1, file);

    // Close the file
    fclose(file);

    // Free the buffer
    free(buffer);
}

// MARK: ToLowerCase
char *toLowerCase(char *str) {
    char *lowerCase = (char *)malloc(strlen(str) + 1);
    if (lowerCase == NULL) {
        printf("Memory allocation failed.\n");
        fflush(stdout);
        return NULL;
    }

    for (int i = 0; i < strlen(str); i++) {
        lowerCase[i] = tolower(str[i]);
    }
    lowerCase[strlen(str)] = '\0';

    return lowerCase;
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

// MARK: Read line
size_t read_line(FILE *file, char **line, size_t *line_size) {
    char buf[MAX_LINE_LENGTH];
    size_t len = 0;

    while (fgets(buf, sizeof(buf), file)) {
        size_t new_len = len + strlen(buf);
        *line = realloc(*line, new_len + 1);
        if (*line == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        memcpy(*line + len, buf, strlen(buf));
        len = new_len;
        if (buf[strlen(buf) - 1] == '\n') {
            break;
        }
    }
    (*line)[len] = '\0';
    *line_size = len;
    return len;
}

// MARK: Find File
char *findFile(int argc, char *argv[]){
    char *locationOfExecution = getLocationOfExecution();
    if (locationOfExecution == NULL) {
        perror("getLocationOfExecution");
        exit(EXIT_FAILURE);
    }

    char passwordsFolder[MAX_PATH];
    snprintf(passwordsFolder, sizeof(passwordsFolder), "%s\\passwords", locationOfExecution);
    free(locationOfExecution);

    // Check if directory exists
    if (!initPasswordDirectory(passwordsFolder)) {
        printf("No passwords directory was found, created one\n");
        fflush(stdout);
    }

    // Construct the specific path to file
    char *argumentToLowerCase = toLowerCase(argv[2]);
    char *searchPath = (char *)malloc(MAX_PATH);
    //snprintf(searchPath, sizeof(searchPath), "%s\\%s.txt", passwordsFolder, argumentToLowerCase);
    strncat(searchPath, passwordsFolder, MAX_PATH-100);
    strcat(searchPath, "\\");
    strncat(searchPath, argumentToLowerCase, 100);
    strcat(searchPath, ".txt");
    free(argumentToLowerCase);

    struct stat st;
    if (stat(searchPath, &st) != 0) { // if file doesn't exist
        fprintf(stderr, "File not found: %s\n", searchPath);
        fprintf(stderr, "arg: %s\n", argumentToLowerCase);
        fprintf(stderr, "locationOfExe: %s\n", passwordsFolder);
        exit(EXIT_FAILURE);
    }

    return searchPath;
}

// MARK: init pwd dir
int initPasswordDirectory(const char *passwordsFolder) {
    struct stat st;
    if (stat(passwordsFolder, &st) == 0 && (st.st_mode & S_IFDIR)) {
        printf("Folder already exists\n");
        return 1;
    } else {
        create_directory(passwordsFolder);
        return 0;
    }
}

// MARK: get loc of exe 
char *getLocationOfExecution(void) {
    char executablePath[MAX_PATH];
    GetModuleFileName(NULL, executablePath, sizeof(executablePath));
    char *lastBackslash = strrchr(executablePath, '\\');
    if (lastBackslash != NULL) {
        *lastBackslash = '\0';
    }
    return strdup(executablePath);
}

// MARK: Mkdir
void create_directory(const char* path) {
    if (CreateDirectory(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        printf("Directory created or already exists: %s\n", path);
    } else {
        printf("Error creating directory: %s\n", path);
    }
}
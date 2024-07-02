/***************************************/
/*          PASSWORD MANAGER           */
/***************************************/

/*
* TODO:
    - Gen: Which characters
    - ...
    - Add: password strength indicator
    - Overall: recovery codes reader
    - Gen: Cryptographically secure random number
    - ...
    - Encryption?
    - Speed dial?
    - History?
*/

#include <dirent.h> // Directory management
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
#define MAX_NAME_LENGTH 100
#define MAX_LINE_LENGTH 1024 // -> Max password length will be MAX_LINE_LENGTH - 1
#define INT_LENGTH 10

typedef struct Flags {
    bool m; // add metadata / show metadata
    bool metaDataAdd;
    bool c; // copy to clipboard
    bool h; // help
    bool a; // add lines to metadata
    bool r; // replace lines in metadata
    bool i; // replace lines in metadata
    char metadata[MAX_LINE_LENGTH];
    int genLength;
}Flags;

struct Flags flags = {0, 0, 0, 0, 0, 0, 0, "", 0};

typedef enum actionType {
    UNDEF,
    SHOW,
    ADD,
    GEN,
    EDIT,
    RM,
    LS,
    GREP
} actionType;

// main functions
int showPassword(char *path);
int addPassword(char *filePath, int random);
int editPassword(char *filePath, char *argumentToLowerCase);
int deletePassword(char *filePath, char *argumentToLowerCase);
int lsDir(const char *pathDir);

// text/file manipulation
int writeToFile(FILE *file, char *key);
void deleteTextExceptFirstLine(char *filePath);
size_t read_line(FILE *file, char **line, size_t *line_size);

// Get file data
char *findFile(int argc, char *argv[], bool checkIfExists);
void create_directory(const char* path);
char *getLocationOfExecution(void);
int initPasswordDirectory(const char *passwordsFolder);
char *findDir(void);
char *constructFilePath(char *passwordsFolder, char *fileName);

// grepping
int grep(char *pattern, char *directory);
int searchInFile(const char *filePath, const char *pattern, bool ignoreCase);
void traverseDirectory(const char *basePath, const char *pattern, bool ignoreCase);

// helper functions
char *randomPassword(int len);
char *toLowerCase(const char *str);
void CopyToClipboard(const char* str);

// CLA reader
actionType checkFlags(int argc, char *argv[], Flags *flags);


// MARK: MAIN
int main(int argc, char *argv[]) {
    int addPasswordSuccess = 0;
    int showPasswordSuccess = 0;
    int editPasswordSuccess = 0;
    int deletePasswordSuccess = 0;
    
    if (argc > 1 && argc < 7) {
        actionType action = checkFlags(argc, argv, &flags);

        char *fileSecondArg;
        char *directory;
        char *argumentToLowerCase;
        if(action == SHOW || action == EDIT || action == RM) {
            argumentToLowerCase = toLowerCase(argv[2]);
            fileSecondArg = findFile(argc, argv, true);
        }
        else if (action == ADD || action == GEN){
            fileSecondArg = findFile(argc, argv, false);
        }
        else if(action == GREP || action == LS) {
            directory = findDir();
        }
        else if(action == UNDEF) {
            fprintf(stderr, "No function defined");
            exit(1);
        }


        switch (action) {
            case SHOW: {
                showPasswordSuccess = showPassword(fileSecondArg);
                break;
            }
            case ADD:  {
                addPasswordSuccess = addPassword(fileSecondArg, 0);
                break;
            }
            case GEN:  {
                addPasswordSuccess = addPassword(fileSecondArg, 1);
                break;
            }
            case EDIT: {
                editPasswordSuccess = editPassword(fileSecondArg, argumentToLowerCase);
                break;
            }
            case RM: {
                deletePasswordSuccess = deletePassword(fileSecondArg, argumentToLowerCase);
                break;
            }
            case LS: {
                lsDir(directory);
                break;
            }
            case GREP: {
                if(argc < 3) {
                    printf("Usage: %s grep <pattern> [-i | case insensitive]\n", argv[0]);
                }

                char *pattern = argv[2];
                grep(pattern, directory);
                break;
            }
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
actionType checkFlags(int argc, char *argv[], Flags *flags){
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

    actionType action = UNDEF;
    if(argcCopy > 1){
        if((strcasecmp(argvCopy[1], "show") == 0)) action = SHOW;
        else if(strcasecmp(argvCopy[1], "edit") == 0) action = EDIT;
        else if(strcasecmp(argvCopy[1], "add") == 0) action = ADD;
        else if(strcasecmp(argvCopy[1], "delete") == 0 || strcasecmp(argvCopy[1], "del") == 0 || strcasecmp(argvCopy[1], "rm") == 0) action = RM;
        else if(strcasecmp(argvCopy[1], "ls") == 0) action = LS;
        else if(strcasecmp(argvCopy[1], "gen") == 0 || strcasecmp(argvCopy[1], "generate") == 0) action = GEN;
        else if(strcasecmp(argvCopy[1], "grep") == 0) action = GREP;
    }
    else {
        fprintf(stderr, "Expected 1 argument, see 'pass -h' for help");
        exit(1);
    }

    while ((c = getopt_long(argcCopy, argvCopy, "ciarlm::h", long_options, NULL)) != -1) {
        switch (c) {
            case 'c':
                if(!(action == ADD || action == GEN || action == SHOW)) {
                    printf("Found -c flag without the add, generate or show command\n");
                    printf("Use: pass add <name> [-c] [-m] <data in double quotes>");
                    printf("Use: pass gen | generate <name> [-c] [-m] <data in double quotes>");
                    printf("--> password.exe show <name> [-m | -c] \n\t--> show a password (and metadata, copy to clipboard and don't show in terminal)\n");
                    fflush(stdout);
                    exit(0);
                }
                //printf("Found -c flag\n");
                flags->c = true;
                break;
           case 'm':
                if (optind < argcCopy && argvCopy[optind][0] != '-') {
                    if(action == SHOW || action == RM) {
                        printf("Found -m flag with argument: %s\n", argvCopy[optind]);
                        printf("Use: pass show <name> [-m]");
                        fflush(stdout);
                        exit(1);
                    }
                    //printf("Found -m flag with argument: %s\n", argvCopy[optind]);
                    strncpy(flags->metadata, argvCopy[optind], MAX_LINE_LENGTH);
                    flags->metaDataAdd = true;
                    optind++;
                } else {
                    if(!(action == SHOW || action == RM)) {
                        printf("Found -m flag without argument\n");
                        printf("Use: pass show <name> [-m] <metadata>");
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
                printf("--> password.exe gen | generate <name> [-m] <metadata> [-c] -l <length> -dsmM \n\t--> generate a password and add it (and add metadata, specify length, specify which types of characters: d - digits, s - special characters, m - miniscules, M - majuscules)\n");
                printf("--> password.exe edit <name> [-a | -r] <metadata> \n\t--> edit a password (a: add inline, r: replace with, auto: open vim)\n");
                printf("--> password.exe delete <name> [-m] \n\t--> delete a password, or only the metadata\n");
                printf("--> password.exe ls \n\t--> list all passwords\n");
                printf("--> password.exe grep <pattern> [-i] \n\t--> list all passwords\n");
                fflush(stdout);
                exit(0);
                flags->h = true;
                break;
            case 'a':
                if(action != EDIT) {
                    printf("Found -a flag without the edit command\n");
                    printf("Use: pass edit <name> [-a | -r] <data in double quotes>");
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
                    printf("Use: pass edit <file> [-a | -r] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                    flags->metadata[0] = '\0';
                }
                
                flags->a = true;
                break;
            case 'r':
                if(action != EDIT) {
                    printf("Found -r flag without the edit command\n");
                    printf("Use: pass edit <name> [-a | -r] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                }

                if (optind < argcCopy && argvCopy[optind][0] != '-') {
                    printf("Found -r flag with argument: %s\n", argvCopy[optind]);
                    strncpy(flags->metadata, argvCopy[optind], MAX_LINE_LENGTH);
                    flags->metaDataAdd = true;
                    optind++;
                } else {
                    printf("Found -r flag without argument\n");
                    printf("Use: pass edit <name> [-a | -r] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                    flags->metadata[0] = '\0';
                }

                flags->r = true;
                break;

            case 'i':
                if(action != GREP) {
                    printf("Found -i flag without the grep command\n");
                    printf("Use: pass edit <name> [-a | -r] <data in double quotes>");
                    fflush(stdout);
                    exit(1);
                }
                flags->i = true;
                break;
            case 'l':
                if(action != GEN) {
                    printf("Found -l flag without the gen command\n");
                    printf("--> password.exe gen | generate <name> [-m] <metadata> [-c] -l <length> -dsmM \n\t--> generate a password and add it (and add metadata, specify length, specify which types of characters: d - digits, s - special characters, m - miniscules, M - majuscules)\n");
                }
 
                if (optind < argcCopy && argvCopy[optind][0] != '-') {
                    //printf("Found -l flag with argument: %s\n", argvCopy[optind]);
                    if(strlen(argvCopy[optind]) >= INT_LENGTH){
                        printf("Argument with -l is too long, please enter an integer from 0 to 2^(32)");
                    }
                    flags->genLength = atoi(argvCopy[optind]);
                    optind++;
                } else {
                    printf("Found -l flag without argument\n");
                    printf("--> password.exe gen | generate <name> [-m] <metadata> [-c] -l <length> -dsmM \n\t--> generate a password and add it (and add metadata, specify length, specify which types of characters: d - digits, s - special characters, m - miniscules, M - majuscules)\n");
                    fflush(stdout);
                    exit(1);
                }
            default:
                fprintf(stderr, "Invalid option: %s\n", argvCopy[optind]);
                fprintf(stderr, "Run pass --help for help\n");
                exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < argcCopy; i++) {
        free(argvCopy[i]);
    }
    free(argvCopy);

    return action;
}

// MARK: Add
int addPassword(char *filePath, int random) {
    if(flags.m && !flags.metaDataAdd){
        printf("Use: pass <name> [-m] <data in double quotes>\n");
        fflush(stdout);
        return 1;
    }

    char *key = (char *)malloc(MAX_LINE_LENGTH - 1 * sizeof(char));
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
        if (flags.genLength == 0){
            srand((unsigned int)(counter.QuadPart % UINT_MAX));
            length = 20 + rand() % 10;  // length gets values between 20 and 30
        }
        else {
            length = flags.genLength;
        }
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
        while ((ch = getch()) != '\n' && i < (MAX_LINE_LENGTH - 1)) {
            key[i++] = ch;
        }
        key[i] = '\0'; // Null-terminate the string

        addstr("\n");

        char *confirmPassword = (char *)malloc((MAX_LINE_LENGTH - 1) * sizeof(char));
        addstr("Confirm password: ");
        refresh();
        i = 0;
        while ((ch = getch()) != '\n' && i < (MAX_LINE_LENGTH - 1)) {
            confirmPassword[i++] = ch;
        }
        confirmPassword[i] = '\0'; // Null-terminate the string

        // Would be kinda nonsence outputting the password but it's possible
        //addstr("Password entered: "); 
        //addstr(confirmPassword); // Display a confirmation message
        //refresh();
        //getch(); // Wait for a key press
        endwin(); // Clean up and exit ncurses

        if(strncmp(confirmPassword, key, (MAX_LINE_LENGTH - 1))) {
            printf("Passwords do not match\n");
            return 1;
        }

        key[strcspn(key, "\n")] = 0;  // remove newline character
        if (strlen(key) > (MAX_LINE_LENGTH - 1)) {
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
int showPassword(char *path) {
    FILE *file = fopen(path, "r");
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

    return 0;
}

// MARK: Edit
int editPassword(char *filePath, char *argumentToLowerCase) {
    FILE *file;
    if (!flags.a && !flags.r) {
        const char *command = "vim"; // or "vi"
        char cmd[100];
        snprintf(cmd, sizeof(cmd), "%s %s", command, filePath);

        // Call Vi/Vim using system()
        if (system(cmd) == -1) {
            perror("Error executing Vi/Vim");
            return EXIT_FAILURE;
        }
    }
    else {
        file = fopen(filePath, "a");
        if (file == NULL) {
            fprintf(stderr, "Error opening the file\n");
            exit(EXIT_FAILURE);
        }

        if(flags.r){
            deleteTextExceptFirstLine(filePath);
        }

        fprintf(file, "%s", flags.metadata);
        fprintf(file, "%s", "\n");
    }

    printf("File '%s' edited successfully.\n", argumentToLowerCase);

    fclose(file);

    return 0;
}

// MARK: Delete
int deletePassword(char *filePath, char *argumentToLowerCase) {
    if(flags.m){
        deleteTextExceptFirstLine(filePath);

        printf("Metadata of '%s' deleted successfully.\n", argumentToLowerCase);
    }
    else {
        if (remove(filePath) != 0) {
            perror("Error deleting file");
            return EXIT_FAILURE;
        }

        printf("File '%s' deleted successfully.\n", argumentToLowerCase);
    }

    return 0;
}

// MARK: List
int lsDir(const char *pathDir) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(pathDir);
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

    return EXIT_SUCCESS;
}

// MARK: Grep
int grep(char *pattern, char *directory) {
    printf("Searching for: '%s' in directory: %s (Case %ssensitive)\n", 
           pattern, directory, flags.i ? "in" : "");
    fflush(stdout);

    traverseDirectory(directory, pattern, flags.i);

    return EXIT_SUCCESS;
}

// MARK: traverse directory
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
        snprintf(path, sizeof(path), "%s\\%s", basePath, dp->d_name);

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

    if(totalMatches == 0) {
        printf("No matches found\n");
    }
    else if(totalMatches == 1) {
        printf("Found 1 match\n");
    }
    else{
        printf("Found %d matches\n", totalMatches);
    }
    fflush(stdout);
}

// MARK: Search in file
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
    char numbers[10] = "1234567890";

    char allChars[200];
    allChars[0] = '\0';
    strcat(allChars, characters);
    strcat(allChars, majuscule);
    strcat(allChars, miniscule);
    strcat(allChars, numbers);

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
char *toLowerCase(const char *str) {
    char *result = strdup(str);
    for (int i = 0; result[i]; i++) {
        result[i] = tolower((unsigned char)result[i]);
    }
    return result;
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

// MARK: Find dir
char *findDir(void) {
    char *locationOfExecution = getLocationOfExecution();
    if (locationOfExecution == NULL) {
        perror("getLocationOfExecution");
        exit(EXIT_FAILURE);
    }
    //printf("Executable path: %s\n", locationOfExecution);

    char passwordsFolder[MAX_PATH];
    snprintf(passwordsFolder, sizeof(passwordsFolder), "%s\\passwords", locationOfExecution);
    free(locationOfExecution);

    // Check if directory exists
    if (!initPasswordDirectory(passwordsFolder)) {
        printf("No passwords directory was found, created one\n");
        fflush(stdout);
    }

    //printf("Password directory: %s\n", passwordsFolder);

    return strdup(passwordsFolder);
}

// MARK: Construct file path
char *constructFilePath(char *passwordsFolder, char *fileName) {
    char *searchPath = (char *)malloc(MAX_PATH * sizeof(char));
    // Use snprintf to safely construct the path
    if (snprintf(searchPath, MAX_PATH, "%s\\%s.txt", passwordsFolder, fileName) >= MAX_PATH) {
        fprintf(stderr, "Path too long\n");
        free(searchPath);
        exit(EXIT_FAILURE);
    }

    return searchPath;
}

// MARK: Find File
char *findFile(int argc, char *argv[], bool checkIfExists) {
    char *passwordsFolder = findDir();
    if (passwordsFolder == NULL) {
        perror("findDir");
        exit(EXIT_FAILURE);
    }

    // Construct the specific path to file
    char *argumentToLowerCase = toLowerCase(argv[2]);
    char *searchPath = constructFilePath(passwordsFolder, argumentToLowerCase);

    // check if file exists 
    struct stat st;
    if (stat(searchPath, &st) != 0) { // if file doesn't exist
        if(checkIfExists){
            fprintf(stderr, "FILE NOT FOUND\n");
            fprintf(stderr, "File location: %s\n", searchPath);
            exit(EXIT_FAILURE);
        }
    }
    else if(!checkIfExists) {
        fprintf(stderr, "FILE ALREADY EXISTS\n");
        fprintf(stderr, "File location: %s\n", searchPath);
        exit(EXIT_FAILURE);
    }
    free(argumentToLowerCase);

    return searchPath;
}

// MARK: init pwd dir
int initPasswordDirectory(const char *passwordsFolder) {
    struct stat st;
    if (stat(passwordsFolder, &st) == 0 && (st.st_mode & S_IFDIR)) {
        // Folder found
        return 1;
    } else {
        // Folder not found -> create it
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
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("test\n");

    const char* s = getenv("PATH");

    // If the environment variable doesn't exist, it returns NULL
    printf("%s\n", (s != NULL) ? s : "getenv returned NULL");
}
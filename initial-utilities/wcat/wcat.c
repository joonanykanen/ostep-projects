// wcat.c, JN, 05.05.2024
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    FILE *fp;
    char buffer[BUFFER_SIZE];

    if (argc == 1) {
        // No files specified, exit with status code 0
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("wcat: cannot open file\n");
            return 1;
        }

        while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
            printf("%s", buffer);
        }

        fclose(fp);
    }

    return 0;
}

// eof
// wgrep.c, JN, 05.05.2024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    FILE *fp = stdin;
    char *search_term;
    char buffer[BUFFER_SIZE];

    if (argc < 2) {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }

    search_term = argv[1];

    if (argc > 2) {
        // Process each file specified on the command line
        for (int i = 2; i < argc; i++) {
            fp = fopen(argv[i], "r");
            if (fp == NULL) {
                printf("wgrep: cannot open file\n");
                return 1;
            }
        }
    }

    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        if (strstr(buffer, search_term) != NULL) {
            printf("%s", buffer);
        }
    }

    if (fp != stdin) {
        fclose(fp);
    }

    return 0;
}

// eof

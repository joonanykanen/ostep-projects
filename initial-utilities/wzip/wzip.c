// wzip.c, JN, 05.05.2024
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    int current_char, prev_char, count;

    if (argc < 2) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    prev_char = -1;
    count = 0;

    for (int i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("wzip: cannot open file\n");
            return 1;
        }

        while ((current_char = fgetc(fp)) != EOF) {
            if (current_char == prev_char) {
                count++;
            } else {
                if (prev_char != -1) {
                    fwrite(&count, sizeof(int), 1, stdout);
                    fputc(prev_char, stdout);
                }
                prev_char = current_char;
                count = 1;
            }
        }

        fclose(fp);
    }

    // Last case before EOF will be treated after the while loop
    if (prev_char != -1) {
        fwrite(&count, sizeof(int), 1, stdout);
        fputc(prev_char, stdout);
    }

    return 0;
}

// eof

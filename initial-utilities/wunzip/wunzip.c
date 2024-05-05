// wunzip.c, JN, 05.05.2024
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    int count;
    char current_char;

    if (argc < 2) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }

    // Note: The compressed file is binary data (rb)
    for (int i = 1; i < argc; i++) {
        fp = fopen(argv[i], "rb");
        if (fp == NULL) {
            printf("wunzip: cannot open file\n");
            return 1;
        }

        while (fread(&count, sizeof(int), 1, fp) == 1) {
            fread(&current_char, sizeof(char), 1, fp);
            for (int j = 0; j < count; j++) {
                printf("%c", current_char);
            }
        }

        fclose(fp);
    }

    return 0;
}

// eof

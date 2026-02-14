#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

const unsigned char jpeg_soi[] = {'\xFF', '\xD8'};
const unsigned char jpeg_eoi[] = {'\xFF', '\xD9'};

int search_pattern(FILE *file, const unsigned char pattern[], size_t size);
int is_jpeg_file(FILE *file);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Использование:%s <файл>\n", argv[0]);
        return 1;
    }
    char *filename = argv[1];

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Can't open file : %s\n", strerror(errno));
        fprintf(stderr, "File: %s\n", filename);
        return 1;
    }

    if (is_jpeg_file(file) <= 0)
        fprintf(stderr, "Not jpeg!\n");
    else
        fprintf(stderr, "jpeg!\n");

    fclose(file);
    return 0;
}

// search_pattern
int search_pattern(FILE *file, const unsigned char pattern[], size_t size)
{
    int match_pos = 0;
    int c;
    long start_pos = ftell(file);

    while ((c = fgetc(file)) != EOF) {
        if (c == pattern[match_pos]) {
            match_pos++;
            if (match_pos == size) {
                return 1;
            }
        } else {
            if (match_pos > 0) {
                fseek(file, -match_pos, SEEK_CUR);
                match_pos = 0;
            }
        }
    }

    fseek(file, start_pos, SEEK_SET);
    return 0;
}

// is_jpeg_file
int is_jpeg_file(FILE *file)
{
    if (!search_pattern(file, jpeg_soi, 2)) {
        printf("Not JPEG: No SOI marker\n");
        return 0;
    }

    if (!search_pattern(file, jpeg_eoi, 2)) {
        printf("Not JPEG: No EOI marker\n");
        return 0;
    }
}

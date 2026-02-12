#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

const unsigned char jpeg_soi[] = {'\xFF', '\xD8'};
const unsigned char jpeg_eoi[] = {'\xFF', '\xD9'};

int main(int argc, char *argv[]) {
    // Проверка аргументов
    if (argc != 2) {
        printf("Использование: %s <файл>\n", argv[0]);
        return 1;
    }
    char *filename = argv[1];

    // open file
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Can't open file : %s\n", strerror(errno));
        fprintf(stderr, "File: %s\n", filename);
        return 1;
    }

    // 1. СНАЧАЛА проверяем SOI в начале файла
    unsigned char buffer[2];
    size_t bytes_read = fread(buffer, 1, 2, file);
    if (bytes_read != 2) {
        fprintf(stderr, "Error: The file is too short or corrupted\n");
        fclose(file);
        return 1;
    }

    if (buffer[0] != jpeg_soi[0] || buffer[1] != jpeg_soi[1]) {
        printf("Not JPEG: No SOI marker\n");
        fclose(file);
        return 1;
    }

    // 2. ПОТОМ ищем EOI по всему файлу
    unsigned char eoi_check[2];
    int found_eoi = 0;

    while (fread(eoi_check, 1, 2, file) == 2) {
        if (eoi_check[0] == jpeg_eoi[0] && eoi_check[1] == jpeg_eoi[1]) {
            found_eoi = 1;
            break;
        }
        fseek(file, -1, SEEK_CUR);
    }

    // 3. Результат
    if (found_eoi) {
        printf("JPEG\n");
        fclose(file);
        return 0;
    } else {
        printf("Not JPEG: No EOI marker found\n");
        fclose(file);
        return 1;
    }

}

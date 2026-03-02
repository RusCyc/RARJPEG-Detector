#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

// Паттерны для поиска и их длины
const unsigned char jpeg_soi[] = {'\xFF', '\xD8'};
const size_t jpeg_soi_size = 2;

const unsigned char jpeg_eoi[] = {'\xFF', '\xD9'};
const size_t jpeg_eoi_size = 2;

// zip сигнатуры
const unsigned char zip_eocd[] = {'\x50', '\x4B', '\x05', '\x06'};
const size_t zip_eocd_size = 4;

const unsigned char zip_cd[] = {'\x50', '\x4B', '\x01', '\x02'};
const size_t zip_cd_size = 4;

#pragma pack(push, 1)
struct lfh_tail {
    uint8_t something[14];
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_len;
    uint16_t extra_len;
};
#pragma pack(pop)

int search_pattern(FILE *file, const unsigned char pattern[], size_t size);
int is_jpeg_file(FILE *file);

long find_zip_end(FILE *file);
int read_eocd_info(FILE *file, long eocd_pos, uint16_t *total_entries, uint32_t *cd_size);
void read_central_directory(FILE *file, long cd_start, uint16_t total_entries);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Использование: %s <файл>\n", argv[0]);
        return 1;
    }
    char *filename = argv[1];

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Can't open file : %s\n", strerror(errno));
        fprintf(stderr, "File: %s\n", filename);
        return 1;
    }

    if (is_jpeg_file(file) == 1)
        printf("JPEG file detected!\n");
    else
        printf("Not a JPEG file!\n");

    // возвращаемся в начало файла
    rewind(file);
    long zip_pos = find_zip_end(file);

    if (zip_pos != -1) {
        printf("ZIP найден на позиции: %ld\n", zip_pos);

        // читаем информацию из хвоста
        uint16_t total_entries;
        uint32_t cd_size;
        if (read_eocd_info(file, zip_pos, &total_entries, &cd_size)) {
            printf("  Файлов в архиве: %u\n", total_entries);
            printf("  Размер оглавления: %u байт\n", cd_size);
            printf("\nСодержимое архива:\n");
            read_central_directory(file, zip_pos - cd_size, total_entries);
        }
    } else {
        printf("ZIP не найден\n");
    }

    fclose(file);
    return 0;
}

int search_pattern(FILE *file, const unsigned char pattern[], size_t size)
{
    int match_pos = 0;
    int c;
    long start_pos = ftell(file);

    while ((c = fgetc(file)) != EOF) {
        if (c == pattern[match_pos]) {
            match_pos++;
            if (match_pos == size) {
                fseek(file, start_pos, SEEK_SET);
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

int is_jpeg_file(FILE *file)
{
    if (!search_pattern(file, jpeg_soi, jpeg_soi_size)) {
        printf("Not JPEG: No SOI marker\n");
        return 0;
    }

    if (!search_pattern(file, jpeg_eoi, jpeg_eoi_size)) {
        printf("Not JPEG: No EOI marker\n");
        return 0;
    }

    return 1;
}

// поиск конца архива
long find_zip_end(FILE *file) {
    long saved_pos = ftell(file);

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    // Ищем с конца файла (минус 4 байта, так как паттерн 4-байтный)
    for (long pos = file_size - 4; pos >= 0; pos--) {
        fseek(file, pos, SEEK_SET);
        if (search_pattern(file, zip_eocd, 4)) {
            fseek(file, saved_pos, SEEK_SET);
            return pos;
        }
    }

    fseek(file, saved_pos, SEEK_SET);
    return -1;
}

int read_eocd_info(FILE *file, long eocd_pos, uint16_t *total_entries, uint32_t *cd_size) {
    fseek(file, eocd_pos, SEEK_SET);
    fseek(file, 10, SEEK_CUR);

    if (fread(total_entries, 2, 1, file) != 1) {
        return 0;
    }

    if (fread(cd_size, 4, 1, file) != 1) {
        return 0;
    }

    return 1;
}
void read_central_directory(FILE *file, long cd_start, uint16_t total_entries) {
    // Переходим к началу оглавления
    fseek(file, cd_start, SEEK_SET);

    for (int i = 0; i < total_entries; i++) {
        // 1. Проверяем метку PK\x01\x02
        unsigned char sig[4];
        if (fread(sig, 1, 4, file) != 4) {
            printf("Ошибка чтения сигнатуры\n");
            break;
        }

        if (sig[0] != 0x50 || sig[1] != 0x4B ||
            sig[2] != 0x01 || sig[3] != 0x02) {
            printf("Неверная метка записи #%d\n", i + 1);
            break;
        }

        // 2. Пропускаем 28 байт (до длины имени)
        fseek(file, 24, SEEK_CUR);

        // 3. Читаем длину имени (2 байта)
        uint16_t filename_len;
        if (fread(&filename_len, 2, 1, file) != 1)
            break;

        // 4. Читаем длины extra и comment (4 байта)
        uint16_t extra_len, comment_len;
        if (fread(&extra_len, 2, 1, file) != 1)
            break;
        if (fread(&comment_len, 2, 1, file) != 1)
            break;

        // 5. Пропускаем 14 байт (до имени файла)
        fseek(file, 12, SEEK_CUR);

        // 6. Читаем имя файла
        char *filename = malloc(filename_len + 1);
        if (!filename) {
            printf("Ошибка выделения памяти\n");
            break;
        }

        if (fread(filename, filename_len, 1, file) != 1) {
            free(filename);
            break;
        }
        filename[filename_len] = '\0';


        printf("  %d. %s\n", i + 1, filename);

        free(filename);


        fseek(file, extra_len + comment_len, SEEK_CUR);
    }
}

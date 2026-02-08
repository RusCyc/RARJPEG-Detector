#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
   //proverka
    if (argc !=2) {
       printf("use: %s<file>\n", argv[0]);
       return 1;
}


char *filename = argv[1];

// open file
FILE *file = fopen(filename, "rb");

    if (file == NULL) {
       printf("eror: file not foud\n");
   return 1;

}
//Proverka JPEG
unsigned char bytes[4];
fread(bytes,1,4,file);

int is_jpeg = 0;
if (bytes[0] ==0xFF && bytes [1] == 0xD8) {
    is_jpeg =1;
}


fseek(file, -100, SEEK_END);
unsigned char end_bytes[100];
fread(end_bytes,1,100,file);

int found_zip=0;
for (int i=0; i<98; i++) {
    if (end_bytes[i] == 'p' && end_bytes[i+1] == 'k') {
        found_zip =1;
        break;
    }

}


fclose(file);

//result
if (is_jpeg && found_zip) {
    printf("RARJPEG\n");
} else if (is_jpeg) {
    printf("JPEG\n");
} else if (found_zip) {
    printf("ZIP\n");
} else {
    printf("unknown");
}
return 0;

}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
   //proverka
    if (argc !=2) {
       fprintf(stderr,"use: %s <file>\n", argv[0]);
       return 1;
}


char *filename = argv[1];

// open file
FILE *file = fopen(filename, "rb");

    if (file == NULL) {
    fprintf(stderr,"Error: Can't open file : %s\n",strerror(errno));
        fprintf(stderr,"File: %s\n", filename);

   return 1;

}
//Proverka JPEG
unsigned char bytes[4];
size_t bytes_read = fread(bytes,1,4,file);

if (bytes_read < 4) {
    fprintf(stderr, "Erorr; The file is too short or corrupted \n");
     fprintf(stderr,"File: %s\n", filename);
    fclose(file);
    return 1;
}


int is_jpeg = 0;
if (bytes[0] ==0xFF && bytes [1] == 0xD8) {
    is_jpeg =1;
}


if (fseek(file, -100, SEEK_END) !=0) {
    fseek(file, 0,SEEK_SET);
    long file_size =ftell(file);
    fseek(file,0,SEEK_SET);

    if (file_size < 2) {
        fprintf(stderr, "Erorr: The file is too small to analize\n");
         fprintf(stderr,"File: %s\n", filename);
        fclose(file);
        return 1;
    }



}
unsigned char end_bytes[100];
bytes_read = fread(end_bytes,1,100,file);

int found_zip=0;
for (int i=0; i< bytes_read-1; i++) {
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

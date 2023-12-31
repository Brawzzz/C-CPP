#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <png.h>

#define PNG_DEBUG 3

int x, y;
int width, height;
int number_of_passes;

png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
png_bytep * row_pointers;

void abort_(const char * s, ...){
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void read_png_file(char* file_name){

    int png_size = 8;    
    char header[png_size];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp){
        abort_("[read_png_file] File %s could not be opened for reading", file_name);
    }

    fread(header, 1, png_size, fp);
    if (png_sig_cmp(header, 0, png_size)){
        abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);
    }

    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr){
        abort_("[read_png_file] png_create_read_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr){
        abort_("[read_png_file] png_create_info_struct failed");
    }

    if (setjmp(png_jmpbuf(png_ptr))){
        abort_("[read_png_file] Error during init_io");
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(png_ptr))){
        abort_("[read_png_file] Error during read_image");
    }

    printf("row_bytes_1 = %ld\n" , png_get_rowbytes(png_ptr,info_ptr));
    printf("row_bytes_2 = %ld\n" ,sizeof(png_byte) * sizeof(int) * width);

    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (y = 0 ; y < height ; y++){
        row_pointers[y] = (png_byte*) malloc(sizeof(png_byte) * sizeof(int) * width); //malloc(png_get_rowbytes(png_ptr,info_ptr));
    }
    png_read_image(png_ptr, row_pointers);

    fclose(fp);

    printf("width : %d , height : %d , color_type : %d , bit_depth : %d , number_of_passes : %d\n" , width , height , color_type , bit_depth , number_of_passes);
}

void write_png_file(char* file_name){
        
    /* create file */
    FILE *fp = fopen(file_name, "wb");
    if (!fp){
        abort_("[write_png_file] File %s could not be opened for writing", file_name);
    }

    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr){
        abort_("[write_png_file] png_create_write_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr){
        abort_("[write_png_file] png_create_info_struct failed");
    }

    if (setjmp(png_jmpbuf(png_ptr))){
        abort_("[write_png_file] Error during init_io");
    }
    png_init_io(png_ptr, fp);

    /* write header */
    if (setjmp(png_jmpbuf(png_ptr))){
        abort_("[write_png_file] Error during writing header");
    }

    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr))){
        abort_("[write_png_file] Error during writing bytes");
    }
    png_write_image(png_ptr, row_pointers);

    /* end write */
    if (setjmp(png_jmpbuf(png_ptr))){
        abort_("[write_png_file] Error during end of write");
    }
    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation */
    for (y=0; y<height; y++){
        free(row_pointers[y]);
    }
    free(row_pointers);
    fclose(fp);
}


void process_file(void){
    
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB){
        abort_("[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA " "(lacks the alpha channel)");
    }

    if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA){
        abort_("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)", 
        PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));
    }

    for (y = 0 ; y < height ; y++) {

        png_byte* row = row_pointers[y];
        for (x = 0 ; x < width/3 ; x++) {
                png_byte* ptr = &(row[x*4]);

                // printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n", x, y, ptr[0], ptr[1], ptr[2], ptr[3]);
                ptr[0] = 0;
                ptr[1] = 0;
                ptr[2] = 255;
                ptr[3] = 255; 
        }
        for (x = width/3 ; x < (2*width)/3 ; x++) {
                png_byte* ptr = &(row[x*4]);

                // printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n", x, y, ptr[0], ptr[1], ptr[2], ptr[3]);
                ptr[0] = 255;
                ptr[1] = 255;
                ptr[2] = 255;
                ptr[3] = 255; 
        }
        for (x = (2*width)/3 ; x < width ; x++) {
                png_byte* ptr = &(row[x*4]);

                // printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n", x, y, ptr[0], ptr[1], ptr[2], ptr[3]);
                ptr[0] = 255;
                ptr[1] = 0;
                ptr[2] = 0;
                ptr[3] = 255; 
        }
    }
}

png_bytep * allocates_image_memory(int width , int height){
        png_bytep * row_pointers;

        row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
        for (y = 0 ; y < height ; y++){
                row_pointers[y] = (png_byte*) malloc(sizeof(png_byte) * sizeof(int) * width); //malloc(png_get_rowbytes(png_ptr,info_ptr));
        }
        png_read_image(png_ptr, row_pointers);

        return row_pointers;
}


int main(int argc, char **argv){

    if (argc != 3){
        abort_("Usage : program_name <file_in> <file_out>");
    }

    read_png_file(argv[1]);
    process_file();
    write_png_file(argv[2]);
    printf("DONE ! \n");
        
    return 0;
}



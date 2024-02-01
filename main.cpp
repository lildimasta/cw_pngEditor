#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

typedef struct Png{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
}Png;

typedef struct Imageconf{
    int x1, y1;
    int x2, y2;
    int x3, y3;
    int line_color_r, line_color_g, line_color_b;
    int fill_color_r, fill_color_g, fill_color_b;
    int line_width;
    int ispour;

    int imageY, imageX;

    int flag;
    char *output;
}Imageconf;

void read_png_file(char *file_name, struct Png *image) {
    int y;
    char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp){
        printf("error, file could not be opened\n");
        exit(1);
        // Some error handling: file could not be opened
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)){
        printf("error, file is not a PNG\n");
        exit(1);
        // Some error handling: file is not recognized as a PNG
    }

    /* initialize stuff */
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr){
        printf("error, png_create_read_struct failed\n");
        exit(1);
        // Some error handling: png_create_read_struct failed
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("png_create_read_struct failed\n");
        exit(1);
        // Some error handling: png_create_info_struct failed
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during init_io\n");
        exit(1);
        // Some error handling: error during init_io
    }

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);

    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

    image->number_of_passes = png_set_interlace_handling(image->png_ptr);
    png_read_update_info(image->png_ptr, image->info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during read_image\n");
        exit(1);
        // Some error handling: error during read_image
    }

    image->row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    for (y = 0; y < image->height; y++)
        image->row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));

    png_read_image(image->png_ptr, image->row_pointers);

    fclose(fp);
}

void write_png_file(char *file_name, struct Png *image) {
    int y;
    /* create file */
    FILE *fp = fopen(file_name, "wb");
    if (!fp){
        printf("error, file could not be opened\n");
        exit(1);
    }

    /* initialize stuff */
    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr){
        printf("error, png_create_write_struct failed\n");
        exit(1);
        // Some error handling: png_create_write_struct failed
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("error, png_create_info_struct failed\n");
        exit(1);
        // Some error handling: png_create_info_struct failed
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during init_io\n");
        exit(1);
        // Some error handling: error during init_io
    }

    png_init_io(image->png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during writing header\n");
        exit(1);
    }

    png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height,
                 image->bit_depth, image->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(image->png_ptr, image->info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during writing bytes\n");
        exit(1);
    }

    png_write_image(image->png_ptr, image->row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during end of write\n");
        exit(1);
    }

    png_write_end(image->png_ptr, NULL);

    /* cleanup heap allocation */
    for (y = 0; y < image->height; y++)
        free(image->row_pointers[y]);
    free(image->row_pointers);

    fclose(fp);
}

void set_pixel(Png *image, int x, int y, int width_pixel, int red, int green, int blue){

    if(x >= 0 && x < image->width && y >= 0 && y < image->height){

        png_byte* pixel = &(image->row_pointers[y][x*width_pixel]);
        pixel[0] = red;
        pixel[1] = green;
        pixel[2] = blue;
        if(width_pixel == 4){
            pixel[3] = 255;
        }
    }
}

void draw_Circle(Png *image, int x0, int y0, int width_pixel, int radius, int redl, int greenl, int bluel) {

    int x = 0;
    int y = radius;
    int err;
    int delta = 1 - 2 * radius;
    int start = y0 - radius;
    int end = y0 + radius;
    while (y >= 0) {

        set_pixel(image, x0 + x, y0 + y, width_pixel, redl, greenl, bluel);
        set_pixel(image, x0 + x, y0 - y, width_pixel, redl, greenl, bluel);
        set_pixel(image, x0 - x, y0 + y, width_pixel, redl, greenl, bluel);
        set_pixel(image, x0 - x, y0 - y, width_pixel, redl, greenl, bluel);

        err = 2 * (delta + y) - 1;

        while (start <= y0){
            for(int i = abs(x - x0); i < (x + x0); i++){
                set_pixel(image, i,start, width_pixel, redl, greenl, bluel);
                set_pixel(image, i, end, width_pixel, redl, greenl, bluel);
            }
            if(err > 0){
                start++;
                end--;
            }
            break;
        }
        err = 2 * (delta + y) - 1;
        if (delta < 0 && err <= 0) {
            ++x;
            delta += 2 * x + 1;
            continue;
        }
        err = 2 * (delta - x) - 1;
        if(delta > 0 && err > 0) {
            --y;
            delta += 1 - 2 * y;
            continue;
        }
        ++x;
        delta += 2 * (x - y);
        --y;
    }
}

void set_line(Png *image, int width_pixel, int x0, int y0, int x1, int y1, int line_width,
              int redline, int greenline, int blueline){

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int error = dx - dy;
    set_pixel(image, x0,y0,width_pixel,redline,greenline,blueline);
    draw_Circle(image, x0, y0, width_pixel, line_width / 2, redline, greenline,blueline);
    while (x0 != x1 || y0 != y1){
        int e2 = 2*error;
        if(e2 > -dy){
            error -= dy;
            x0 += sx;
        }
        if (e2 < dx){
            error += dx;
            y0 += sy;
        }
        set_pixel(image,x0,y0,width_pixel,redline,greenline,blueline);
        draw_Circle(image, x0, y0, width_pixel, line_width / 2 , redline, greenline,blueline);
    }
}

int check_max(int a, int b){
    if(a>b){
        return a;
    }else{
        return b;
    }
}

int check_min(int a, int b){
    if(a<b){
        return a;
    }else{
        return b;
    }
}

int is_inside(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
    int one = (x1 - x0) * (y2 - y1) - (x2 - x1) * (y1 - y0);
    int two = (x2 - x0) * (y3 - y2) - (x3 - x2) * (y2 - y0);
    int three = (x3 - x0) * (y1 - y3) - (x1 - x3) * (y3 - y0);
    if ((one < 0 && two < 0 && three < 0) || (one > 0 && two > 0 && three > 0)) {
        return 1;
    }else{
        return 0;
    }
}

int check_triangle(int x1, int y1, int x2, int y2, int x3, int y3){
    double AB = sqrt(pow((x2-x1),2) + pow((y2-y1),2));
    double BC = sqrt(pow((x2-x3),2) + pow((y2-y3),2));
    double CA = sqrt(pow((x3-x1),2) + pow((y3-y1),2));
    if((AB+BC>CA) &&(BC+CA>AB) && (CA+AB>BC)){
        return 0;
    }
    else{
        return 1;
    }
}

void fill_triangle(Png *image, int width_pixel,int x1, int y1, int x2, int y2,int x3, int y3, int redf, int greenf, int bluef){

    int min_x, max_x, min_y, max_y;

    max_x = check_max(check_max(x1,x2),x3);
    min_x = check_min(check_min(x1,x2),x3);
    max_y = check_max(check_max(y1,y2),y3);
    min_y = check_min(check_min(y1,y2),y3);

    for(int x = min_x; x <= max_x; x++){
        for(int y = min_y; y <= max_y; y++){

            if(is_inside(x,y,x1,y1,x2,y2,x3,y3)){
                set_pixel(image, x, y, width_pixel, redf, greenf,bluef);
            }
        }
    }
}

void draw_triangle(Png *image, int width_pixel,int line_width,int ispour, int x1,int y1,int x2,int y2,
                   int x3,int y3, int redline, int greenline, int blueline, int redf, int greenf, int bluef){

    if(x1 > image->width || x1 < 0 || x2 > image->width || x2 < 0 || x3 > image->width
       || x3 < 0 || y1 > image->height ||
       y1 < 0 || y2 > image->height || y2 < 0 || y3 > image->height || y3 < 0){
        printf("wrong coordinates, out of PNG\n");
        return;
    }
    if(check_triangle(x1,y1,x2,y2,x3,y3) == 1){
        printf("wrong triangle coordinates\n");
        return;
    }


    set_line(image, width_pixel,x1,y1,x2,y2, line_width, redline, greenline, blueline);
    set_line(image, width_pixel,x2,y2,x3,y3, line_width, redline, greenline, blueline);
    set_line(image, width_pixel,x3,y3, x1, y1, line_width, redline, greenline, blueline);

    if(ispour){
        fill_triangle(image, width_pixel,x1,y1, x2, y2, x3, y3, redf, greenf, bluef);
    }
}

void find_rectangle(Png *image, int width_pixel, int redfind,int greenfind, int bluefind, int redfill,int greenfill, int bluefill){


    int maxArea = 0;
    int startX, startY, endX, endY;

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {

            int currentArea = 1;
            int currentStartX = x;
            int currentStartY = y;

            if (image->row_pointers[y][x * width_pixel] == redfind &&
                image->row_pointers[y][x * width_pixel + 1] == greenfind &&
                image->row_pointers[y][x * width_pixel + 2] == bluefind) {

                while (x < image->width &&
                       image->row_pointers[y][x * width_pixel] == redfind &&
                       image->row_pointers[y][x * width_pixel + 1] == greenfind &&
                       image->row_pointers[y][x * width_pixel + 2] == bluefind) {
                    x++;
                    currentArea++;
                }

                int currentEndX = x - 1;
                int isValidRow = 1;

                while (y < image->height) {
                    for (int i = currentStartX; i <= currentEndX; i++) {

                        if (image->row_pointers[y][i * width_pixel] != redfind ||
                            image->row_pointers[y][i * width_pixel + 1] != greenfind ||
                            image->row_pointers[y][i * width_pixel + 2] != bluefind) {
                            isValidRow = 0;
                            break;
                        }
                    }
                    if (!isValidRow) {
                        break;
                    }
                    currentArea += currentEndX - currentStartX + 1;
                    y++;
                }

                if (currentArea > maxArea) {
                    maxArea = currentArea;
                    startX = currentStartX;
                    startY = currentStartY;
                    endX = currentEndX;
                    endY = y - 1;
                }
            }
        }
    }

    if(maxArea == 0){
        printf("Rectangle didnt found\n");
        return;
    }

    for (int h = startY; h <= endY; h++) {
        for (int x = startX; x <= endX; x++) {
            set_pixel(image,x,h,width_pixel,redfill,greenfill,bluefill);
        }
    }
}

void replace_pixel(png_byte *newpixel, png_byte *oldpixel, int width_pixel){
    for(int i = 0; i < width_pixel; i++){
        newpixel[i] = oldpixel[i];
    }
}

void make_collage(Png *image, int width_pixel, int N, int M){

    if(N <= 0 || M <= 0){
        printf("wrong image count\n");
        return;
    }

    int new_width = image->width*N;
    int new_height = image->height*M;

    png_byte **new_image = (png_byte**) malloc(sizeof (png_byte* ) * new_height);

    for(int y = 0; y < new_height; y++){
        new_image[y] = (png_byte*) malloc(sizeof (png_byte) * new_width * width_pixel);
    }
    for(int y1 = 0; y1 < new_height; y1++){

        int old_y = y1 % image->height;
        png_byte *new_row = new_image[y1];
        png_byte *old_row = image->row_pointers[old_y];

        for(int x = 0; x < new_width; x++){

            int old_x = x % image->width;
            png_byte *new_pixel = &(new_row[x * width_pixel]);
            png_byte *old_pixel = &(old_row[old_x * width_pixel]);

            replace_pixel(new_pixel, old_pixel, width_pixel);
        }
    }
    for(int i = 0; i < image->height; i++){
        free(image->row_pointers[i]);
    }
    free(image->row_pointers);
    image->width = new_width;
    image->height = new_height;
    image->row_pointers = new_image;
}

void printHelp() {
    printf("Формат ввода: \n./a.out [имя исходного файла] [имя функции"
           " которую необходимо выполнить] -[сокращенный ключ] или --[полный ключ]"
           " [первое значение ключа,второе значение ключа] ...\n"
           "Виды функций и ключей поддерживаемых программой:\n"
           "triangle - функция рисует треугольник с возможностью"
           " его заливки и выбором цвета для заливки\n"
           "    -f или --first [x,y] - ключ"
           " для создания первой вершины треугольника\n"
           "    -s или --second [x,y] - ключ"
           " для создания второй вершины треугольника\n"
           "    -t или --third [x,y] - ключ"
           " для создания третьей вершины треугольника\n"
           "    -l или --linewidth [число] - толщина сторон треугольника\n"
           "    -S или --sidecolor [r,g,b] - цвет сторон треугольника\n"
           "    -c или --color [r,g,b] - цвет заливки треугольника\n"
           "    -F или --toflood - заливка треугольника, по умолчанию без заливки\n"
           "rectangle - функция ищет самый большой прямоугольник заданного цвета"
           " и перекрашивает его в другой цвет\n"
           "    -S или --sidecolor [r,g,b] - цвет искомого прямоугольника\n"
           "    -c или --color [r,g,b] - цвет перекраски прямоугольника\n"
           "collage - функция создаёт коллаж из изображения\n"
           "    -x или --xcount [число] - количество изображений по оси x\n"
           "    -y или --ycount [число] - количество изображений по оси y\n"
           "-i или --info - вывод информации о изображении\n"
           "-o или --output - файл для вывода, по умолчанию исходный файл\n"
           "-h или --help - помощь по работе с программой\n");
}

void PNGinfo(Png *image){
    printf("Информация о исходном изображении:\n");
    printf("width: %d\n", image->width);
    printf("height: %d\n", image->height);
    printf("bit depth: %u\n", image->bit_depth);
    printf("color type: %u\n", image->color_type);
}

void chooseoption(Imageconf *conf, char opt){
    switch (opt) {
        case 'f':
            sscanf(optarg, "%d,%d", &conf->x1, &conf->y1);
            break;
        case 's':
            sscanf(optarg, "%d,%d", &conf->x2, &conf->y2);
            break;
        case 't':
            sscanf(optarg, "%d,%d", &conf->x3, &conf->y3);
            break;
        case 'l':
            conf->line_width = atoi(optarg);

            if(conf->line_width <= 0){
                printf("wrong line width\n");
                exit(-1);
            }
            break;
        case 'S':
            sscanf(optarg, "%d,%d,%d", &conf->line_color_r, &conf->line_color_g, &conf->line_color_b);

            if(conf->line_color_r < 0 || conf->line_color_r > 255 || conf->line_color_g < 0 || conf->line_color_g > 255
               || conf->line_color_b < 0 || conf->line_color_b > 255){
                printf("wrong colors\n");
                exit(-1);
            }
            break;
        case 'i':
            conf->flag = 1;
            break;
        case 'c':
            sscanf(optarg, "%d,%d,%d", &conf->fill_color_r, &conf->fill_color_g, &conf->fill_color_b);

            if(conf->fill_color_r < 0 || conf->fill_color_r > 255 || conf->fill_color_g < 0 || conf->fill_color_g > 255
               || conf->fill_color_b < 0 || conf->fill_color_b > 255){
                printf("wrong fill colors\n");
                exit(-1);
            }
            break;
        case 'F':
            conf->ispour = 1;
            break;
        case 'x':
            sscanf(optarg, "%d", &conf->imageX);
            break;
        case 'y':
            sscanf(optarg, "%d", &conf->imageY);
            break;
        case 'o':
            conf->output = optarg;
            break;
        case 'h':
            printHelp();
            exit(0);
        case '?':
            printf("invalid option or missing argument\n");
            exit(-1);
    }
}

int main(int argc, char  **argv){

    if(argc == 1){
        printf("didnt find any arguments\n"
               "put the file name\n");
        exit(-1);
    }

    char *output = (char *) argv[1];
    char *func = (char *) argv[2];
    char *file = (char *) argv[1];

    Png image;
    Imageconf conf = {0,0,0,0,0,0,
                      0,0,0,0,
                      0,0,0,0,
                      0,0,0, output};

    char *shortopts = "f:s:t:l:S:c:Fx:y:io:h";
    struct option longopts[] = {
            {"first", required_argument, NULL, 'f'},
            {"second", required_argument, NULL, 's'},
            {"third", required_argument, NULL, 't'},
            {"linewidth", required_argument, NULL, 'l'},
            {"sidecolor", required_argument, NULL, 'S'},
            {"color", required_argument, NULL, 'c'},
            {"toflood", no_argument, NULL, 'F'},
            {"xcount", required_argument, NULL, 'x'},
            {"ycount", required_argument, NULL, 'y'},
            {"info", no_argument, NULL, 'i'},
            {"output", required_argument, NULL, 'o'},
            {"help", no_argument, NULL, 'h'},
            {NULL, no_argument, NULL, 0}
    };

    int opt;
    int indexptr;

    opt = getopt_long(argc, argv, shortopts, longopts, &indexptr);

    while (opt != -1){
        chooseoption(&conf, opt);
        opt = getopt_long(argc, argv, shortopts, longopts, &indexptr);
    }

    if(optind == 1){
        printf("didnt find any keys\n");
        printHelp();
        exit(-1);
    }

    read_png_file(file, &image);

    if(conf.flag == 1){
        PNGinfo(&image);
    }

    int width_pixel;

    if(image.color_type == 6){
        width_pixel = 4;
    }else{
        width_pixel = 3;
    }

    if(strcmp(func, "triangle") == 0){
        draw_triangle(&image, width_pixel, conf.line_width, conf.ispour, conf.x1, conf.y1, conf.x2, conf.y2,
                      conf.x3, conf.y3, conf.line_color_r, conf.line_color_g, conf.line_color_b,
                      conf.fill_color_r, conf.fill_color_g, conf.fill_color_b);
    }
    if(strcmp(func, "rectangle") == 0){
        find_rectangle(&image, width_pixel,conf.line_color_r, conf.line_color_g, conf.line_color_b,
                       conf.fill_color_r, conf.fill_color_g, conf.fill_color_b);
    }
    if(strcmp(func, "collage") == 0){
        make_collage(&image, width_pixel, conf.imageX, conf.imageY);
    }

    write_png_file(conf.output, &image);

    return 0;
}
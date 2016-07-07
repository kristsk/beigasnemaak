#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <sys/time.h>
#include <errno.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
int main(int argc, char *argv[]) {

    if (argc != 3) {

        printf("Usage:\n\tbeigasnamaakc <quoted_glob_for_source_rgba> <output_rgba_filename>\n");
        printf("Example:\n\tbeigasnamaakc \"../manyPng/*.rgba\" sum.rgba\n");
        printf("To convert png to rgba:\n\tls *.png | xargs -t -I {} convert {} -crop 500x500+0+0 {}.rgba\n");
        printf("To convert result rgba to png:\n\tconvert -size 500x500 -depth 8 sum.rgba sum.png\n");
        exit(-1);
    }

    uint16_t sums[500 * 500] = {0};
    uint16_t max_sum = 1;

    uint32_t pixels[500 * 500];

    glob_t input_files_glob;
    int glob_result = glob(argv[1], GLOB_ERR, NULL, &input_files_glob);
    if (glob_result) {

        printf("Fatal: Glob \"%s\" failed with error %d\n", argv[1], glob_result);
        exit(-2);
    }

    printf("Found %d files to process\n", (int) input_files_glob.gl_pathc);

    struct timeval time;
    for (int file_number = 0; file_number < input_files_glob.gl_pathc; file_number++) {

        gettimeofday(&time, NULL);
        long file_start_time = time.tv_sec * 1000000 + time.tv_usec;

        printf("In: %s, ", input_files_glob.gl_pathv[file_number]);

        FILE *input_file = fopen(input_files_glob.gl_pathv[file_number], "r");

        if (input_file == NULL) {

            printf("could not open, error %d", errno);
            exit(-2);
        }
        size_t pixels_read = fread(&pixels, sizeof(uint32_t), 500 * 500, input_file);
        fclose(input_file);

        printf("read %d pixels, ", (int) pixels_read);

        if (pixels_read != 500 * 500) {

            printf("it is not enough\n");
            exit(-2);
        }

        printf("processing ... ");

        uint16_t increment;
        for (int i = 0; i < 500 * 500; i++) {

            uint16_t dx = (uint16_t) ((i / 500) - 249);
            uint16_t dy = (uint16_t) ((i % 500) - 249);

            if ((dx * dx) + (dy * dy) > (250 * 250)) {

                continue;
            }

            switch (pixels[i]) {

                case 0xffc70000:
                    increment = 1;
                    break;
                case 0xfffe3400:
                    increment = 2;
                    break;
                case 0xfffe7900:
                    increment = 3;
                    break;
                case 0xfffea21a:
                    increment = 5;
                    break;
                case 0xfffed053:
                    increment = 7;
                    break;
                case 0xfffef086:
                    increment = 10;
                    break;
                case 0xfffefefe:
                    increment = 14;
                    break;
                case 0xffc0f7fe:
                    increment = 20;
                    break;
                case 0xff00e5fe:
                    increment = 31;
                    break;
                case 0xff00bcfe:
                    increment = 45;
                    break;
                case 0xff0073fe:
                    increment = 66;
                    break;
                case 0xff003ffe:
                    increment = 95;
                    break;
                case 0xff0000c7:
                    increment = 140;
                    break;
                case 0xff0000a0:
                    increment = 220;
                    break;
                case 0xff000080:
                    increment = 300;
                    break;
                default:
                    increment = 0;
            }

            sums[i] = sums[i] + increment;

            if (sums[i] > max_sum) {
                max_sum = sums[i];
            }
        }

        gettimeofday(&time, NULL);
        long file_end_time = time.tv_sec * 1000000 + time.tv_usec;

        printf("done in %lius\n", file_end_time - file_start_time);
    }

    globfree(&input_files_glob);

    for (int i = 0; i < 500 * 500; i++) {

        if (sums[i] > 0) {

            switch (sums[i]) {

                case 1:
                    pixels[i] = 0xffc70000;
                    break;
                case 2:
                    pixels[i] = 0xfffe3400;
                    break;
                case 3:
                    pixels[i] = 0xfffe7900;
                    break;
                case 4 ... 5:
                    pixels[i] = 0xfffea21a;
                    break;
                case 6 ... 7:
                    pixels[i] = 0xfffed053;
                    break;
                case 8 ... 10:
                    pixels[i] = 0xfffef086;
                    break;
                case 11 ... 14:
                    pixels[i] = 0xfffefefe;
                    break;
                case 15 ... 20:
                    pixels[i] = 0xffc0f7fe;
                    break;
                case 21 ... 31:
                    pixels[i] = 0xff00e5fe;
                    break;
                case 32 ... 45:
                    pixels[i] = 0xff00bcfe;
                    break;
                case 46 ... 66:
                    pixels[i] = 0xff0073fe;
                    break;
                case 67 ... 95:
                    pixels[i] = 0xff003ffe;
                    break;
                case 96 ... 140:
                    pixels[i] = 0xff0000c7;
                    break;
                case 141 ... 220:
                    pixels[i] = 0xff0000a0;
                    break;
                case 221 ... 65535:
                    pixels[i] = 0xff000080;
                    break;
                default:
                    pixels[i] = 0;
                    break;
            }
        }
        else {

            pixels[i] = 0;
        }
    }

    FILE *output_file = fopen(argv[2], "w");

    if (output_file == NULL) {

        printf("Could not open output file %s, error: %d\n", argv[2], errno);
        exit(-2);
    }

    fwrite(&pixels, sizeof(uint32_t), 500 * 500, output_file);
    fclose(output_file);

    printf("Done\n");

    return 0;
}
#pragma clang diagnostic pop
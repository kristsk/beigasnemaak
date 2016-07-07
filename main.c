#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <sys/time.h>
#include <errno.h>

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage:\n\tbeigasnamaakc <quoted_glob_for_source_rgba> <output_rgba_filename>\n");
        printf("Example:\n\t beigasnamaakc \"../manyPng/*.rgba\" sum.rgba\n");
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

            uint8_t p = (uint8_t) ((sums[i] * 230) / max_sum);

            pixels[i] = (
                    (uint32_t) (0xff << 24) +
                    (uint32_t) ((230 - p) << 16) +
                    (uint32_t) ((230 - p) << 8) +
                    (uint32_t) ((230 - p) << 0)
            );
        }
        else {

            pixels[i] = 0xffc0f7fe; // rgb(0xfe, 0xf7, 0xc0)
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


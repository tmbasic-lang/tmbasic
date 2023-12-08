/* A minimal tar utility using microtar. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microtar.h>

/* Function to check and handle errors */
void check_error(int result, mtar_t *tar) {
    if (result != MTAR_ESUCCESS) {
        fprintf(stderr, "Error: %s\n", mtar_strerror(result));
        if (tar != NULL) {
            mtar_close(tar);
        }
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <tarfile> <file1> <file2> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    mtar_t tar;
    const char *tarfilename = argv[1];

    /* Open the tar file */
    check_error(mtar_open(&tar, tarfilename, "wb"), NULL);

    for (int i = 2; i < argc; i++) {
        FILE *file = fopen(argv[i], "rb");
        if (file == NULL) {
            fprintf(stderr, "Error opening file %s\n", argv[i]);
            mtar_close(&tar);
            return EXIT_FAILURE;
        }

        /* Seek to end to get the file size */
        fseek(file, 0, SEEK_END);
        unsigned size = ftell(file);
        rewind(file);

        /* Allocate memory for reading the file */
        char *buffer = malloc(size);
        if (buffer == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            fclose(file);
            mtar_close(&tar);
            return EXIT_FAILURE;
        }

        /* Read file into buffer */
        fread(buffer, 1, size, file);
        fclose(file);

        /* Write file header and data to the tar archive */
        check_error(mtar_write_file_header(&tar, argv[i], size), &tar);
        check_error(mtar_write_data(&tar, buffer, size), &tar);

        free(buffer);
    }

    check_error(mtar_finalize(&tar), &tar);

    check_error(mtar_close(&tar), NULL);

    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Function declarations
void Gaussian_Blur();
void Sobel();
void read_image(const char* filename);
void write_image2(const char* filename, unsigned char* output_image);
void openfile(const char* filename, FILE** finput);
int getint(FILE* fp);
void skip_comments(FILE* fp);

// Global variables
char header[100]; // This stores the image header (P2, P5, etc.)
int M, N; // Image dimensions (width and height)
unsigned char* frame1;  // Input image
unsigned char* filt;    // Output filtered image (Gaussian)
unsigned char* gradient; // Output gradient image (Sobel)

// Gaussian mask (5x5)
const signed char Mask[5][5] = {
    {2,4,5,4,2},
    {4,9,12,9,4},
    {5,12,15,12,5},
    {4,9,12,9,4},
    {2,4,5,4,2}
};

// Sobel masks (3x3)
const signed char GxMask[3][3] = {
    {-1,0,1},
    {-2,0,2},
    {-1,0,1}
};

const signed char GyMask[3][3] = {
    {-1,-2,-1},
    {0,0,0},
    {1,2,1}
};

// Function to process all images
void process_all_images(const char *input_dir, const char *output_dir1, const char *output_dir2) {
    for (int i = 0; i < 31; i++) {
        char input_filename[256], output_filename1[256], output_filename2[256];

        // Create filenames based on the image number (a0.pgm to a30.pgm)
        sprintf(input_filename, "%s/a%d.pgm", input_dir, i);
        sprintf(output_filename1, "%s/blurred_a%d.pgm", output_dir1, i);
        sprintf(output_filename2, "%s/edge_detection_a%d.pgm", output_dir2, i);

        // Read the image
        read_image(input_filename);

        // Apply Gaussian Blur and Sobel edge detection
        Gaussian_Blur();
        Sobel();

        // Write the processed images to disk
        write_image2(output_filename1, filt);
        write_image2(output_filename2, gradient);

        // Free dynamically allocated memory after processing each image
        free(frame1);
        free(filt);
        free(gradient);
    }
}

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments are passed
    if (argc != 4) {
        printf("Usage: %s <input_images_path> <output_images_path> <output_images2_path>\n", argv[0]);
        return 1;
    }

    // Get directories from command-line arguments
    char *input_dir = argv[1];
    char *output_dir1 = argv[2];
    char *output_dir2 = argv[3];

    // Process all images using the directories passed as arguments
    process_all_images(input_dir, output_dir1, output_dir2);

    return 0;
}

void Gaussian_Blur() {
    int row, col, rowOffset, colOffset;
    int newPixel;
    unsigned char pix;
    const unsigned short int size = 2;

    // Apply Gaussian Blur
    for (row = 0; row < N; row++) {
        for (col = 0; col < M; col++) {
            newPixel = 0;
            for (rowOffset = -size; rowOffset <= size; rowOffset++) {
                for (colOffset = -size; colOffset <= size; colOffset++) {
                    if ((row + rowOffset < 0) || (row + rowOffset >= N) || (col + colOffset < 0) || (col + colOffset >= M))
                        pix = 0;
                    else
                        pix = frame1[M * (row + rowOffset) + (col + colOffset)];

                    newPixel += pix * Mask[size + rowOffset][size + colOffset];
                }
            }
            filt[M * row + col] = (unsigned char)(newPixel / 159);
        }
    }
}

void Sobel() {
    int row, col, rowOffset, colOffset;
    int Gx, Gy;

    // Apply Sobel edge detection
    for (row = 1; row < N - 1; row++) {
        for (col = 1; col < M - 1; col++) {
            Gx = 0;
            Gy = 0;

            for (rowOffset = -1; rowOffset <= 1; rowOffset++) {
                for (colOffset = -1; colOffset <= 1; colOffset++) {
                    Gx += filt[M * (row + rowOffset) + (col + colOffset)] * GxMask[rowOffset + 1][colOffset + 1];
                    Gy += filt[M * (row + rowOffset) + (col + colOffset)] * GyMask[rowOffset + 1][colOffset + 1];
                }
            }

            // Compute the gradient magnitude and clamp it to [0, 255]
            int magnitude = (int)sqrt(Gx * Gx + Gy * Gy);
            if (magnitude > 255) magnitude = 255;  // Ensure the value stays within valid range
            gradient[M * row + col] = (unsigned char)magnitude;
        }
    }
}

void read_image(const char* filename) {
    FILE* finput;
    int temp;

    // Open the image file
    openfile(filename, &finput);

    // Parse the header to get image dimensions (P2 or P5 format)
    if ((header[0] == 'P') && (header[1] == '5')) {
        skip_comments(finput);
        M = getint(finput);  // Read width (M)
        N = getint(finput);  // Read height (N)
    }
    else if ((header[0] == 'P') && (header[1] == '2')) {
        skip_comments(finput);
        M = getint(finput);  // Read width (M)
        N = getint(finput);  // Read height (N)
    }
    else {
        printf("\nError: Unsupported image format.");
        exit(EXIT_FAILURE);
    }

    // Debug: Print image dimensions
    printf("Image dimensions: M = %d, N = %d\n", M, N);

    // Check if image dimensions are valid (non-zero and reasonable)
    if (M <= 0 || N <= 0) {
        printf("Error: Invalid image dimensions.\n");
        exit(EXIT_FAILURE);
    }

    // Dynamically allocate memory for the image
    frame1 = (unsigned char*)malloc(M * N * sizeof(unsigned char));
    filt = (unsigned char*)malloc(M * N * sizeof(unsigned char));
    gradient = (unsigned char*)malloc(M * N * sizeof(unsigned char));

    // Check if memory allocation succeeded
    if (!frame1 || !filt || !gradient) {
        printf("Memory allocation failed. Image dimensions are M = %d, N = %d\n", M, N);
        exit(EXIT_FAILURE);
    }

    // Initialize memory to avoid using uninitialized memory
    memset(filt, 0, M * N * sizeof(unsigned char));
    memset(gradient, 0, M * N * sizeof(unsigned char));

    // Read the image data
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < M; i++) {
            temp = getc(finput);
            frame1[M * j + i] = (unsigned char)temp;
        }
    }

    fclose(finput);
    printf("\nImage successfully read from disk: %s\n", filename);
}

void write_image2(const char* filename, unsigned char* output_image) {
    FILE* foutput;

    printf("Writing result to disk...\n");

    // Open output file in text mode for P2 (ASCII format)
    foutput = fopen(filename, "w");
    if (foutput == NULL) {
        fprintf(stderr, "Unable to open the output file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Write PGM header (P2 format)
    fprintf(foutput, "P2\n");
    fprintf(foutput, "%d %d\n", M, N);
    fprintf(foutput, "255\n");

    // Write the image data
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            fprintf(foutput, "%d ", output_image[M * i + j]);
        }
        fprintf(foutput, "\n");
    }

    fclose(foutput);
    printf("Output written successfully: %s\n", filename);
}

void openfile(const char* filename, FILE** finput) {
    if ((*finput = fopen(filename, "rb")) == NULL) {
        printf("Unable to open the file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fgets(header, sizeof(header), *finput);  // Read the PGM header (P2 or P5)
}

void skip_comments(FILE* fp) {
    char c;
    while ((c = getc(fp)) == '#') {
        while (getc(fp) != '\n') {} // Skip the rest of the line
    }
    ungetc(c, fp); // Put the non-comment character back into the stream
}

int getint(FILE* fp) {
    int i = 0;
    char c;
    while ((c = getc(fp)) == ' ' || c == '\n' || c == '\r') {}
    i = 0;
    do {
        i = i * 10 + (c - '0');
        c = getc(fp);
    } while (c >= '0' && c <= '9');
    return i;
}

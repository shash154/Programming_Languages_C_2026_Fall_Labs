// week5_task1_file_io.c
// Name: SHASHANK SAGAR  Surname: NARWA  Student ID: 251ADB118
//
// Task 1: Read and write data from text files
// Week 5 – Files & Modular Programming
// TODO: Fill in the missing parts marked below.
//
// Build: make lab5   Run: ./bin/week5_task1_file_io

#include <stdio.h>
#include <stdlib.h>
 
#define DEFAULT_FILENAME "data.txt"
#define LINE_LEN 256
 
int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    const char *filename = DEFAULT_FILENAME;
    char line[LINE_LEN];
    int line_count = 0;
 
    // BONUS: use the file name from the command line if given
    if (argc > 1) {
        filename = argv[1];
    }
 
    // 1-2. Open the file for writing and check for NULL
    printf("Writing lines to %s...\n", filename);
    fp = fopen(filename, "w");
    if (fp == NULL) {
        perror(filename);
        return EXIT_FAILURE;
    }
 
    // 3. Write some lines
    fprintf(fp, "Hello, file I/O in C!\n");
    fprintf(fp, "This is another line.\n");
 
    // 4. Close the file
    fclose(fp);
 
    // 5. Open the file again for reading and check for NULL
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror(filename);
        return EXIT_FAILURE;
    }
 
    // 6. Read line by line and print each line
    printf("Reading contents:\n");
    while (fgets(line, sizeof line, fp) != NULL) {
        fputs(line, stdout);
        line_count++;
    }
 
    // 7. Close the file
    fclose(fp);
 
    // BONUS: number of lines read
    printf("Lines read: %d\n", line_count);
 
    return EXIT_SUCCESS;
}
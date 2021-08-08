////////////////////////////////////////////////////////////////////////////////
// Main File:        my-look.c, across.c, my-diff.c
// This File:        my-diff.c
// Other Files:      my-look.c, across.c
// Semester:         CS 537 Fall 2019
//
// Author:           Yingjie Shen
// Email:            shen92@wisc.edu
// CS Login:         yingjie
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of
//                   of any information you find.
////////////////////////////////////////////////////////////////////////////////
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// gcc -o my-diff my-diff.c -Wall -Werror

/**
 * This is the main function of the program
 * This method checks the user's input argument and implements the my-diff
 * utility
 *
 * @param int argc, char *argv[]
 * @return int
 */
int main(int argc, char *argv[]) {
  // Check user input
  int num_argc = argc - 1;

  if (num_argc != 2) {  // If num_argc is not 2, exit the program
    printf("my-diff: invalid number of arguments\n");
    exit(1);

  } else {
    // Open the two files and read their contents to differnt arrays
    char *fileA_name = argv[1];
    char *fileB_name = argv[2];

    FILE *fp_A = fopen(fileA_name, "r");
    if (fp_A == NULL) {
      printf("my-diff: cannot open file\n");
      exit(1);
    }
    FILE *fp_B = fopen(fileB_name, "r");
    if (fp_B == NULL) {
      printf("my-diff: cannot open file\n");
      exit(1);
    }

    // Declare and initialize the arrays for file content
    char **fileA_content;
    fileA_content = malloc(sizeof(char *) * 1024);
    if (fileA_content != NULL) {
      for (int i = 0; i < 1024; i++) {
        fileA_content[i] = malloc(sizeof(char) * 256);
      }
    } else {
      printf("my-diff: cannot allocate memory\n");
      exit(1);
    }
    char **fileB_content;
    fileB_content = malloc(sizeof(char *) * 1024);
    if (fileB_content != NULL) {
      for (int i = 0; i < 1024; i++) {
        fileB_content[i] = malloc(sizeof(char) * 256);
      }
    } else {
      printf("my-diff: cannot allocate memory\n");
      exit(1);
    }

    int fileA_size = 0;
    int fileB_size = 0;

    // Read through the files and copy them to the content array
    char line[1024] = {""};
    while (fgets(line, 256, fp_A) != NULL) {
      strtok(line, "\n");
      strcpy(fileA_content[fileA_size], line);
      fileA_size++;
    }
    while (fgets(line, 256, fp_B) != NULL) {
      strtok(line, "\n");
      strcpy(fileB_content[fileB_size], line);
      fileB_size++;
    }

    // Close the two files
    if (fclose(fp_A) != 0) {
      printf("my-diff: cannot close file\n");
      exit(1);
    }
    if (fclose(fp_B) != 0) {
      printf("my-diff: cannot close file\n");
      exit(1);
    }

    // Check the arrays of file contents
    if (fileA_size < fileB_size) {
      int is_different = 0;  // default: the two files are different
      int index = 0;        // index to track the two arrays
      while (index < fileA_size) {
        if (strcmp(fileA_content[index], fileB_content[index]) != 0) {
          if (is_different == 0) {  // different for the first time
            printf("%d\n", index + 1);
            is_different = 1;
          }
          printf("< %s\n", fileA_content[index]);
          printf("> %s\n", fileB_content[index]);
        } else {
          is_different = 0;
        }
        index++;
      }
      if (is_different == 0) {  // different for the first time
        printf("%d\n", index + 1);
      }
      while (index < fileB_size) {
        printf("> %s\n", fileB_content[index]);
        index++;
      }
    } else if (fileA_size > fileB_size) {
      int is_different = 0;  // default: the two files are different
      int index = 0;        // index to track the two arrays
      while (index < fileB_size) {
        if (strcmp(fileA_content[index], fileB_content[index]) != 0) {
          if (is_different == 0) {  // different for the first time
            printf("%d\n", index + 1);
            is_different = 1;
          }
          printf("< %s\n", fileA_content[index]);
          printf("> %s\n", fileB_content[index]);
        } else {
          is_different = 0;
        }
        index++;
      }
      if (is_different == 0) {  // different for the first time
        printf("%d\n", index + 1);
      }
      while (index < fileA_size) {
        printf("< %s\n", fileA_content[index]);
        index++;
      }
    } else {                // the length of two files equals
      int is_different = 0;  // default: the two files are different
      int index = 0;        // index to track the two arrays
      while (index < fileA_size) {
        if (strcmp(fileA_content[index], fileB_content[index]) != 0) {
          if (is_different == 0) {  // different for the first time
            printf("%d\n", index + 1);
            is_different = 1;
          }
          printf("< %s\n", fileA_content[index]);
          printf("> %s\n", fileB_content[index]);
        } else {
          is_different = 0;
        }
        index++;
      }
    }

    // Free the array of contents after output
    for (int i = 0; i < 256; i++) {
      free(fileA_content[i]);
    }
    free(fileA_content);
    fileA_content = NULL;

    for (int i = 0; i < 256; i++) {
      free(fileB_content[i]);
    }
    free(fileB_content);
    fileB_content = NULL;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Main File:        my-look.c, across.c, my-diff.c
// This File:        my-look.c
// Other Files:      across.c, my-diff.c
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

/**
 * This is the main function of the program
 * This method checks the user's input argument and implements the my-look
 * utility
 *
 * @param int argc, char *argv[]
 * @return int
 */
int main(int argc, char *argv[]) {
  // Check user input
  int num_argc = argc - 1;

  if (num_argc == 1) { // If num_argc is 1, search the file
                       // /usr/share/dict/words
    char *target = argv[1];

    // Open the file the user inputs
    FILE *fp = fopen("/usr/share/dict/words", "r");
    if (fp == NULL) {
      printf("my-look: cannot open file\n");
      exit(1);
    }

    int target_length = strlen(argv[1]);

    // Read the file and make comparison
    char line[1024] = {""};
    while (fgets(line, 256, fp) != NULL) {
      // Format the line in the file
      strtok(line, "\n");

      char line_sub_string[256] = {""};
      strncpy(line_sub_string, line, target_length);
      if (strcasecmp(line_sub_string, target) == 0) {
        printf("%s\n", line);
      }
    }

    // Close the file after look
    if (fclose(fp) != 0) {
      printf("my-look: cannot close file\n");
      exit(1);
    }
  } else if (num_argc == 2) {  // if num_argc is 2, open the file and search
    char *target = argv[1];
    char *file_name = argv[2];

    // Open the file the user inputs
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
      printf("my-look: cannot open file\n");
      exit(1);
    }

    int target_length = strlen(argv[1]);

    // Read the file and make comparison
    char line[1024] = {""};
    while (fgets(line, 256, fp) != NULL) {
      // Format the line in the file
      strtok(line, "\n");

      char line_sub_string[256] = {""};
      strncpy(line_sub_string, line, target_length);
      if (strcasecmp(line_sub_string, target) == 0) {
        printf("%s\n", line);
      }
    }

    // Close the file after look
    if (fclose(fp) != 0) {
      printf("my-look: cannot close file\n");
      exit(1);
    }
  } else {  // print error information to the screen
    printf("my-look: invalid number of arguments\n");
    exit(1);
  }
  return 0;
}

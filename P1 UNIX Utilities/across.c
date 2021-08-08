////////////////////////////////////////////////////////////////////////////////
// Main File:        my-look.c, across.c, my-diff.c
// This File:        across.c
// Other Files:      my-look.c, my-diff.c
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
 * This method checks if the string meets the requirement
 * Discard if non-alpha or upper
 *
 * @param char *word
 * @return int 1 if correct, 0 if not correct
 */
int is_correct(char word[]) {
  for (int i = 0; i < strlen(word); i++) {
    // Check if upper case and punctuation exits in the string
    if (!isalpha(word[i])) {
      return 0;
    }
    if (isupper(word[i])) {
      return 0;
    }
  }
  return 1;
}

/**
 * This is the main function of the program
 * This method checks the user's input argument and implements the across
 * utility
 *
 * @param int argc, char *argv[]
 * @return int
 */
int main(int argc, char *argv[]) {
  // Check user input
  int num_argc = argc - 1;

  if (num_argc == 3) {  // If num_argc is 3
    // Check the length of input
    int sub_string_length = strlen(argv[1]);
    int user_string_length = atoi(argv[3]) - atoi(argv[2]);

    if (user_string_length <
        sub_string_length) { // If the user length is smaller or equal to the
                             // length of substring
      printf("across: invalid position\n");
      exit(1);

    } else {  // If the user length is valid
      // Open the file the user inputs
      FILE *fp = fopen("/usr/share/dict/words", "r");
      if (fp == NULL) {
        printf("across: cannot open file\n");
        exit(1);
      }

      // Read the file and make comparison
      char line[256] = {""};
      while (fgets(line, 256, fp) != NULL) {
        strtok(line, "\n");
        if (is_correct(line)) {
          // Get substring from a line according to the user
          char line_sub_string[256] = {""};
          strncpy(line_sub_string, line + atoi(argv[2]), user_string_length);

          // If substring matches and length of string matched, print result to
          // screen
          if (strncmp(argv[1], line_sub_string, strlen(argv[1])) == 0 &&
              strlen(line) == atoi(argv[3])) {
            printf("%s\n", line);
          }
        }
      }

      // Close the file after look
      if (fclose(fp) != 0) {
        printf("across: cannot close file\n");
        exit(1);
      }
    }

  } else if (num_argc == 4) {  // If num_argc is 4
    // Check the length of input
    int sub_string_length = strlen(argv[1]);
    int user_string_length = atoi(argv[3]) - atoi(argv[2]);

    if (user_string_length <
        sub_string_length) { // If the user length is smaller or equal to the
                             // length of substring
      printf("across: invalid position\n");
      exit(1);

    } else {  // If the user length is valid
      // Open the file the user inputs
      char *file_name = argv[4];

      FILE *fp = fopen(file_name, "r");
      if (fp == NULL) {
        printf("across: cannot open file\n");
        exit(1);
      }

      // Read the file and make comparison
      char line[256] = {""};
      while (fgets(line, 256, fp) != NULL) {
        strtok(line, "\n");
        if (is_correct(line)) {
          // Get substring from a line according to the user
          char line_sub_string[256] = {""};
          strncpy(line_sub_string, line + atoi(argv[2]), user_string_length);

          // If substring matches and length of string matched, print result to
          // screen
          if (strncmp(argv[1], line_sub_string, strlen(argv[1])) == 0 &&
              strlen(line) == atoi(argv[3])) {
            printf("%s\n", line);
          }
        }
      }

      // Close the file after look
      if (fclose(fp) != 0) {
        printf("across: cannot close file\n");
        exit(1);
      }
    }

  } else {
    printf("across: invalid number of arguments\n");
    exit(1);
  }
  return 0;
}

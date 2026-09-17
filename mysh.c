#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Maximum number of arguments for one command
#define MAX_ARGS 128

int main(int argc, char** argv) {
  char* line = NULL;     // Pointer holding the line read in
  size_t line_size = 0;  // Bytes available in line

  // Loop forever
  while (true) {
    // Print the shell prompt
    printf("$ ");

    // Get a line of stdin
    if (getline(&line, &line_size, stdin) == -1) {
      if (errno == EINVAL) {
        perror("Unable to read command line");
        exit(2);
      } else {
        printf("\nShutting down...\n");
        break;
      }
    }

    char *linePointer = line;
    char *currSentence;

    //semicolon splitting
    while ((currSentence = strsep(&linePointer, ";&")) != NULL) {
      char** args = malloc((MAX_ARGS + 1) * sizeof(char*));
      int counter = 0;
      char* currWord;

      // spaces or newline splitting
      while ((currWord = strsep(&currSentence, " \n")) != NULL) {
        if (*currWord != '\0') {
          args[counter] = currWord;
          counter++;
        }
      }

      //sets our last element to null
      args[counter] = NULL;

      // If only "enter" was pressed
      if (counter == 0) {
        free(args);
        continue;
      }

      // Execute command
      pid_t child_id = fork();
      if (child_id == 0) {
        execvp(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
      } else if (child_id > 0) {
        int status;
        wait(&status);
        printf("%s exited with status %d\n", args[0], WEXITSTATUS(status));
      } else {
        perror("fork failed");
        free(args);
        exit(EXIT_FAILURE);
      }

      free(args);
    }
  }

  if (line != NULL) {
    free(line);
  }

  return 0;
}
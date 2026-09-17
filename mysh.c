#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// This is the maximum number of arguments your shell should handle for one command
#define MAX_ARGS 128

int main(int argc, char** argv) {
  char* line = NULL;     // Pointer that will hold the line we read in
  size_t line_size = 0;  // The number of bytes available in line
  // char*[] args = new char*[MAX_ARGS + 1];

  // Loop forever
  while (true) {
    // Print the shell prompt
    printf("$ ");

    // Get a line of stdin, storing the string pointer in line
    if (getline(&line, &line_size, stdin) == -1) {
      if (errno == EINVAL) {
        perror("Unable to read command line");
        exit(2);
      } else {
        // Must have been end of file (ctrl+D)
        printf("\nShutting down...\n");

        // Exit the infinite loop
        break;
      }
    }

    // TODO: Execute the command instead of printing it below
    int counter = 0;
    char* token = line;
    char** args = malloc((MAX_ARGS + 1)* sizeof(char*));
    char* cur_arg;
    for (int i = 0; (cur_arg = strsep(&token, " ")) != NULL; i++) {
      if (*cur_arg != '\0') {
        counter++;
        args[i] = cur_arg;
      }
    }
    args[counter] = NULL;
    // forking and executing the command
    printf("Received command: %s\n", line);

    int curr_process = 0;
    while (curr_process < counter) {
      pid_t child_id = fork();
      if (child_id == 0) {
        execvp(args[curr_process], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
      } else if (child_id > 0) {
        int status;
        wait(&status);
        printf("%s exited with status %d\n", args[curr_process], WEXITSTATUS(status));
      } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
      }
      curr_process++;
    }
  }

  // If we read in at least one line, free this space
  if (line != NULL) {
    free(line);
  }

  return 0;
}

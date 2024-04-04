  #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ENV_VARIABLES 100
#define MAX_ENV_VAR_LENGTH 100

void print_environment_variables(char *envp[]) {
    char *variables[MAX_ENV_VARIABLES];
    int count = 0;

    for (int i = 0; envp[i] != NULL && count < MAX_ENV_VARIABLES; i++) {
        variables[count++] = envp[i];
    }

    qsort(variables, count, sizeof(char *), (int (*)(const void *, const void *))strcmp);

    for (int i = 0; i < count; i++) {
        printf("%s\n", variables[i]);
    }
}

int main(int argc, char *argv[], char *envp[]) {
    char *child_path = getenv("CHILD_PATH");
    char option;
    int child_count = 0;

    // из файла получаем колл запущ ./parent
    FILE *count_file = fopen("child_count.txt", "r");   
    if (count_file != NULL) {
        fscanf(count_file, "%d", &child_count);
        fclose(count_file);
    }

    if (child_path == NULL) {
        fprintf(stderr, "CHILD_PATH variable is not set.\n");
        return 1;
    }

    while (1) {
        printf("Enter an option (+, *, &, q): ");
        scanf(" %c", &option);

        if (option == '+' || option == '*' || option == '&') {
            // cздаем новый процесс
            pid_t pid = fork();
            if (pid == 0) {  // Дочерний процесс
                char child_program_name[10];
                snprintf(child_program_name, sizeof(child_program_name), "child_%02d", child_count);

                child_count++;

                FILE *count_file = fopen("child_count.txt", "w");
                if (count_file != NULL) {
                    fprintf(count_file, "%d", child_count);
                    fclose(count_file);
                }

                char *minimal_environment[] = {
                    "SHELL", getenv("SHELL"),
                    "HOME", getenv("HOME"),
                    "HOSTNAME", getenv("HOSTNAME"),
                    "LOGNAME", getenv("LOGNAME"),
                    "LANG", getenv("LANG"),
                    "TERM", getenv("TERM"),
                    "USER", getenv("USER"),
                    "LC_COLLATE", getenv("LC_COLLATE"),
                    "PATH", getenv("PATH"),
                    NULL
                };

                switch (option) {
                    case '+':
                        execve(child_path, (char *[]) { child_program_name, "/home/ilya/Osisp_BSUIR/lab2/env.txt",NULL }, minimal_environment);
                        break;
                    case '*':
                        execve(child_path, (char *[]) { child_program_name, NULL }, envp);
                        break;
                    case '&':
                        execve(child_path, (char *[]) { child_program_name, NULL }, envp);
                        break;
                    default:
                        fprintf(stderr, "Invalid option\n");
                        return 1;
                }
            } else if (pid > 0) {  // Род процесс
                wait(NULL);
            } else {  
                fprintf(stderr, "Failed to fork.\n");
                return 1;
            }
        } else if (option == 'q') { 
            break;
        } else {  
            printf("Invalid option.\n");
        }
    }

    return 0;
}

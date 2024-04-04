#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

int main(int argc, char *argv[]) {
    // Выводим информацию о себе
    printf("Child process:\n");
    printf("Name: %s\n", argv[0]);
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LENGTH];

    // читаем и выводим переменные окружения из файла
    while (fgets(line, sizeof(line), file) != NULL) {
        char *delimiter = strchr(line, '=');
        if (delimiter != NULL) {
            *delimiter = '\0'; 
            char *var_name = line;
            char *var_value = delimiter + 1;

            // удаляем символ новой строки из значенияб если он есть
            char *newline = strchr(var_value, '\n');
            if (newline != NULL) {
                *newline = '\0';
            }

            printf("%s: %s\n", var_name, var_value);
        }
    }

    fclose(file);
    return 0;
}

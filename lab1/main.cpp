#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

typedef struct Options{
    bool l;
    bool d;
    bool f;
    bool s;
    int basicPath;
}Options;

Options switchFlags(int argc, char* argv[]) {
    Options options = {false, false, false, false, -1};
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') options.basicPath = i;
    }

    for (int i = 1;i < argc; i++) {
        if (argv[i][0] == '-') {
            int j = 1;
            while (argv[i][j] != '\0') {
                if (argv[i][j] == 'l') options.l = true;
                if (argv[i][j] == 'd') options.d = true;
                if (argv[i][j] == 'f') options.f = true;
                if (argv[i][j] == 's') options.s = true;
                j++;
            }
        }
    }

    return options;
}

int compare(const struct dirent **a, const struct dirent **b) {
    return strcmp((*a)->d_name, (*b)->d_name);
}

void printDirectory(Options *options, char *path) {

    struct dirent **dir;
    int n;

    if(options->s) n = scandir(path, &dir, NULL, compare);
    else n = scandir(path, &dir, NULL, NULL);

    for(int i = 0; i < n;i++) {
        if (options->f && dir[i]->d_type == DT_REG) printf("%s/%s\n", path, dir[i]->d_name);
        if (options->d && dir[i]->d_type == DT_DIR) printf("%s/%s\n", path, dir[i]->d_name);
        if (options->l && dir[i]->d_type == DT_LNK) printf("%s/%s\n", path, dir[i]->d_name);

        if(dir[i]->d_type == DT_DIR)
        {
            if (strcmp(dir[i]->d_name, ".") == 0 || strcmp(dir[i]->d_name, "..") == 0) continue;
            char newPath[1024];
            strcpy(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, dir[i]->d_name);
            printDirectory(options, newPath);
        }
    }
}

int main(int argc, char* argv[]) {

    Options options = switchFlags(argc, argv);
    char path[256];

    options.basicPath == -1 ? strcpy(path, "./") : strcpy(path, argv[options.basicPath]);

    if(!options.s && !options.f && !options.d && !options.l) {
        options.l = true;
        options.s = true;
        options.d = true;
        options.f = true;
    }

    printDirectory(&options, path);

    return 0;
}
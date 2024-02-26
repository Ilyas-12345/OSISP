#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define PATH_LEN 1024

struct Options {
    int l;
    int d;
    int f;
    int s;
};

void process_directory(const char* directory, const struct Options* opt) {
    struct dirent** entries;
    int num_entries;

    num_entries = opt->s ? scandir(directory, &entries, NULL, alphasort) : scandir(directory, &entries, NULL, NULL);

    if (num_entries < 0) {
        perror("scandir");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_entries; ++i) {
        char full_path[PATH_LEN];
        snprintf(full_path, PATH_LEN, "%s/%s", directory, entries[i]->d_name);

        struct stat buf;
        if (lstat(full_path, &buf) == 0) {
            if ((S_ISLNK(buf.st_mode) && opt->l) ||
                (S_ISDIR(buf.st_mode) && opt->d) ||
                (S_ISREG(buf.st_mode) && opt->f) ||
                (opt->l == 0 && opt->d == 0 && opt->f == 0)) {
                printf("%s\n", full_path);
            }
            if (S_ISDIR(buf.st_mode) && strcmp(entries[i]->d_name, ".") != 0 && strcmp(entries[i]->d_name, "..") != 0) {
                process_directory(full_path, opt);
            }
        }
        free(entries[i]);
    }
    free(entries);
}

void init_options(struct Options* opt) {
    opt->l = 0;
    opt->d = 0;
    opt->f = 0;
    opt->s = 0;
}

int main(int argc, char* argv[]) {
    struct Options options;
    init_options(&options);
    int opt;

    while ((opt = getopt(argc, argv, "ldfs")) != -1) {
        switch (opt) {
            case 'l':
                options.l = 1;
                break;
            case 'd':
                options.d = 1;
                break;
            case 'f':
                options.f = 1;
                break;
            case 's':
                options.s = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-d] [-f] [-s] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    char directory[PATH_LEN];
    strcpy(directory, argc > optind ? argv[optind] : ".");

    process_directory(directory, &options);

    return 0;
}

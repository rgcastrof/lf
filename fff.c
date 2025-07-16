#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <linux/limits.h>

#define VERSION "0.0.4"

typedef struct Context {
    char *base_path;
    char *current_path;
    size_t path_size;
    const char *file;
} Context;

Context*
create_context(char *path, const char *file)
{
    Context *c = malloc(sizeof(Context));
    if (!c)
        return NULL;
    c->base_path = strdup(path);
    if (!c->base_path) {
        free(c);
        return NULL;
    }
    c->current_path = malloc(strlen(path) + 1);
    if (!c->current_path) {
        free(c->base_path);
        free(c);
        return NULL;
    }

    strcpy(c->current_path, path);
    c->path_size = strlen(path) + 1;
    c->file = file;
    return c;
}

void
destroy_context(Context *c)
{
    free(c->base_path);
    free(c->current_path);
    free(c);
}

void
dfs(Context *c)
{
    DIR* dir = opendir(c->current_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (entry->d_type == DT_REG && strcmp(entry->d_name, c->file) == 0) {
            closedir(dir);
            printf("%s/%s\n", c->current_path, c->file);
            return;
        }

        if (entry->d_type == DT_DIR) {
            size_t last_size = strlen(c->current_path);
            size_t reserve = last_size + strlen(entry->d_name) + 2;
            if (reserve > c->path_size) {
                char *temp = realloc(c->current_path, reserve);
                if (!temp) {
                    return;
                }
                c->current_path = temp;
                c->path_size = reserve;
            }
            if (c->current_path[strlen(c->current_path) - 1] != '/') {
                strcat(c->current_path, "/");
            }
            strcat(c->current_path, entry->d_name);
            dfs(c);
            c->current_path[last_size] = '\0';
        }
    }

    closedir(dir);
    return;
}

int
main(int argc, char *argv[])
{
    Context *c = NULL;
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0) {
            printf("fff version: %s\n", VERSION);
            return EXIT_SUCCESS;
        } else {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                c = create_context(cwd, argv[1]);
                dfs(c);
            } else {
                perror("getcwd");
                return 1;
            }
        }
    } else if (argc == 3) {
        c = create_context(argv[1], argv[2]);
        dfs(c);
    }
    destroy_context(c);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define VERSION "0.0.9"

typedef struct Context {
    char *base_path;
    char *current_path;
    size_t path_size;
    const char *file;
    const char *ext;
    int found;
    int limit;
} Context;

typedef void (*Callback) (Context *, const char *);

static Context* create_context(char *path, const char *file, const char *ext, int limit);
static void destroy_context(Context *c);
static void check_file(Context *c, const char *file);
static void dfs(Context *c, Callback cb);
static void search_file(Context *c);
static void usage(const char *command);
static void help(const char *command);

static const char *exts[] = {
    ".pdf", ".txt", ".c", ".h", ".jpg", ".png", ".jpeg", ".go", ".java", ".md", ".odt",
};
static int n_exts = sizeof(exts) / sizeof(exts[0]);

static Context*
create_context(char *path, const char *file, const char *ext, int limit)
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
    c->ext = ext;
    c->file = file;
    c->found = 0;
    c->limit = limit;
    return c;
}

static void
destroy_context(Context *c)
{
    free(c->base_path);
    free(c->current_path);
    free(c);
}

static void
check_file(Context *c, const char *file)
{
    if (c->ext != NULL && c->file == NULL) {
        int ext_flag = 0;
        for (int i = 0; i < n_exts; i++) {
            if (strcmp(c->ext, exts[i]) == 0) {
                ext_flag = 1;
                break;
            }
        }
        if (ext_flag) {
            const char *dot = strrchr(file, '.');
            if (dot && strcmp(c->ext, dot) == 0) {
                printf("%s/%s\n", c->current_path, file);
                c->found++;
            }
        }
    } else if (c->ext == NULL && c->file != NULL) {
        if (strstr(file, c->file)) {
            printf("%s/%s\n", c->current_path, file);
            c->found++;
        }
    }
}

static void
dfs(Context *c, Callback cb)
{
    DIR* dir = opendir(c->current_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (entry->d_type == DT_REG) {
            cb(c, entry->d_name);
            continue;
        }

        if (entry->d_type == DT_DIR) {
            size_t last_size = strlen(c->current_path);
            size_t reserve = last_size + strlen(entry->d_name) + 2;
            if (reserve > c->path_size) {
                char *temp = realloc(c->current_path, reserve);
                if (!temp) {
                    closedir(dir);
                    return;
                }
                c->current_path = temp;
                c->path_size = reserve;
            }
            if (c->current_path[strlen(c->current_path) - 1] != '/') {
                strcat(c->current_path, "/");
            }
            strcat(c->current_path, entry->d_name);
            dfs(c, cb);
            if (c->limit > 0 && c->found >= c->limit) {
                return;
            }
            c->current_path[last_size] = '\0';
        }
    }

    closedir(dir);
    return;
}

static void
search_file(Context *c)
{
    dfs(c, check_file);
}

static void
usage(const char *command)
{
    printf("Usage: %s -d [dir] -f [file]\n", command);
    printf("if a directory is not passed as an argument, %s will look in the current directory\n\n", command);
    printf("Type %s -h to see a list of all options.\n", command);
}

static void
help(const char *command)
{
    printf("usage: %s -d [dir] -f [file]\n\n", command);
    printf("%s version: %s\n\n", command, VERSION);
    printf("options:\n");
    printf("-h,            show this help message and exit\n");
    printf("-f,            specify a file\n");
    printf("-d,            specify a directory\n");
    printf("-l,            specify total lines\n");
    printf("-e,            search files by their extension\n");
}

int
main(int argc, char *argv[])
{
    const char* file = NULL;
    char *start_dir =  NULL;
    const char *ext = NULL;
    int limit = -1;
    Context *c = NULL;
    int opt;
    int free_start_dir = 0;
    if (argc == 1) {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }
    
    while ((opt = getopt(argc, argv, "f:d:l:e:h")) != -1) {
        switch (opt) {
            case 'f':
                file = optarg;
                break;
            case 'd':
                start_dir = optarg;
                break;
            case 'l':
                limit = atoi(optarg);
                break;
            case 'e':
                ext = optarg;
                break;
            case 'h':
                help(argv[0]);
                return EXIT_SUCCESS;
            default:
                printf("type %s -h for help\n", argv[0]);
                return EXIT_SUCCESS;
        }
    }
    if (!start_dir) {
        start_dir = getcwd(NULL, 0);
        if(!start_dir) {
            perror("getcwd");
            return EXIT_FAILURE;
        }
        free_start_dir = 1;
    }
    if (ext && file) {
        printf("Error: -f and -e parameters cannot be used at the same time\n");
        return EXIT_FAILURE;
    }
    c = create_context(start_dir, file, ext, limit);
    search_file(c);
    destroy_context(c);
    if (free_start_dir)
        free(start_dir);
    return 0;
}

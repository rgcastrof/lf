#include <stdio.h>
#include <dirent.h>
#include <string.h>

#define MAXLEN 1000

static void find(const char *path, const char *file);
static void dfs(const char *path, const char *file);
static int isdotdir(unsigned char dirtype, const char *dirname);
static int isfound(unsigned char dirtype, const char *dirname, const char *file);
static int emptyname(const char *file);
static void printerr(const char *fmt, ...);

int
main(int argc, char *argv[])
{
	char fullpath[MAXLEN];
	find(argv[1], argv[2]);
	return 0;
}

static void
dfs(const char *path, const char *file)
{
	DIR *dir = opendir(path);
	if (!dir) {
		printerr("Could not open dir: %s", path);
		return;
	};

	struct dirent *entry;
	while ((entry = readdir(dir))) {
		if (isdotdir(entry->d_type, entry->d_name))
			continue;
		else {
			char fullpath[MAXLEN];
			snprintf(fullpath, MAXLEN, "%s/%s", path, entry->d_name);
			if (isfound(entry->d_type, entry->d_name, file) || emptyname(file))
				printf("%s\n", fullpath);

			if (entry->d_type == DT_DIR)
				dfs(fullpath, file);
		}
	}

	closedir(dir);
}

static int
isdotdir(unsigned char dirtype, const char *dirname)
{
	if (dirtype == DT_DIR &&
	(strcmp(dirname, ".") == 0 ||
	 strcmp(dirname, "..") == 0)) {
		return 1;
	}
	return 0;
}

static int
isfound(unsigned char dirtype, const char *dirname, const char *file)
{
	if (dirtype == DT_REG && strcmp(dirname, file) == 0)
		return 1;
	return 0;
}

static int
emptyname(const char *file)
{
	if (!file)
		return 1;
	return 0;
}

static void
printerr(const char *fmt, ...)
{
	va_list args;
	fputs("Error: ", stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
}

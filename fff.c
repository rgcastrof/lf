#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN 4096

static void find(const char *path, const char *file);
static void dfs(const char *path, const char *file);
static int isdotdir(unsigned char dirtype, const char *dirname);
static int isfound(unsigned char dirtype, const char *dirname, const char *file);
static int emptyname(const char *file);
static void printerr(const char *fmt, ...);
static void die(const char *fmt, ...);
static void usage(void);

int
main(int argc, char *argv[])
{
	if (argc <= 2) {
		char cwd[MAXLEN];
		if (!getcwd(cwd, MAXLEN)) {
			printerr("Failed getcwd");
			return 1;
		} else {
			if (argc == 1)
				find(cwd, "");
			else
				find(cwd, argv[1]);
			return 0;
		}
	}
	
	find(argv[1], argv[2]);
	return 0;
}

static void
find(const char *path, const char *file)
{
	dfs(path, file);
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

		char fullpath[MAXLEN];
		if (path && path[strlen(path)-1] == '/')
			snprintf(fullpath, MAXLEN, "%s%s", path, entry->d_name);
		else
			snprintf(fullpath, MAXLEN, "%s/%s", path, entry->d_name);

		if (isfound(entry->d_type, entry->d_name, file) || emptyname(file))
			printf("%s\n", fullpath);

		if (entry->d_type == DT_DIR)
			dfs(fullpath, file);
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
	if (file && file[0] == '\0')
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

static void
die(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	exit(EXIT_SUCCESS);
}

static void
usage(void)
{
	die("Usage: fff [-h] [-v] [path] [filename]\n\n"
			"Options:\n"
			"  -h		Show this help message and exit\n"
			"  -v		Show version\n");
}

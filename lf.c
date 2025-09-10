#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLEN 4096
#define VERSION "lf-0.9"

typedef struct {
	const char *startpath;
	const char *file;
	int maxdepth;
	FILE *out;
} Args;

static void initargs(Args *args);
static void find(const Args *args, const char *currentpath, int depth);
static int isdotdir(const unsigned char type, const char *name);
static int matchfile(const unsigned char type, const char *name, const char *file);
static int isempty(const char *file);
static void writefile(FILE **out, char *outfile);
static void error(const char *fmt, ...);
static void fatal(FILE *stream, const char *fmt, ...);
static void usage(void);

int
main(int argc, char *argv[])
{
	char cwd[MAXLEN];

	if (argc == 1) {
		if (!getcwd(cwd, MAXLEN)) fatal(stderr, "Failed getcwd");
		find(cwd, "");
		return EXIT_SUCCESS;
	}
	if (argc == 2) {
		if (!strcmp(argv[1], "-v")) {
			puts(VERSION);
			return EXIT_SUCCESS;
		}
		else if (!strcmp(argv[1], "-h")) usage();
		else if (isdir(argv[1])) find(argv[1], "");
		else {
			if (!getcwd(cwd, MAXLEN)) fatal(stderr, "Failed getcwd");
			find(cwd, argv[1]);
		}
		return EXIT_SUCCESS;
	}
	find(argv[1], argv[2]);
	return EXIT_SUCCESS;
}

static void
initargs(Args *args)
{
	args->startpath = ".";
	args->file = "";
	args->maxdepth = -1;
	args->out = stdout;
}

static void
find(const Args *args, const char *currentpath, int depth)
{
	DIR *dir = opendir(currentpath);

	if (args->maxdepth >= 0 && depth > args->maxdepth)
		return;
	
	if (!dir) {
		error("Could not open dir: %s", currentpath);
		return;
	}

	struct dirent *entry;
	while ((entry = readdir(dir))) {
		if (isdotdir(entry->d_type, entry->d_name))
			continue;

		char fullpath[MAXLEN];
		snprintf(fullpath, MAXLEN, "%s%s%s",
			currentpath,
			(currentpath[strlen(currentpath)-1] == '/') ? "" : "/", entry->d_name);

		if (matchfile(entry->d_type, entry->d_name, args->file) || isempty(args->file))
			fprintf(args->out, "%s\n", fullpath);
		if (entry->d_type == DT_DIR)
			find(args, fullpath, depth + 1);
	}

	closedir(dir);
}

static void
writefile(FILE **out, char *outfile)
{
	*out = fopen(outfile, "w");
	if (!(*out))
		fatal(stderr, "failed to open file");
}

static int
isdotdir(const unsigned char type, const char *name)
{
	return type == DT_DIR && (!strcmp(name, ".") || !strcmp(name, ".."));
}

static int
matchfile(const unsigned char type, const char *name, const char *file)
{
	return type == DT_REG && !strcmp(name, file);
}

static int
isempty(const char *file)
{
	return file && file[0] == '\0';
}

static void
error(const char *fmt, ...)
{
	va_list args;
	fputs("Error: ", stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
}

static void
fatal(FILE *stream, const char *fmt, ...)
{
	va_list args;
	fputs("Error: ", stream);
	va_start(args, fmt);
	vfprintf(stream, fmt, args);
	va_end(args);
	fputc('\n', stream);
	exit(EXIT_FAILURE);
}

static void
usage(void)
{
	printf("Usage: lf [-h] [-v] [path] [filename]\n\n"
			"Options:\n"
			"  -h		Show this help message and exit\n"
			"  -v		Show version\n");
	exit(EXIT_SUCCESS);
}

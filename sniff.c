#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN 4096

typedef struct {
	const char *startpath;
	const char *file;
	int maxdepth;
	char type;
	FILE *out;
	int pflag;
} Args;

static void initargs(Args *args);
static void find(const Args *args, const char *currentpath, int depth);
static int isdotdir(const unsigned char type, const char *name);
static int matchfile(const unsigned char type, const char *name, const char *file);
static int matchpartial(const unsigned char type, const char *name, const char *file);
static int isempty(const char *file);
static void writefile(FILE **out, char *outfile);
static void error(const char *fmt, ...);
static void fatal(FILE *stream, const char *fmt, ...);
static void usage(void);

int
main(int argc, char *argv[])
{
	int opt;
	Args args;
	char *outfile = NULL;
	initargs(&args);

	if (argc == 1) {
		find(&args, args.startpath, 0);
		return EXIT_SUCCESS;
	}

	while ((opt = getopt(argc, argv, "hvd:o:t:p")) != -1) {
		switch (opt) {
			case 'v':
				puts("sniff-"VERSION);
				return EXIT_SUCCESS;
			case 'd':
				args.maxdepth = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 't':
				args.type = optarg[0];
				break;
			case 'p':
				args.pflag = 1;
				break;
			case 'h':
			default:
				usage();
		}
	}

	if (optind < argc)
		args.startpath = argv[optind];
	if (optind + 1 < argc)
		args.file = argv[optind+1];
	if (outfile)
		writefile(&args.out, outfile);

	find(&args, args.startpath, 0);

	if (outfile && args.out != stdout)
		fclose(args.out);
	return EXIT_SUCCESS;
}

static void
initargs(Args *args)
{
	args->startpath = ".";
	args->file = "";
	args->maxdepth = -1;
	args->type = '\0';
	args->out = stdout;
	args->pflag = 0;
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

		if (matchfile(entry->d_type, entry->d_name, args->file) || isempty(args->file) ||
		(args->pflag && matchpartial(entry->d_type, entry->d_name, args->file)))
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
matchpartial(const unsigned char type, const char *name, const char *file)
{
	return type == DT_REG && strstr(name, file);
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
	printf("Usage: sniff [-h] [-v] [-d depth] [-o file] [-p] [path] [filename]\n\n"
			"Options:\n"
			"  -d depth		Limit search to a maximum directory depth\n"
			"  -o file		Write output to the specified file instead of stdout\n"
			"  -p			Enable partial match on filename\n"
			"  -h			Show this help message and exit\n"
			"  -v			Show version\n");
	exit(EXIT_SUCCESS);
}

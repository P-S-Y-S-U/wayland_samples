#include <stdio.h>
#include <fcntl.h>
#include <time.h>

static int
current_time_str(char *str, size_t len, const char *fmt)
{
	time_t t;
	struct tm *t_local;
	int ret;

	t = time(NULL);
	t_local = localtime(&t);
	if (!t_local) {
		return -1;
	}

	ret = strftime(str, len, fmt, t_local);
	if (ret == 0) {
		return -1;
	}

	return ret;
}

FILE *
file_create_dated(const char *path, const char *prefix, const char *suffix,
		  char *name_out, size_t name_len)
{
	char timestr[128];
	int ret;
	int fd;
	int cnt = 0;
	int with_path;

	with_path = path && path[0];

	if (current_time_str(timestr, sizeof(timestr), "%F_%H-%M-%S") < 0)
		return NULL;

	ret = snprintf(name_out, name_len, "%s%s%s%s%s",
		       with_path ? path : "", with_path ? "/" : "",
		       prefix, timestr, suffix);
	if (ret < 0 || (size_t)ret >= name_len) {
		return NULL;
	}

	fd = open(name_out, O_RDWR | O_CLOEXEC | O_CREAT | O_EXCL, 00666);

	if (fd == -1)
		return NULL;

	return fdopen(fd, "w");
}

int main( int argc, const char* argv[] )
{
	char fname[1024];
	FILE* fp;

	fp = file_create_dated(NULL, "surfaceshot-", ".pam", fname, sizeof(fname));
	
	char str[] = "set string\n";
	fwrite( str, 1, sizeof(str), fp);

	return 0;
}

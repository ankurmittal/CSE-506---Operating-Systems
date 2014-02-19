#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define __NR_xconcat	349	/* our private syscall number */

struct option_args {
	int output_mode;
	char atomic_concat_mode;
	char return_no_of_files;
	char return_percent_data;
	char mode_set;
	char mode_arg;
	char print_short_usuage_string;
};

void parseOptions(int argc, char *argv[], struct option_args *options)
{
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "acteANPmh:")) != -1) {
		switch (c) {
		case 'a':
			options->output_mode |= O_APPEND;
			break;
		case 'c':
			options->output_mode |= O_CREAT;
			break;
		case 't':
			options->output_mode |= O_TRUNC;
			break;
		case 'e':
			options->output_mode |= O_EXCL;
			break;
		case 'A':
			options->atomic_concat_mode = 1;
			break;
		case 'N':
			options->return_no_of_files = 1;
			break;
		case 'P':
			options->return_percent_data = 1;
			break;
		case 'h':
			options->print_short_usuage_string = 1;
			break;
		case 'm':
			options->mode_set = 0 ;
			options->mode_arg = *optarg;
			break;
		case '?':
			if (optopt == 'c')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			exit(1);
		default:
			abort();
		}
	}

}

int main(int argc, char *argv[])
{
	int index;
	struct option_args options = {0};
	parseOptions(argc, argv, &options);
	for (index = optind; index < argc; index++)
		printf("Non-option argument %s\n", argv[index]);
	/*int rc;
	  void *dummy = (void *) atoi(argv[1]);

	  rc = syscall(__NR_xconcat, dummy);
	  if (rc == 0)
	  printf("syscall returned %d\n", rc);
	  else
	  printf("syscall returned %d (errno=%d)\n", rc, errno);

	  exit(rc);*/
	return 0;
}





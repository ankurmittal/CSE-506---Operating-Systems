#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define __NR_xconcat	349	/*our private syscall number*/
struct option_args {
	int output_mode;
	uint atomic_concat_mode : 1;
	uint return_no_of_files : 1;
	uint return_percent_data : 1;
	uint mode_set : 1;
	int mode_arg;
	uint print_short_usuage_string : 1;
};

struct syscall_params {
	const char *outfile; /*name of output file*/
	const char **infiles; /*array with names of input files*/
	unsigned int infile_count; /*number of input files in infiles array*/
	int oflags; /*Open flags to change behavior of syscall*/
	mode_t mode; /*default permission mode for newly created outfile*/
	unsigned int flags; /*special flags to change behavior of syscall*/
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
	int index, rc;

	struct syscall_params sys_param;
	struct option_args options = {0};
	parseOptions(argc, argv, &options);
	if (optind < argc)
		sys_param.outfile = argv[optind];
	if (optind + 1 >= argc) {
		printf("Insufficient Arguments");
		return 1;
	}
	sys_param.infile_count = argc-optind-1;
	sys_param.infiles = malloc(sizeof(char *)*(sys_param.infile_count));
	for (index = optind + 1; index < argc; index++)
		sys_param.infiles[index-optind-1] = argv[index];
	sys_param.oflags = options.output_mode;
	sys_param.flags = 1;
	void *p = ((void *) &sys_param);

	rc = syscall(__NR_xconcat, p);
	if (rc == 0)
		printf("syscall returned %d\n", rc);
	else
		printf("syscall returned %d (errno=%d)\n", rc, errno);

	exit(rc);
	return 0;
}





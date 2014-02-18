#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define __NR_xconcat	349	/* our private syscall number */

int main(int argc, char *argv[])
{
	
  char append_mode = 0;
  char create_mode = 0;
  char trunc_mode = 0;
  char excl_mode = 0;
  char atomic_concat_mode = 0;
  char return_no_of_files = 0;
  char return_percent_data = 0;
  char mode_set = 0;
  char mode_arg = 0;
  char print_short_usuage_string = 0;
  int index;
  int c;

  opterr = 0;

  while ((c = getopt (argc, argv, "acteANPmh:")) != -1)
    switch (c)
    {
      case 'a':
        append_mode = 1;
        break;
      case 'c':
        create_mode = 1;
        break;
      case 't':
        trunc_mode = 1;
        break;
      case 'e':
        excl_mode = 1;
        break;
      case 'A':
        atomic_concat_mode = 1;
        break;
      case 'N':
        return_no_of_files = 1;
        break;
      case 'P':
        return_percent_data = 1;
        break;
      case 'h':
        print_short_usuage_string = 1;
        break;
      case 'm':
        mode_set = 0 ;
        mode_arg = *optarg;
       break;
      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        return 1;
      default:
        abort ();
     }

	printf ("aflag = %d, bflag = %d, cvalue = %d\n",append_mode, create_mode, trunc_mode);

	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);
	/*int rc;
	void *dummy = (void *) atoi(argv[1]);

  	rc = syscall(__NR_xconcat, dummy);
	if (rc == 0)
		printf("syscall returned %d\n", rc);
	else
		printf("syscall returned %d (errno=%d)\n", rc, errno);

	exit(rc);*/
	return 1;
}



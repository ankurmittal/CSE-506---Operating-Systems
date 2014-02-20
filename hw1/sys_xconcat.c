#include <linux/linkage.h>
#include <linux/moduleloader.h>

struct syscall_params {
        const char *outfile; // name of output file
        const char **infiles; // array with names of input files
        unsigned int infile_count; // number of input files in infiles array
        int oflags; // Open flags to change behavior of syscall
        mode_t mode; // default permission mode for newly created outfile
        unsigned int flags; // special flags to change behavior of syscall
};

asmlinkage extern long (*sysptr)(void *arg);

asmlinkage long xconcat(void *arg)
{
	struct syscall_params *p;
	/* dummy syscall: returns 0 for non null, -EINVAL for NULL */
	if (arg == NULL)
		return -EINVAL;
	else {
		p = (struct syscall_params*)arg;
		printk("%d",p->outfile[1]);
		return 0;
	}
}

static int __init init_sys_xconcat(void)
{
	printk("installed new sys_xconcat module\n");
	if (sysptr == NULL)
		sysptr = xconcat;
	return 0;
}
static void  __exit exit_sys_xconcat(void)
{
	if (sysptr != NULL)
		sysptr = NULL;
	printk("removed sys_xconcat module\n");
}
module_init(init_sys_xconcat);
module_exit(exit_sys_xconcat);
MODULE_LICENSE("GPL");

#include <linux/linkage.h>
#include <linux/fs.h>
#include <linux/moduleloader.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#define BUFFER_SIZE BUFFER_SIZE
struct syscall_params {
        const char *outfile; // name of output file
        const char **infiles; // array with names of input files
        unsigned int infile_count; // number of input files in infiles array
        int oflags; // Open flags to change behavior of syscall
        mode_t mode; // default permission mode for newly created outfile
        unsigned int flags; // special flags to change behavior of syscall
};

asmlinkage extern long (*sysptr)(void *arg);

/*
  *Function Declarations.
 */
struct file *file_open(const char *path, int flags, int rights);
int file_write(struct file *file, unsigned long long offset,
		unsigned char *data, unsigned int size);
int file_read(struct file *file, unsigned long long offset,
		unsigned char *data, unsigned int size);
void file_close(struct file *file);
int read_write(struct syscall_params *params)
{
	int i,ret,count,bytes_written=0;
	struct file *outfile = file_open(params->outfile,
			params->oflags,mode);
	struct file *infile;
	//TODO:Check for outputfile error
	for(i=0;i<params->infile_count;i++){
		ret = 0;
		count = 0;
		//TODO:need to check read access
		infile = file_open(params->infiles[i],O_RDONLY,0);
		//TODO: Check error Condition
		do{
			ret = file_read(infile,count*BUFFER_SIZE,
				data,BUFFER_SIZE);
			if(ret > BUFFER_SIZE)
				data[ret] = 0;
			count++;
			file_write(outfile,bytes_written,
				data, BUFFER_SIZE);
			bytes_written += ret;
		}while(ret != BUFFER_SIZE);
	}

}
asmlinkage long xconcat(void *arg)
{
	unsigned char data[BUFFER_SIZE];
	struct file *infile;
	int ret;
	struct syscall_params *p;
	if (arg == NULL)
		return -EINVAL;
	else {
		p = arg;
		infile = file_open("test.txt",O_RDONLY,0);
		//TODO: Check error Condition
		ret = file_read(infile,0,data,BUFFER_SIZE);
		data[ret] = 0;
		printk("%d, %s", ret, data);
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

struct file *file_open(const char *path, int flags, int rights) {
	struct file *filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

void file_close(struct file *file) {
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset,
		unsigned char *data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_write(struct file *file, unsigned long long offset,
		unsigned char *data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

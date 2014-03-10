#include <linux/linkage.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/moduleloader.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <linux/uaccess.h>
#include <linux/uaccess.h>
#define BUFFER_SIZE PAGE_SIZE
#define DEBUGGING 1

struct syscall_params {
	const char *outfile; /*name of output file*/
	const char **infiles; /*array with names of input files*/
	unsigned int infile_count; /*number of input files in infiles array*/
	int oflags; /*Open flags to change behavior of syscall*/
	mode_t mode; /*default permission mode for newly created outfile*/
	unsigned int flags; /*special flags to change behavior of syscall*/
};

asmlinkage extern long (*sysptr)(void *arg, int argslen);

#if DEBUGGING
void showArgs(struct syscall_params *params)
{
	int i;
	printk(KERN_INFO "outfile:%s\n", params->outfile);
	printk(KERN_INFO "infile_count:%d\n", params->infile_count);
	printk(KERN_INFO "oflags:%d\n", params->oflags);
	printk(KERN_INFO "mode:%d\n", params->mode);
	printk(KERN_INFO "flags:%d\n", params->flags);
	for (i = 0; i < params->infile_count; i++)
		printk(KERN_INFO "File %d: %s\n", i+1, params->infiles[i]);
}
#endif

int file_open(const char *path, int flags, int rights, struct file **fileptr)
{
	struct file *filp = NULL;
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if (IS_ERR(filp)) {
		*fileptr = NULL;
		return PTR_ERR(filp);
	}
	*fileptr = filp;
	return 0;
}

void file_close(struct file **file)
{
	if (*file != NULL)
		filp_close(*file, NULL);
	*file = NULL;

}

int file_read(struct file *file, unsigned long long offset,
		unsigned char *data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

int file_write(struct file *file, unsigned long long offset,
		unsigned char *data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_write(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

int file_sync(struct file *file)
{
	vfs_fsync(file, 0);
	return 0;
}

long read_write(struct syscall_params *params)
{
	int i, ret, count, bytes_written = 0, err = 0, bytes_read = 0;
	struct file *infile = NULL, *outfile = NULL;
	unsigned char *data = kmalloc(sizeof(char)*BUFFER_SIZE, GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	err = file_open(params->outfile,
			O_WRONLY|O_APPEND, params->mode, &outfile);
	if ((err == -ENOENT && (params->oflags & O_CREAT) == 0)
			|| (err < 0 && err != -ENOENT)) {
		printk(KERN_INFO "Error opening output file  %s:%d",
				params->outfile, err);
		goto clean_all;
	}

	for (i = 0; i < params->infile_count; i++) {
		err = file_open(params->infiles[i], O_RDONLY, 0, &infile);
		if (err < 0) {
			printk(KERN_INFO "error opening %s:%d",
					params->infiles[i], err);
			goto clean_all;
		}
		if (outfile &&
		infile->f_dentry->d_inode == outfile->f_dentry->d_inode) {
			err = -EPERM;
			goto clean_all;
		}
		file_close(&infile);
	}
	file_close(&outfile);
	err = file_open(params->outfile,
			O_WRONLY|params->oflags, params->mode, &outfile);
	if (err < 0) {
		printk(KERN_INFO "Error opening output file  %s:%d",
			params->outfile, err);
		goto clean_all;

	}
	for (i = 0; i < params->infile_count; i++) {
		ret = 0;
		count = 0;
		/*TODO:need to check read access*/
		err = file_open(params->infiles[i], O_RDONLY, 0, &infile);
		if (err < 0) {
			printk("error:%d", err);
			goto return_back;
		}
		printk("err: %d", err);
		do {
			ret = file_read(infile, count*BUFFER_SIZE,
					data, BUFFER_SIZE);
			if (ret < 0) {
				file_close(&infile);
				printk(KERN_INFO "Error reading from file. Error Code:%d", ret);
				goto return_back;
			}
			count++;
			bytes_read += ret;
			bytes_written += file_write(outfile,
					bytes_written, data, ret);

			printk(KERN_INFO "bytes_written, %d", bytes_written);
		} while (ret == BUFFER_SIZE);
		file_close(&infile);
	}
return_back:
	file_sync(outfile);
	if (params->flags == 0x00)
		err = bytes_written;
	else if ((params->flags & 0x02) == 0x02)
		err = (bytes_written * 100) / bytes_read;
	else if ((params->flags & 0x01) == 0x01)
		err = i;
	else
		err = bytes_written;
clean_all:
	kfree(data);
	file_close(&outfile);
	return err;

}

/*int read_write_atomic(struct syscall_params *params)
  {
  char *temp_file = kmalloc(sizeof(char *)), *outf;
  int count = 0, err = 0, bwritten = 0;
  int atomic_err = 0;
  struct *infile = NULL, temp_out = NULL;
  file_open(params->outfile,O_RDONLY, params->mode, &infile);
  if (outfile) {
  do {
  count++;
  file_close(temp_out);
  file_open(temp_file, O_WRONLY|O_APPEND, params->mode, &temp_out);

  } while (temp_out);
  file_open(temp_file,O_WRONLY|O_CREAT, params->mode, &temp_out);
  count = 0;

  do {
  ret = file_read(infile, count*BUFFER_SIZE,
  data, BUFFER_SIZE);
  if (ret < 0) {
  file_close(infile);
  printk(KERN_INFO "Error reading from file. Returning. Error Code:%d", ret);
  goto clean_up;
  }
  count++;
  bwritten = file_write(temp_out,
  bytes_written, data, ret);

  } while (ret == BUFFER_SIZE);
  }
  err = read_write(params);

clean_up :
kfree(temp_file);
file_close(temp_out);
file_close(infiles);



}*/

long check_passed_args(void *arg, int argslen, struct syscall_params **p)
{
	long err = 0;
	int i = 0;
	const char **infiles = NULL;
	struct syscall_params *q;
	if (sizeof(struct syscall_params) != argslen)
		return -EINVAL;
	err = access_ok(VERIFY_READ, arg, argslen);
	if (err < 0)
		return -EINVAL;
	*p = kmalloc(sizeof(struct syscall_params), GFP_KERNEL);
	q = *p;
	err = copy_from_user(q, arg, argslen);
	if (err > 0) {
		err = -EINVAL;
		goto cleanup;
	}
	if (q->mode > 0777) {
		 printk(KERN_INFO "Mode is incorrect: %o", q->mode);
		 err = -EINVAL;
		 goto cleanup;
	}
	if ((q->oflags & (O_APPEND | O_CREAT | O_TRUNC | O_EXCL))
			!= q->oflags) {
		 printk(KERN_INFO "Open flags are incorrect: %d", q->oflags);
		 err = -EINVAL;
		 goto cleanup;
	}
	if (q->flags == 3 || q->flags > 6) {
		printk(KERN_INFO "Extra flags are incorrect: %d", q->flags);
		err = -EINVAL;
		goto cleanup;
	}
	if (q->infile_count <= 0 && q->infile_count > 15) {
		printk(KERN_INFO "Infile count of range: %d", q->infile_count);
		err = -EINVAL;
		goto cleanup;
	}
	infiles = kmalloc(sizeof(char *)*(q->infile_count), GFP_KERNEL);
	for (i = 0; i < q->infile_count; i++) {
		infiles[i] = getname(q->infiles[i]);
		if (*infiles[i] < 0) {
			err = (int) *infiles[i];
			goto cleanup;
		}
	}
	q->outfile = getname(q->outfile);
	if (*q->outfile < 0) {
		err = (int) *q->outfile;
		goto cleanup;
	}
	q->infiles = infiles;
	return 0;

cleanup:
	i--;
	while (i >= 0) {
		putname(infiles[i]);
		i--;
	}
	kfree(infiles);
	kfree(*p);
	return err;
}

asmlinkage long xconcat(void *arg, int argslen)
{
	struct syscall_params *p;
	int i = 0;
	int err = 0;
	if (!arg)
		return -EINVAL;
	else {
		printk("argl: %d", argslen);

		argslen = sizeof(struct syscall_params);
		err = check_passed_args(arg, argslen, &p);
		if (err < 0)
			return err;
#if DEBUGGING
		showArgs(p);
#endif
		if ((p->flags == 0x04))
			err = read_write(p);
		else
			err = read_write(p);
		i = p->infile_count - 1;
		while (i >= 0) {
			putname(p->infiles[i]);
			i--;
		}
		kfree(p->infiles);
		putname(p->outfile);
		kfree(p);
		return err;
	}
}

static int __init init_sys_xconcat(void)
{
	printk(KERN_INFO "installed new sys_xconcat module\n");
	if (sysptr == NULL)
		sysptr = xconcat;
	return 0;
}

static void  __exit exit_sys_xconcat(void)
{
	if (sysptr != NULL)
		sysptr = NULL;
	printk(KERN_INFO "removed sys_xconcat module\n");
}

module_init(init_sys_xconcat);
module_exit(exit_sys_xconcat);
MODULE_LICENSE("GPL");

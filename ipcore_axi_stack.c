
#include "ipcore_axi.h"

struct rtdm_driver driver_mac;
struct sample_module_data *driver_data;

int rtdm_fd_ops_open(struct rtdm_fd *fd, int oflags);
void rtdm_fd_ops_close(struct rtdm_fd *fd);
ssize_t rtdm_fd_ops_read(struct rtdm_fd *fd, void __user *buf, size_t size);
ssize_t rtdm_fd_ops_write(struct rtdm_fd *fd, const void __user *buf, size_t size);
int rtdm_fd_ops_ioctl(struct rtdm_fd *fd, unsigned int request, void *arg);

int rtdm_fd_ops_open(struct rtdm_fd *fd, int oflags)
{
	rtdm_printk("rtdm_fd_ops_open\n");
	return 0;
}

void rtdm_fd_ops_close(struct rtdm_fd *fd)
{
}

ssize_t rtdm_fd_ops_read(struct rtdm_fd *fd, void __user *buf, size_t size)
{
	unsigned address;
	unsigned __user *ptr = (unsigned __user *)buf;
	unsigned value;

	get_user(address, ptr);
//rtdm_printk("read %x %d\n", address, size);

	switch(size) {
	case 1: value = ReadByteController(&driver_data->controller, address);
		break;
	case 2: value = ReadWordController(&driver_data->controller, address);
		break;
	case 4: value = ReadDWordController(&driver_data->controller, address);
		break;
	default: size = 0;
	}

	if(size) {
		rtdm_copy_to_user(fd, buf, (void*)&value, size);
	}

    return size;
}

ssize_t rtdm_fd_ops_write(struct rtdm_fd *fd, const void __user *buf, size_t size)
{
	unsigned __user *ptr = (unsigned __user*)buf;
	unsigned address;
	unsigned value;

	get_user(address, ptr);
	ptr++;
	get_user(value, ptr);

//rtdm_printk("write %x %x %d\n", address, value, size);
//return size;
	switch(size) {
	case 1: WriteByteController(&driver_data->controller, address, value);
		break;
	case 2: WriteWordController(&driver_data->controller, address, value);
		break;
	case 4: WriteDWordController(&driver_data->controller, address, value);
		break;
	}

	return size;
}

int rtdm_fd_ops_ioctl(struct rtdm_fd *fd, unsigned int request, void *arg)
{
	int ret = 0;
	unsigned __user *buf = (unsigned __user*)arg;
	unsigned data, address, size;
	unsigned char __user *data_ptr;
	unsigned char ch;
	int i;

	if(buf == NULL)
		return -1;

	get_user(data, buf);
//rtdm_printk("ioctl %x %x %d\n", data, address, size);

	switch(request)
	{
	case IO_IPCORE_DRV_MEMORY_READ:
		get_user(address, buf + 1);
		get_user(size, buf + 2);
		data_ptr = (unsigned char __user *)data;
		for(i = 0; i < size; i ++, data_ptr++, address++) {
			ch = ReadByteController(&driver_data->controller, address);
			put_user(ch, data_ptr);
//rtdm_printk("r %x\n", address);
		}
		break;
	case IO_IPCORE_DRV_MEMORY_WRITE:
		get_user(address, buf + 1);
		get_user(size, buf + 2);
		data_ptr = (unsigned char __user *)data;
		for(i = 0; i < size; i ++, data_ptr++, address++) {
			get_user(ch, data_ptr);
			WriteByteController(&driver_data->controller, address, ch);
//rtdm_printk("w %x\n", address);
		}
		break;
	case IO_IPCORE_DRV_WAIT_EVENT:
		data = WaitEvent(&driver_data->controller);
		put_user(data, buf);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static int __init init_function(void)
{
	int ret = -1;

	if((driver_data = kmalloc(sizeof(struct sample_module_data), GFP_KERNEL)) != NULL) {
		strcpy(driver_data->name, IPCORE_DRV_NAME_FULL);
		driver_data->device.label = driver_data->name;
		driver_data->device.driver = &driver_mac;
		driver_data->device.device_data = driver_data;
		driver_data->device.minor = 0;

		driver_mac.profile_info.name = driver_data->name;
		driver_mac.profile_info.class_id = 0;
		driver_mac.profile_info.subclass_id = 0;
		driver_mac.profile_info.version = 1;
		driver_mac.profile_info.magic = ~RTDM_CLASS_MAGIC;
		driver_mac.profile_info.owner = THIS_MODULE;
		driver_mac.profile_info.kdev_class = NULL;
		driver_mac.device_flags = RTDM_NAMED_DEVICE;
		driver_mac.context_size = 0;
		driver_mac.device_count = 1;
		driver_mac.ops.open = rtdm_fd_ops_open;
		driver_mac.ops.close = rtdm_fd_ops_close;
		driver_mac.ops.write_rt = rtdm_fd_ops_write;
		driver_mac.ops.read_rt = rtdm_fd_ops_read;
		driver_mac.ops.ioctl_rt = rtdm_fd_ops_ioctl;

		if((ret = rtdm_dev_register(&driver_data->device)) == 0) {
			ret = OpenController(&driver_data->controller);
		}

		if(ret != 0) {
			kfree(driver_data);
			driver_data = 0;
		}
	}

	return ret;
}

static void __exit exit_function(void)
{
	if(driver_data) {
		rtdm_dev_unregister(&driver_data->device);
		CloseController(&driver_data->controller);
		kfree(driver_data);
		driver_data = 0;
	}
}

module_init(init_function);
module_exit(exit_function);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kuchynski");
MODULE_VERSION("1.0");

static int minor = 0;
module_param(minor, int, S_IRUGO);

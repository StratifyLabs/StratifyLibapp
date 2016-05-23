//Copyright 2011-2016 Tyler Gilbert; All Rights Reserved

#include <stratify/stratify.h>
#include <iface/link.h>

#include "Sys/Kernel.hpp"
using namespace Sys;

Kernel::Kernel() {
	// TODO Auto-generated constructor stub
	current_task = 0;
}

int Kernel::launch(const char * path, char * exec_path, const char * args, int options, int ram_size, int (*update_progress)(int, int), char *const envp[]){
	return ::launch(path, exec_path, args, options, ram_size, update_progress, envp);
}

int Kernel::free_ram(const char * path, link_transport_mdriver_t * driver){
	int fd;
	int ret;
#if defined __link
	if( (fd = link_open(driver, path, O_RDONLY)) < 0 ){
		return -1;
	}
	ret = link_ioctl(driver, fd, I_APPFS_FREE_RAM);
	link_close(driver, fd);
#else
	if( (fd = ::open(path, O_RDONLY)) < 0 ){
		return -1;
	}
	driver = 0;
	ret = ::ioctl(fd, I_APPFS_FREE_RAM);
	::close(fd);
#endif
	return ret;
}

int Kernel::reclaim_ram(const char * path, link_transport_mdriver_t * driver){
	int fd;
	int ret;
#if defined __link
	if( (fd = link_open(driver, path, O_RDONLY)) < 0 ){
		return -1;
	}
	ret = link_ioctl(driver, fd, I_APPFS_RECLAIM_RAM);
	link_close(driver, fd);
#else
	if( (fd = ::open(path, O_RDONLY)) < 0 ){
		return -1;
	}
	driver = 0;
	ret = ::ioctl(fd, I_APPFS_RECLAIM_RAM);
	::close(fd);
#endif
	return ret;
}

#if !defined __link
void Kernel::powerdown(int count){
	powerdown(count);
}

int Kernel::hibernate(int count){
	return hibernate(count);
}
#endif

int Kernel::attr(sys_attr_t & attr){
	return ioctl(I_SYS_GETATTR, &attr);
}


int Kernel::taskattr(sys_taskattr_t * attr, int task){

	if( task == -1 ){
		task = current_task;
	} else {
		current_task = task;
	}

	attr->tid = current_task;
	current_task++;

	return ioctl(I_SYS_GETTASK, attr);

}
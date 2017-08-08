//Copyright 2011-2016 Tyler Gilbert; All Rights Reserved


#include <unistd.h>
#include <fcntl.h>
#include "hal/Core.hpp"
using namespace hal;

Core::Core(port_t port) : Periph(CORE_PERIPH_CORE, port){}


int Core::set_pin_function(const core_pinfunc_t & req){
	return ioctl(I_CORE_SETPINFUNC, &req);
}

int Core::get_mcu_board_config(mcu_board_config_t & config){
	return ioctl(I_CORE_GETMCUBOARDCONFIG, &config);
}

void Core::reset(){
	core_attr_t attr;
	attr.o_flags = CORE_FLAG_EXEC_RESET;
	set_attr(attr);
}

void Core::invoke_bootloader(){
	core_attr_t attr;
	attr.o_flags = CORE_FLAG_EXEC_INVOKE_BOOTLOADER;
	set_attr(attr);
}


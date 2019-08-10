#include <sos/link/types.h>
#include "hal/Switchboard.hpp"

using namespace hal;

Switchboard::Switchboard(){}


int Switchboard::open(const var::ConstString & name, const fs::OpenFlags & flags){
	int ret;
	ret = File::open(name, flags);
	if (ret < 0 ){ return ret; }

	switchboard_info_t info;
	//see if the device responds to
	return ioctl(I_SWITCHBOARD_GETINFO, &info);
}

SwitchboardConnection Switchboard::get_connection(u16 id) const {
	SwitchboardConnection connection;
	if( read(
			 fs::Location(id*sizeof(switchboard_connection_t)),
			 fs::DestinationBuffer(&connection.m_connection),
			 fs::Size(sizeof(switchboard_connection_t))
			 )  < 0 ){
		connection = SwitchboardConnection();
	}
	return connection;
}

int Switchboard::get_connection(SwitchboardConnection & connection) const {
	if( connection.id() != SwitchboardConnection::invalid_id() ){
		if( read(
				 fs::Location(connection.id()*sizeof(switchboard_connection_t)),
				 fs::DestinationBuffer(&connection.m_connection),
				 fs::Size(sizeof(switchboard_connection_t))
				 )  < 0 ){
			connection = SwitchboardConnection();
		}
	}
	return -1;
}

int Switchboard::get_available_connection() const {
	int ret = seek(fs::Location(0));
	if( ret < 0 ){
		return ret;
	}

	int id = 0;
	switchboard_status_t status;
	do {
		ret = read(
					fs::DestinationBuffer(&status),
					fs::Size(sizeof(status))
					);
		if( ret == sizeof(status) ){
			if( status.o_flags != 0 ){
				id++; //this one is busy
			}
		}
	} while( (ret == sizeof(status)) && (status.o_flags != 0) );

	if( ret != sizeof(status) ){
		//no connections
		return -1;
	}

	return id;
}

int Switchboard::get_active_connection_count() const {
	int ret = seek(fs::Location(0));
	if( ret < 0 ){
		return ret;
	}
	int count = 0;
	switchboard_status_t status;
	do {
		ret = read(
					fs::DestinationBuffer(&status),
					fs::Size(sizeof(status))
					);
		if( ret == sizeof(status) ){
			if( status.o_flags != 0 ){
				count++; //this one is active
			}
		}
	} while( ret == sizeof(status) );

	return count;
}

void Switchboard::print_connections() const {
	int ret = seek(fs::Location(0));
	if( ret < 0 ){
		return;
	}
	int id = 0;
	switchboard_status_t status;
	do {
		ret = read(
					fs::DestinationBuffer(&status),
					fs::Size(sizeof(status))
					);
		if( ret == sizeof(status) ){
			if( status.o_flags != 0 ){
				printf("%d:%s -> %s\n", id, status.input.name, status.output.name);
			}
			id++;
		}
	} while( ret == sizeof(status) );
}


SwitchboardInfo Switchboard::get_info() const {
	SwitchboardInfo info;
	get_info(info.m_info);
	return info;
}

int Switchboard::get_info(switchboard_info_t & info) const {
	return ioctl(I_SWITCHBOARD_GETINFO, &info);
}

int Switchboard::set_attr(switchboard_attr_t & attr) const {
	return ioctl(I_SWITCHBOARD_SETATTR, &attr);
}

int Switchboard::create_persistent_connection(
		const SwitchboardTerminal & input,
		const SwitchboardTerminal & output,
		s32 nbyte, u32 o_flags) const {
	int id = get_available_connection();
	int ret;

	if( id < 0 ){
#if !defined __link
		set_error_number(ENOSPC);
#endif
		return -1;
	}

	switchboard_attr_t attr;
	attr.id = id;
	attr.o_flags = CONNECT | IS_PERSISTENT | o_flags;
	attr.input = input.m_terminal;
	attr.output = output.m_terminal;
	attr.nbyte = nbyte; //max packet size
	ret = set_attr(attr);
	if( ret < 0 ){ return ret; }

	return id;
}

int Switchboard::create_fixed_size_connection(
		const SwitchboardTerminal & input,
		const SwitchboardTerminal & output,
		s32 nbyte
		) const {
	int id = get_available_connection();

	if( id < 0 ){
#if !defined __link
		set_error_number(ENOSPC);
#endif
		return -1;
	}

	switchboard_attr_t attr;
	attr.id = id;
	attr.o_flags = CONNECT | IS_FIXED_SIZE;
	attr.input = input.m_terminal;
	attr.output = output.m_terminal;
	attr.nbyte  = nbyte; //max packet size
	if( set_attr(attr) < 0 ){
		return -1;
	}
	return id;
}


int Switchboard::destroy_connection(u16 id) const {
	switchboard_attr_t attr;
	attr.id = id;
	attr.o_flags = DISCONNECT;
	return set_attr(attr);
}

int Switchboard::destroy_connection(SwitchboardConnection & connection){
	u16 id = connection.id();
	connection = SwitchboardConnection(); //invalidate the connection
	return destroy_connection(id);
}

void SwitchboardConnection::print() const {
	if( id() != invalid_id() ){
		printf("%s -> %s total:" F32D " size:" F32D "\n", input().name().str(), output().name().str(), input().bytes_transferred(), nbyte());
	} else {
		printf("Invalid Connection ID\n");
	}
}




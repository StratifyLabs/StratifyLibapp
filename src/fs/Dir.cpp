//Copyright 2011-2018 Tyler Gilbert; All Rights Reserved

#include <sys/stat.h>
#include <unistd.h>

#include "var/String.hpp"
#include "var/Tokenizer.hpp"
#include "fs/File.hpp"
#include "fs/Dir.hpp"
using namespace fs;
using namespace arg;



#if defined __link
Dir::Dir(link_transport_mdriver_t * driver){
	m_dirp = 0;
	m_driver = driver;
	m_dirp_local = 0;
}

#else
Dir::Dir(){
	m_dirp = 0;
}

#endif

Dir::~Dir(){
	close();
}

#if !defined __link
int Dir::remove(
		const arg::SourceDirectoryPath path,
		const arg::IsRecursive recursive
		){
#else
int Dir::remove(
		const arg::SourceDirectoryPath path,
		const arg::IsRecursive recursive,
		link_transport_mdriver_t * driver
		){
#endif
	int ret = 0;
	if( recursive.argument() ){
#if defined __link
		Dir d(driver);
#else
		Dir d;
#endif
		if( d.open(
				 path
				 ) == 0 ){
			var::String entry;
			while( (entry = d.read()).is_empty() == false ){
#if defined __link
				Stat info(driver);
#else
				Stat info;
#endif
				var::String entry_path;
				entry_path << path.argument() << "/" << entry;
				info = File::get_info(
							SourceFilePath(entry_path)
							);
				if( info.is_directory() ){
					if( entry != "." && entry != ".."){
#if defined __link
						ret = Dir::remove(
									arg::SourceDirectoryPath(entry_path),
									arg::IsRecursive(true),
									driver);
#else
						ret = Dir::remove(
									arg::SourceDirectoryPath(entry_path),
									arg::IsRecursive(true)
									);
#endif
					}
				} else {
#if defined __link
					ret = File::remove(
								arg::SourceFilePath(entry_path),
								driver
								);
#else
					ret = File::remove(
								arg::SourceFilePath(entry_path)
								);
#endif
				}
			}
		}
		d.close();
	}

	if( ret >= 0 ){
		//this will remove an empty directory or a file
#if defined __link
		if( driver ){
			ret = File::remove(
						arg::SourceFilePath(path.argument()),
						driver
						);
		} else {
			ret = ::rmdir(
						path.argument().cstring()
						);
		}
#else
		ret = File::remove(
					arg::SourceFilePath(path.argument())
					);
#endif
	}

	return ret;
}

#if defined __link
int Dir::create(
		const DestinationDirectoryPath & path,
		const Permissions & permissions,
		link_transport_mdriver_t * driver
		){
	int result;
	if( driver ){
		result = link_mkdir(driver,
								  path.argument().cstring(),
								  permissions.permissions());
	} else {
		//open a directory on the local system (not over link)
#if defined __win32
		result = mkdir(path.argument().cstring());
#else
		result = mkdir(path.argument().cstring(), permissions.permissions());
#endif
	}
	return result;
}
#else
int Dir::create(
		const DestinationDirectoryPath & path,
		const Permissions & permissions
		){
	return mkdir(
				path.argument().cstring(),
				permissions.permissions()
				);
}
#endif

#if defined __link
int Dir::create(
		const DestinationDirectoryPath & path,
		const Permissions & permissions,
		bool is_recursive,
		link_transport_mdriver_t * driver
		){
	if( is_recursive == false ){
		return create(path, permissions, driver);
	}
#else
int Dir::create(
		const DestinationDirectoryPath & path,
		const Permissions & permissions,
		bool is_recursive){
	if( is_recursive == false ){
		return create(path, permissions);
	}
#endif
	var::Tokenizer path_tokens(
				arg::TokenEncodedString(path.argument()),
				arg::TokenDelimeters("/")
				);
	var::String base_path;

	if( path.argument().find(
			 arg::StringToFind("/")
			 ) == 0 ){
		base_path << "/";
	}

	for(u32 i=0; i < path_tokens.count(); i++){
		base_path << path_tokens.at(i);
		create(
					DestinationDirectoryPath(base_path),
					permissions
#if defined __link
					, driver
#endif
					);
		base_path << "/";
	}


	return 0;
}



#if defined __link
bool Dir::exists(
		const SourceDirectoryPath & path,
		link_transport_mdriver_t * driver
		){
	Dir d(driver);
#else
bool Dir::exists(
		const SourceDirectoryPath & path
		){
	Dir d;
#endif
	if( d.open(path) < 0 ){ return false; }
	d.close();
	return true;
}


int Dir::open(const SourceDirectoryPath & name){
#if defined __link
	if( driver() ){
		m_dirp = link_opendir(driver(), name.argument().cstring());
	} else {
		//open a directory on the local system (not over link)

		m_dirp_local = opendir(name.argument().cstring());
		if( m_dirp_local == 0 ){
			return -1;
		}
		m_path.assign(name.argument());
		return 0;
	}
#else
	m_dirp = opendir(name.argument().cstring());
#endif

	if( m_dirp == 0 ){
		set_error_number_to_errno();
		return -1;
	}

	m_path.assign(name.argument());
	return 0;
}


#if !defined __link
int Dir::count(){
	long loc;
	int count;

	if( !is_open() ){
		return -1;
	}

#if defined __link

#else
	loc = tell();
#endif

#if defined __link

#else
	rewind();
#endif

	count = 0;
	while( read() != 0 ){
		count++;
	}

	seek(loc);

	return count;

}

#endif


#if defined __link
var::Vector<var::String> Dir::read_list(
		const SourceDirectoryPath & path,
		link_transport_mdriver_t * driver
		){
	Dir directory(driver);
#else
var::Vector<var::String> Dir::read_list(
		const SourceDirectoryPath & path
		){
	Dir directory;
#endif
	var::Vector<var::String> result;
	if( directory.open(path) < 0 ){ return result; }
	result = directory.read_list();
	directory.close();
	return result;
}


var::Vector<var::String> Dir::read_list(){
	var::Vector<var::String> result;
	var::String entry;

	do {
		entry.clear();
		entry = read();
		if( !entry.is_empty() ){
			result.push_back(entry);
		}
	} while( entry.is_empty() == false );

	return result;
}

const char * Dir::read(){

#if defined __link
	if( driver() ){
		struct link_dirent * result;
		memset(&m_entry, 0, sizeof(m_entry));
		if( link_readdir_r(driver(), m_dirp, &m_entry, &result) < 0 ){
			return 0;
		}
	} else {
		struct dirent * result_local;
		if( (readdir_r(m_dirp_local, &m_entry_local, &result_local) < 0) || (result_local == 0) ){
			return 0;
		}
		return m_entry_local.d_name;
	}
#else
	struct dirent * result;
	if( readdir_r(m_dirp, &m_entry, &result) < 0 ){
		set_error_number_to_errno();
		return 0;
	}
#endif
	return m_entry.d_name;
}

bool Dir::get_entry(var::String & path_dest){
	const char * entry = read();

	if( entry == 0 ){
		return false;
	}

	path_dest.clear();
	path_dest << m_path;
	if( m_path.is_empty() == false ){
		m_path << "/" << entry;
	}
	return true;
}

var::String Dir::get_entry(){
	var::String entry;
	get_entry(entry);
	return entry;
}

int Dir::close(){
	m_path.clear();
	if( m_dirp ){
#if defined __link
		if( driver() ){
			link_closedir(driver(), m_dirp);
		}
#else //__link
		if( closedir(m_dirp) < 0 ){
			set_error_number_to_errno();
		}
#endif
		m_dirp = 0;
	}

#if defined __link
	if( m_dirp_local ){
		DIR * dirp_copy = m_dirp_local;
		m_dirp_local = 0;
		if( closedir(dirp_copy) < 0 ){
			return -1;
		}
		return 0;
	}
#endif


	return 0;
}

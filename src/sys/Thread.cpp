/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.
//Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc

#include <errno.h>
#include <cstdio>
#include "sys/Thread.hpp"
#include "chrono.hpp"
using namespace sys;

Thread::Thread(StackSize stacksize, IsDetached detached){
	init(stacksize.argument(), detached.argument());
}

Thread::Thread(const Options & options){
	init(options.stack_size(), options.detach_state() == detach_state_detached);
}

Thread::~Thread(){
	//what if thread is still running?
	if( is_running() ){
		//must be stopped or assert

	}

	if( !is_id_error() ){
		pthread_attr_destroy(&m_pthread_attr);
	}
}

bool Thread::is_valid() const {
	return !is_id_pending() && !is_id_error();
}

int Thread::init(int stack_size, bool detached){
	set_id_error();

	if( set_error_number_if_error(pthread_attr_init(&m_pthread_attr)) < 0 ){
		return -1;
	}

	if( set_error_number_if_error(pthread_attr_setstacksize(&m_pthread_attr, stack_size)) < 0 ){
		return -1;
	}

	if( detached == true ){
		if( set_error_number_if_error(pthread_attr_setdetachstate(&m_pthread_attr, PTHREAD_CREATE_DETACHED)) < 0 ){
			return -1;
		}
	} else {
		if( set_error_number_if_error(pthread_attr_setdetachstate(&m_pthread_attr, PTHREAD_CREATE_JOINABLE)) < 0 ){
			return -1;
		}
	}

	set_id_pending();
	return 0;
}

int Thread::set_stacksize(int size){

	if( is_running() ){
		set_error_number(EBUSY);
		return -1;
	}

	return set_error_number_if_error(pthread_attr_setstacksize(&m_pthread_attr, size));
}

int Thread::get_stacksize() const {
	u32 stacksize = 0;
	set_error_number_if_error( pthread_attr_getstacksize(&m_pthread_attr, (size_t*)&stacksize));
	return stacksize;
}

int Thread::get_detachstate() const {
	int value = 0;
	set_error_number_if_error(pthread_attr_getdetachstate(&m_pthread_attr, &value));
	return value;
}

int Thread::set_detachstate(enum detach_states value){

	if( is_running() ){
		set_error_number(EBUSY);
		return -1;
	}

	return set_error_number_if_error(pthread_attr_setdetachstate(&m_pthread_attr, value));
}

int Thread::set_priority(
		int prio,
		enum Sched::policy policy
		){
	if( is_valid() ){
		struct sched_param param = {0};
		param.sched_priority = prio;
		return set_error_number_if_error(pthread_setschedparam(m_id, policy, &param));
	}

	set_error_number(EAGAIN);
	return -1;
}

int Thread::get_priority() const {
	struct sched_param param;
	int policy;
	if( is_valid() ){
		if( set_error_number_if_error(pthread_getschedparam(m_id, &policy, &param)) < 0 ){
			return -1;
		}
	} else {
		set_error_number(EAGAIN);
		return -1;
	}
	return param.sched_priority;
}

int Thread::cancel() const {
	return set_error_number_if_error( pthread_cancel(id()) );
}

int Thread::set_cancel_type(enum cancel_types cancel_type){
	return pthread_setcanceltype(cancel_type, nullptr);
}


int Thread::set_cancel_state(enum cancel_states cancel_state){
	return pthread_setcancelstate(cancel_state, nullptr);
}

int Thread::get_policy() const {
	struct sched_param param;
	int policy;
	if( is_valid() ){
		if( set_error_number_if_error(pthread_getschedparam(m_id, &policy, &param)) < 0 ){
			return -1;
		}
	} else {
		set_error_number(EAGAIN);
		return -1;
	}
	return policy;
}

int Thread::create(
		const CreateOptions & options
		){
	if( reset() < 0 ){
		set_error_number(EBUSY);
		return -1;
	}

	if( !is_id_pending() ){
		set_error_number(EBUSY);
		return -1;
	}

	if( set_error_number_if_error(pthread_attr_setschedpolicy(&m_pthread_attr, options.policy())) < 0 ){
		return -1;
	}

	struct sched_param param = {0};
	param.sched_priority = options.priority();
	if( set_error_number_if_error(pthread_attr_setschedparam(&m_pthread_attr, &param)) < 0 ){
		return -1;
	}

	//First create the thread
	return set_error_number_if_error(
				pthread_create(
					&m_id,
					&m_pthread_attr,
					options.function(),
					options.argument()
					)
				);
}

bool Thread::is_running(){
	//check to see if the thread is running
	if( is_id_pending() || is_id_error() ){ return false; }
	if( pthread_kill(m_id, 0) == 0 ){
		return true;
	}

	reset();
	return false;
}

int Thread::wait(
		Return ret,
		DelayInterval interval
		){

	void * dummy;

	if( is_valid() ){

		//if thread is joinable, then join it
		if( is_joinable() ){
			if( ret.argument() != 0 ){
				join(ret);
			} else {
				join(Return(&dummy));
			}
		} else {
			//just keep sampling until the thread completes
			while( is_running() ){
				interval.argument().wait();
			}
		}
	}
	return 0;
}

int Thread::reset(){

	if( is_id_pending() ){
		return 0;
	}

	if( is_valid() && (pthread_kill(m_id, 0) < 0) ){
		bool detached = !is_joinable();
		u32 stacksize = get_stacksize();
		if( set_error_number_if_error(pthread_attr_destroy(&m_pthread_attr)) < 0 ){
			return -1;
		}
		return init(stacksize, detached);
	}

	return -1;
}

int Thread::join(
		Thread & thread_to_join,
		void ** result
		){
	void * tmp_ptr;
	void ** ptr;
	if( result == nullptr ){
		ptr = &tmp_ptr;
	} else {
		ptr = result;
	}
	int local_result = pthread_join(thread_to_join.id(), ptr);
	if( local_result == 0 ){
		//resets the thread that just completed
		thread_to_join.is_running();
	}
	return local_result;
}

int Thread::join(void ** result){
	return join(*this, result);
}

bool Thread::is_joinable() const{
	int detach_state = get_detachstate();
	return detach_state == detach_state_joinable;
}




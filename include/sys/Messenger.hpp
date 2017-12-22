/* Copyright 2017 tgil All Rights Reserved */

#ifndef MESSENGER_HPP_
#define MESSENGER_HPP_

#include <pthread.h>
#include "Thread.hpp"
#include "../fmt/Son.hpp"
#include "File.hpp"
#include "Mutex.hpp"

namespace sys {

/*! \brief Messenger Class
 * \details This class creates a new thread dedicated to handling message passing.
 *
 */
class Messenger {
public:
	/*! \details Constructs a new messenger with the specified \a stack size for the thread.
	 *
	 * @param stack_size The number of bytes to use for the Messenger thread stack size
	 */
	Messenger(int stack_size = 2048);

	/*! \details Starts the messenger.
	 *
	 * @param device The path to the device where messages will be passed
	 * @param read_channel The channel to use for reading messages
	 * @param write_channel The channel to use for writing messages
	 * @return Zero on success
	 */
	int start(const char * device, int read_channel, int write_channel);

	/*! \details Stops the messenger.
	 *
	 */
	void stop();

	/*! \details Returns true if the messenger thread is running. */
	bool is_running() const { return !m_is_stopped; }

	/*! \details Sends a message.
	 *
	 * @param message A reference to the message
	 * @return Zero if message was successfully queued
	 *
	 * This method will acquire a mutex on the messenger
	 * before sending. The mutex may be locked waiting for an
	 * incoming message in which case send_message() will wait
	 * until that completes before sending.
	 *
	 */
	int send_message(fmt::Son & message);

	/*! \details Handles incoming messages.
	 *
	 * @param message A reference to the incoming message
	 *
	 * This method is called within the context of the messenger thread.
	 * It is a pure vitual method that must be implemented by the class
	 * that inherits Messenger.
	 *
	 */
	virtual void handle_message(fmt::Son & message) = 0;

	u16 max_message_size() const { return m_max_message_size; }

	void set_max_message_size(u16 size){
		m_max_message_size = size;
	}

private:
	static void * listener_work(void * args);
	void listener();

	volatile bool m_stop;
	volatile bool m_is_stopped;
	Thread m_listener;
	u8 m_read_channel;
	u8 m_write_channel;
	u16 m_max_message_size;
	u16 m_timeout_ms;
	File m_device;
	sys::Mutex m_mutex;
};

};

#endif /* MESSENGER_HPP_ */
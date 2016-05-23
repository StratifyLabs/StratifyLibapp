/*! \file */ //Copyright 2011-2016 Tyler Gilbert; All Rights Reserved

#ifndef PIN_HPP_
#define PIN_HPP_

#include "Pio.hpp"

namespace Hal {


/*! \brief Pin class
 * \details This class controls the operation of a single pin.  It works similarly
 * to the Pio class, but the mask is built-in.
 *
 * Here is an example of using the Pin class:
 *
 * \code
 * #include <stfy/Hal/Pin.hpp>
 * using namespace Hal;
 *
 * Pin pin(1,0); //control pin P1.0
 *  //Or if working with the pinmask Pin  pin(1, 1<<0, true);
 *
 * pin.init(Pin::OUTPUT); //initialize as an output
 *
 *  //These three all set the pin high
 * pin = true;
 * pin.set();
 *
 *  //These turn the pin off
 * pin = false;
 * pin.clr();
 *
 *  //now convert to an input
 * pin.setattr(Pin::INPUT | Pin::PULLUP); //or use Pin::FLOAT, Pin::PULLDOWN, etc
 *  //or to init as an input use pin.init(Pin::INPUT);
 *
 *  //now check to see if the value is high or low
 *  if( pin.value() == true ){
 *  	//pin is high
 *  } else {
 *  	//pin is low
 *  }
 *
 *  pin.close(); //close the associated file descriptor (pin keeps its IO properties and state)
 *
 * \endcode
 *
 */
class Pin : public Pio {
public:
	/*! \details Initialize the object with a port/pin combination. */
	Pin(port_t port, uint32_t pin, bool ismask = false) : Pio(port){
		if( ismask ){
			_pinmask = pin;
		} else {
			_pinmask = (1<<pin);
		}
	}

	Pin(pio_t p) : Pio(p.port) { _pinmask = 1<<p.pin; }

	using Pio::set_attr;

	/*! \details Initialize the pin with the specified mode */
	inline int init(int mode){
		return Pio::init(_pinmask, mode);
	}

	/*! \details Set the pin attributes */
	inline int set_attr(int mode){
		return Pio::set_attr(_pinmask, mode);
	}

	/*! \details Assign a boolean to the pin (true is high, false is low) */
	Pin& operator = (bool on){
		if( on ){
			set_mask(_pinmask);
		} else {
			clear_mask(_pinmask);
		}
		return *this;
	}

	/*! \details Get the value of the pin (true is high, false is low) */
	inline bool value(){ return (Pio::value() & _pinmask) != 0; }

	/*! \details Set the pin high (assign value 1) */
	inline int set(void){ return set_mask(_pinmask); }
	/*! \details Clear the pin low (assign value 0) */
	inline int clear(void){ return clear_mask(_pinmask); }

	pio_sample_t pinmask() const { return _pinmask; }

private:
	pio_sample_t _pinmask;
};

};

#endif /* PIN_HPP_ */
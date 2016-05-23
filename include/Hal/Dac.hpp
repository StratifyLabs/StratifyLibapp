/*! \file */ //Copyright 2011-2016 Tyler Gilbert; All Rights Reserved

#ifndef DAC_HPP_
#define DAC_HPP_

#include <iface/dev/dac.h>
#include "Periph.hpp"

namespace Hal {

/*! \brief DAC Class
 * \details DAC (Digital to Analog Converter) Class
 *
 * The DAC class is used to write data to the digital to analog converter.  Here is an example:
 *
 * \code
 * #include <stfy/Hal.hpp>
 * #include <stfy/Sys.hpp>
 *
 * int main(int argc, char * argv[]){
 * 	Dac dac(0);
 * 	dac_sample_t samples[50];
 * 	dac.init(1<<0, 50000); //enable channel 0 at 50KHz
 *
 *  //here, write meaningful values to samples
 *
 * 	dac.write(0, samples, 50*sizeof(dac_sample_t)); //This returns when all 50 samples are written
 *
 *  //if you want to do other things while the sample is written use AIO
 * 	Aio aio(samples, 50*sizeof(dac_sample_t));
 *
 * 	dac.write(aio);
 *  //Here you can do other things while the data is written
 * 	while( aio.inprogress() == true ){
 * 		Timer::wait_usec(500); //wait until the write is complete
 * 	}
 *
 * 	dac.close(); //this will power off the DAC if nothing else has it open
 *
 * 	return 0;
 * }
 *
 * \endcode
 *
 */
class Dac : public Periph {
public:
	Dac(port_t port);
	/*! \details Get the DAC attributes */
	int attr(dac_attr_t * attr);
	/*! \details Set the DAC attributes */
	int setattr(const dac_attr_t * attr);

	/*! \details Get the current value of the dac */
	dac_sample_t value(u32 channel = 0);
	dac_sample_t get(u32 channel = 0){ return value(channel); }

	/*! \details Set the DAC attributes  using specified values */
	int setattr(uint16_t enabled_channels /*! Enabled Channels */,
			int freq = 1000000 /*! DAC output frequency */,
			uint8_t pin_assign = 0 /*! Pin assignment value */){
		dac_attr_t attr;
		attr.enabled_channels = enabled_channels;
		attr.freq = freq;
		attr.pin_assign = pin_assign;
		return setattr(&attr);
	}

	/*! \details This method opens the DAC then sets the DAC
	 * attributes as specified.
	 */
	int init(uint16_t enabled_channels /*! Enabled Channels */,
			int freq = 1000000 /*! DAC clock frequency */,
			uint8_t pin_assign = 0 /*! Pin assignment value */){
		if( open() < 0 ){
			return -1;
		}
		return setattr(enabled_channels, freq, pin_assign);
	}

	/*! \details Set the value of the DAC
	 *
	 * @param value Value to write to the DAC
	 * @param channel The DAC channel
	 * @return Zero on success
	 */
	int set(dac_sample_t value, u32 channel = 0);

#ifdef __MCU_ONLY__
	using Pblock::write;
	int write(void * buf, int nbyte);
	int close(void);
#endif

private:



};

};

#endif /* DAC_HPP_ */
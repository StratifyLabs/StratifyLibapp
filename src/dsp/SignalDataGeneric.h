/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.

native_type SignalType::mean() const {
	native_type result;
	if( arm_dsp_api_function()->mean == 0 ){
		exit_fatal("arm_dsp_api_function()->mean == 0");
	}
	arm_dsp_api_function()->mean((native_type*)to_const_void(), count(), &result);
	return result;
}

big_type SignalType::power() const {
	big_type result;
	arm_dsp_api_function()->power((native_type*)to_const_void(), count(), &result);
	return result;
}

native_type SignalType::variance() const {
	native_type result;
	arm_dsp_api_function()->var((native_type*)to_const_void(), count(), &result);
	return result;
}

native_type SignalType::rms() const {
	native_type result;
	arm_dsp_api_function()->rms((native_type*)to_const_void(), count(), &result);
	return result;
}

native_type SignalType::std() const {
	native_type result;
	arm_dsp_api_function()->std((native_type*)to_const_void(), count(), &result);
	return result;
}

native_type SignalType::min() const {
	native_type result;
	u32 idx;
	arm_dsp_api_function()->min((native_type*)to_const_void(), count(), &result, &idx);
	return result;
}

native_type SignalType::min(u32 & idx) const {
	native_type result;
	arm_dsp_api_function()->min((native_type*)to_const_void(), count(), &result, &idx);
	return result;
}

native_type SignalType::max() const {
	native_type result;
	u32 idx;
	arm_dsp_api_function()->max((native_type*)to_const_void(), count(), &result, &idx);
	return result;
}

native_type SignalType::max(u32 & idx) const {
	native_type result;
	arm_dsp_api_function()->max((native_type*)to_const_void(), count(), &result, &idx);
	return result;
}

SignalType SignalType::abs() const {
	SignalType result = SignalType(count());
	arm_dsp_api_function()->abs(
				(native_type *)to_const_void(),
				result.data(),
				count());
	return result;
}

void SignalType::abs(SignalType & output) const {
	arm_dsp_api_function()->abs(
				(native_type *)to_const_void(),
				output.data(),
				count());
}

big_type SignalType::dot_product(const SignalType & a) const {
	big_type result;
	arm_dsp_api_function()->dot_prod(
				(native_type*)to_const_void(),
				(native_type*)a.data(),
				count(),
				&result);
	return result;
}

SignalType SignalType::negate() const {
	SignalType ret = SignalType((count()));
	arm_dsp_api_function()->negate(
				(native_type*)to_const_void(),
				ret.data(),
				count()
				);
	return ret;
}

void SignalType::negate(SignalType & output) const {
	arm_dsp_api_function()->negate((native_type*)to_const_void(), output.data(), count());
}


SignalType SignalType::convolve(const SignalType & a) const {
	SignalType ret((count() + a.count() - 1));
#if IS_FLOAT == 0
	arm_dsp_api_function()->conv_fast(
				(native_type*)to_const_void(),
				count(),
				(native_type*)a.to_const_void(),
				a.count(),
				ret.data()
				);
#else
	arm_dsp_api_function()->conv((native_type*)to_const_void(), count(), (native_type*)a.to_const_void(), a.count(), ret.data());
#endif
	return ret;
}

void SignalType::convolve(SignalType & output, const SignalType & a) const {
	//check output length?
#if IS_FLOAT == 0
	arm_dsp_api_function()->conv_fast((native_type*)to_const_void(), count(), (native_type*)a.to_const_void(), a.count(), output.data());
#else
	arm_dsp_api_function()->conv((native_type*)to_const_void(), count(), (native_type*)a.to_const_void(), a.count(), output.data());
#endif
}

#if IS_FLOAT == 0
void SignalType::shift(SignalType & output, s8 value) const {
	arm_dsp_api_function()->shift((native_type*)to_const_void(), value, output.data(), count());
}

SignalType SignalType::shift(s8 value) const {
	SignalType ret = SignalType((count()));
	arm_dsp_api_function()->shift((native_type*)to_const_void(), value, ret.data(), count());
	return ret;
}

SignalType & SignalType::shift_assign(s8 value){
	arm_dsp_api_function()->shift((native_type*)to_const_void(), value, data(), count());
	return *this;
}
#endif

SignalType SignalType::scale(native_type scale_fraction, s8 shift) const {
	SignalType ret = SignalType((count()));
#if IS_FLOAT == 0
	arm_dsp_api_function()->scale((native_type*)to_const_void(), scale_fraction, shift, ret.data(), count());
#else
	arm_dsp_api_function()->scale((native_type*)to_const_void(), scale_fraction, ret.data(), count());
#endif

	return ret;
}

void SignalType::scale(SignalType & output, native_type scale_fraction, s8 shift) const {
#if IS_FLOAT == 0
	arm_dsp_api_function()->scale((native_type*)to_const_void(), scale_fraction, shift, output.data(), count());
#else
	arm_dsp_api_function()->scale((native_type*)to_const_void(), scale_fraction, output.data(), count());
#endif
}


SignalType SignalType::add(native_type offset_value) const {
	SignalType ret = SignalType((count()));
	arm_dsp_api_function()->offset((native_type*)to_const_void(), offset_value, ret.data(), count());

	return ret;
}

SignalType & SignalType::add_assign(native_type offset_value){
	arm_dsp_api_function()->offset((native_type*)to_const_void(), offset_value, data(), count());
	return *this;
}

SignalType SignalType::add(const SignalType & a) const {
	SignalType ret = SignalType((count()));
	if( ret.count() != count() ){
		return ret;
	}
	arm_dsp_api_function()->add((native_type*)to_const_void(), (native_type*)a.to_const_void(), ret.data(), count());
	return ret;
}


SignalType & SignalType::add_assign(const SignalType & a){
	arm_dsp_api_function()->add((native_type*)to_const_void(), (native_type*)a.to_const_void(), data(), count());
	return *this;
}

SignalType SignalType::multiply(native_type value) const {
	return scale(value);
}

SignalType & SignalType::multiply_assign(native_type value){
	scale(*this, value);
	return *this;
}

SignalType SignalType::multiply(const SignalType & a) const {
	SignalType ret = SignalType((count()));
	arm_dsp_api_function()->mult((native_type*)to_const_void(), (native_type*)a.to_const_void(), ret.data(), count());

	return ret;
}

SignalType & SignalType::multiply_assign(const SignalType & a){
	arm_dsp_api_function()->mult((native_type*)to_const_void(), (native_type*)a.to_const_void(), data(), count());
	return *this;
}

//subtract
SignalType SignalType::subtract(const SignalType & a) const{
	SignalType ret = SignalType((count()));
	arm_dsp_api_function()->sub((native_type*)to_const_void(), (native_type*)a.to_const_void(), ret.data(), count());

	return ret;
}

SignalType & SignalType::subtract_assign(const SignalType & a){
	arm_dsp_api_function()->sub((native_type*)to_const_void(), (native_type*)a.to_const_void(), data(), count());
	return *this;
}

SignalType SignalType::filter(const BiquadFilterType & filter) const {
	SignalType ret = SignalType((count()));
#if IS_FLOAT == 0
	arm_dsp_api_function()->biquad_cascade_df1_fast(filter.instance(), (native_type*)to_const_void(), ret.data(), count());
#else
	arm_dsp_api_function()->biquad_cascade_df1(filter.instance(), (native_type*)to_const_void(), ret.data(), count());
#endif

	return ret;
}

void SignalType::filter(SignalType & output, const BiquadFilterType & filter) const {
#if IS_FLOAT == 0
	arm_dsp_api_function()->biquad_cascade_df1_fast(filter.instance(), (native_type*)to_const_void(), output.data(), count());
#else
	arm_dsp_api_function()->biquad_cascade_df1(filter.instance(), (native_type*)to_const_void(), output.data(), count());
#endif
}

SignalType SignalType::filter(const FirFilterType & filter) const {
	SignalType ret = SignalType((count()));
#if IS_FLOAT == 0
	arm_dsp_api_function()->fir_fast(filter.instance(), (native_type*)to_const_void(), ret.data(), count());
#else
	arm_dsp_api_function()->fir(filter.instance(), (native_type*)to_const_void(), ret.data(), count());
#endif

	return ret;
}

void SignalType::filter(SignalType & output, const FirFilterType & filter) const {
#if IS_FLOAT == 0
	arm_dsp_api_function()->fir_fast(filter.instance(), (native_type*)to_const_void(), output.data(), count());
#else
	arm_dsp_api_function()->fir(filter.instance(), (native_type*)to_const_void(), output.data(), count());
#endif
}

#if IS_FLOAT == 1
SignalType SignalType::create_sin_wave(native_type wave_frequency, native_type sampling_frequency, u32 nsamples, native_type phase){
#else
SignalType SignalType::create_sin_wave(u32 wave_frequency, u32 sampling_frequency, u32 nsamples, native_type phase){
#endif
	SignalType ret = SignalType((nsamples));
	u32 i;
#if IS_FLOAT == 1
	native_type theta = phase; //theta 0 to max is 0 to 2*pi
	native_type theta_step = wave_frequency * sampling_frequency;
#else
	unsigned_native_type theta = phase; //theta 0 to max is 0 to 2*pi
	unsigned_native_type theta_step = (big_type)wave_frequency * LOCAL_INT_MAX / sampling_frequency;
#endif

	//fsamp = 16K and fmax = 8k - theta step is pi or (1/2)
	//theta step is f / fsamp
	for(i=0; i < nsamples; i++){
		ret.at(i) = arm_dsp_api_function()->sin(theta);
		theta += theta_step;
#if IS_FLOAT == 0
		if( theta > LOCAL_INT_MAX ){
			theta -= (LOCAL_INT_MAX);
		}
#endif
	}


	return ret;
}



void SignalComplexType::transform(
		FftComplexType & fft,
		bool is_inverse,
		bool is_bit_reversal
		){
	arm_dsp_api_function()->cfft(
				fft.instance(),
				data(),
				is_inverse,
				is_bit_reversal
				);
}

void SignalComplexType::transform(
		SignalComplexType & output,
		FftRealType & fft,
		bool is_inverse
		){
#if IS_FLOAT == 0
	fft.instance()->ifftFlagR = is_inverse;
	arm_dsp_api_function()->rfft(
				fft.instance(),
				(q31_t*)data(),
				(q31_t*)output.data()
				);
#else
	arm_dsp_api_function()->rfft_fast(
				fft.instance(),
				data(),
				output.data(),
				is_inverse
				);
#endif
}

SignalComplexType SignalComplexType::transform(FftRealType & fft, bool is_inverse){
	int samples = count();
	if( !is_inverse ){ samples *= 2; }
	SignalComplexType ret = SignalComplexType((samples));
#if IS_FLOAT == 0
	fft.instance()->ifftFlagR = is_inverse;
	arm_dsp_api_function()->rfft(
				fft.instance(),
				data(),
				ret.data()
				);
#else
	arm_dsp_api_function()->rfft_fast(
				fft.instance(),
				data(),
				ret.data(),
				is_inverse);
#endif

	return ret;
}



//Copyright 2011-2016 Tyler Gilbert; All Rights Reserved


#include <errno.h>
#include <sgfx/FontObject.hpp>

using namespace sgfx;

static const char ascii_charset_[FontObject::CHARSET_SIZE+1] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";


const char * FontObject::charset(){
	return ascii_charset_;
}



int FontObject::to_charset(char ascii){
	if( (ascii < ' ') ||
			(ascii > '~') ){
		return -1;
	}
	return (int)(ascii - ' ');
}


FontObject::FontObject() {
	m_bitmap = 0;
	m_offset = 0;
	m_space_size = 8;
	m_letter_spacing = 1;
}

int FontObject::calc_len(const char * str) const {
	int l;
	l = 0;

	while( *str != 0 ){

		if( *str == ' ' ){
			l += space_size();
		} else {
			if( load_char(m_char, *str, true) < 0){
				return -1;
			}
			l += m_char.xadvance;
		}
		str++;
	}
	return l;
}

int FontObject::set_char(char c, Bitmap & bitmap, sg_point_t point) const {

	load_char(m_char, c, true);

	point.x += m_char.xoffset;
	point.y += m_char.yoffset;

	bitmap.set_bitmap(this->bitmap(c, true), point);

	//this needs to be char width not bitmap width
	return m_char.xadvance;
}

int FontObject::clear_char(char c, Bitmap & bitmap, sg_point_t point) const {

	load_char(m_char, c, true);

	point.x += m_char.xoffset;
	point.y += m_char.yoffset;

	bitmap.clr_bitmap(this->bitmap(c, true), point);

	return m_char.xadvance;
}


int FontObject::clear_str(const char * str, Bitmap & bitmap, sg_point_t point) const {
	char c;
	sg_size_t w;

	//draw characters on the bitmap
	while( (c = *(str++)) != 0){
		if( c == ' ' ){
			w = space_size();
		} else {
			w = clear_char(c, bitmap, point);
		}
		if( w < 0 ){
			return -1;
		}

		//apply kerning
		w += load_kerning(c, str[1]);
		point.x += w;
	}
	return 0;
}

int FontObject::set_str(const char * str, Bitmap & bitmap, sg_point_t point) const {
	char c;
	sg_size_t w;

	//draw characters on the bitmap
	while( (c = *(str++)) != 0){
		if( c == ' ' ){
			w = space_size();
		} else {
			set_char(c, bitmap, point);
			w = m_char.xadvance;
		}

		//apply kerning
		w += load_kerning(c, *str);

		if( w < 0 ){
			return -1;
		}

		point.x += w;

	}
	return 0;
}


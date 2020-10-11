/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see
             // LICENSE.md for rights.
// Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc

#include <cstdio>
#include <cstring>
#include <stdint.h>

#include "var/Base64.hpp"
#include "var/Data.hpp"

using namespace var;

int Base64Encoder::transform(const Transform &options) const {
  if (options.output().size() < get_output_size(options.input().size())) {
    return -1;
  }

  return Base64::encode(
    options.output().to_char(),
    options.input().to_const_void(),
    options.input().size());
}

int Base64Decoder::transform(const Transform &options) const {
  if (options.output().size() < get_output_size(options.input().size())) {
    return -1;
  }

  return Base64::decode(
    options.output().to_void(),
    options.input().to_const_char(),
    options.input().size());
}

String Base64::encode(View input) const {
  String result;
  result.resize(get_encoded_size(input.size()));
  if (result.status().is_error()) {
    API_RETURN_VALUE_ASSIGN_ERROR(String(), "", ENOMEM);
  }
  encode(result.to_char(), input.to_const_void(), input.size());
  return result;
}

Data Base64::decode(StringView input) const {
  Data result;
  int len = get_decoded_size(input.length());
  if (result.resize(len).status().is_error()) {
    API_RETURN_VALUE_ASSIGN_ERROR(Data(), "", ENOMEM);
    return Data();
  }

  len -= decode(result.data(), input.cstring(), input.length());

  result.resize(len);
  return result;
}

int Base64::encode(char *dest, const void *src, int nbyte) {

  int i;
  int j;
  int k;
  int len;
  u8 six_bits[4];
  len = nbyte;
  const char *data;
  data = (const char *)src;

  k = 0;
  // We need to encode three bytes at a time in to four encoded bytes
  for (i = 0; i < len; i += 3) {
    // First the thress bytes are broken down into six-bit sections
    six_bits[0] = (data[i] >> 2) & 0x3F;
    six_bits[1] = ((data[i] << 4) & 0x30) + ((data[i + 1] >> 4) & 0x0F);
    six_bits[2] = ((data[i + 1] << 2) & 0x3C) + ((data[i + 2] >> 6) & 0x03);
    six_bits[3] = data[i + 2] & 0x3F;
    // now we use the helper function to convert from six-bits to base64
    for (j = 0; j < 4; j++) {
      dest[k + j] = encode_six(six_bits[j]);
    }
    k += 4;
  }

  // at the end, we add = if the input is not divisible by 3
  if ((len % 3) == 1) {
    // two equals at end
    dest[k - 2] = '=';
    dest[k - 1] = '=';
  } else if ((len % 3) == 2) {
    dest[k - 1] = '=';
  }

  // finally, zero terminate the output string
  dest[k] = 0;

  return strlen(dest);
}

size_t Base64::get_encoded_size(size_t nbyte) {
  return ((((nbyte * 4 + 2) / 3) + 3) / 4) * 4;
}

int Base64::decode(void *dest, const char *src, int nbyte) {
  char *data = (char *)dest;
  int i;
  int j;

  u8 eight_bits[4] = {0};

  for (i = 0, j = 0; j < nbyte; i += 3, j += 4) {
    eight_bits[0] = decode_eight(src[j]);
    eight_bits[1] = decode_eight(src[j + 1]);
    eight_bits[2] = decode_eight(src[j + 2]);
    eight_bits[3] = decode_eight(src[j + 3]);
    data[i] = (eight_bits[0] << 2) | (eight_bits[1] >> 4);
    data[i + 1] = (eight_bits[1] << 4) | (eight_bits[2] >> 2);
    data[i + 2] = (eight_bits[2] << 6) | (eight_bits[3]);
  }

  int padding = 0;
  if (j >= 4) {
    if (src[j - 4 + 2] == '=') {
      padding = 2;
    } else if (src[j - 4 + 3] == '=') {
      padding = 1;
    }
  }

  return padding;
}

size_t Base64::get_decoded_size(size_t nbyte) {
  // doesn't account for padding
  return (nbyte * 3 + 3) / 4;
}

// This is a helper function to convert a six-bit value to base64
char Base64::encode_six(u8 six_bit_value) {
  u8 x;
  char c = -1;
  x = six_bit_value & ~0xC0; // remove top two bits (should be zero anyway)
  if (x < 26) {
    c = 'A' + x;
  } else if (x < 52) {
    c = 'a' + (x - 26);
  } else if (x < 62) {
    c = '0' + (x - 52);
  } else if (x == 62) {
    c = '+';
  } else if (x == 63) {
    c = '/';
  }
  return c;
}

char Base64::decode_eight(u8 eight_bit_value) {
  u8 x;
  x = eight_bit_value;
  if ((x >= 'A') && (x <= 'Z')) {
    return x - 'A';
  }

  if ((x >= 'a') && (x <= 'z')) {
    return x - 'a' + 26;
  }

  if ((x >= '0') && (x <= '9')) {
    return x - '0' + 52;
  }

  if (x == '+') {
    return 62;
  }

  if (x == '/') {
    return 63;
  }

  return 0;
}

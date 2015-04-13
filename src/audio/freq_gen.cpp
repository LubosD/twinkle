/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "freq_gen.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include "rtp_telephone_event.h"

#define PI	3.141592653589793

t_freq_gen::t_freq_gen(vector<uint16> frequencies, int8 db_level) {
	assert(frequencies.size() > 0);
	assert(db_level <= 0);
	
	_frequencies = frequencies;
	
	// dB = 20 * log(amplitude / 32768)
	// 32767 is used below as +32768 does not fit in 16 bits
	_amplitude = int16(pow(10.0, db_level / 20.0) * 32767 / _frequencies.size());
}

t_freq_gen::t_freq_gen(uint8 dtmf, int8 db_level) : _frequencies(2)
{
	assert(db_level <= 0);
	
	switch (dtmf) {
	case TEL_EV_DTMF_1:
		_frequencies[0] = 697;
		_frequencies[1] = 1209;
		break;
	case TEL_EV_DTMF_2:
		_frequencies[0] = 697;
		_frequencies[1] = 1336;
		break;
	case TEL_EV_DTMF_3:
		_frequencies[0] = 697;
		_frequencies[1] = 1477;
		break;
	case TEL_EV_DTMF_A:
		_frequencies[0] = 697;
		_frequencies[1] = 1633;
		break;
	case TEL_EV_DTMF_4:
		_frequencies[0] = 770;
		_frequencies[1] = 1209;
		break;
	case TEL_EV_DTMF_5:
		_frequencies[0] = 770;
		_frequencies[1] = 1336;
		break;
	case TEL_EV_DTMF_6:
		_frequencies[0] = 770;
		_frequencies[1] = 1477;
		break;
	case TEL_EV_DTMF_B:
		_frequencies[0] = 770;
		_frequencies[1] = 1633;
		break;
	case TEL_EV_DTMF_7:
		_frequencies[0] = 852;
		_frequencies[1] = 1209;
		break;
	case TEL_EV_DTMF_8:
		_frequencies[0] = 852;
		_frequencies[1] = 1336;
		break;
	case TEL_EV_DTMF_9:
		_frequencies[0] = 852;
		_frequencies[1] = 1477;
		break;
	case TEL_EV_DTMF_C:
		_frequencies[0] = 852;
		_frequencies[1] = 1633;
		break;
	case TEL_EV_DTMF_STAR:
		_frequencies[0] = 941;
		_frequencies[1] = 1209;
		break;
	case TEL_EV_DTMF_0:
		_frequencies[0] = 941;
		_frequencies[1] = 1336;
		break;
	case TEL_EV_DTMF_POUND:
		_frequencies[0] = 941;
		_frequencies[1] = 1477;
		break;
	case TEL_EV_DTMF_D:
		_frequencies[0] = 941;
		_frequencies[1] = 1633;
		break;
	default:
		assert(false);
	}
		
	// dB = 20 * log(amplitude / 32768)
	// 32767 is used below as +32768 does not fit in 16 bits
	_amplitude = int16(pow(10.0, db_level / 20.0) * 32767 / _frequencies.size());
}

int16 t_freq_gen::get_sample(uint32 ts_usec) const {
	double freq_sum = 0.0;

	for (vector<uint16>::const_iterator f = _frequencies.begin();
	     f != _frequencies.end(); f++)
	{
		freq_sum += sin(*f * 2.0 * PI * (double)ts_usec / 1000000.0);
	}
	
	return (int16)(_amplitude * freq_sum);
}

void t_freq_gen::get_samples(int16 *sample_buf, uint16 buf_len, 
			uint32 ts_start, double interval) const
{
	for (uint16 i = 0; i < buf_len; i++) {
		sample_buf[i] = get_sample(uint32(ts_start + interval * i));
	}
}

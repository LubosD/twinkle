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

/**
 * @file
 * Sequence number operations.
 */

#ifndef _SEQUENCE_NUMBER_H
#define _SEQUENCE_NUMBER_H

#include <cc++/config.h>

/**
 * Sequence numbers.
 * Sequence numbers with comparison operators that deal with sequence number
 * roll-overs using serial number arithmetic.
 * See http://en.wikipedia.org/wiki/Serial_Number_Arithmetic
 *
 * @param U The unsigned int type for the sequence number.
 * @param S The corresponsing signed int type having the same width.
 */
template< typename U, typename S >
class sequence_number_t {
private:
	/**
	 * The sequence number.
	 */
	U _number;

public:
	/**
	 * Constructor.
	 * @param number The sequence number.
	 */
	explicit sequence_number_t(U number) : _number(number)
	{};

	/**
	 * Get the sequence number.
	 * @return The sequence number.
	 */
	U get_number(void) const {
		return _number;
	}

	/**
	 * Cast to the sequence number.
	 */
	operator U(void) const {
		return get_number();
	}

	/**
	 * Calculate the distance to another sequence number.
	 * @param number The sequence number to which the distance must be calculated.
	 * @return The distance.
	 */
	S distance(const sequence_number_t &number) const {
		return static_cast<S>(_number - number.get_number());
	}

	/**
	 * Calculate the distance to another distance sequence number.
	 * @param number The sequence number to which the distance must be calculated.
	 * @return The distance.
	 */
	S operator-(const sequence_number_t &number) const {
		return distance(number);
	}

	/**
	 * Less-than comparison.
	 * @param number The sequence number to compare with.
	 * @return true, if this sequence number is less than number.
	 * @return false, otherwise.
	 */
	bool operator<(const sequence_number_t &number) const {
		return (distance(number) < 0);
	}

	/**
	 * Less-than-equal comparison.
	 * @param number The sequence number to compare with.
	 * @return true, if this sequence number is less than or equal to number.
	 * @return false, otherwise.
	 */
	bool operator<=(const sequence_number_t &number) const {
		return (distance(number) <= 0);
	}

	/**
	 * Equality comparison.
	 * @param number The sequence number to compare with.
	 * @return true, if this sequence number is equal to number.
	 * @return false, otherwise.
	 */
	bool operator==(const sequence_number_t &number) const {
		return (number.get_number() == _number);
	}

	/**
	 * Greater-than comparison.
	 * @param number The sequence number to compare with.
	 * @return true, if this sequence number is greater than number.
	 * @return false, otherwise.
	 */
	bool operator>(const sequence_number_t &number) const {
		return (distance(number) > 0);
	}

	/**
	 * Greater-than-equal comparison.
	 * @param number The sequence number to compare with.
	 * @return true, if this sequence number is greater than or equal to number.
	 * @return false, otherwise.
	 */
	bool operator>=(const sequence_number_t &number) const {
		return (distance(number) >= 0);
	}
};

/**
 * 16-bit sequence number
 */
typedef sequence_number_t<uint16, int16> seq16_t;

/**
 * 32-bit sequence number
 */
typedef sequence_number_t<uint32, int32> seq32_t;

#endif

/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>
    Copyright (C) 2022       Frédéric Brière <fbriere@fbriere.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "hdr_reason.h"
#include "parse_ctrl.h"
#include "util.h"


t_reason::t_reason(const std::string &_protocol) :
	protocol(_protocol)
{
}

void t_reason::set_cause(const std::string &_cause) {
	cause = _cause;
}

void t_reason::set_text(const std::string &_text) {
	text = _text;
}

void t_reason::add_extension(const t_parameter &p) {
	extensions.push_back(p);
}

std::string t_reason::encode(void) const {
	std::string s;

	s = protocol;

	if (!cause.empty()) {
		s += ";cause=";
		s += cause;
	}

	if (!text.empty()) {
		s += ";text=\"";
		s += text;
		s += "\"";
	}

	s += param_list2str(extensions);

	return s;
}


t_hdr_reason::t_hdr_reason() : t_header("Reason") {}

void t_hdr_reason::add_reason(const t_reason &reason) {
	populated = true;
	reason_list.push_back(reason);
}

std::string t_hdr_reason::get_display_text() const {
	for (const auto &r : reason_list) {
		if (r.protocol == "SIP") {
			std::vector<std::string> elems;

			if (!r.cause.empty()) {
				elems.push_back(r.cause);
			}
			if (!r.text.empty()) {
				elems.push_back(r.text);
			}

			if (!elems.empty()) {
				return join_strings(elems, " ");
			}
		}
	}

	return {};
}

std::string t_hdr_reason::encode(void) const {
	return (t_parser::multi_values_as_list ?
			t_header::encode() : encode_multi_header());
}

std::string t_hdr_reason::encode_multi_header(void) const {
	std::string s;

	if (!populated) return s;

	for (const auto &r : reason_list) {
		s += header_name;
		s += ": ";
		s += r.encode();
		s += CRLF;
	}

	return s;
}

std::string t_hdr_reason::encode_value(void) const {
	if (!populated) return {};

	std::vector<std::string> encoded_values;

	for (const auto &r : reason_list) {
		encoded_values.push_back(r.encode());
	}

	return join_strings(encoded_values, ",");
}

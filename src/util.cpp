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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <locale>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/time.h>

#include "util.h"

#include "twinkle_config.h"

string month_abbrv[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", 
			"Aug", "Sep", "Oct", "Nov", "Dec"};
			
string month_full[] = {"January", "February", "March", "April", "May", "June", "July",
		       "August", "September", "October", "November", "December"};
			
string day_abbrv[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

string random_token(int length) {
	string s;

	for (int i = 0; i < length; i++) {
		s += char(rand() % 26 + 97);
	}

	return s;
}

string random_hexstr(int length) {
	string s;
	int x;

	for (int i = 0; i < length; i++) {
		x = rand() % 16;
		if (x <= 9)
			s += '0' + x;
		else
			s += 'a' + x - 10;
	}

	return s;
}

string float2str(float f, int precision) {
	ostringstream s;
	
	// Force the locale to POSIX, such that a dot is used for
	// the decimal point.
	s.imbue(locale("POSIX"));
	s.setf(ios::fixed,ios::floatfield);
	s.precision(precision);
	s << f;
	
	return s.str();
}

string int2str(int i, const char *format) {
	char buf[32];

	snprintf(buf, 32, format, i);
	return string(buf);
}

string int2str(int i) {
	return int2str(i, "%d");
}

string ulong2str(unsigned long i, const char *format) {
	char buf[32];

	snprintf(buf, 32, format, i);
	return string(buf);
}

string ulong2str(unsigned long i) {
	return ulong2str(i, "%u");
}

string ptr2str(void *p) {
	char buf[32];
	
	snprintf(buf, 32, "%p", p);
	return string(buf);
}

string bool2str(bool b) {
	return (b ? "true" : "false");
}

string time2str(time_t t, const char *format) {
	struct tm tm;
	char buf[64];
	
	localtime_r(&t, &tm);
	strftime(buf, 64, format, &tm);
	return string(buf);
}

string current_time2str(const char *format) {
	struct timeval t;
	
	gettimeofday(&t, NULL);
	return time2str(t.tv_sec, format);
}

string weekday2str(int wkday) {
	if (wkday >= 0 && wkday <= 6) return day_abbrv[wkday];
	return "XXX";
}

string month2str(int month) {
	if (month >= 0 && month <= 11) return month_abbrv[month];
	return "XXX";
}

int str2month_full(const string &month) {
	for (int i = 0; i < 12; i++) {
		if (cmp_nocase(month_full[i], month) == 0) {
			return i;
		}
	}
	
	return 0;
}

string duration2str(unsigned long seconds) {
	string result;
	long remainder, h, m, s;
	
	h = seconds / 3600;
	remainder = seconds % 3600;
	m = remainder / 60;
	s = remainder % 60;

	if (h > 0) {
		result = ulong2str(h);
		result += "h ";
	}
	
	if (!result.empty() || m > 0) {
		result += ulong2str(m);
		result += "m ";
	}
	
	result += ulong2str(s);
	result += "s";

	return result;
}

string timer2str(unsigned long seconds) {
	string result;
	unsigned long remainder, h, m, s;
	
	h = seconds / 3600;
	remainder = seconds % 3600;
	m = remainder / 60;
	s = remainder % 60;
	
	char buf[16];
	snprintf(buf, 16, "%01lu:%02lu:%02lu", h, m, s);
	return string(buf);
}

static uint8 hexdig2value(char hexdig) {
	uint8 val = 0;
	
	if (hexdig >= '0' && hexdig <= '9')
		val = hexdig - '0';
	else if (hexdig >= 'a' && hexdig <= 'f')
		val = hexdig - 'a' + 10;
	else if (hexdig >= 'A' && hexdig <= 'F')
		val = hexdig - 'A' + 10;
		
	return val;
}

static char value2hexdig(uint8 val) {
	char c = '0';
	
	if (val <= 9) {
		c = '0' + val;
	} else if (val <= 15) {
		c = 'a' + val - 10;
	}
	
	return c;
}

unsigned long hex2int(const string &h) {
	unsigned long u = 0;

	int power = 1;
	for (string::const_reverse_iterator i = h.rbegin(); i != h.rend(); ++i) {
		u += hexdig2value(*i) * power;
		power = power * 16;
	}

	return u;
}

void hex2binary(const string &h, uint8 *buf) {
	uint8 *p = buf;
	
	bool hi_nibble = true;
	for (string::const_iterator i = h.begin() ; i != h.end(); ++i) {
		if (hi_nibble) {
			*p = hexdig2value(*i) << 4;
		} else {
			*(p++) |= hexdig2value(*i);
		}
		
		hi_nibble = !hi_nibble;
	}
}

string binary2hex(uint8 *buf, unsigned long len) {
	string s;
	
	for (uint8 *p = buf; p < buf + len; ++p) {
		s += value2hexdig((*p >> 4) & 0xf);
		s += value2hexdig(*p & 0xf);
	}
	
	return s;
}

string tolower(const string &s) {
	string result;

	for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
		result += tolower(*i);
	}

	return result;
}

string toupper(const string &s) {
	string result;

	for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
		result += toupper(*i);
	}

	return result;
}

string rtrim(const string &s) {
	string::size_type i;

	i = s.find_last_not_of(' ');
	if (i == string::npos) return "";
	if (i == s.size()-1) return s;
	return s.substr(0, i+1);
}

string ltrim(const string &s) {
	string::size_type i;

	i = s.find_first_not_of(' ');
	if (i == string::npos) return "";
	if (i == 0) return s;
	return s.substr(i, s.size()-i+1);
}

string trim(const string &s) {
	return ltrim(rtrim(s));
}

string padleft(const string &s, char c, unsigned long len) {
	string result(c, len);
	result += s;
	return result.substr(result.size() - len);
}

int cmp_nocase(const string &s1, const string &s2) {
	string::const_iterator i1 = s1.begin();
	string::const_iterator i2 = s2.begin();

	while (i1 != s1.end() && i2 != s2.end()) {
		if (toupper(*i1) != toupper(*i2)) {
			return (toupper(*i1) < toupper(*i2)) ? -1 : 1;
		}
		++i1;
		++i2;
	}

	if (s1.size() == s2.size()) return 0;
	if (s1.size() < s2.size()) return -1;
	return 1;
}

bool must_quote(const string &s) {
	string special("()<>@,;:\\\"/[]?={} \t");

	if (s.size() == 0) return true;
	return (s.find_first_of(special) != string::npos);
}

string escape(const string &s, char c) {
	string result;

	for (string::size_type i = 0; i < s.size(); i++) {
		if (s[i] == '\\' || s[i] == c) {
			result += '\\';
		}
		
		result += s[i];
	}
	
	return result;
}

string unescape(const string &s) {
	string result;

	for (string::size_type i = 0; i < s.size(); i++) {
		if (s[i] == '\\' && i < s.size() - 1) {
			i++;
		}
		
		result += s[i];
	}
	
	return result;
}

string escape_hex(const string &s, const string &unreserved) {
	string result;
	
	for (string::size_type i = 0; i < s.size(); i++) {
		if (unreserved.find(s[i], 0) != string::npos) {
			// Unreserved symbol
			result += s[i];
		} else {
			// Reserved symbol
			result += int2str((int)s[i], "%%%02x");
		}
	}
	
	return result;
}
string unescape_hex(const string &s) {
	string result;
	
	for (string::size_type i = 0; i < s.size(); i++) {
		if (s[i] == '%' && i < s.size() - 2 &&
		    isxdigit(s[i+1]) && isxdigit(s[i+2])) 
		{
			// Escaped hex-value
			string hexval = s.substr(i+1, 2);
			result += static_cast<char>(hex2int(hexval));
			i += 2;
		} else {
			result += s[i];
		}
	}
	
	return result;
}

string replace_char(const string &s, char from, char to) {
	string result = s;

	for (string::size_type i = 0; i < result.size(); i++) {
        	if (result[i] == from) result[i] = to;
   	}
   	
   	return result;
}

string replace_first(const string &s, const string &from, const string &to) {
	string result = s;
	
	string::size_type i = result.find(from, 0);
	if (i != string::npos) {
		result.replace(i, from.size(), to);
	}
	
	return result;
}

vector<string> split(const string &s, char c) {
	string::size_type i;
	string::size_type j = 0;
	vector<string> l;

	while (true) {
		i = s.find(c, j);
		if (i == string::npos) {
			l.push_back(s.substr(j));
			return l;
		}

		if (i == j)
			l.push_back("");
		else
			l.push_back(s.substr(j, i-j));

		j = i+1;

		if (j == s.size()) {
			l.push_back("");
			return l;
		}
	}
}

vector<string> split(const string &s, const string& separator) {
	string::size_type i;
	string::size_type j = 0;
	vector<string> l;

	while (true) {
		i = s.find(separator, j);
		if (i == string::npos) {
			l.push_back(s.substr(j));
			return l;
		}

		if (i == j)
			l.push_back("");
		else
			l.push_back(s.substr(j, i-j));

		j = i + separator.size();

		if (j == s.size()) {
			l.push_back("");
			return l;
		}
	}
}

vector<string> split_linebreak(const string &s) {
	if (s.find("\r\n") != string::npos) {
		return split(s, "\r\n");
	} else if (s.find("\r") != string::npos) {
		return split(s, "\r");
	}
	
	return split(s, "\n");
}

vector<string> split_on_first(const string &s, char c) {
	vector<string> l;
	string::size_type i = s.find(c);
	if (i == string::npos) {
		l.push_back(s);
	} else {
		if (i == 0) {
			l.push_back("");
		} else {
			l.push_back(s.substr(0, i));
		}
		
		if (i == s.size() - 1) {
			l.push_back("");
		} else {
			l.push_back(s.substr(i + 1));
		}
	}
	
	return l;
}

vector<string> split_on_last(const string &s, char c) {
	vector<string> l;
	string::size_type i = s.find_last_of(c);
	if (i == string::npos) {
		l.push_back(s);
	} else {
		if (i == 0) {
			l.push_back("");
		} else {
			l.push_back(s.substr(0, i));
		}
		
		if (i == s.size() - 1) {
			l.push_back("");
		} else {
			l.push_back(s.substr(i + 1));
		}
	}
	
	return l;
}

vector<string> split_escaped(const string &s, char c) {
	vector<string> l;
	
	string::size_type start_pos = 0;
	for (string::size_type i = 0; i < s.size(); i++) {
		if (s[i] == '\\') {
			// Skip escaped character
			if (i < s.size()) i++;
			continue;
		}
		
		if (s[i] == c) {
			l.push_back(unescape(s.substr(start_pos, i - start_pos)));
			start_pos = i + 1;
		}
	}
	
	if (start_pos < s.size()) {
		l.push_back(unescape(s.substr(start_pos, s.size() - start_pos)));
	} else if (start_pos == s.size()) {
		l.push_back("");
	}
	
	return l;
}

vector<string> split_ws(const string &s, bool quote_sensitive) {
        vector<string> l;
        bool in_quotes = false;

        string::size_type start_pos = 0;
        for (string::size_type i = 0; i < s.size(); i++ ) {
                if (quote_sensitive && s[i] == '"') {
                        in_quotes = !in_quotes;
                        continue;
                }

                if (in_quotes) continue;

                if (s[i] == ' ' || s[i] == '\t') {
                        // Skip consecutive white space
                        if (start_pos != i) {
                                l.push_back(s.substr(start_pos, i - start_pos));
                        }
                        start_pos = i + 1;
                }
        }

        if (start_pos < s.size()) {
                l.push_back(s.substr(start_pos, s.size() - start_pos));
        }

        return l;
}

string join_strings(const vector<string> &v, const string &separator) {
	string text;
	for (vector<string>::const_iterator it = v.begin(); it != v.end(); ++it)
	{
		if (it != v.begin()) {
			text += separator;
		}
		text += *it;
	}
	
	return text;
}

string unquote(const string &s) {
        if (s.size() <= 1) return s;

        if (s[0] == '"' && s[s.size() - 1] == '"')
                return s.substr(1, s.size() - 2);

        return s;
}

bool is_number(const string &s) {
	if (s.empty()) return false;
	
        for (string::size_type i = 0; i < s.size(); i++ ) {
		if (!isdigit(s[i])) return false;
	}

	return true;
}

bool is_ipaddr(const string &s) {
	vector<string> l = split(s, '.');
	if (l.size() != 4) return false;
	
	for (vector<string>::iterator i = l.begin(); i != l.end(); ++i) {
		if (!is_number(*i) || atoi(i->c_str()) > 255) return false;
	}
	
	return true;
}

bool yesno2bool(const string &yesno) {
	return (yesno == "yes" ? true : false);
}
string bool2yesno(bool b) {
	return (b ? "yes" : "no");
}

string str2dtmf(const string &s) {
	string result;
	string to_convert = tolower(s);
	
	for (string::size_type i = 0; i < to_convert.size(); i++) {
		switch (to_convert[i]) {
		case '1':
			result += '1';
			break;
		case '2':
		case 'a':
		case 'b':
		case 'c':
			result += '2';
			break;
		case '3':
		case 'd':
		case 'e':
		case 'f':
			result += '3';
			break;
		case '4':
		case 'g':
		case 'h':
		case 'i':
			result += '4';
			break;
		case '5':
		case 'j':
		case 'k':
		case 'l':
			result += '5';
			break;
		case '6':
		case 'm':
		case 'n':
		case 'o':
			result += '6';
			break;
		case '7':
		case 'p':
		case 'q':
		case 'r':
		case 's':
			result += '7';
			break;
		case '8':
		case 't':
		case 'u':
		case 'v':
			result += '8';
			break;
		case '9':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
			result += '9';
			break;
		case '0':
		case ' ':
			result += '0';
			break;
		case '#':
		case '*':
			result += to_convert[i];
			break;
		}
	}
	
	return result;
}

bool looks_like_phone(const string &s, const string &special_symbols) {
	string phone_symbols= special_symbols + "0123456789*#+ \t";
	string t;
	
	for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
		if (phone_symbols.find(*i) == string::npos) return false;
	}

	return true;
}

string remove_symbols(const string &s, const string &special_symbols) {
	string result;
	
	for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
		if (special_symbols.find(*i) == string::npos) {
			result += *i;
		}
	}

	return result;
}

string remove_white_space(const string &s) {
	string result;
	
	for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
		if (*i != ' ' && *i != '\t') {
			result += *i;
		}
	}
	
	return result;
}

string dotted_truncate(const string &s, string::size_type len) {
	if (len >= s.size()) return s;
	
	return s.substr(0, len) + "...";
}

string to_printable(const string &s) {
	string result;
	
	for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
		if (isprint(*i) || *i == '\n' || *i == '\r') {
			result += *i;
		} else {
			result += '.';
		}
	}
	
	return result;
}

string get_error_str(int errnum) {
#if HAVE_STRERROR_R
	char buf[81];
	memset(buf, 0, sizeof(buf));
#if STRERROR_R_CHAR_P
	string errmsg(strerror_r(errnum, buf, sizeof(buf)-1));
#else
	string errmsg;
	if (strerror_r(errnum, buf, sizeof(buf)-1) == 0) {
		errmsg = buf;
	} else {
		errmsg = "unknown error: ";
		errmsg += int2str(errnum);
	}
#endif
#else
	string errmsg("strerror_r is not available: ");
	errmsg += int2str(errnum);
#endif
	return errmsg;
}

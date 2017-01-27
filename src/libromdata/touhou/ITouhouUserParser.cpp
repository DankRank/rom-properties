/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* ITouhouUserParser.cpp: Touhou Replay USER parser.                       *
*                                                                         *
* Copyright (c) 2017 by Egor.                                             *
*                                                                         *
* This program is free software; you can redistribute it and/or modify it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation; either version 2 of the License, or (at your  *
* option) any later version.                                              *
*                                                                         *
* This program is distributed in the hope that it will be useful, but     *
* WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
* GNU General Public License for more details.                            *
*                                                                         *
* You should have received a copy of the GNU General Public License along *
* with this program; if not, write to the Free Software Foundation, Inc., *
* 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
***************************************************************************/

#include "ITouhouUserParser.hpp"

#include <cassert>

namespace LibRomData {
	std::string ITouhouUserParser::spaceString(std::string line, const std::string& keyword) {
		size_t pos = line.find(' ');

		if (pos != std::string::npos) {
			// Check the keyword
			if (line.substr(0, pos) != keyword) is_broken = true;

			return line.substr(pos + 1);
		}
		else {
			is_broken = true;
			return "";
		}
	}

	time_t ITouhouUserParser::parseDate(std::string datestr) {
		// format: yy/mm/dd hh:mm
		if (datestr.length() != 14
			|| datestr[2] != '/'
			|| datestr[5] != '/'
			|| datestr[8] != ' '
			|| datestr[11] != ':') return -1;

		tm t;
		t.tm_year = 100 + std::stoi(datestr);
		t.tm_mon = std::stoi(datestr.substr(3));
		t.tm_mday = std::stoi(datestr.substr(6));
		t.tm_hour = std::stoi(datestr.substr(9));
		t.tm_min = std::stoi(datestr.substr(12));
		t.tm_sec = 0;
		t.tm_isdst = 0; // unknown
						// other fields don't need to be set for mktime to work.
		return mktime(&t);
	}

	bool ITouhouUserParser::breakLines(char* text, uint32_t size, std::string *lines, int linecount) {
		// Make sure there's a null at the very end
		assert(!text[size - 1]);
		if (text[size - 1]) return false;

		char *p = text;
		for (int i = 0; i < linecount; i++) {
			// Look for a new-line
			char* p2 = strstr(p, "\r\n");
			if (!p2) return false; // line not found

			lines[i] = std::string(p, p2 - p);
			p = p2 + 2; // skip \r\n
		}

		return true;
	}

	ITouhouUserParser::ITouhouUserParser() : is_valid(false), is_broken(false),
		time(-1), is_clear(false), score(0), slowrate(99.99f), comment_present(false)
	{
	}
	bool ITouhouUserParser::isValid() {
		return is_valid;
	}
	bool ITouhouUserParser::isBroken() {
		return is_broken;
	}
	rp_string ITouhouUserParser::getVersion() {
		return cp1252_sjis_to_rp_string(version.c_str(), -1);
	}
	rp_string ITouhouUserParser::getName() {
		return cp1252_sjis_to_rp_string(name.c_str(), -1);
	}
	time_t ITouhouUserParser::getTime() {
		return time;
	}
	rp_string ITouhouUserParser::getChara() {
		return cp1252_sjis_to_rp_string(chara.c_str(), -1);
	}
	rp_string ITouhouUserParser::getRank() {
		return cp1252_sjis_to_rp_string(rank.c_str(), -1);
	}

	rp_string ITouhouUserParser::isClear() {
		return is_clear ? _RP("Yes") : _RP("No");
	}
	int ITouhouUserParser::getScore() {
		return score;
	}
	rp_string ITouhouUserParser::getSlowRate() {
		char buf[6];
		snprintf(buf, sizeof(buf), "%2.2f", slowrate);
		return cp1252_sjis_to_rp_string(buf, -1);
	}
	rp_string ITouhouUserParser::getComment() {
		if (comment_present) {
			return cp1252_sjis_to_rp_string(comment.c_str(), -1);
		}
		else {
			return _RP("None");
		}
	}
	ITouhouUserParser::~ITouhouUserParser()
	{
	}
}
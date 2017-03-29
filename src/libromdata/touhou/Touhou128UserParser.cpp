/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* Touhou128UserParser.cpp: Touhou 128 Replay USER parser.                 *
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

#include "Touhou128UserParser.hpp"
#include "TouhouUserReader.hpp"
#include "TextFuncs.hpp"

#include <string>

namespace LibRomData {
	bool Touhou128UserParser::parseLines(const std::string(&lines)[9]) {
		// Line 0 - "東方星蓮船 リプレイファイル情報" - "Tohuou Seirensen replay file info"
		// (skipped)

		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		time = parseDate(spaceString(lines[3], "Date"));

		// Line 4 - "Route %s"
		route = spaceString(lines[2], "Name");

		// Line 5 - "Rank %s"
		rank = spaceString(lines[5], "Rank");

		// Line 6 - which stage was completed
		if (lines[6] == "Extra Stage Clear") {
			is_extra = true;
			is_clear = true;
		}
		else if (lines[6] == "Extra Stage") {
			is_extra = true;
			is_clear = false;
		}
		else if (lines[6] == "Stage All Clear") {
			is_extra = false;
			is_clear = true;
		}
		else {
			is_extra = false;
			is_clear = false;
			std::string stage = spaceString(lines[6], "Stage");
			size_t pos = stage.find(' ');
			beginStage = std::stoi(stage.substr(0, pos));
			if (pos == std::string::npos) {
				endStage = beginStage;
			}
			else {
				endStage = std::stoi(spaceString(stage.substr(pos + 1), "\x81\x60")); // \x81\x60 is SJIS '〜'
			}
		}

		// Line 7 - "Score %d"
		score = std::stol(spaceString(lines[7], "Score"));

		// Line 8 - "Slow Rate %2.2f"
		// TODO: maybe make it less hacky?
		slowrate = std::stof(spaceString(spaceString(lines[8], "Slow"), "Rate"));

		// In FW you always play as Cirno.
		chara = "Cirno";

		return true;
	}

	rp_string Touhou128UserParser::getStage() {
		// TODO: figure out how route relates to stage number
		char buf[6];
		if (is_extra) {
			return _RP("Extra");
		}
		else if (is_clear) {
			return _RP("All");
		}
		else if (beginStage != endStage) {
			snprintf(buf, sizeof(buf), "%d ~ %d", beginStage, endStage);
			return cp1252_sjis_to_rp_string(buf, -1);
		}
		else {
			snprintf(buf, sizeof(buf), "%d", beginStage);
			return cp1252_sjis_to_rp_string(buf, -1);
		}
	}
	Touhou128UserParser::Touhou128UserParser(int gameType, IRpFile* file) :
		beginStage(0), endStage(0), is_extra(false) {
		TouhouUserReader ur(gameType, file);
		if (ur.comment) {
			comment = std::string(ur.comment);
			comment_present = true;
		}
		else {
			comment_present = false;
		}

		if (!ur.text) {
			return;
		}

		std::string lines[9];
		if (!breakLines(ur.text, ur.text_sz, lines, 9)) {
			return;
		}

		is_valid = parseLines(lines);

	}
}
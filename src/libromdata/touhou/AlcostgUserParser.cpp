/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* AlcostgUserParser.cpp: Alcostg Replay USER parser.                      *
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

#include "AlcostgUserParser.hpp"
#include "TouhouUserReader.hpp"
#include "TextFuncs.hpp"

#include <string>

namespace LibRomData {
	bool AlcostgUserParser::parseLines(const std::string(&lines)[7]) {
		// Line 0 - "リプレイファイル情報" - "replay file info"
		// (skipped)

		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		time = parseDate(spaceString(lines[3], "Date"));

		// Line 4 - which stage was completed
		std::string stage = spaceString(lines[4], "Stage");
		if (stage == "All Clear") {
			is_clear = true;
		}
		else {
			is_clear = false;
			size_t pos = stage.find(' ');
			beginStage = std::stoi(stage.substr(0, pos));
			if (pos == std::string::npos) {
				endStage = beginStage;
			}
			else {
				endStage = std::stoi(spaceString(stage.substr(pos + 1), "\x81\x60")); // \x81\x60 is SJIS '〜'
			}
		}

		// Line 5 - "Score %d"
		score = std::stol(spaceString(lines[5], "Score"));

		// Line 6 - "Slow Rate %2.2f"
		// TODO: maybe make it less hacky?
		slowrate = std::stof(spaceString(spaceString(lines[6], "Slow"), "Rate"));

		// In alcostg you always play as Isami.
		chara = "Isami Asama";

		// There are no Ranks in alcostg
		rank = "N/a";

		return true;
	}

	rp_string AlcostgUserParser::getStage() {
		char buf[6];
		if (is_clear) {
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
	AlcostgUserParser::AlcostgUserParser(int gameType, IRpFile* file) :
		beginStage(0), endStage(0) {
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

		std::string lines[7];
		if (!breakLines(ur.text, ur.text_sz, lines, 7)) {
			return;
		}

		is_valid = parseLines(lines);

	}
}
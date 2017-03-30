/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* Touhou095UserParser.cpp: Touhou 095 Replay USER parser.                 *
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

#include "Touhou095UserParser.hpp"
#include "TouhouUserReader.hpp"
#include "TextFuncs.hpp"

#include <string>

namespace LibRomData {
	bool Touhou095UserParser::parseLines(const std::string(&lines)[8]) {
		// Line 0 - "東方文花帖 リプレイファイル情報" - "Touhou Bunkachou replay file info"
		// (skipped)

		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - Level
		// Line 4 - Scene
		// the stage is combination of level and scene, e.g. 1-1, 4-2, etc.
		// level can also be EX (extra) e.g. EX-1, EX-2

		std::string strlevel = spaceString(lines[3], "Level");
		if (strlevel == "EX") {
			is_extra = true;
			level = 0;
		}
		else {
			is_extra = false;
			level = std::stoi(strlevel);
		}

		scene = std::stoi(spaceString(lines[4], "Scene"));

		// Line 5 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		time = parseDate(spaceString(lines[5], "Date"));

		// Line 6 - "Score %d"
		score = std::stol(spaceString(lines[6], "Score"));

		// Line 7 - "Slow Rate %2.2f"
		slowrate = std::stof(spaceString(lines[7], "Slow Rate"));

		// In StB you always play as Aya.
		chara = "Aya Shameimaru";

		// There are no Ranks in StB
		rank = "N/a";

		return true;
	}

	rp_string Touhou095UserParser::getStage() {
		char buf[5];
		if (is_extra) {
			snprintf(buf, sizeof(buf), "EX-%d", scene);
		}
		else {
			snprintf(buf, sizeof(buf), "%d-%d", level, scene);
		}
		return cp1252_sjis_to_rp_string(buf, -1);
	}

	Touhou095UserParser::Touhou095UserParser(int gameType, IRpFile* file) :
		level(0), scene(0), is_extra(false) {
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

		std::string lines[8];
		if (!breakLines(ur.text, ur.text_sz, lines, 8)) {
			return;
		}

		is_valid = parseLines(lines);

	}

	rp_string Touhou095UserParser::isClear() {
		// TODO: figure out how to find this value
		return _RP("N/a");
	}
}
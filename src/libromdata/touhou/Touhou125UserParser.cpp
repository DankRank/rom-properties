/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* Touhou125UserParser.cpp: Touhou 125 Replay USER parser.                 *
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

#include "Touhou125UserParser.hpp"
#include "TouhouUserReader.hpp"
#include "TextFuncs.hpp"

#include <string>

namespace LibRomData {
	bool Touhou125UserParser::parseLines(const std::string(&lines)[8]) {
		// Line 0 - "東方文花帖 リプレイファイル情報" - "Touhou Bunkachou replay file info"
		// (skipped)

		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		time = parseDate(spaceString(lines[3], "Date"));

		// Line 4 - "Chara %s"
		chara = spaceString(lines[4], "Chara");

		// Line 5 - "%s-%d" (where %s is number from 1 to 12, or "EX" or "SP")
		stage = lines[5];

		// Line 6 - "Score %d"
		score = std::stol(spaceString(lines[6], "Score"));

		// Line 7 - "Slow Rate %2.2f"
		slowrate = std::stof(spaceString(lines[7], "Slow Rate"));

		// There are no Ranks in DS
		rank = "N/a";

		return true;
	}

	rp_string Touhou125UserParser::getStage() {
		return cp1252_sjis_to_rp_string(stage.c_str(), -1);
	}

	Touhou125UserParser::Touhou125UserParser(int gameType, IRpFile* file) {
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

	rp_string Touhou125UserParser::isClear() {
		// TODO: figure out how to find this value
		return _RP("N/a");
	}
}
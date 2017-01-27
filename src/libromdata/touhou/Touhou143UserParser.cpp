/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* Touhou143UserParser.cpp: Touhou 143 Replay USER parser.                 *
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

#include "Touhou143UserParser.hpp"
#include "TouhouUserReader.hpp"
#include "TextFuncs.hpp"

#include <string>

namespace LibRomData {
	bool Touhou143UserParser::parseLines(const std::string(&lines)[8]) {
		// Line 0 - "弾幕アマノジャク リプレイファイル情報" - "Danmaku Amanojaku replay file info"
		// (skipped)

		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		time = parseDate(spaceString(lines[3], "Date"));

		// Line 4 - "Day %d"
		// Line 5- "Scene %d"
		day = std::stoi(spaceString(lines[4], "Day"));
		scene = std::stoi(spaceString(lines[5], "Scene"));

		// Line 6 - "Score %d"
		score = std::stol(spaceString(lines[6], "Score"));

		// Line 7 - "Slow Rate %2.2f"
		// TODO: maybe make it less hacky?
		slowrate = std::stof(spaceString(spaceString(lines[7], "Slow"), "Rate"));


		// In ISC you always play as Seija.
		chara = "Seija Kijin";

		// There are no Ranks in ISC
		rank = "N/a";

		return true;
	}

	rp_string Touhou143UserParser::getStage() {
		char buf[6];
		snprintf(buf, sizeof(buf), "%d-%d", day, scene);
		return cp1252_sjis_to_rp_string(buf, -1);
	}

	Touhou143UserParser::Touhou143UserParser(int gameType, IRpFile* file) :
		day(0), scene(0) {
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

	rp_string Touhou143UserParser::isClear() {
		// TODO: figure out how to find this value
		return _RP("N/a");
	}
}
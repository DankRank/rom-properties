/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* Touhou06UserParser.cpp: Touhou 6 Replay USER parser.                    *
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

#include "Touhou06UserParser.hpp"
#include "TouhouUserReader.hpp"
#include "TextFuncs.hpp"
#include "TouhouCryptFile.hpp"

#include <string>

namespace LibRomData {
	rp_string Touhou06UserParser::getStage() {
		/*
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
		*/
		return _RP("N/a");
	}

	rp_string Touhou06UserParser::isClear() {
		return _RP("N/a");
	}

	Touhou06UserParser::Touhou06UserParser(int gameType, IRpFile* file) {
		// Read header to get the decryption key
		file->rewind();
		file->read(&thrpHeader, sizeof(thrpHeader));

		// Read header again, this time with decryption and stage headers.
		TouhouCryptFile cf(file, thrpHeader.key, offsetof(T6RP_Header, unknown));
		cf.rewind();
		cf.read(&thrpHeader, sizeof(thrpHeader));
		for (int i = 0; i < 7; i++) {
			cf.seek(thrpHeader.stage_offset[i]);
			cf.read(&stageHeader[i], sizeof(stageHeader[i]));
		}

		// Parse header

		// TODO: Version
	
		// Name
		thrpHeader.name[sizeof(thrpHeader.name) - 1] = 0;
		name = thrpHeader.name;
		
		// Time
		thrpHeader.date[sizeof(thrpHeader.date) - 1] = 0;
		time = parseDate06(thrpHeader.date);

		// Chara
		static std::string th06chara[] = {
			"ReimuA", "ReimuB", "MarisaA", "MarisaB"
		};
		chara = rangeString(thrpHeader.chara, th06chara, ARRAY_SIZE(th06chara));

		// Rank
		static std::string th06rank[] = {
			"Easy", "Normal", "Hard", "Lunatic", "Extra"
		};
		rank = rangeString(thrpHeader.rank, th06rank, ARRAY_SIZE(th06rank));

		// TODO: Stage / isClear
		
		// Score
		score = thrpHeader.score;

		// Slow Rate
		slowrate = thrpHeader.slowdown_rate;

		// Comment
		comment_present = false;

		is_valid = true;
	}
}
/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* ITouhouUserParser.hpp: Touhou Replay USER parser.                       *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_ITOUHOUUSERPARSER_HPP__
#define __ROMPROPERTIES_LIBROMDATA_ITOUHOUUSERPARSER_HPP__

#include "TextFuncs.hpp"
#include "file/IRpFile.hpp"

#include <ctime>
#include <cstdint>

#include <string>

namespace LibRomData {
	class ITouhouUserParser {
	protected:
		bool is_valid;
		bool is_broken; // if this variable is true, something went wrong during parsing
		std::string version;
		std::string name;
		time_t time;
		std::string chara;
		std::string rank;
		bool is_clear;
		int score;
		float slowrate;

		bool comment_present;
		std::string comment;

		/* Utility functions */
		std::string spaceString(std::string line, const std::string& keyword);
		time_t parseDate(std::string datestr);
		bool breakLines(char* text, uint32_t size, std::string *lines, int linecount);

		ITouhouUserParser();
	public:
		virtual bool isValid();
		virtual bool isBroken();

		virtual rp_string getVersion();
		virtual rp_string getName();
		virtual time_t getTime();
		virtual rp_string getChara();
		virtual rp_string getRank();
		virtual rp_string getStage() = 0; // NOTE: pure virtual
		virtual rp_string isClear();
		virtual int getScore();
		virtual rp_string getSlowRate();
		virtual rp_string getComment();

		virtual ~ITouhouUserParser();

	private:
		template<typename T>
		static ITouhouUserParser* construct(int gameType, IRpFile* file);
	public:
		static ITouhouUserParser* getInstance(int gameType, IRpFile* file);
	};
}

#endif /* __ROMPROPERTIES_LIBROMDATA_ITOUHOUUSERPARSER_HPP__ */
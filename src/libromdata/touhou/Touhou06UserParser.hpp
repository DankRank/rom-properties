/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* Touhou06UserParser.hpp: Touhou 6 Replay USER parser.                    *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_TOUHOU06USERPARSER_HPP__
#define __ROMPROPERTIES_LIBROMDATA_TOUHOU06USERPARSER_HPP__

#include "ITouhouUserParser.hpp"
#include "file/IRpFile.hpp"
#include "th_structs.h"

namespace LibRomData {
	class Touhou06UserParser : public ITouhouUserParser {
		T6RP_Header thrpHeader;
		T6RP_Stage stageHeader[7];
	public:
		rp_string getStage();
		rp_string isClear();
		Touhou06UserParser(int gameType, IRpFile* file);
	};
}

#endif /* __ROMPROPERTIES_LIBROMDATA_TOUHOU06USERPARSER_HPP__ */
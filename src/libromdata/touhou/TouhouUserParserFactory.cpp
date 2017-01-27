/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* TouhouUserParserFactory.hpp: Touhou Replay USER parser factory.         *
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

#include "TouhouUserParserFactory.hpp"
#include "TouhouReplay.hpp"
#include "Touhou095UserParser.hpp"
#include "Touhou125UserParser.hpp"
#include "Touhou143UserParser.hpp"
#include "Touhou10UserParser.hpp"

namespace LibRomData {
	template<typename T>
	ITouhouUserParser* TouhouUserParserFactory::construct(int gameType, IRpFile* file) {
		return new T(gameType, file);
	}
	ITouhouUserParser* TouhouUserParserFactory::getInstance(int gameType, IRpFile* file) {
		switch (gameType) {
		case TouhouReplay::TH_095:
			return construct<Touhou095UserParser>(gameType, file);
		case TouhouReplay::TH_125:
			return construct<Touhou125UserParser>(gameType, file);
		case TouhouReplay::TH_143:
			return construct<Touhou143UserParser>(gameType, file);
		case TouhouReplay::TH_10:
		case TouhouReplay::TH_11:
		case TouhouReplay::TH_12:
		case TouhouReplay::TH_13:
		case TouhouReplay::TH_14:
		case TouhouReplay::TH_15:
			return construct<Touhou10UserParser>(gameType, file);
		default:
			return nullptr; // not supported
		}
	}
}
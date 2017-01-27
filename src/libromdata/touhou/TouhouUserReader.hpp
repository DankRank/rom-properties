/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* TouhouUserReader.hpp: Touhou Replay USER section reader.                *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_TOUHOUUSERREADER_HPP__
#define __ROMPROPERTIES_LIBROMDATA_TOUHOUUSERREADER_HPP__

#include "file/IRpFile.hpp"

#include <cstdint>

namespace LibRomData {
	class TouhouUserReader {
	private:
		bool is_valid;
	public:
		char* text;
		uint32_t text_sz;
		char* comment;
		uint32_t  comment_sz;
	private:
		TouhouUserReader(const TouhouUserReader& other);
		TouhouUserReader& operator=(const TouhouUserReader& other);
	public:
		TouhouUserReader(int gameType, IRpFile* file);
		~TouhouUserReader();
		bool isValid();
	};
}

#endif /* __ROMPROPERTIES_LIBROMDATA_TOUHOUUSERREADER_HPP__ */
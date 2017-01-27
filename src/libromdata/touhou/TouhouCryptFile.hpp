/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* TouhouCryptFile.hpp: Touhou Replay file decryption.                     *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_TOUHOUCRYPTFILE_HPP__
#define __ROMPROPERTIES_LIBROMDATA_TOUHOUCRYPTFILE_HPP__

#include "file/IRpFile.hpp"

namespace LibRomData {
	class TouhouCryptFile : public IRpFile
	{
	private:
		IRpFile* file;
		uint8_t key;
		int64_t offset; // file offset where the encrypted area starts
	public:
		// TODO: copy-paste doxygen stuff here -Egor
		TouhouCryptFile(IRpFile *file, uint8_t key, int64_t offset);
		virtual bool isOpen(void) const final;
		virtual IRpFile *dup(void) final;
		virtual void close(void) final;
		virtual size_t read(void *ptr, size_t size) final;
		virtual size_t write(const void *ptr, size_t size) final;
		virtual int seek(int64_t pos) final;
		virtual int64_t tell(void) final;
		virtual int truncate(int64_t size = 0) final;
		virtual int64_t fileSize(void) final;
		virtual rp_string filename(void) const final;

	};
}

#endif /* __ROMPROPERTIES_LIBROMDATA_TOUHOUCRYPTFILE_HPP__ */
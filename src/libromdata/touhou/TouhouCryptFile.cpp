/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* TouhouCryptFile.cpp: Touhou Replay file decryption.                     *
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

#include "TouhouCryptFile.hpp"

#include <cstdint>

#include <algorithm>

namespace LibRomData {
	TouhouCryptFile::TouhouCryptFile(IRpFile *file, uint8_t key, int64_t offset) {
		this->file = file;
		this->key = key;
		this->offset = offset;
	}
	bool TouhouCryptFile::isOpen(void) const {
		return file && file->isOpen();
	}
	IRpFile *TouhouCryptFile::dup(void) {
		return nullptr; // TODO: maybe do some dup'ing? -Egor
	}
	void TouhouCryptFile::close(void) {
		file = nullptr;
	}
	size_t TouhouCryptFile::read(void *ptr, size_t size) {
		if (!isOpen()) return 0;
		int64_t cpos = tell();
		size_t count = 0, count2 = 0;

		// First, read an unencrypted area as-is, and advance ptr, size and cpos
		if (cpos < offset) {
			count = file->read(ptr, (size_t)(std::min<uint64_t>(offset, cpos + size) - cpos));
			*(char**)&ptr += count;
			cpos += count;
			size -= count;
		}

		// Now, if there's still data to read, do the encrypted area.
		if (size != 0) {
			count2 = file->read(ptr, size);

			uint8_t k = (uint8_t)(key + 7 * (cpos - offset)); // calculate key for our file position
			for (size_t i = 0; i<count2; i++) {
				((uint8_t*)ptr)[i] -= k;
				k += 7;
			}
		}
		return count + count2;
	}
	size_t TouhouCryptFile::write(const void *ptr, size_t size) {
		return 0; // read-only
	}
	int TouhouCryptFile::seek(int64_t pos) {
		if (!isOpen()) return -1;
		return file->seek(pos);
	}
	int64_t TouhouCryptFile::tell(void) {
		if (!isOpen()) return -1;
		return file->tell();
	}
	int TouhouCryptFile::truncate(int64_t size) {
		return -1; // read-only
	}
	int64_t TouhouCryptFile::fileSize(void) {
		if (!isOpen()) return -1;
		return file->fileSize();
	}
	rp_string TouhouCryptFile::filename(void) const {
		if (!isOpen()) rp_string(_RP(""));
		return file->filename();
	}
}
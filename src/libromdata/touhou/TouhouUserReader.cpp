/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* TouhouUserReader.cpp: Touhou Replay USER section reader.                *
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

#include "TouhouUserReader.hpp"
#include "file/IRpFile.hpp"
#include "th_structs.h"
#include "TouhouReplay.hpp"

#include <cassert>

namespace LibRomData {
	TouhouUserReader::TouhouUserReader(int gameType, IRpFile *file)
		:is_valid(false), text(nullptr), text_sz(0), comment(nullptr), comment_sz(0) {

		assert(file);
		assert(file->isOpen());
		if (file && file->isOpen()) {
			// Read offset of the USER header
			file->seek(offsetof(THRP_GenericHeader, useroffset));
			uint32_t offset;
			if (sizeof(offset) != file->read(&offset, sizeof(offset))) {
				assert(0);
			}

			file->seek(offset);
			// Read all USER sections
			while ((uint64_t)file->tell() <= file->fileSize() - sizeof(THRP_USERHeader)) {
				// TODO: don't allocate/read stuff until last section is found
				// this doesn't really matter, since there's never going to be 2 sections with same type
				// Why do we support duplicate sections? Because replayview.exe does

				// Read USER header
				THRP_USERHeader uhead;
				if (sizeof(uhead) != file->read(&uhead, sizeof(uhead))) {
					break;
				}

				// Check magic
				static const char user_magic[] = { 'U', 'S', 'E', 'R' };
				if (!memcmp(uhead.magic, user_magic, sizeof(user_magic))) {
					// Make sure the size is valid
					assert(uhead.size >= sizeof(uhead));
					if (uhead.size >= sizeof(uhead)) {
						// Read the text data
						uint32_t size = uhead.size - sizeof(uhead);
						char *ntext = new char[size + 1]; // allocating +1 in case there's no terminating null
						if (size != file->read(ntext, size)) {
							// read failed, deallocate the text
							delete[] ntext;
							assert(0);
							continue;
						}

						// Read successful
						if (ntext[size - 1] != 0) {
							assert(!"USER section should end with null");
							ntext[size] = 0;
							size++;
						}

						switch (uhead.type) {
						case 0:
							delete[] text; // delete old text
							text = ntext;
							text_sz = size;
							break;
						case 1:
							delete[] comment; // delete old text
							comment = ntext;
							comment_sz = size;
							break;
						default:
							assert(!"Unknown USER section type");
							delete[] ntext;
							break;
						}

					}
				}
				// Try to skip the unknown section
				else if (-1 == file->seek(file->tell() + uhead.size)) {
					break;
				}
			}

			// If comment is default one, delete it.
			// This way we can provide a translatable string in the Ui
			if (comment) {
				// SJIS for "コメントを書けます"
				static const uint8_t youCanAddComments[] = {
					0x83, 0x52, 0x83, 0x81, 0x83, 0x93, 0x83, 0x67,
					0x82, 0xF0, 0x8F, 0x91, 0x82, 0xAF, 0x82, 0xDC,
					0x82, 0xB7,
				};
				// english version of the above string as seen in th11 patch
				static const uint8_t englishComments[] = {
					'W', 'i', 'l', 'l', ' ',
					'w', 'r', 'i', 't', 'e', ' ',
					'c', 'o', 'm', 'm', 'e', 'n', 't', 's',
				};

				if (// Check for SJIS default comment
					gameType >= TouhouReplay::TH_095 // th08/09 don't have a default comment at all
					&& comment_sz >= sizeof(youCanAddComments)
					&& !memcmp(comment, youCanAddComments, sizeof(youCanAddComments))
					|| // Check for english comment
					gameType == TouhouReplay::TH_11
					&& comment_sz >= sizeof(englishComments)
					&& !memcmp(comment, englishComments, sizeof(englishComments))
					) {
					// replace comment with null
					delete[] comment;
					comment = nullptr;
					comment_sz = 0;
				}
			}
		}
	}
	TouhouUserReader::~TouhouUserReader() {
		delete[] text;
		delete[] comment;
	}
	bool TouhouUserReader::isValid() {
		return is_valid;
	}
}
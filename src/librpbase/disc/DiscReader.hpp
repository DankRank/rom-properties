/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * DiscReader.hpp: Basic disc reader interface.                            *
 * This class is a "null" interface that simply passes calls down to       *
 * libc's stdio functions.                                                 *
 *                                                                         *
 * Copyright (c) 2016-2019 by David Korth.                                 *
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
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPBASE_DISCREADER_HPP__
#define __ROMPROPERTIES_LIBRPBASE_DISCREADER_HPP__

#include "IDiscReader.hpp"

namespace LibRpBase {

class IRpFile;
class DiscReader : public IDiscReader
{
	public:
		/**
		 * Construct a DiscReader with the specified file.
		 * The file is ref()'d, so the original file can be
		 * unref()'d by the caller afterwards.
		 * @param file File to read from.
		 */
		explicit DiscReader(IRpFile *file);

		/**
		 * Construct a DiscReader with the specified file.
		 * The file is ref()'d, so the original file can be
		 * unref()'d by the caller afterwards.
		 * @param file File to read from.
		 * @param offset Starting offset.
		 * @param length Disc length. (-1 for "until end of file")
		 */
		DiscReader(IRpFile *file, int64_t offset, int64_t length);

	private:
		typedef IDiscReader super;
		RP_DISABLE_COPY(DiscReader)

	public:
		/** Disc image detection functions. **/

		/**
		 * Is a disc image supported by this class?
		 * @param pHeader Disc image header.
		 * @param szHeader Size of header.
		 * @return Class-specific disc format ID (>= 0) if supported; -1 if not.
		 */
		static int isDiscSupported_static(const uint8_t *pHeader, size_t szHeader);

		/**
		 * Is a disc image supported by this object?
		 * @param pHeader Disc image header.
		 * @param szHeader Size of header.
		 * @return Class-specific disc format ID (>= 0) if supported; -1 if not.
		 */
		int isDiscSupported(const uint8_t *pHeader, size_t szHeader) const override;

	public:
		/**
		 * Read data from the disc image.
		 * @param ptr Output data buffer.
		 * @param size Amount of data to read, in bytes.
		 * @return Number of bytes read.
		 */
		size_t read(void *ptr, size_t size) override;

		/**
		 * Set the disc image position.
		 * @param pos Disc image position.
		 * @return 0 on success; -1 on error.
		 */
		int seek(int64_t pos) override;

		/**
		 * Get the disc image position.
		 * @return Disc image position on success; -1 on error.
		 */
		int64_t tell(void) final;

		/**
		 * Get the disc image size.
		 * @return Disc image size, or -1 on error.
		 */
		int64_t size(void) override;

	protected:
		// Offset/length. Useful for e.g. GameCube TGC.
		int64_t m_offset;
		int64_t m_length;
};

}

#endif /* __ROMPROPERTIES_LIBRPBASE_DISCREADER_HPP__ */

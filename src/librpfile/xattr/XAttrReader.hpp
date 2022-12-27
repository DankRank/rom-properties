/***************************************************************************
 * ROM Properties Page shell extension. (librpfile)                        *
 * XAttrReader.hpp: Extended Attribute reader                              *
 *                                                                         *
 * Copyright (c) 2016-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPFILE_XATTRREADER_HPP__
#define __ROMPROPERTIES_LIBRPFILE_XATTRREADER_HPP__

// Common macros
#include "common.h"
#include "dll-macros.h"	// for RP_LIBROMDATA_PUBLIC

// C++ includes
#include <map>
#include <string>
#include <utility>

namespace LibRpFile {

class XAttrReaderPrivate;
class RP_LIBROMDATA_PUBLIC XAttrReader
{
	public:
		XAttrReader(const char *filename);
#ifdef _WIN32
		XAttrReader(const wchar_t *filename);
#endif /* _WIN32 */

	private:
		RP_DISABLE_COPY(XAttrReader)
	protected:
		friend class XAttrReaderPrivate;
		XAttrReaderPrivate *const d_ptr;

	public:
		/**
		 * Get the last error number.
		 * @return Last error number (POSIX error code)
		 */
		int lastError(void) const;

	public:
		/**
		 * Does this file have Linux attributes?
		 * @return True if it does; false if not.
		 */
		bool hasLinuxAttributes(void) const;

		/**
		 * Get this file's Linux attributes.
		 * @return Linux attributes
		 */
		int linuxAttributes(void) const;

		/**
		 * Does this file have MS-DOS attributes?
		 * @return True if it does; false if not.
		 */
		bool hasDosAttributes(void) const;

		/**
		 * Get this file's MS-DOS attributes.
		 * @return MS-DOS attributes.
		 */
		unsigned int dosAttributes(void) const;

		/**
		 * Does this file have generic extended attributes?
		 * (POSIX xattr on Linux; ADS on Windows)
		 * @return True if it does; false if not.
		 */
		bool hasGenericXAttrs(void) const;

		/**
		 * Extended attribute map (UTF-8)
		 * - Key: Name
		 * - Value: Value
		 */
		typedef std::map<std::string, std::string> XAttrList;

		/**
		 * Get the list of extended attributes.
		 * @return Extended attributes
		 */
		const XAttrList &genericXAttrs(void) const;
};

}

#endif /* __ROMPROPERTIES_LIBRPFILE_IRPFILE_HPP__ */

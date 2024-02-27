/***************************************************************************
 * ROM Properties Page shell extension. (GTK+ common)                      *
 * is-supported.hpp: Check if a URI is supported.                          *
 *                                                                         *
 * Copyright (c) 2017-2024 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "is-supported.hpp"

#include <glib.h>

// librpfile, librpbase, libromdata
#include "libromdata/RomDataFactory.hpp"
using namespace LibRpBase;
using namespace LibRpFile;
using LibRomData::RomDataFactory;

/**
 * Attempt to open a RomData object from the specified GVfs URI.
 * If it is, the RomData object will be opened.
 * @param uri URI from e.g. nautilus_file_info_get_uri() [UTF-8]
 * @return RomData object if supported; nullptr if not.
 */
RomDataPtr rp_gtk_open_uri(const char *uri)
{
	g_return_val_if_fail(uri != nullptr && uri[0] != '\0', nullptr);

	// TODO: Check file extensions and/or MIME types?

	// Check if the URI maps to a local file.
	IRpFilePtr file;
	gchar *const filename = g_filename_from_uri(uri, nullptr, nullptr);
	if (filename) {
		// Local file. Use RpFile.
		file = std::make_shared<RpFile>(filename, RpFile::FM_OPEN_READ_GZ);
		g_free(filename);
	} else {
		// Not a local file. Use RpFileGio.
		file = std::make_shared<RpFileGio>(uri);
	}

	if (!file->isOpen()) {
		// Unable to open the file...
		// TODO: Return an error code?
		return {};
	}

	// Create the RomData object.
	return RomDataFactory::create(file);
}

/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * AmigaModule.cpp: Tracker Module reader.                               *
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

#include "AmigaModule.hpp"
#include "RomData_p.hpp"

#include "mod_structs.h"

#include "librpbase/common.h"
#include "librpbase/byteswap.h"
#include "librpbase/TextFuncs.hpp"
#include "librpbase/file/IRpFile.hpp"
using namespace LibRpBase;

// C includes.
#include <stdlib.h>

// C includes. (C++ namespace)
#include <cassert>
#include <cctype>
#include <cstring>

// C++ includes.
#include <memory>
#include <vector>
using std::unique_ptr;
using std::vector;

namespace LibRomData {

class AmigaModulePrivate : public RomDataPrivate
{
	public:
		AmigaModulePrivate(AmigaModule *q, IRpFile *file);

	private:
		typedef RomDataPrivate super;
		RP_DISABLE_COPY(AmigaModulePrivate)

	public:
		int channels;
		
		// Headers.
		// NOTE: Must be byteswapped on access.
		MOD_Header modHeader;		// Amiga Module header.
};

/** AmigaModulePrivate **/

AmigaModulePrivate::AmigaModulePrivate(AmigaModule *q, IRpFile *file)
	: super(q, file),
	channels(-1)
{
	// Clear the various structs.
	memset(&modHeader, 0, sizeof(modHeader));
}

/** AmigaModule **/

/**
 * Read an Amiga Module.
 *
 * A ROM file must be opened by the caller. The file handle
 * will be dup()'d and must be kept open in order to load
 * data from the ROM.
 *
 * To close the file, either delete this object or call close().
 *
 * NOTE: Check isValid() to determine if this is a valid ROM.
 *
 * @param file Open ROM file.
 */
AmigaModule::AmigaModule(IRpFile *file)
	: super(new AmigaModulePrivate(this, file))
{
	// TODO: Only validate that this is an MOD here.
	// Load fields elsewhere.
	RP_D(AmigaModule);
	if (!d->file) {
		// Could not dup() the file handle.
		return;
	}

	// Seek to the beginning of the file.
	d->file->rewind();

	// Read the ROM header. [1084 bytes]
	uint8_t header[sizeof(MOD_Header)];
	size_t size = d->file->read(header, sizeof(header));
	if (size != sizeof(header))
		return;

	// Check if this ROM is supported.
	DetectInfo info;
	info.header.addr = 0;
	info.header.size = sizeof(header);
	info.header.pData = header;
	info.ext = nullptr;	// Not needed for MOD.
	info.szFile = 0;	// Not needed for MOD.
	d->channels = isRomSupported_static(&info);
	d->fileType = FTYPE_TRACKER_MODULE;
	d->isValid = (d->channels >= 0);
	if (d->isValid) {
		memcpy(&d->modHeader, header, sizeof(d->modHeader));
	}
}

/** ROM detection functions. **/

/**
 * Is a ROM image supported by this class?
 * @param info DetectInfo containing ROM detection information.
 * @return Class-specific system ID (>= 0) if supported; -1 if not.
 */
int AmigaModule::isRomSupported_static(const DetectInfo *info)
{
	assert(info != nullptr);
	assert(info->header.pData != nullptr);
	assert(info->header.addr == 0);
	if (!info || !info->header.pData ||
	    info->header.addr != 0 ||
	    info->header.size < sizeof(MOD_Header))
	{
		// Either no detection information was specified,
		// or the header is too small.
		return -1;
	}

	// ROM header.
	const uint8_t *const pHeader = info->header.pData;

	// Magic strings.

	static const struct {
		const char magic[4];
		int channels;
	} mod_magic[] = {
		// ProTracker modules
		{{'M','.','K','.'}, 4},
		{{'M','!','K','!'}, 4},
		
		// StartTracker modules
		{{'F','L','T','4'}, 4},
		{{'F','L','T','8'}, 8},
		
		// FastTracker modules
		{{'2','C','H','N'}, 2},
		{{'4','C','H','N'}, 4},
		{{'6','C','H','N'}, 6},
		{{'8','C','H','N'}, 8},
		{{'1','0','C','H'}, 10},
		{{'1','2','C','H'}, 12},
		{{'1','4','C','H'}, 14},
		{{'1','6','C','H'}, 16},
		{{'1','8','C','H'}, 18},
		{{'2','0','C','H'}, 20},
		{{'2','2','C','H'}, 22},
		{{'2','4','C','H'}, 24},
		{{'2','6','C','H'}, 26},
		{{'2','8','C','H'}, 28},
		{{'3','0','C','H'}, 30},
		{{'3','2','C','H'}, 32},
		
		// TakeTracker Modules
		{{'T','D','Z','1'}, 1},
		{{'T','D','Z','2'}, 2},
		{{'T','D','Z','3'}, 3},
		{{'5','C','H','N'}, 5},
		{{'7','C','H','N'}, 7},
		{{'9','C','H','N'}, 9},
		{{'1','1','C','H'}, 11},
		{{'1','3','C','H'}, 13},
		{{'1','5','C','H'}, 15},
		
		// Falcon Module
		{{'C','D','8','1'}, 8},
		
		// 8 channel module
		{{'O','C','T','A'}, 8},
		
	};

	if (info->header.size >= sizeof(MOD_Header)) {
		const MOD_Header* mod_header = reinterpret_cast<const MOD_Header*>(pHeader);
		for(int i=0;i<ARRAY_SIZE(mod_magic);i++) {
			if(!memcmp(mod_magic[i].magic, mod_header->magic, 4)) {
				return mod_magic[i].channels;
			}
		}
	}

	// Not supported.
	return -1;
}

/**
 * Is a ROM image supported by this object?
 * @param info DetectInfo containing ROM detection information.
 * @return Class-specific system ID (>= 0) if supported; -1 if not.
 */
int AmigaModule::isRomSupported(const DetectInfo *info) const
{
	return isRomSupported_static(info);
}

/**
 * Get the name of the system the loaded ROM is designed for.
 * @param type System name type. (See the SystemName enum.)
 * @return System name, or nullptr if type is invalid.
 */
const rp_char *AmigaModule::systemName(uint32_t type) const
{
	RP_D(const AmigaModule);
	if (!d->isValid)
		return nullptr;

	static_assert(SYSNAME_TYPE_MASK == 3,
		"AmigaModule::systemName() array index optimization needs to be updated.");

	static const rp_char *const sysNames[4] = {
		_RP("Amiga Module"), _RP("Module"), _RP("MOD"), nullptr,
	};
	return sysNames[type & SYSNAME_TYPE_MASK];
}

/**
 * Get a list of all supported file extensions.
 * This is to be used for file type registration;
 * subclasses don't explicitly check the extension.
 *
 * NOTE: The extensions include the leading dot,
 * e.g. ".bin" instead of "bin".
 *
 * NOTE 2: The array and the strings in the array should
 * *not* be freed by the caller.
 *
 * @return NULL-terminated array of all supported file extensions, or nullptr on error.
 */
const rp_char *const *AmigaModule::supportedFileExtensions_static(void)
{
	static const rp_char *const exts[] = {
		_RP(".mod"), _RP(".ft"),
		nullptr
	};
	return exts;
}

/**
 * Get a list of all supported file extensions.
 * This is to be used for file type registration;
 * subclasses don't explicitly check the extension.
 *
 * NOTE: The extensions include the leading dot,
 * e.g. ".bin" instead of "bin".
 *
 * NOTE 2: The array and the strings in the array should
 * *not* be freed by the caller.
 *
 * @return NULL-terminated array of all supported file extensions, or nullptr on error.
 */
const rp_char *const *AmigaModule::supportedFileExtensions(void) const
{
	return supportedFileExtensions_static();
}

/**
 * Load field data.
 * Called by RomData::fields() if the field data hasn't been loaded yet.
 * @return Number of fields read on success; negative POSIX error code on error.
 */
int AmigaModule::loadFieldData(void)
{
	RP_D(AmigaModule);
	if (d->fields->isDataLoaded()) {
		// Field data *has* been loaded...
		return 0;
	} else if (!d->file || !d->file->isOpen()) {
		// File isn't open.
		// NOTE: We already loaded the header,
		// so *maybe* this is okay?
		return -EBADF;
	} else if (!d->isValid || d->channels < 0) {
		// Unknown ROM image type.
		return -EIO;
	}

	const MOD_Header *modHeader = &d->modHeader;
	d->fields->reserve(3); // Maximum of 3 fields.

	d->fields->addField_string(_RP("Title"),
		latin1_to_rp_string(modHeader->title, sizeof(modHeader->title)));
	
	d->fields->addField_string_numeric(_RP("Channels"), d->channels);

	vector<vector<rp_string>>* sample_list = new vector<vector<rp_string>>();
	for(int i=0; i<ARRAY_SIZE(modHeader->samples); i++) {
		sample_list->push_back(vector<rp_string>{
			latin1_to_rp_string(modHeader->samples[i].name, sizeof(modHeader->samples[i].name))
			});
	}
	d->fields->addField_listData(_RP("Samples"), new vector<rp_string>{_RP("Name")}, sample_list);

	// Finished reading the field data.
	return (int)d->fields->count();
}

}

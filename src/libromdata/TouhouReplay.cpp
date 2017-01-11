/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* TouhouReplay.cpp: Touhou Replay reader.                                 *
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

#include "TouhouReplay.hpp"
#include "RomData_p.hpp"

#include "th_structs.h"
#include "common.h"
#include "byteswap.h"
#include "TextFuncs.hpp"
#include "file/IRpFile.hpp"

// C includes.
#include <stdlib.h>

// C includes. (C++ namespace)
#include <cassert>
#include <cctype>
#include <cstring>

// C++ includes.
#include <vector>
#include <algorithm>
using std::vector;

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

	/** TouhouCryptFile **/

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
			count = file->read(ptr, std::min(offset, cpos + size) - cpos);
			*(char**)&ptr += count;
			cpos += count;
			size -= count;
		}

		// Now, if there's still data to read, do the encrypted area.
		if (size != 0) {
			count2 = file->read(ptr, size);
			
			char k = key + 7 * (cpos - offset); // calculate key for our file position
			for (int i = 0; i<count2; i++) {
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
	int TouhouCryptFile::truncate(int64_t size = 0) {
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
	
	class TouhouReplayPrivate : public RomDataPrivate
	{
	public:
		TouhouReplayPrivate(TouhouReplay *q, IRpFile *file);

	private:
		typedef RomDataPrivate super;
		TouhouReplayPrivate(const TouhouReplayPrivate &other);
		TouhouReplayPrivate &operator=(const TouhouReplayPrivate &other);

	public:
		/** RomFields **/

		// ROM fields.
		static const struct RomFields::Desc th_fields[];

		/** Internal ROM data. **/


	public:
		int romType;		// ROM type.

	public:
		// ROM header.
		// TODO: rename this member -Egor
		T6RP_Header romHeader;		// ROM header.
		T6RP_Stage stageHeader[7]; // TODO: make this better -Egor
	};

	/** TouhouReplayPrivate **/

	// ROM fields.
	const struct RomFields::Desc TouhouReplayPrivate::th_fields[] = {
		// TODO: fix me -Egor
	};

	TouhouReplayPrivate::TouhouReplayPrivate(TouhouReplay *q, IRpFile *file)
		: super(q, file, th_fields, ARRAY_SIZE(th_fields))
		, romType(0/* TODO: rom types -Egor */)
	{
		// Clear the various structs.
		memset(&romHeader, 0, sizeof(romHeader));
	}

	/** Internal ROM data. **/

	/** MegaDrive **/

	/**
	* Read a Touhou Replay.
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
	TouhouReplay::TouhouReplay(IRpFile *file)
		: super(new TouhouReplayPrivate(this, file))
	{
		// TODO: completely rewrite this -Egor
		// TODO: Only validate that this is an MD ROM here.
		// Load fields elsewhere.
		RP_D(TouhouReplay);
		if (!d->file) {
			// Could not dup() the file handle.
			return;
		}

		// Seek to the beginning of the file.
		d->file->rewind();

		// Read the ROM header. [0x400 bytes]
		uint8_t header[0x400];
		size_t size = d->file->read(header, sizeof(header));
		if (size != sizeof(header))
			return;

		// Check if this ROM is supported.
		DetectInfo info;
		info.header.addr = 0;
		info.header.size = sizeof(header);
		info.header.pData = header;
		info.ext = nullptr;	// Not needed for MD.
		info.szFile = 0;	// Not needed for MD.
		d->romType = isRomSupported_static(&info);

		if (d->romType >= 0) {
			// Save the header for later.
			// TODO (remove before committing): Does gcc/msvc optimize this into a jump table?
			switch (d->romType & TouhouReplayPrivate::ROM_FORMAT_MASK) {
			case TouhouReplayPrivate::ROM_FORMAT_CART_BIN:
				d->fileType = FTYPE_ROM_IMAGE;

				// MD header is at 0x100.
				// Vector table is at 0.
				memcpy(&d->vectors, header, sizeof(d->vectors));
				memcpy(&d->romHeader, &header[0x100], sizeof(d->romHeader));
				break;

			case MegaDrivePrivate::ROM_FORMAT_CART_SMD: {
				d->fileType = FTYPE_ROM_IMAGE;

				// Save the SMD header.
				memcpy(&d->smdHeader, header, sizeof(d->smdHeader));

				// First bank needs to be deinterleaved.
				uint8_t smd_data[MegaDrivePrivate::SMD_BLOCK_SIZE];
				uint8_t bin_data[MegaDrivePrivate::SMD_BLOCK_SIZE];
				d->file->seek(512);
				size = d->file->read(smd_data, sizeof(smd_data));
				if (size != sizeof(smd_data)) {
					// Short read. ROM is invalid.
					d->romType = MegaDrivePrivate::ROM_UNKNOWN;
					break;
				}

				// Decode the SMD block.
				d->decodeSMDBlock(bin_data, smd_data);

				// MD header is at 0x100.
				// Vector table is at 0.
				memcpy(&d->vectors, bin_data, sizeof(d->vectors));
				memcpy(&d->romHeader, &bin_data[0x100], sizeof(d->romHeader));
				break;
			}

			case MegaDrivePrivate::ROM_FORMAT_DISC_2048:
				d->fileType = FTYPE_DISC_IMAGE;

				// MCD-specific header is at 0. [TODO]
				// MD-style header is at 0x100.
				// No vector table is present on the disc.
				memcpy(&d->romHeader, &header[0x100], sizeof(d->romHeader));
				break;

			case MegaDrivePrivate::ROM_FORMAT_DISC_2352:
				d->fileType = FTYPE_DISC_IMAGE;

				// MCD-specific header is at 0x10. [TODO]
				// MD-style header is at 0x110.
				// No vector table is present on the disc.
				memcpy(&d->romHeader, &header[0x110], sizeof(d->romHeader));
				break;

			case MegaDrivePrivate::ROM_FORMAT_UNKNOWN:
			default:
				d->fileType = FTYPE_UNKNOWN;
				d->romType = MegaDrivePrivate::ROM_UNKNOWN;
				break;
			}
		}

		d->isValid = (d->romType >= 0);
		if (d->isValid) {
			// Parse the MD region code.
			d->md_region = MegaDriveRegions::parseRegionCodes(
				d->romHeader.region_codes, sizeof(d->romHeader.region_codes));
		}
	}

	/** ROM detection functions. **/

	/**
	* Is a ROM image supported by this class?
	* @param info DetectInfo containing ROM detection information.
	* @return Class-specific system ID (>= 0) if supported; -1 if not.
	*/
	int TouhouReplay::isRomSupported_static(const DetectInfo *info)
	{
		assert(info != nullptr);
		assert(info->header.pData != nullptr);
		assert(info->header.addr == 0);
		if (!info || !info->header.pData ||
			info->header.addr != 0 ||
			info->header.size < 0x200)
		{
			// Either no detection information was specified,
			// or the header is too small.
			return -1;
		}

		// ROM header.
		const uint8_t *const pHeader = info->header.pData;

		// Magic strings.
		static const char sega_magic[4] = { 'S','E','G','A' };
		static const char segacd_magic[16] =
		{ 'S','E','G','A','D','I','S','C','S','Y','S','T','E','M',' ',' ' };;

		static const struct {
			const char system_name[16];
			uint32_t system_id;
		} cart_magic[] = {
			{ { 'S','E','G','A',' ','P','I','C','O',' ',' ',' ',' ',' ',' ',' ' }, MegaDrivePrivate::ROM_SYSTEM_PICO },
			{ { 'S','E','G','A',' ','3','2','X',' ',' ',' ',' ',' ',' ',' ',' ' }, MegaDrivePrivate::ROM_SYSTEM_32X },
			{ { 'S','E','G','A',' ','M','E','G','A',' ','D','R','I','V','E',' ' }, MegaDrivePrivate::ROM_SYSTEM_MD },
			{ { 'S','E','G','A',' ','G','E','N','E','S','I','S',' ',' ',' ',' ' }, MegaDrivePrivate::ROM_SYSTEM_MD },
		};

		if (info->header.size >= 0x200) {
			// Check for Sega CD.
			// TODO: Gens/GS II lists "ISO/2048", "ISO/2352",
			// "BIN/2048", and "BIN/2352". I don't think that's
			// right; there should only be 2048 and 2352.
			// TODO: Detect Sega CD 32X.
			if (!memcmp(&pHeader[0x0010], segacd_magic, sizeof(segacd_magic))) {
				// Found a Sega CD disc image. (2352-byte sectors)
				return MegaDrivePrivate::ROM_SYSTEM_MCD |
					MegaDrivePrivate::ROM_FORMAT_DISC_2352;
			}
			else if (!memcmp(&pHeader[0x0000], segacd_magic, sizeof(segacd_magic))) {
				// Found a Sega CD disc image. (2048-byte sectors)
				return MegaDrivePrivate::ROM_SYSTEM_MCD |
					MegaDrivePrivate::ROM_FORMAT_DISC_2048;
			}

			// Check for SMD format. (Mega Drive only)
			if (info->header.size >= 0x300) {
				// Check if "SEGA" is in the header in the correct place
				// for a plain binary ROM.
				if (memcmp(&pHeader[0x100], sega_magic, sizeof(sega_magic)) != 0 &&
					memcmp(&pHeader[0x101], sega_magic, sizeof(sega_magic)) != 0)
				{
					// "SEGA" is not in the header. This might be SMD.
					const SMD_Header *smdHeader = reinterpret_cast<const SMD_Header*>(pHeader);
					if (smdHeader->id[0] == 0xAA && smdHeader->id[1] == 0xBB &&
						smdHeader->smd.file_data_type == SMD_FDT_68K_PROGRAM &&
						smdHeader->file_type == SMD_FT_SMD_GAME_FILE)
					{
						// This is an SMD-format ROM.
						// TODO: Show extended information from the SMD header,
						// including "split" and other stuff?
						return MegaDrivePrivate::ROM_SYSTEM_MD |
							MegaDrivePrivate::ROM_FORMAT_CART_SMD;
					}
				}
			}

			// Check for other MD-based cartridge formats.
			for (int i = 0; i < ARRAY_SIZE(cart_magic); i++) {
				if (!memcmp(&pHeader[0x100], cart_magic[i].system_name, 16) ||
					!memcmp(&pHeader[0x101], cart_magic[i].system_name, 15))
				{
					// Found a matching system name.
					return MegaDrivePrivate::ROM_FORMAT_CART_BIN | cart_magic[i].system_id;
				}
			}
		}

		// Not supported.
		return MegaDrivePrivate::ROM_UNKNOWN;
	}

	/**
	* Is a ROM image supported by this object?
	* @param info DetectInfo containing ROM detection information.
	* @return Class-specific system ID (>= 0) if supported; -1 if not.
	*/
	int TouhouReplay::isRomSupported(const DetectInfo *info) const
	{
		return isRomSupported_static(info);
	}

	/**
	* Get the name of the system the loaded ROM is designed for.
	* @param type System name type. (See the SystemName enum.)
	* @return System name, or nullptr if type is invalid.
	*/
	const rp_char *TouhouReplay::systemName(uint32_t type) const
	{
		RP_D(const TouhouReplay);
		if (!d->isValid || !isSystemNameTypeValid(type))
			return nullptr;

		// FIXME: Lots of system names and regions to check.
		// Also, games can be region-free, so we need to check
		// against the host system's locale.
		// For now, just use the generic "Mega Drive".

		static_assert(SYSNAME_TYPE_MASK == 3,
			"MegaDrive::systemName() array index optimization needs to be updated.");

		uint32_t romSys = (d->romType & MegaDrivePrivate::ROM_SYSTEM_MASK);
		if (romSys > MegaDrivePrivate::ROM_SYSTEM_MAX) {
			// Invalid system type. Default to MD.
			romSys = MegaDrivePrivate::ROM_SYSTEM_MD;
		}

		// sysNames[] bitfield:
		// - Bits 0-1: Type. (short, long, abbreviation)
		// - Bits 2-4: System type.
		uint32_t idx = (romSys << 2) | (type & SYSNAME_TYPE_MASK);
		if (idx >= 20) {
			// Invalid index...
			idx &= SYSNAME_TYPE_MASK;
		}

		static_assert(SYSNAME_REGION_MASK == (1 << 2),
			"MegaDrive::systemName() region type optimization needs to be updated.");
		if ((type & SYSNAME_REGION_MASK) == SYSNAME_REGION_GENERIC) {
			// Generic system name.
			static const rp_char *const sysNames[20] = {
				_RP("Sega Mega Drive"), _RP("Mega Drive"), _RP("MD"), nullptr,
				_RP("Sega Mega CD"), _RP("Mega CD"), _RP("MCD"), nullptr,
				_RP("Sega 32X"), _RP("Sega 32X"), _RP("32X"), nullptr,
				_RP("Sega Mega CD 32X"), _RP("Mega CD 32X"), _RP("MCD32X"), nullptr,
				_RP("Sega Pico"), _RP("Pico"), _RP("Pico"), nullptr
			};
			return sysNames[idx];
		}

		// Get the system branding region.
		const MegaDriveRegions::MD_BrandingRegion md_bregion =
			MegaDriveRegions::getBrandingRegion(d->md_region);
		switch (md_bregion) {
		case MegaDriveRegions::MD_BREGION_JAPAN:
		default: {
			static const rp_char *const sysNames_JP[20] = {
				_RP("Sega Mega Drive"), _RP("Mega Drive"), _RP("MD"), nullptr,
				_RP("Sega Mega CD"), _RP("Mega CD"), _RP("MCD"), nullptr,
				_RP("Sega Super 32X"), _RP("Super 32X"), _RP("32X"), nullptr,
				_RP("Sega Mega CD 32X"), _RP("Mega CD 32X"), _RP("MCD32X"), nullptr,
				_RP("Sega Kids Computer Pico"), _RP("Kids Computer Pico"), _RP("Pico"), nullptr
			};
			return sysNames_JP[idx];
		}

		case MegaDriveRegions::MD_BREGION_USA: {
			static const rp_char *const sysNames_US[20] = {
				// TODO: "MD" or "Gen"?
				_RP("Sega Genesis"), _RP("Genesis"), _RP("MD"), nullptr,
				_RP("Sega CD"), _RP("Sega CD"), _RP("MCD"), nullptr,
				_RP("Sega 32X"), _RP("Sega 32X"), _RP("32X"), nullptr,
				_RP("Sega CD 32X"), _RP("Sega CD 32X"), _RP("MCD32X"), nullptr,
				_RP("Sega Pico"), _RP("Pico"), _RP("Pico"), nullptr
			};
			return sysNames_US[idx];
		}

		case MegaDriveRegions::MD_BREGION_EUROPE: {
			static const rp_char *const sysNames_EU[20] = {
				_RP("Sega Mega Drive"), _RP("Mega Drive"), _RP("MD"), nullptr,
				_RP("Sega Mega CD"), _RP("Mega CD"), _RP("MCD"), nullptr,
				_RP("Sega Mega Drive 32X"), _RP("Mega Drive 32X"), _RP("32X"), nullptr,
				_RP("Sega Mega CD 32X"), _RP("Sega Mega CD 32X"), _RP("MCD32X"), nullptr,
				_RP("Sega Pico"), _RP("Pico"), _RP("Pico"), nullptr
			};
			return sysNames_EU[idx];
		}

		case MegaDriveRegions::MD_BREGION_SOUTH_KOREA: {
			static const rp_char *const sysNames_KR[20] = {
				// TODO: "MD" or something else?
				_RP("Samsung Super Aladdin Boy"), _RP("Super Aladdin Boy"), _RP("MD"), nullptr,
				_RP("Samsung CD Aladdin Boy"), _RP("CD Aladdin Boy"), _RP("MCD"), nullptr,
				_RP("Samsung Super 32X"), _RP("Super 32X"), _RP("32X"), nullptr,
				_RP("Sega Mega CD 32X"), _RP("Sega Mega CD 32X"), _RP("MCD32X"), nullptr,
				_RP("Sega Pico"), _RP("Pico"), _RP("Pico"), nullptr
			};
			return sysNames_KR[idx];
		}

		case MegaDriveRegions::MD_BREGION_BRAZIL: {
			static const rp_char *const sysNames_BR[20] = {
				_RP("Sega Mega Drive"), _RP("Mega Drive"), _RP("MD"), nullptr,
				_RP("Sega CD"), _RP("Sega CD"), _RP("MCD"), nullptr,
				_RP("Sega Mega 32X"), _RP("Mega 32X"), _RP("32X"), nullptr,
				_RP("Sega CD 32X"), _RP("Sega CD 32X"), _RP("MCD32X"), nullptr,
				_RP("Sega Pico"), _RP("Pico"), _RP("Pico"), nullptr
			};
			return sysNames_BR[idx];
		}
		}

		// Should not get here...
		return nullptr;
	}

	/**
	* Get a list of all supported file extensions.
	* This is to be used for file type registration;
	* subclasses don't explicitly check the extension.
	*
	* NOTE: The extensions include the leading dot,
	* e.g. ".bin" instead of "bin".
	*
	* NOTE 2: The strings in the std::vector should *not*
	* be freed by the caller.
	*
	* @return List of all supported file extensions.
	*/
	vector<const rp_char*> TouhouReplay::supportedFileExtensions_static(void)
	{
		static const rp_char *const exts[] = {
			_RP(".rpy")
		};
		return vector<const rp_char*>(exts, exts + ARRAY_SIZE(exts));
	}

	/**
	* Get a list of all supported file extensions.
	* This is to be used for file type registration;
	* subclasses don't explicitly check the extension.
	*
	* NOTE: The extensions include the leading dot,
	* e.g. ".bin" instead of "bin".
	*
	* NOTE 2: The strings in the std::vector should *not*
	* be freed by the caller.
	*
	* @return List of all supported file extensions.
	*/
	vector<const rp_char*> TouhouReplay::supportedFileExtensions(void) const
	{
		return supportedFileExtensions_static();
	}

	/**
	* Load field data.
	* Called by RomData::fields() if the field data hasn't been loaded yet.
	* @return Number of fields read on success; negative POSIX error code on error.
	*/
	int TouhouReplay::loadFieldData(void)
	{
		RP_D(TouhouReplay);
		if (d->fields->isDataLoaded()) {
			// Field data *has* been loaded...
			return 0;
		}
		else if (!d->file || !d->file->isOpen()) {
			// File isn't open.
			// NOTE: We already loaded the header,
			// so *maybe* this is okay?
			return -EBADF;
		}
		else if (!d->isValid || d->romType < 0) {
			// Unknown ROM image type.
			return -EIO;
		}

		// MD ROM header, excluding the vector table.
		const MD_RomHeader *romHeader = &d->romHeader;

		// Read the strings from the header.
		d->fields->addData_string(cp1252_sjis_to_rp_string(romHeader->system, sizeof(romHeader->system)));
		d->fields->addData_string(cp1252_sjis_to_rp_string(romHeader->copyright, sizeof(romHeader->copyright)));

		// Determine the publisher.
		// Formats in the copyright line:
		// - "(C)SEGA"
		// - "(C)T-xx"
		// - "(C)T-xxx"
		// - "(C)Txxx"
		const rp_char *publisher = nullptr;
		unsigned int t_code = 0;
		if (!memcmp(romHeader->copyright, "(C)SEGA", 7)) {
			// Sega first-party game.
			publisher = _RP("Sega");
		}
		else if (!memcmp(romHeader->copyright, "(C)T", 4)) {
			// Third-party game.
			int start = 4;
			if (romHeader->copyright[4] == '-')
				start++;
			char *endptr;
			t_code = strtoul(&romHeader->copyright[start], &endptr, 10);
			if (t_code != 0 &&
				endptr > &romHeader->copyright[start] &&
				endptr < &romHeader->copyright[start + 3])
			{
				// Valid T-code. Look up the publisher.
				publisher = MegaDrivePublishers::lookup(t_code);
			}
		}

		if (publisher) {
			// Publisher identified.
			d->fields->addData_string(publisher);
		}
		else if (t_code > 0) {
			// Unknown publisher, but there is a valid T code.
			char buf[16];
			int len = snprintf(buf, sizeof(buf), "T-%u", t_code);
			if (len > (int)sizeof(buf))
				len = sizeof(buf);
			d->fields->addData_string(len > 0 ? latin1_to_rp_string(buf, len) : _RP(""));
		}
		else {
			// Unknown publisher.
			d->fields->addData_string(_RP("Unknown"));
		}

		// Titles, serial number, and checksum.
		d->fields->addData_string(cp1252_sjis_to_rp_string(romHeader->title_domestic, sizeof(romHeader->title_domestic)));
		d->fields->addData_string(cp1252_sjis_to_rp_string(romHeader->title_export, sizeof(romHeader->title_export)));
		d->fields->addData_string(cp1252_sjis_to_rp_string(romHeader->serial, sizeof(romHeader->serial)));
		if (!d->isDisc()) {
			d->fields->addData_string_numeric(be16_to_cpu(romHeader->checksum), RomFields::FB_HEX, 4);
		}
		else {
			// Checksum is not valid in Mega CD headers.
			d->fields->addData_invalid();
		}

		// Parse I/O support.
		uint32_t io_support = d->parseIOSupport(romHeader->io_support, sizeof(romHeader->io_support));
		d->fields->addData_bitfield(io_support);

		if (!d->isDisc()) {
			// ROM range.
			d->fields->addData_string_address_range(
				be32_to_cpu(romHeader->rom_start),
				be32_to_cpu(romHeader->rom_end), 8);

			// RAM range.
			d->fields->addData_string_address_range(
				be32_to_cpu(romHeader->ram_start),
				be32_to_cpu(romHeader->ram_end), 8);

			// SRAM range.
			// Info format: 'R', 'A', %1x1yz000, 0x20
			const uint32_t sram_info = be32_to_cpu(romHeader->sram_info);
			if ((sram_info & 0xFFFFA7FF) == 0x5241A020) {
				// SRAM is present.
				// x == 1 for backup (SRAM), 0 for not backup
				// yz == 10 for even addresses, 11 for odd addresses
				// TODO: Print the 'x' bit.
				const rp_char *suffix;
				switch ((sram_info >> (8 + 3)) & 0x03) {
				case 2:
					suffix = _RP("(even only)");
					break;
				case 3:
					suffix = _RP("(odd only)");
					break;
				default:
					// TODO: Are both alternates 16-bit?
					suffix = _RP("(16-bit)");
					break;
				}

				d->fields->addData_string_address_range(
					be32_to_cpu(romHeader->sram_start),
					be32_to_cpu(romHeader->sram_end),
					suffix, 8);
			}
			else {
				// TODO: Non-monospaced.
				d->fields->addData_string(_RP("None"));
			}
		}
		else {
			// ROM, RAM, and SRAM ranges are not valid in Mega CD headers.
			d->fields->addData_invalid();
			d->fields->addData_invalid();
			d->fields->addData_invalid();
		}

		// Region codes.
		// TODO: Validate the Mega CD security program?
		d->fields->addData_bitfield(d->md_region);

		// Vectors.
		if (!d->isDisc()) {
			d->fields->addData_string_numeric(
				be32_to_cpu(d->vectors.initial_pc), RomFields::FB_HEX, 8);
			d->fields->addData_string_numeric(
				be32_to_cpu(d->vectors.initial_sp), RomFields::FB_HEX, 8);
		}
		else {
			// Discs don't have vector tables.
			// Add dummy entries for the vectors.
			d->fields->addData_invalid();
			d->fields->addData_invalid();
		}

		// Finished reading the field data.
		return (int)d->fields->count();
	}

}

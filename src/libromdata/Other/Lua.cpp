/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * Lua.cpp: Lua binary chunk reader.                                       *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * Copyright (c) 2016-2020 by Egor.                                        *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "Lua.hpp"

// librpbase, librpfile
using namespace LibRpBase;
using LibRpFile::IRpFile;

// C++ STL classes.
using std::string;
using std::vector;

namespace LibRomData {

ROMDATA_IMPL(Lua)

/* Actual header sizes:
 * 2.4: 11
 * 2.5: 14
 * 4.0: 13+Number
 * 5.0: 14+Number
 * 5.1: 12
 * 5.2: 18
 * 5.3: 17+Integer+Number (the biggest one)
 * 5.4: 15+Integer+Number
 */
#define LUA_MAGIC "\033Lua"
#define LUA_TAIL "\x19\x93\r\n\x1a\n"
#define LUA_HEADERSIZE (17+8+8)

class LuaPrivate final : public RomDataPrivate
{
	public:
		LuaPrivate(Lua *q, IRpFile *file);

	private:
		typedef RomDataPrivate super;
		RP_DISABLE_COPY(LuaPrivate)

	public:
		enum class Version {
			Unknown = -1,
			Lua2_4 = 0,
			Lua2_5 = 1,
			Lua3_1 = 2,
			Lua3_2 = 3,
			Lua4_0 = 4,
			Lua5_0 = 5,
			Lua5_1 = 6,
			Lua5_2 = 7,
			Lua5_3 = 8,
			Lua5_4 = 9,
			Max
		};

		Version version;

		/**
		 * Converts version byte to Version enum
		 */
		static Version to_version(uint8_t version);

		/**
		 * Compares two byte ranges.
		 * @param lhs buffer to be compared
		 * @param rhs buffer to compare with
		 * @param len size of the buffers
		 * @param endianness 0 is used for be==be or le==le comparisions, 1 is used for le==be.
		 */
		static bool compare(const uint8_t *lhs, const uint8_t *rhs, size_t len, int endianness);

		/**
		 * Parses lua header into individual fields.
		 */
		void parse();

	public:
		// Lua header.
		uint8_t header[LUA_HEADERSIZE];
};

/** LuaPrivate **/

LuaPrivate::LuaPrivate(Lua *q, IRpFile *file)
	: super(q, file)
{
	// Clear the header struct.
	memset(&header, 0, sizeof(header));
}

/**
 * Converts version byte to Version enum
 */
Version LuaPrivate::to_version(uint8_t version) {
	switch (version) {
	// Bytecode dumping was introduced in 2.3, which was never publicly released.
	// 2.4 kept the same format, so we refer to the 0x23 format as "2.4".
	case 0x23: return Lua2_4;
	case 0x25: return Lua2_5; // Also used by 3.0
	case 0x31: return Lua3_1;
	case 0x32: return Lua3_2;
	case 0x40: return Lua4_0;
	case 0x50: return Lua5_0;
	case 0x51: return Lua5_1;
	case 0x52: return Lua5_2;
	case 0x53: return Lua5_3;
	case 0x54: return Lua5_4;
	default:   return Unknown;
	}
}

/**
 * Compares two byte ranges.
 * @param lhs buffer to be compared
 * @param rhs buffer to compare with
 * @param len size of the buffers
 * @param endianness 0 is used for be==be or le==le comparisions, 1 is used for le==be.
 */
bool LuaPrivate::compare(const uint8_t *lhs, const uint8_t *rhs, size_t len, int endianness)
{
	assert(endianness == 0 || endianness == 1);
	if (endianness == 0) {
		return !memcmp(lhs, rhs, len);
	} else if (endianness == 1) {
		for (size_t i = 0; i < len ; i++)
			if (lhs[i] != rhs[len-1-i])
				return false;
		return true;
	}
	return false;
}

/**
 * Parses lua header into individual fields.
 */
void LuaPrivate::parse()
{
	endianness = -1;
	int_size = -1;
	size_t_size = -1;
	Instruction_size = -1;
	weird_layout = false;
	Integer_size = -1;
	Number_size = -1;
	is_integral = -1;
	is_float_swapped = false;
	corrupted = false;

	uint8_t *p = header += 4;

	uint8_t version = *p++;

	/* We don't parse 2.x.
	 *
	 * Here is the header structure of 2.3/2.5 (2.5-only fields are marked with %)
	 * - (4) # "\033Lua" magic
	 * - (1) # version byte (0x23 or 0x25)
	 * - (1) % size of word (MUST be 2)
	 * - (1) % size of float (MUST be 4)
	 * - (1) % pointer size (MUST match the host ptr size)
	 * - (2) # test word 0x1234
	 * - (4) # test float 0.123456789e-23
	 * total size: 11 (2.3), 14 (2.5)
	 *
	 * Note that the float is an actual `float' type, not lua_Number (aka real).
	 *
	 * TODO: Actually implement it. I'd put it into a separate func.
	 */
	if (version < 0x31)
		return;

	// Format byte. 0 means official format. Apparently it's meant to be used by forks(?)
	if (version >= 0x51)
		if (*p++ != 0)
			return;

	// Some magic bytes for detecting transmission failures. Very similar to PNG magic.
	// 5.2 had this at the end of the header.
	if (version >= 0x53) {
		if (memcmp(p, LUA_TAIL, sizeof(LUA_TAIL)-1)) {
			corrupted = true;
			return;
		}
		p += sizeof(LUA_TAIL)-1;
	}

	if (versions < 0x53) {
		endianness = *p > 1 ? -1 : *p;
		p++;
	}

	// Lua 5.4 encodes int/size_t as varints, so it doesn't need to know their size.
	if (versions < 0x54) {
		int_size = *p++;
		size_t_size = *p++;
	}

	Instruction_size = *p++;
	if (version == 0x40) {
		uint8_t INSTRUCTION_bits = *p++;
		uint8_t OP_bits = *p++;
		uint8_t B_bits = *p++;
		if (INSTRUCTION_bits != 32 || OP_bits != 6 || B_bits != 9)
			weird_layout = true;
	} else if (version == 0x50) {
		uint8_t OP_bits = *p++;
		uint8_t A_bits = *p++;
		uint8_t B_bits = *p++;
		uint8_t C_bits = *p++;
		if (OP_bits != 6 || A_bits != 8 || B_bits != 9 || C_bits != 9)
			weird_layout = true;
	}

	// Lua 5.3 introduced support for a separate integer type.
	if (version >= 0x53)
		Integer_size = *p++;

	Number_size = *p++;

	if (version >= 0x53) {
		// A test number for lua_Integer (0x5678)
		const uint8_t *test_int64 = (const uint8_t*)"\x00\x00\x00\x00\x00\x00\x56\x78";
		const uint8_t *test_int = nullptr;
		if (Integer_size == 8)
			test_int = test_int64;
		else if (Integer_size == 4)
			test_int = test_int64 + 4;
		if (test_int) {
			if (compare(p, test_int, Integer_size, 0))
				endianness = 0;
			else if (compare(p, test_int, Integer_size, 1))
				endianness = 1;
			p += Integer_size;
		}
		// Note that if this fails, we end up with endianness == -1, and so the test
		// for lua_Number gets skipped.
	}

	if (version == 0x51 || version == 0x52) {
		// Lua 5.1 and 5.2 just have a flag to specify whether lua_Number is int or float.
		is_integral = *p > 1 ? -1 : *p;
		p++;
		// End of header for 5.1
	} else if (endianness != -1) {
		// 4.0, 5.0 and 5.3+ have a test number, from which we can tell the format of lua_Number.

		// NOTE: 5.0 and earlier don't compare the fractional part of the test number.

		// Pick the right set of constants based on version
		const uint8_t *test_int64, *test_float32, *test_float64;
		if (version == 0x40) {
			// This is supposed to be 3.14159265358979323846e8 cast to lua_Number
			test_int64 = (const uint8_t*)"\x00\x00\x00\x00\x12\xB9\xB0\xA1";
			test_float32 = (const uint8_t*)"\x4D\x95\xCD\x85";
			test_float64 = (const uint8_t*)"\x41\xB2\xB9\xB0\xA1\x5B\xE6\x12";
		} else if (version == 0x50) {
			// This is supposed to be 3.14159265358979323846e7 cast to lua_Number
			test_int64 = (const uint8_t*)"\x00\x00\x00\x00\x01\xDF\x5E\x76";
			test_float32 = (const uint8_t*)"\x4B\xEF\xAF\x3B";
			test_float64 = (const uint8_t*)"\x41\x7D\xF5\xE7\x68\x93\x09\xB6";
		} else {
			// This is supposed to be 370.5 cast to lua_Number
			test_int64 = (const uint8_t*)"\x00\x00\x00\x00\x00\x00\x01\x72";
			test_float32 = (const uint8_t*)"\x43\xB9\x40\x00";
			test_float64 = (const uint8_t*)"\x40\x77\x28\x00\x00\x00\x00\x00";
		}

		const uint8_t *test_int = nullptr;
		const uint8_t *test_float = nullptr;
		if (Number_size == 8) {
			test_int = test_int64;
			test_float = test_float64;
		} else if (Number_size == 4) {
			test_int = test_int64 + 4;
			test_float = test_float32;
		}
		if (test_int && test_float) {
			if (compare(p, test_float, Number_size, endianness))
				is_integral = 0;
			else if (compare(p, test_int, Number_size, endianness))
				is_integral = 1;
			else if (compare(p, test_float, Number_size, !endianness))
				is_integral = 0, is_float_swapped = true;
		}
		// End of header for 5.0, 5.3, 5.4
	}

	if (version == 0x52) {
		if (memcmp(p, LUA_TAIL, sizeof(LUA_TAIL)-1))
			corrupted = true;
		// End of header for 5.2
	}
}

/**
 * Is character a valid JIS X 0201 codepoint?
 * @param c The character
 * @return Wether or not character is valid
 */
bool inline VirtualBoyPrivate::isJISX0201(unsigned char c){
	return (c >= ' ' && c <= '~') || (c > 0xA0 && c < 0xE0);
}

/**
 * Is character a valid Game ID character?
 * @param c The character
 * @return Wether or not character is valid
 */
bool inline VirtualBoyPrivate::isPublisherID(char c){
	// Valid characters:
	// - Uppercase letters
	// - Digits
	return (ISUPPER(c) || ISDIGIT(c));
}

/**
 * Is character a valid Game ID character?
 * @param c The character
 * @return Wether or not character is valid
 */
bool inline VirtualBoyPrivate::isGameID(char c){
	// Valid characters:
	// - Uppercase letters
	// - Digits
	// - Space (' ')
	// - Hyphen ('-')
	return (ISUPPER(c) || ISDIGIT(c) || c == ' ' || c == '-');
}

/** Lua **/

/**
 * Read a Lua binary chunk.
 *
 * A ROM file must be opened by the caller. The file handle
 * will be ref()'d and must be kept open in order to load
 * data from the ROM.
 *
 * To close the file, either delete this object or call close().
 *
 * NOTE: Check isValid() to determine if this is a valid ROM.
 *
 * @param file Open ROM file.
 */
Lua::Lua(IRpFile *file)
	: super(new Lua(this, file))
{
	RP_D(Lua);
	d->className = "Lua";
	d->mimeType = "text/x-lua";	// unofficial

	if (!d->file) {
		// Could not ref() the file handle.
		return;
	}

	// Seek to the beginning of the header.
	d->file->rewind();

	// Read the ROM header.
	size_t size = d->file->read(&d->header, sizeof(d->header));
	if (size != sizeof(d->header)) {
		UNREF_AND_NULL_NOCHK(d->file);
		return;
	}

	// Check if this file is supported.
	DetectInfo info;
	info.header.addr = 0;
	info.header.size = sizeof(d->header);
	info.header.pData = d->header;
	info.ext = nullptr;	// Not needed for Lua.
	info.szFile = nullptr;	// Not needed for Lua.
	d->version = static_cast<LuaPrivate::Version>(isRomSupported_static(&info));
	d->isValid = ((int)d->romType >= 0);

	if (!d->isValid) {
		UNREF_AND_NULL_NOCHK(d->file);
	}
}

/** ROM detection functions. **/

/**
 * Is a ROM image supported by this class?
 * @param info DetectInfo containing ROM detection information.
 * @return Class-specific system ID (>= 0) if supported; -1 if not.
 */
int Lua::isRomSupported_static(const DetectInfo *info)
{
	assert(info != nullptr);
	assert(info->header.pData != nullptr);
	if (!info || !info->header.pData ||
	    info->header.addr != 0 ||
	    info->header.size < LUA_HEADERSIZE)
	{
		return static_cast<int>(LuaPrivate::Version::Unknown);
	}

	const uint8_t *header = info->header.pData;
	if (!memcmp(header, LUA_MAGIC, sizeof(LUA_MAGIC)-1)) {
		uint8_t version = header[4];
		uint8_t format = version >= 0x51 ? header[5] : 0;
		if (format == 0)
			return static_cast<int>(LuaPrivate::to_version(version));
	}
		
	return static_cast<int>(LuaPrivate::Version::Unknown);
}

/**
 * Get the name of the system the loaded ROM is designed for.
 * @return System name, or nullptr if not supported.
 */
const char *Lua::systemName(unsigned int type) const
{
	RP_D(const Lua);
	if (!d->isValid || !isSystemNameTypeValid(type))
		return nullptr;

	static_assert(SYSNAME_TYPE_MASK == 3,
		"Lua::systemName() array index optimization needs to be updated.");
	static_assert((int)LuaPrivate::Version::Max == 10,
		"Lua::systemName() array index optimization needs to be updated.");
	
	static const char *const sysNames[10][4] = {
		{"PUC Lua 2.4", "Lua 2.4", "Lua", nullptr},
		{"PUC Lua 2.5/3.0", "Lua 2.5/3.0", "Lua", nullptr},
		{"PUC Lua 3.1", "Lua 3.1", "Lua", nullptr},
		{"PUC Lua 3.2", "Lua 3.2", "Lua", nullptr},
		{"PUC Lua 4.0", "Lua 4.0", "Lua", nullptr},
		{"PUC Lua 5.0", "Lua 5.0", "Lua", nullptr},
		{"PUC Lua 5.1", "Lua 5.1", "Lua", nullptr},
		{"PUC Lua 5.2", "Lua 5.2", "Lua", nullptr},
		{"PUC Lua 5.3", "Lua 5.3", "Lua", nullptr},
		{"PUC Lua 5.4", "Lua 5.4", "Lua", nullptr},
	};

	return sysNames[(int)d->version][type & SYSNAME_TYPE_MASK];
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
const char *const *Lua::supportedFileExtensions_static(void)
{
	// NOTE: These extensions may cause conflicts on
	// Windows if fallback handling isn't working.
	static const char *const exts[] = {
		".lua",	// Lua source code.
		".out", // from luac.out, the default output filename of luac.
		".lub", // Lua binary
		// TODO: Others?
		nullptr
	};
	return exts;
}

/**
 * Get a list of all supported MIME types.
 * This is to be used for metadata extractors that
 * must indicate which MIME types they support.
 *
 * NOTE: The array and the strings in the array should
 * *not* be freed by the caller.
 *
 * @return NULL-terminated array of all supported file extensions, or nullptr on error.
 */
const char *const *Lua::supportedMimeTypes_static(void)
{
	static const char *const mimeTypes[] = {
		// Unofficial MIME types from FreeDesktop.org.
		// FIXME: these are the MIME types for Lua source code
		"application/x-lua",
		"text/x-lua",

		nullptr
	};
	return mimeTypes;
}

/**
 * Load field data.
 * Called by RomData::fields() if the field data hasn't been loaded yet.
 * @return Number of fields read on success; negative POSIX error code on error.
 */
int Lua::loadFieldData(void)
{
	RP_D(Lua);
	if (!d->fields->empty()) {
		// Field data *has* been loaded...
		return 0;
	} else if (!d->file || !d->file->isOpen()) {
		// File isn't open.
		return -EBADF;
	} else if (!d->isValid) {
		// ROM image isn't valid.
		return -EIO;
	}

	// Virtual Boy ROM header, excluding the vector table.
	const VB_RomHeader *const romHeader = &d->romHeader;
	d->fields->reserve(5);	// Maximum of 5 fields.

	// Title
	d->fields->addField_string(C_("RomData", "Title"),
		cp1252_sjis_to_utf8(romHeader->title, sizeof(romHeader->title)));

	// Game ID and publisher.
	string id6(romHeader->gameid, sizeof(romHeader->gameid));
	id6.append(romHeader->publisher, sizeof(romHeader->publisher));
	d->fields->addField_string(C_("RomData", "Game ID"), latin1_to_utf8(id6));

	// Look up the publisher.
	const char *const publisher = NintendoPublishers::lookup(romHeader->publisher);
	string s_publisher;
	if (publisher) {
		s_publisher = publisher;
	} else {
		if (ISALNUM(romHeader->publisher[0]) &&
		    ISALNUM(romHeader->publisher[1]))
		{
			s_publisher = rp_sprintf(C_("RomData", "Unknown (%.2s)"),
				romHeader->publisher);
		} else {
			s_publisher = rp_sprintf(C_("RomData", "Unknown (%02X %02X)"),
				static_cast<uint8_t>(romHeader->publisher[0]),
				static_cast<uint8_t>(romHeader->publisher[1]));
		}
	}
	d->fields->addField_string(C_("RomData", "Publisher"), s_publisher);

	// Revision
	d->fields->addField_string_numeric(C_("RomData", "Revision"),
		romHeader->version, RomFields::Base::Dec, 2);

	// Region
	const char *s_region;
	switch (romHeader->gameid[3]) {
		case 'J':
			s_region = C_("Region", "Japan");
			break;
		case 'E':
			s_region = C_("Region", "USA");
			break;
		default:
			s_region = nullptr;
			break;
	}
	if (s_region) {
		d->fields->addField_string(C_("RomData", "Region Code"), s_region);
	} else {
		d->fields->addField_string(C_("RomData", "Region Code"),
			rp_sprintf(C_("RomData", "Unknown (0x%02X)"),
				static_cast<uint8_t>(romHeader->gameid[3])));
	}

	return static_cast<int>(d->fields->count());
}

}

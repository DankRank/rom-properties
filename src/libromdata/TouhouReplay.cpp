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
#include <ctime>

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
			count = file->read(ptr, (size_t)(std::min<uint64_t>(offset, cpos + size) - cpos));
			*(char**)&ptr += count;
			cpos += count;
			size -= count;
		}

		// Now, if there's still data to read, do the encrypted area.
		if (size != 0) {
			count2 = file->read(ptr, size);
			
			uint8_t k = (uint8_t)( key + 7 * (cpos - offset) ); // calculate key for our file position
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
	
	class Touhou10USERParser {
		// if this variable is true, something went wrong during parsing
		bool staticPatch;

		bool is_valid;

		std::string version;
		std::string name;
		time_t date;
		std::string chara;
		std::string rank;
		std::string stage;
		long score;
		float slowrate;

		char* readFile(IRpFile* file, uint32_t& size);
		bool breakLines(char* text, uint32_t size, std::string (&lines)[9]);
		std::string spaceString(std::string line, const std::string& keyword);
		bool parseLines(const std::string (&lines)[9]);
		time_t parseDate(std::string datestr);
	public:
		explicit Touhou10USERParser(IRpFile* file);
		bool isValid();
	};
	char* Touhou10USERParser::readFile(IRpFile* file, uint32_t& size) {
		assert(file);
		assert(file->isOpen());
		if (file && file->isOpen()) {
			// Read offset of the USER header
			file->seek(offsetof(THRP_GenericHeader, useroffset));
			uint32_t offset;
			if (sizeof(offset) != file->read(&offset, sizeof(offset))) {
				assert(0);
				return nullptr;
			}

			// Read USER header
			file->seek(offset);
			THRP_USERHeader uhead;
			file->read(&uhead, sizeof(uhead));

			// Check magic
			static char user_magic[] = { 'U', 'S', 'E', 'R' };
			if (!memcmp(uhead.magic, user_magic, sizeof(user_magic))) {
				// Make sure the size is valid
				assert(uhead.size >= sizeof(uhead));
				if (uhead.size >= sizeof(uhead)) {
					// Read the text data
					size = uhead.size - sizeof(uhead);
					char *text = new char[size];
					if (size != file->read(text, size)) {
						// read failed, deallocate the text
						delete[] text;
						assert(0);
						return nullptr;
					}
					return text;
				}
			}
		}
		return nullptr;
	}
	std::string Touhou10USERParser::spaceString(std::string line, const std::string& keyword) {
		size_t pos = line.find(' ');

		if (pos != std::string::npos) {
			// Check the keyword
			if (line.substr(0, pos) != keyword) staticPatch = true;

			return line.substr(pos + 1);
		}
		else {
			staticPatch = true;
			return "";
		}
	}
	time_t Touhou10USERParser::parseDate(std::string datestr) {
		// format: yy/mm/dd hh:mm
		if (datestr.length() != 14
			|| datestr[2] != '/'
			|| datestr[5] != '/'
			|| datestr[8] != ' '
			|| datestr[11] != ':') return -1;
		
		tm t;
		t.tm_year = 100 + std::stoi(datestr);
		t.tm_mon = std::stoi(datestr.substr(3));
		t.tm_mday = std::stoi(datestr.substr(6));
		t.tm_hour = std::stoi(datestr.substr(9));
		t.tm_min = std::stoi(datestr.substr(12));
		t.tm_sec = 0;
		t.tm_isdst = 0; // unknown
		// other fields don't need to be set for mktime to work.
		return mktime(&t);
	}
	bool Touhou10USERParser::parseLines(const std::string (&lines)[9]) {
		// Line 0 - "東方XYZ リプレイファイル情報" - "Touhou XYZ replay file info"
		// (skipped)
		
		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		date = parseDate(spaceString(lines[3], "Date"));

		// Line 4 - "Chara %s"
		chara = spaceString(lines[4], "Chara");

		// Line 5 - "Rank %s"
		rank = spaceString(lines[5], "Rank");

		// Line 6 - which stage was completed
		// TODO: make a better break down with enums and stuff
		if (lines[6] == "Extra Stage Clear") {
			stage = lines[6];
		}
		else {
			stage = spaceString(lines[6], "Stage");
			// there could be three possible formats starting with Stage:
			// Stage All Clear
			// Stage %d
			// Stage %d \x81\x60[fullwidth ~] %d
		}

		// Line 7 - "Score %d"
		score = std::stol ( spaceString(lines[7], "Score") );

		// Line 8 - "Slow Rate %2.2f"
		// TODO: maybe make it less hacky?
		slowrate = std::stof( spaceString(spaceString(lines[8], "Slow"), "Rate") );
		
		return true;
	}
	bool Touhou10USERParser::breakLines(char* text, uint32_t size, std::string(&lines)[9]) {
		// Make sure there's a null at the very end
		assert(!text[size - 1]);
		if (text[size - 1]) return false;

		char *p = text;
		for (int i = 0; i < 9; i++) {
			// Look for a new-line
			char* p2 = strstr(p, "\r\n");
			if (!p2) return false; // line not found

			lines[i] = std::string(p, p2 - p);
			p = p2 + 2; // skip \r\n
		}

		// just some extra checking for the double-null at the very end
		assert(p - text == size - 2);
		assert(!p[0] && !p[1]);
		return true;
	}
	Touhou10USERParser::Touhou10USERParser(IRpFile* file) :is_valid(false), staticPatch(false), date(-1), score(0), slowrate(99.99f){
		uint32_t size;
		char* text = readFile(file, size);
		if (!text) {
			return;
		}
		
		std::string lines[9];
		if (!breakLines(text,size,lines)) {
			delete[] text;
			return;
		}
		delete[] text;

		is_valid = parseLines(lines);
		
	}
	bool Touhou10USERParser::isValid() {
		return is_valid;
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
		enum TH_GameType {
			TH_UNKNOWN = -1,
			TH_06 = 0,
			TH_07,
			TH_08,
			TH_09,
			TH_095,
			TH_10,
			TH_11,
			TH_12,
			TH_125,
			TH_128,
			TH_13,
			TH_14,
			TH_143,
			TH_15,
			TH_LAST = TH_15
		};
		int gameType;		// Game type.
	public:
		// THRP header.
		T6RP_Header thrpHeader;		// ROM header.
		T6RP_Stage stageHeader[7];
	};

	/** TouhouReplayPrivate **/

	// ROM fields.
	const struct RomFields::Desc TouhouReplayPrivate::th_fields[] = {
		{ _RP("Player Name"), RomFields::RFT_STRING, nullptr },
		{ _RP("Date"), RomFields::RFT_STRING, nullptr },
		{ _RP("Player"), RomFields::RFT_STRING, nullptr },
		{ _RP("Rank"), RomFields::RFT_STRING, nullptr },

	};

	TouhouReplayPrivate::TouhouReplayPrivate(TouhouReplay *q, IRpFile *file)
		: super(q, file, th_fields, ARRAY_SIZE(th_fields))
		, gameType(TH_UNKNOWN)
	{
		// Clear the various structs.
		memset(&thrpHeader, 0, sizeof(thrpHeader));

		fileType = RomData::FTYPE_REPLAY_FILE;
	}

	/** Internal ROM data. **/

	/** TouhouReplay **/

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
		// TODO: Only validate that this is an TH replay here.
		// Load fields elsewhere.
		RP_D(TouhouReplay);
		if (!d->file) {
			// Could not dup() the file handle.
			return;
		}

		// Seek to the beginning of the file.
		d->file->rewind();

		// Read the ROM header. [0x400 bytes]
		uint8_t header[sizeof(T6RP_Header)];
		size_t size = d->file->read(header, sizeof(header));
		if (size != sizeof(header))
			return;

		// Check if this ROM is supported.
		DetectInfo info;
		info.header.addr = 0;
		info.header.size = sizeof(header);
		info.header.pData = header;
		info.ext = nullptr;	// Not needed for TH.
		info.szFile = 0;	// Not needed for TH.
		d->gameType = isRomSupported_static(&info);

		if (d->gameType == TouhouReplayPrivate::TH_06) {
			memcpy(&d->thrpHeader, header, sizeof(d->thrpHeader));
			TouhouCryptFile cf(d->file, d->thrpHeader.key, offsetof(T6RP_Header, unknown));
			cf.rewind();
			cf.read(header, sizeof(header));
			memcpy(&d->thrpHeader, header, sizeof(d->thrpHeader));
			for (int i = 0; i < 7; i++) {
				cf.seek(d->thrpHeader.stage_offset[i]);
				cf.read(&d->stageHeader[i], sizeof(d->stageHeader[i]));
			}
		}

		// Touhou 13 and 14 have the same magic, so we need to check the USER section
		if (d->gameType == TouhouReplayPrivate::TH_13) {
			static uint8_t th14_magic[] = {
				// this says: 東方輝針城 リプレイファイル情報
				0x93, 0x8C, 0x95, 0xFB, 0x8B, 0x50, 0x90, 0x6A,
				0x8F, 0xE9, 0x20, 0x83, 0x8A, 0x83, 0x76, 0x83,
				0x8C, 0x83, 0x43, 0x83, 0x74, 0x83, 0x40, 0x83,
				0x43, 0x83, 0x8B, 0x8F, 0xEE, 0x95, 0xF1,
			};
			uint8_t magicbuf[sizeof(th14_magic)] = {0};

			d->file->seek((*(uint32_t*)&header[offsetof(THRP_GenericHeader,useroffset)]) + sizeof(THRP_USERHeader));
			d->file->read(magicbuf, sizeof(th14_magic));
			if (!memcmp(magicbuf, th14_magic, sizeof(th14_magic))) {
				d->gameType = TouhouReplayPrivate::TH_14;
			}
		}

		d->isValid = (d->gameType == TouhouReplayPrivate::TH_06); // TODO: add support for other games later -Egor
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
			info->header.size < 0x4)
		{
			// Either no detection information was specified,
			// or the header is too small.
			return -1;
		}

		// ROM header.
		const uint8_t *const pHeader = info->header.pData;

		// Magic strings.
		static const char thrp_magic[][4] = {
			{ 'T', '6', 'R', 'P' }, // koumakyou (eosd)
			//{ 'T', '7', 'R', 'P' }, // youyoumu (pcb)
			//{ 'T', '8', 'R', 'P' }, // eiyashou (in)
			//{ 'T', '9', 'R', 'P' }, // kaeidzuka (pofv)
			//{ 't', '9', '5', 'r' }, // bunkachou (stb)
			//{ 't', '1', '0', 'r' }, // fuujinroku (mof)
			//{ 't', '1', '1', 'r' }, // chireiden (sa)
			//{ 't', '1', '2', 'r' }, // seirensen (ufo)
			//{ 't', '1', '2', '5' }, // bunkachou (ds)
			//{ '1', '2', '8', 'r' }, // yousei daisensou
			//{ 't', '1', '3', 'r' }, // shinreibyou (td)
			//{ 0, 0, 0, 0 }, // kishinjou (ddc) - has the same id as th13 for some reason
			//{ 't', '1', '4', '3' }, // danmaku amanojaku (isc)
			//{ 't', '1', '5', 'r' }, // kanjuden (lolk)

		};
		for (int i = 0; i < ARRAY_SIZE(thrp_magic); i++) {
			if (thrp_magic[i] == 0) continue;
			if (!memcmp(pHeader, thrp_magic[i], 4)) {
				return i;
			}
		}
		// Not supported.
		return TouhouReplayPrivate::TH_UNKNOWN;
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

		static_assert(SYSNAME_TYPE_MASK == 3,
			"TouhouReplay::systemName() array index optimization needs to be updated.");

		uint32_t romSys = d->gameType;
		if (romSys > TouhouReplayPrivate::TH_LAST) {
			// Invalid system type. Default to EoSD, because why not.
			romSys = TouhouReplayPrivate::TH_06;
		}

		// sysNames[] bitfield:
		// - Bits 0-1: Type. (short, long, abbreviation)
		// - Bits 2-...: Game type.
		uint32_t idx = (romSys << 2) | (type & SYSNAME_TYPE_MASK);
		
		static const rp_char *const sysNames[4 * 14] = {
			_RP("Touhou Koumakyou ~ the Embodiment of Scarlet Devil"), _RP("Embodiment of Scarlet Devil"),   _RP("EoSD"), nullptr,
			_RP("Touhou Youyoumu ~ Perfect Cherry Blossom"),           _RP("Perfect Cherry Blossom"),        _RP("PCB"), nullptr,
			_RP("Touhou Eiyashou ~ Imperishable Night"),               _RP("Imperishable Night"),            _RP("IN"), nullptr,
			_RP("Touhou Kaeidzuka ~ Phantasmagoria of Flower View"),   _RP("Phantasmagoria of Flower View"), _RP("PoFV"), nullptr,
			_RP("Touhou Bunkachou ~ Shoot the Bullet"),                _RP("Shoot the Bullet"),              _RP("StB"), nullptr,
			_RP("Touhou Fuujinroku ~ Mountain of Faith"),              _RP("Mountain of Faith"),             _RP("MoF"), nullptr,
			_RP("Touhou Chireiden ~ Subterranean Animism"),            _RP("Subterranean Animism"),          _RP("SA"), nullptr,
			_RP("Touhou Seirensen ~ Undefined Fantastic Object"),      _RP("Undefined Fantastic Object"),    _RP("UFO"), nullptr,
			_RP("Double Spoiler ~ Touhou Bunkachou"),                  _RP("Double Spoiler"),                _RP("DS"), nullptr,
			_RP("Yousei Daisensou ~ Touhou Sangetsusei"),              _RP("Yousei Daisensou"),              _RP("FW"), nullptr,
			_RP("Touhou Shinreibyou ~ Ten Desires"),                   _RP("Ten Desires"),                   _RP("TD"), nullptr,
			_RP("Touhou Kishinjou ~ Double Dealing Character"),        _RP("Double Dealing Character"),      _RP("DDC"), nullptr,
			_RP("Danmaku Amanojaku ~ Impossible Spell Card"),          _RP("Impossible Spell Card"),         _RP("ISC"), nullptr,
			_RP("Touhou Kanjuden ~ Legacy of Lunatic Kingdom"),        _RP("Legacy of Lunatic Kingdom"),     _RP("LoLK"), nullptr,
		};

		if (idx >= ARRAY_SIZE(sysNames)) {
			// Invalid index...
			return nullptr;
		}

		return sysNames[idx];
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
		else if (!d->isValid || d->gameType < 0) {
			// Unknown game type.
			return -EIO;
		}

		const T6RP_Header *thrpHeader = &d->thrpHeader;

		// Read the strings from the header.
		d->fields->addData_string(cp1252_to_rp_string(thrpHeader->name, sizeof(thrpHeader->name)));
		d->fields->addData_string(cp1252_to_rp_string(thrpHeader->date, sizeof(thrpHeader->date)));
		d->fields->addData_string_numeric(thrpHeader->player);
		d->fields->addData_string_numeric(thrpHeader->rank);

		// Finished reading the field data.
		return (int)d->fields->count();
	}

}

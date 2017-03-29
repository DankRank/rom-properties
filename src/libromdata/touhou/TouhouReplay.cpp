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

#include "TouhouCryptFile.hpp"
#include "ITouhouUserParser.hpp"
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
using std::vector;

namespace LibRomData {
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
		static const RomFields::StringDesc th_string_warning;
		static const struct RomFields::DateTimeDesc th_datetime;
		// ROM fields.
		static const struct RomFields::Desc th_fields[];

		/** Internal ROM data. **/


	public:

		int gameType;		// Game type.
	};

	/** TouhouReplayPrivate **/

	TouhouReplayPrivate::TouhouReplayPrivate(TouhouReplay *q, IRpFile *file)
		: super(q, file)
		, gameType(TouhouReplay::TH_UNKNOWN)
	{
		// Clear the various structs.
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
		// TODO: Only validate that this is an TH replay here.
		// Load fields elsewhere.
		RP_D(TouhouReplay);
		if (!d->file) {
			// Could not dup() the file handle.
			return;
		}

		// Seek to the beginning of the file.
		d->file->rewind();

		// Read the ROM header.
		THRP_GenericHeader header;
		size_t size = d->file->read(&header, sizeof(header));
		if (size != sizeof(header))
			return;

		// Check if this ROM is supported.
		DetectInfo info;
		info.header.addr = 0;
		info.header.size = sizeof(header);
		info.header.pData = (uint8_t*)&header;
		info.ext = nullptr;	// Not needed for TH.
		info.szFile = 0;	// Not needed for TH.
		d->gameType = isRomSupported_static(&info);

		// Touhou 13 and 14 have the same magic, so we need to check the USER section
		// NOTE: this algorithm is oversimplified, this is not the "canonical" way to read a USER section. See TouhouUserReader.cpp
		if (d->gameType == TH_13) {
			static const uint8_t th14_magic[] = {
				// this says: 東方輝針城 リプレイファイル情報
				0x93, 0x8C, 0x95, 0xFB, 0x8B, 0x50, 0x90, 0x6A,
				0x8F, 0xE9, 0x20, 0x83, 0x8A, 0x83, 0x76, 0x83,
				0x8C, 0x83, 0x43, 0x83, 0x74, 0x83, 0x40, 0x83,
				0x43, 0x83, 0x8B, 0x8F, 0xEE, 0x95, 0xF1,
			};
			uint8_t magicbuf[sizeof(th14_magic)] = { 0 };

			d->file->seek(header.useroffset + sizeof(THRP_USERHeader));
			d->file->read(magicbuf, sizeof(th14_magic));
			if (!memcmp(magicbuf, th14_magic, sizeof(th14_magic))) {
				d->gameType = TH_14;
			}
		}

		d->isValid = (d->gameType < TH_07 || d->gameType > TH_09); // TODO: add support for other games later -Egor
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
			{ 0 },//{ 'T', '7', 'R', 'P' }, // youyoumu (pcb)
			{ 0 },//{ 'T', '8', 'R', 'P' }, // eiyashou (in)
			{ 0 },//{ 'T', '9', 'R', 'P' }, // kaeidzuka (pofv)
			{ 't', '9', '5', 'r' }, // bunkachou (stb)
			{ 't', '1', '0', 'r' }, // fuujinroku (mof)
			{ 'a', 'l', '1', 'r' }, // tasogare sakaba (algostg)
			{ 't', '1', '1', 'r' }, // chireiden (sa)
			{ 't', '1', '2', 'r' }, // seirensen (ufo)
			{ 't', '1', '2', '5' }, // bunkachou (ds)
			{ '1', '2', '8', 'r' }, // yousei daisensou
			{ 't', '1', '3', 'r' }, // shinreibyou (td)
			{ 0 },// kishinjou (ddc) - has the same id as th13 for some reason
			{ 't', '1', '4', '3' }, // danmaku amanojaku (isc)
			{ 't', '1', '5', 'r' }, // kanjuden (lolk)

		};
		for (int i = 0; i < ARRAY_SIZE(thrp_magic); i++) {
			if (thrp_magic[i] == 0) continue;
			if (!memcmp(pHeader, thrp_magic[i], 4)) {
				return i;
			}
		}
		// Not supported.
		return TH_UNKNOWN;
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
		static_assert(TH_LAST == 14,
			"TouhouReplay::systemName() array needs to be updated.");

		uint32_t romSys = d->gameType;
		if (romSys > TH_LAST) {
			// Invalid system type. Default to EoSD, because why not.
			romSys = TH_06;
		}

		// sysNames[] bitfield:
		// - Bits 0-1: Type. (short, long, abbreviation)
		// - Bits 2-...: Game type.
		uint32_t idx = (romSys << 2) | (type & SYSNAME_TYPE_MASK);

		static const rp_char *const sysNames[4 * 15] = {
			_RP("Touhou Koumakyou ~ the Embodiment of Scarlet Devil"), _RP("Embodiment of Scarlet Devil"),   _RP("EoSD"), nullptr,
			_RP("Touhou Youyoumu ~ Perfect Cherry Blossom"),           _RP("Perfect Cherry Blossom"),        _RP("PCB"), nullptr,
			_RP("Touhou Eiyashou ~ Imperishable Night"),               _RP("Imperishable Night"),            _RP("IN"), nullptr,
			_RP("Touhou Kaeidzuka ~ Phantasmagoria of Flower View"),   _RP("Phantasmagoria of Flower View"), _RP("PoFV"), nullptr,
			_RP("Touhou Bunkachou ~ Shoot the Bullet"),                _RP("Shoot the Bullet"),              _RP("StB"), nullptr,
			_RP("Touhou Fuujinroku ~ Mountain of Faith"),              _RP("Mountain of Faith"),             _RP("MoF"), nullptr,
			_RP("Tasogare Sakaba ~Uwabami Breakers~"),                 _RP("Uwabami Breakers"),              _RP("alcostg"), nullptr,
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
	* @return 0 on success; negative POSIX error code on error.
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

		ITouhouUserParser* mofParse = ITouhouUserParser::getInstance(d->gameType, d->file);
		assert(mofParse);
		if (!mofParse || !mofParse->isValid()) {
			// TODO: maybe return posix error?
			d->fields->addField_string(_RP("Warning")
				,_RP("Parsing error has occured.")
				,RomFields::StringDesc::STRF_WARNING);
			return 0;
		}

		// Read the strings from the header.
		if (mofParse->isBroken()) {
			d->fields->addField_string(_RP("Warning")
				,_RP("This file seems to have invalid format. The information displayed below may be inaccurate.")
				,RomFields::StringDesc::STRF_WARNING);
		}
		d->fields->addField_string(_RP("Game Version"), mofParse->getVersion());
		d->fields->addField_string(_RP("Name"), mofParse->getName());
		d->fields->addField_dateTime(_RP("Date"), mofParse->getTime(), RomFields::RFT_DATETIME_HAS_DATE | RomFields::RFT_DATETIME_HAS_TIME);
		d->fields->addField_string(_RP("Character"), mofParse->getChara());
		d->fields->addField_string(_RP("Rank"), mofParse->getRank());
		d->fields->addField_string(_RP("Stage"), mofParse->getStage());
		d->fields->addField_string(_RP("Game Cleared"), mofParse->isClear());
		d->fields->addField_string_numeric(_RP("Score"), mofParse->getScore());
		d->fields->addField_string(_RP("Slow Rate"), mofParse->getSlowRate());
		if (d->gameType >= TH_08 && mofParse->isCommentPresent()) { // comments have been added in th08
			d->fields->addField_string(_RP("Comments"), mofParse->getComment());
		}

		// Finished reading the field data.
		return 0;
	}
}
/***************************************************************************
* ROM Properties Page shell extension. (libromdata)                       *
* th_structs.h: Touhou data structures.                                   *
*                                                                         *
* Copyright (c) 2016 by David Korth.                                      *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_TH_STRUCTS_H__
#define __ROMPROPERTIES_LIBROMDATA_TH_STRUCTS_H__

#include "common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
	// Generic header for games starting from TH08
	typedef struct PACKED _THRP_GenericHeader {
		char magic[4]; // T8RP, etc
		uint8_t unk1[3];
		uint8_t usercount; // count of USER chunks. always 0 for TH10+
		uint32_t unk2;
		uint32_t useroffset; // offset of USER data
	} THRP_GenericHeader;
#pragma pack()
	ASSERT_STRUCT(THRP_GenericHeader, 16);

#pragma pack(1)
	typedef struct PACKED _THRP_USERHeader {
		char magic[4]; // "USER"
		uint32_t size; // size of text data + this header
		uint8_t type; // 0 - game info, 1 - comment
		uint8_t padding[3];
		// text data follows
	} THRP_USERHeader;
#pragma pack()
	ASSERT_STRUCT(THRP_USERHeader, 12);

#pragma pack(1)
	typedef struct PACKED _T6RP_Header {
		char magic[4]; // T6RP
		uint16_t version; // 0x0102 for 1.02h
		uint8_t chara; // 0 = ReimuA, 1 = ReimuB, 2 = MarisaA, 3 = MarisaB
		uint8_t rank; // 0 = Easy, 3 = Lunatic, 4 = Extra
		uint32_t checksum; // (0x3f000318 + key + sum(c for c in decrypted_data)) % (2 ** 32)
		uint16_t unknown0;
		uint8_t key;
		// ENCRYPTED //
		uint8_t unknown;
		char date[9]; // null-terminated string
		char name[9]; // null-terminated string
		uint16_t unknown2;
		uint32_t score; // Total score.
		uint32_t unknown3;
		float slowdown_rate; // As a percentage, not a proper rate
		uint32_t unknown4;
		uint32_t stage_offset[7];
	} T6RP_Header;
#pragma pack()
	ASSERT_STRUCT(T6RP_Header, 80);

#pragma pack(1)
	typedef struct PACKED _T6RP_Stage {
		uint32_t score;
		uint16_t random_seed;
		uint16_t unknown1;
		uint8_t power;
		int8_t lives;
		int8_t bombs;
		uint8_t difficulty; // dynamic difficulty coefficent, that changes throughout the gameplay
							// dubbed "rank" by touhouwiki, but "rank" is how ZUN calls easy/lunatic/etc.
		uint32_t unknown3;
	} T6RP_Stage;
#pragma pack()
	ASSERT_STRUCT(T6RP_Stage, 16);

#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBROMDATA_TH_STRUCTS_H__ */

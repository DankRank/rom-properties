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
	typedef struct PACKED _T6RP_Header {
		// copy-pasted from https://pytouhou.linkmauve.fr/doc/06/t6rp.xhtml for now
		// TODO: write better docs
		char magic[4]; // T6RP
		uint16_t version; // 0x0102 for 1.02h
		uint8_t player; // 0 = ReimuA, 1 = ReimuB, 2 = MarisaA, 3 = MarisaB
		uint8_t rank; // 0 = Easy, 3 = Lunatic, 4 = Extra
		uint32_t checksum; // (0x3f000318 + key + sum(c for c in decrypted_data)) % (2 ** 32)
		uint16_t unknown2; //TODO: seems to be ignored by the game.
		uint8_t key;
		// ENCRYPTED //
		uint8_t unknown; //TODO: seems to be ignored by the game. Padding?
		char date[9]; // null-terminated string
		char name[9]; // null-terminated string
		uint16_t unknown2; //TODO: seems to be ignored by the game. Padding?
		uint32_t score; //TODO: Total score. seems to be ignored by the game.
		uint32_t unknown3; //TODO: seems to be ignored by the game.
		float slowdown_rate; // As a percentage, not a proper rate
		uint32_t unknown4; //TODO: seems to be ignored by the game.
		uint32_t stage_offset[7];
	} T6RP_Header;
	ASSERT_STRUCT(T6RP_Header, 80);

#pragma pack(1)
	typedef struct PACKED _T6RP_Stage {
		// copy-pasted from https://pytouhou.linkmauve.fr/doc/06/t6rp.xhtml for now
		uint32_t score;
		uint16_t random_seed;
		uint16_t unknown1; //TODO: seems to be ignored by the game.
		uint8_t power;
		int8_t lives;
		int8_t bombs;
		uint8_t difficulty; //TODO: WARNING: This has a huge effect on the game!
							// It is also called rank (but we use the term “difficulty” because “rank” is the official name for Easy/Normal/Hard/Lunatic/Extra)
							// See: http://en.touhouwiki.net/wiki/Embodiment_of_Scarlet_Devil/Gameplay#Rank
		uint32_t unknown3; //TODO: seems to be ignored by the game. Padding?
	} T6RP_Stage;
#pragma pack()
	ASSERT_STRUCT(T6RP_Stage, 16);

#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBROMDATA_TH_STRUCTS_H__ */

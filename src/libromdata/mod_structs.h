/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * mod_structs.h: Amiga Module data structures.                            *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_MOD_STRUCTS_H__
#define __ROMPROPERTIES_LIBROMDATA_MOD_STRUCTS_H__

#include "librpbase/common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Amiga Module sample header
 * All fields are big-endian.
 */
#pragma pack(1)
typedef struct PACKED _MOD_Sample_Header {
	char name[22];
	uint16_t sample_length;
	uint8_t finetune;
	uint8_t volume;
	uint16_t repeat_offset;
	uint16_t repeat_length;
} MOD_Sample_Header;
#pragma pack()
ASSERT_STRUCT(MOD_Sample_Header, 30);

/**
 * Amiga Module header
 * All fields are big-endian.
 */
#pragma pack(1)
typedef struct PACKED _MOD_Header {
	char title[20];
	MOD_Sample_Header samples[31]; // TODO: add support for 15-sample modules? (those don't have magic)
	uint8_t pattern_count;
	uint8_t song_end_position;
	uint8_t pattern_table[128];
	char magic[4];
} MOD_Header;
#pragma pack()
ASSERT_STRUCT(MOD_Header, 1084);

#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBROMDATA_MD_STRUCTS_H__ */

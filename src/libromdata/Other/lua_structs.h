/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * lua_structs.h: Lua data structures.                                     *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * Copyright (c) 2016-2020 by Egor.                                        *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_LUA_STRUCTS_H__
#define __ROMPROPERTIES_LIBROMDATA_LUA_STRUCTS_H__

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Lua binary chunk header.
 *
 * References:
 * - lundump.{c,h} from various Lua versions.
 */
typedef struct _Lua_Header {
	char magic[4]; /* '\033Lua' */
	uint8_t version; /* 0x50 = 5.0, 0x51 = 5.1, etc */
} Lua_Header;
ASSERT_STRUCT(Lua_Header, 5);

/**
 * Lua 5.0 binary chunk header.
 */
typedef struct _LuaP_Header {
	Lua_Header header;
	uint8_t endianness; /* 0 = BE, 1 = LE */
	uint8_t int_size;
	uint8_t size_t_size;
	uint8_t Instruction_size;
	uint8_t OP_bits;
	uint8_t A_bits;
	uint8_t B_bits;
	uint8_t C_bits;
	uint8_t Number_size;
	/* followed by a test nubmer, which is 3.14159265358979323846E7 cast to lua_Number */
} LuaP_Header;
ASSERT_STRUCT(LuaP_Header, 14);

#define LUA_MAGIC "\033Lua"
#define LUA_TAIL "\x19\x93\r\n\x1a\n"
#define LUA_TEST_INT32 0x01DF5E76u
#define LUA_TEST_INT64 0x0000000001DF5E76ull
#define LUA_TEST_FLOAT32 0x4BEFAF3Bu
#define LUA_TEST_FLOAT64 0x417DF5E7689309B6ull

/**
 * Lua 5.1 binary chunk header.
 */
typedef struct _LuaQ_Header {
	Lua_Header header;
	uint8_t format; /* 0 = official format */
	uint8_t endianness; /* 0 = BE, 1 = LE */
	uint8_t int_size;
	uint8_t size_t_size;
	uint8_t Instruction_size;
	uint8_t Number_size;
	uint8_t is_integral;
} LuaQ_Header;
ASSERT_STRUCT(LuaQ_Header, 12);

/**
 * Lua 5.2 binary chunk header.
 */
typedef struct _LuaR_Header {
	Lua_Header header;
	uint8_t format; /* 0 = official format */
	uint8_t endianness; /* 0 = BE, 1 = LE */
	uint8_t int_size;
	uint8_t size_t_size;
	uint8_t Instruction_size;
	uint8_t Number_size;
	uint8_t is_integral;
	char tail[6]; /* LUA_TAIL */
} LuaR_Header;
ASSERT_STRUCT(LuaR_Header, 18);

/**
 * Lua 5.3 binary chunk header.
 */
typedef struct _LuaS_Header {
	Lua_Header header;
	uint8_t format; /* 0 = official format */
	char tail[6]; /* LUA_TAIL */
	uint8_t int_size;
	uint8_t size_t_size;
	uint8_t Instruction_size;
	uint8_t Integer_size;
	uint8_t Number_size;
	/* followed by test integer 0x5678 */
	/* followed by test number 370.5 */
} LuaS_Header;
ASSERT_STRUCT(LuaS_Header, 17);

/**
 * Lua 5.4 binary chunk header.
 */
typedef struct _LuaT_Header {
	Lua_Header header;
	uint8_t format; /* 0 = official format */
	char tail[6]; /* LUA_TAIL */
	uint8_t Instruction_size;
	uint8_t Integer_size;
	uint8_t Number_size;
	/* followed by test integer 0x5678 */
	/* followed by test number 370.5 */
} LuaT_Header;
ASSERT_STRUCT(LuaT_Header, 15);
#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBROMDATA_LUA_STRUCTS_H__ */

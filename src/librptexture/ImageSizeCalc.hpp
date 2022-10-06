/***************************************************************************
 * ROM Properties Page shell extension. (librptexture)                     *
 * ImageSizeCalc.hpp: Image size calculation functions.                    *
 *                                                                         *
 * Copyright (c) 2016-2021 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPTEXTURE_IMAGESIZECALC_HPP__
#define __ROMPROPERTIES_LIBRPTEXTURE_IMAGESIZECALC_HPP__

#include "bitstuff.h"

// C includes (C++ namespace)
#include <cassert>

namespace LibRpTexture { namespace ImageSizeCalc {

/**
 * Calculate an image size in bytes from width and height.
 * Helper function to work around int->size_t promotion.
 * @param width
 * @param height
 * @return Size, in bytes
 */
template<typename X, typename Y>
static inline size_t T_calcImageSize(X width, Y height)
{
	assert(width > 0);
	assert(height > 0);
	return static_cast<size_t>(width) * static_cast<size_t>(height);
}

/**
 * Calculate an image size in bytes from width and height.
 * Helper function to work around int->size_t promotion.
 * @param width
 * @param height
 * @param pixelsize sizeof(pixeltype)
 * @return Size, in bytes
 */
template<typename X, typename Y, typename Z>
static inline size_t T_calcImageSize(X width, Y height, Z pixelsize)
{
	assert(width > 0);
	assert(height > 0);
	assert(pixelsize > 0);
	return static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(pixelsize);
}

// OpCode values for calcImageSize_tbl().
enum class OpCode : uint8_t {
	Unknown = 0,
	None,
	Multiply2,
	Multiply3,
	Multiply4,
	Multiply6,
	Multiply8,
	Multiply12,
	Multiply16,
	Divide2,
	Divide4,

	// DXTn requires aligned blocks.
	Align4Divide2,
	Align4,

	// ASTC requires aligned blocks.
	// NOTE: This only works for ASTC_8x8.
	// Other block sizes should use calcImageSizeASTC().
	Align8Divide4,

	Max
};

/**
 * Calculate an image size using the specified format opcode table.
 * @param op_tbl Opcode table
 * @param tbl_size Size of op_tbl (number of entries)
 * @param format Image format ID
 * @param width Image width
 * @param height Image height
 * @return Image size, in bytes
 */
unsigned int calcImageSize_tbl(
	const OpCode *op_tbl, size_t tbl_size,
	unsigned int format, unsigned int width, unsigned int height);

/**
 * Calculate the expected size of a PVRTC-I compressed 2D image.
 * PVRTC-I requires power-of-2 dimensions.
 * @tparam is2bpp True for 2bpp; false for 4bpp.
 * @param width Image width
 * @param height Image height
 * @return Expected size, in bytes
 */
template<bool is2bpp>
static inline unsigned int calcImageSizePVRTC_PoT(int width, int height)
{
	static const int min_width = (is2bpp ? 8 : 4);
	if (width < min_width) {
		width = min_width;
	} else if (!isPow2(width)) {
		width = nextPow2(width);
	}

	if (height < 4) {
		height = 4;
	} else if (!isPow2(height)) {
		height = nextPow2(height);
	}

	return (width * height / (is2bpp ? 4 : 2));
}

/**
 * Validate ASTC block size.
 * @param block_x	[in] Block width
 * @param block_y	[in] Block height
 * @return True if valid; false if not.
 */
static inline bool validateBlockSizeASTC(uint8_t block_x, uint8_t block_y)
{
	assert(block_x >= 4);
	assert(block_x <= 12);
	assert(block_y >= 4);
	assert(block_y <= 12);
	assert(block_x >= block_y);
	if (block_x < 4 || block_x > 12 ||
	    block_y < 4 || block_y > 12 ||
	    block_x < block_y)
	{
		// Invalid block size.
		return false;
	}

	// TODO: Validate combinations?
	return true;
}

/**
 * Align width/height for ASTC.
 * @param width		[in,out] Image width
 * @param height	[in,out] Image height
 * @param block_x	[in] Block width
 * @param block_y	[in] Block height
 */
static inline void alignImageSizeASTC(int& width, int& height, uint8_t block_x, uint8_t block_y)
{
	if (width % block_x != 0) {
		width += (block_x - (width % block_x));
	}
	if (height % block_y != 0) {
		height += (block_y - (height % block_y));
	}
}

/**
 * Calculate the expected size of an ASTC-compressed 2D image.
 * @param width Image width
 * @param height Image height
 * @param block_x Block width
 * @param block_y Block height
 * @return Expected size, in bytes
 */
unsigned int calcImageSizeASTC(int width, int height, uint8_t block_x, uint8_t block_y);

} }

#endif /* __ROMPROPERTIES_LIBRPTEXTURE_IMAGESIZECALC_HPP__ */

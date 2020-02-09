/***************************************************************************
 * ROM Properties Page shell extension. (librptexture)                     *
 * VkEnumStrings.cpp: OpenGL string tables.                                *
 *                                                                         *
 * Copyright (c) 2016-2019 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "VkEnumStrings.hpp"
#include "fileformat/vk_defs.h"

// C includes.
#include <stdlib.h>

namespace LibRpTexture {

class VkEnumStringsPrivate
{
	private:
		// Static class.
		VkEnumStringsPrivate();
		~VkEnumStringsPrivate();
		RP_DISABLE_COPY(VkEnumStringsPrivate)

	public:
		// String tables.
		// NOTE: Must be sorted by id.
		// NOTE: Leaving the "VK_FORMAT_" prefix off of the strings.
		#define STRTBL_ENTRY(x) {VK_FORMAT_##x, #x}
		struct StrTbl_t {
			unsigned int id;
			const char *str;
		};

		/**
		 * Comparison function for bsearch().
		 * @param a
		 * @param b
		 * @return
		 */
		static int RP_C_API compar(const void *a, const void *b);

		/**
		 * VkFormat enumeration.
		 */
		static const StrTbl_t vkFormat_tbl[];
};

/** VkEnumStringsPrivate **/

/**
 * Comparison function for bsearch().
 * @param a
 * @param b
 * @return
 */
int RP_C_API VkEnumStringsPrivate::compar(const void *a, const void *b)
{
	unsigned int id1 = static_cast<const StrTbl_t*>(a)->id;
	unsigned int id2 = static_cast<const StrTbl_t*>(b)->id;
	if (id1 < id2) return -1;
	if (id1 > id2) return 1;
	return 0;
}

/**
 * VkFormat enumeration.
 */
const VkEnumStringsPrivate::StrTbl_t VkEnumStringsPrivate::vkFormat_tbl[] = {
	// TODO: Verify sorting.
	STRTBL_ENTRY(UNDEFINED),
	STRTBL_ENTRY(R4G4_UNORM_PACK8),
	STRTBL_ENTRY(R4G4B4A4_UNORM_PACK16),
	STRTBL_ENTRY(B4G4R4A4_UNORM_PACK16),
	STRTBL_ENTRY(R5G6B5_UNORM_PACK16),
	STRTBL_ENTRY(B5G6R5_UNORM_PACK16),
	STRTBL_ENTRY(R5G5B5A1_UNORM_PACK16),
	STRTBL_ENTRY(B5G5R5A1_UNORM_PACK16),
	STRTBL_ENTRY(A1R5G5B5_UNORM_PACK16),
	STRTBL_ENTRY(R8_UNORM),
	STRTBL_ENTRY(R8_SNORM),
	//STRTBL_ENTRY(R8_USCALED),
	//STRTBL_ENTRY(R8_SSCALED),
	STRTBL_ENTRY(R8_UINT),
	STRTBL_ENTRY(R8_SINT),
	STRTBL_ENTRY(R8_SRGB),
	STRTBL_ENTRY(R8G8_UNORM),
	STRTBL_ENTRY(R8G8_SNORM),
	//STRTBL_ENTRY(R8G8_USCALED),
	//STRTBL_ENTRY(R8G8_SSCALED),
	STRTBL_ENTRY(R8G8_UINT),
	STRTBL_ENTRY(R8G8_SINT),
	STRTBL_ENTRY(R8G8_SRGB),
	STRTBL_ENTRY(R8G8B8_UNORM),
	STRTBL_ENTRY(R8G8B8_SNORM),
	//STRTBL_ENTRY(R8G8B8_USCALED),
	//STRTBL_ENTRY(R8G8B8_SSCALED),
	STRTBL_ENTRY(R8G8B8_UINT),
	STRTBL_ENTRY(R8G8B8_SINT),
	STRTBL_ENTRY(R8G8B8_SRGB),
	STRTBL_ENTRY(B8G8R8_UNORM),
	STRTBL_ENTRY(B8G8R8_SNORM),
	//STRTBL_ENTRY(B8G8R8_USCALED),
	//STRTBL_ENTRY(B8G8R8_SSCALED),
	STRTBL_ENTRY(B8G8R8_UINT),
	STRTBL_ENTRY(B8G8R8_SINT),
	STRTBL_ENTRY(B8G8R8_SRGB),
	STRTBL_ENTRY(R8G8B8A8_UNORM),
	STRTBL_ENTRY(R8G8B8A8_SNORM),
	//STRTBL_ENTRY(R8G8B8A8_USCALED),
	//STRTBL_ENTRY(R8G8B8A8_SSCALED),
	STRTBL_ENTRY(R8G8B8A8_UINT),
	STRTBL_ENTRY(R8G8B8A8_SINT),
	STRTBL_ENTRY(R8G8B8A8_SRGB),
	STRTBL_ENTRY(B8G8R8A8_UNORM),
	STRTBL_ENTRY(B8G8R8A8_SNORM),
	//STRTBL_ENTRY(B8G8R8A8_USCALED),
	//STRTBL_ENTRY(B8G8R8A8_SSCALED),
	STRTBL_ENTRY(B8G8R8A8_UINT),
	STRTBL_ENTRY(B8G8R8A8_SINT),
	STRTBL_ENTRY(B8G8R8A8_SRGB),
	//STRTBL_ENTRY(A8B8G8R8_UNORM_PACK32),
	//STRTBL_ENTRY(A8B8G8R8_SNORM_PACK32),
	//STRTBL_ENTRY(A8B8G8R8_USCALED_PACK32),
	//STRTBL_ENTRY(A8B8G8R8_SSCALED_PACK32),
	//STRTBL_ENTRY(A8B8G8R8_UINT_PACK32),
	//STRTBL_ENTRY(A8B8G8R8_SINT_PACK32),
	//STRTBL_ENTRY(A8B8G8R8_SRGB_PACK32),
	STRTBL_ENTRY(A2R10G10B10_UNORM_PACK32),
	STRTBL_ENTRY(A2R10G10B10_SNORM_PACK32),
	//STRTBL_ENTRY(A2R10G10B10_USCALED_PACK32),
	//STRTBL_ENTRY(A2R10G10B10_SSCALED_PACK32),
	STRTBL_ENTRY(A2R10G10B10_UINT_PACK32),
	STRTBL_ENTRY(A2R10G10B10_SINT_PACK32),
	STRTBL_ENTRY(A2B10G10R10_UNORM_PACK32),
	STRTBL_ENTRY(A2B10G10R10_SNORM_PACK32),
	//STRTBL_ENTRY(A2B10G10R10_USCALED_PACK32),
	//STRTBL_ENTRY(A2B10G10R10_SSCALED_PACK32),
	STRTBL_ENTRY(A2B10G10R10_UINT_PACK32),
	STRTBL_ENTRY(A2B10G10R10_SINT_PACK32),
	STRTBL_ENTRY(R16_UNORM),
	STRTBL_ENTRY(R16_SNORM),
	//STRTBL_ENTRY(R16_USCALED),
	//STRTBL_ENTRY(R16_SSCALED),
	STRTBL_ENTRY(R16_UINT),
	STRTBL_ENTRY(R16_SINT),
	STRTBL_ENTRY(R16_SFLOAT),
	STRTBL_ENTRY(R16G16_UNORM),
	STRTBL_ENTRY(R16G16_SNORM),
	//STRTBL_ENTRY(R16G16_USCALED),
	//STRTBL_ENTRY(R16G16_SSCALED),
	STRTBL_ENTRY(R16G16_UINT),
	STRTBL_ENTRY(R16G16_SINT),
	STRTBL_ENTRY(R16G16_SFLOAT),
	STRTBL_ENTRY(R16G16B16_UNORM),
	STRTBL_ENTRY(R16G16B16_SNORM),
	//STRTBL_ENTRY(R16G16B16_USCALED),
	//STRTBL_ENTRY(R16G16B16_SSCALED),
	STRTBL_ENTRY(R16G16B16_UINT),
	STRTBL_ENTRY(R16G16B16_SINT),
	STRTBL_ENTRY(R16G16B16_SFLOAT),
	STRTBL_ENTRY(R16G16B16A16_UNORM),
	STRTBL_ENTRY(R16G16B16A16_SNORM),
	//STRTBL_ENTRY(R16G16B16A16_USCALED),
	//STRTBL_ENTRY(R16G16B16A16_SSCALED),
	STRTBL_ENTRY(R16G16B16A16_UINT),
	STRTBL_ENTRY(R16G16B16A16_SINT),
	STRTBL_ENTRY(R16G16B16A16_SFLOAT),
	STRTBL_ENTRY(R32_UINT),
	STRTBL_ENTRY(R32_SINT),
	STRTBL_ENTRY(R32_SFLOAT),
	STRTBL_ENTRY(R32G32_UINT),
	STRTBL_ENTRY(R32G32_SINT),
	STRTBL_ENTRY(R32G32_SFLOAT),
	STRTBL_ENTRY(R32G32B32_UINT),
	STRTBL_ENTRY(R32G32B32_SINT),
	STRTBL_ENTRY(R32G32B32_SFLOAT),
	STRTBL_ENTRY(R32G32B32A32_UINT),
	STRTBL_ENTRY(R32G32B32A32_SINT),
	STRTBL_ENTRY(R32G32B32A32_SFLOAT),
	STRTBL_ENTRY(R64_UINT),
	STRTBL_ENTRY(R64_SINT),
	STRTBL_ENTRY(R64_SFLOAT),
	STRTBL_ENTRY(R64G64_UINT),
	STRTBL_ENTRY(R64G64_SINT),
	STRTBL_ENTRY(R64G64_SFLOAT),
	STRTBL_ENTRY(R64G64B64_UINT),
	STRTBL_ENTRY(R64G64B64_SINT),
	STRTBL_ENTRY(R64G64B64_SFLOAT),
	STRTBL_ENTRY(R64G64B64A64_UINT),
	STRTBL_ENTRY(R64G64B64A64_SINT),
	STRTBL_ENTRY(R64G64B64A64_SFLOAT),
	STRTBL_ENTRY(B10G11R11_UFLOAT_PACK32),
	STRTBL_ENTRY(E5B9G9R9_UFLOAT_PACK32),
	STRTBL_ENTRY(D16_UNORM),
	STRTBL_ENTRY(X8_D24_UNORM_PACK32),
	STRTBL_ENTRY(D32_SFLOAT),
	STRTBL_ENTRY(S8_UINT),
	STRTBL_ENTRY(D16_UNORM_S8_UINT),
	STRTBL_ENTRY(D24_UNORM_S8_UINT),
	STRTBL_ENTRY(D32_SFLOAT_S8_UINT),

	STRTBL_ENTRY(BC1_RGB_UNORM_BLOCK),
	STRTBL_ENTRY(BC1_RGB_SRGB_BLOCK),
	STRTBL_ENTRY(BC1_RGBA_UNORM_BLOCK),
	STRTBL_ENTRY(BC1_RGBA_SRGB_BLOCK),
	STRTBL_ENTRY(BC2_UNORM_BLOCK),
	STRTBL_ENTRY(BC2_SRGB_BLOCK),
	STRTBL_ENTRY(BC3_UNORM_BLOCK),
	STRTBL_ENTRY(BC3_SRGB_BLOCK),
	STRTBL_ENTRY(BC4_UNORM_BLOCK),
	STRTBL_ENTRY(BC4_SNORM_BLOCK),
	STRTBL_ENTRY(BC5_UNORM_BLOCK),
	STRTBL_ENTRY(BC5_SNORM_BLOCK),
	STRTBL_ENTRY(BC6H_UFLOAT_BLOCK),
	STRTBL_ENTRY(BC6H_SFLOAT_BLOCK),
	STRTBL_ENTRY(BC7_UNORM_BLOCK),
	STRTBL_ENTRY(BC7_SRGB_BLOCK),

	STRTBL_ENTRY(ETC2_R8G8B8_UNORM_BLOCK),
	STRTBL_ENTRY(ETC2_R8G8B8_SRGB_BLOCK),
	STRTBL_ENTRY(ETC2_R8G8B8A1_UNORM_BLOCK),
	STRTBL_ENTRY(ETC2_R8G8B8A1_SRGB_BLOCK),
	STRTBL_ENTRY(ETC2_R8G8B8A8_UNORM_BLOCK),
	STRTBL_ENTRY(ETC2_R8G8B8A8_SRGB_BLOCK),

	STRTBL_ENTRY(EAC_R11_UNORM_BLOCK),
	STRTBL_ENTRY(EAC_R11_SNORM_BLOCK),
	STRTBL_ENTRY(EAC_R11G11_UNORM_BLOCK),
	STRTBL_ENTRY(EAC_R11G11_SNORM_BLOCK),

	STRTBL_ENTRY(ASTC_4x4_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_4x4_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_5x4_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_5x4_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_5x5_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_5x5_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_6x5_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_6x5_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_6x6_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_6x6_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_8x5_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_8x5_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_8x6_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_8x6_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_8x8_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_8x8_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_10x5_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_10x5_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_10x6_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_10x6_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_10x8_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_10x8_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_10x10_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_10x10_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_12x10_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_12x10_SRGB_BLOCK),
	STRTBL_ENTRY(ASTC_12x12_UNORM_BLOCK),
	STRTBL_ENTRY(ASTC_12x12_SRGB_BLOCK),

	STRTBL_ENTRY(PVRTC1_2BPP_UNORM_BLOCK_IMG),
	STRTBL_ENTRY(PVRTC1_4BPP_UNORM_BLOCK_IMG),
	STRTBL_ENTRY(PVRTC2_2BPP_UNORM_BLOCK_IMG),
	STRTBL_ENTRY(PVRTC2_4BPP_UNORM_BLOCK_IMG),
	STRTBL_ENTRY(PVRTC1_2BPP_SRGB_BLOCK_IMG),
	STRTBL_ENTRY(PVRTC1_4BPP_SRGB_BLOCK_IMG),
	STRTBL_ENTRY(PVRTC2_2BPP_SRGB_BLOCK_IMG),
	STRTBL_ENTRY(PVRTC2_4BPP_SRGB_BLOCK_IMG),

	STRTBL_ENTRY(ASTC_4x4_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_5x4_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_5x5_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_6x5_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_6x6_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_8x5_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_8x6_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_8x8_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_10x5_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_10x6_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_10x8_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_10x10_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_12x10_SFLOAT_BLOCK_EXT),
	STRTBL_ENTRY(ASTC_12x12_SFLOAT_BLOCK_EXT),

	// NOTE: In vulkan_core.h, this is located *before* the PVRTC
	// constants, which is out of order. It should be located here
	// for binary-search functionality.
	STRTBL_ENTRY(G8B8G8R8_422_UNORM),
	STRTBL_ENTRY(B8G8R8G8_422_UNORM),
	STRTBL_ENTRY(G8_B8_R8_3PLANE_420_UNORM),
	STRTBL_ENTRY(G8_B8R8_2PLANE_420_UNORM),
	STRTBL_ENTRY(G8_B8_R8_3PLANE_422_UNORM),
	STRTBL_ENTRY(G8_B8R8_2PLANE_422_UNORM),
	STRTBL_ENTRY(G8_B8_R8_3PLANE_444_UNORM),
	STRTBL_ENTRY(R10X6_UNORM_PACK16),
	STRTBL_ENTRY(R10X6G10X6_UNORM_2PACK16),
	STRTBL_ENTRY(R10X6G10X6B10X6A10X6_UNORM_4PACK16),
	STRTBL_ENTRY(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16),
	STRTBL_ENTRY(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16),
	STRTBL_ENTRY(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16),
	STRTBL_ENTRY(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16),
	STRTBL_ENTRY(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16),
	STRTBL_ENTRY(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16),
	STRTBL_ENTRY(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16),
	STRTBL_ENTRY(R12X4_UNORM_PACK16),
	STRTBL_ENTRY(R12X4G12X4_UNORM_2PACK16),
	STRTBL_ENTRY(R12X4G12X4B12X4A12X4_UNORM_4PACK16),
	STRTBL_ENTRY(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16),
	STRTBL_ENTRY(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16),
	STRTBL_ENTRY(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16),
	STRTBL_ENTRY(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16),
	STRTBL_ENTRY(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16),
	STRTBL_ENTRY(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16),
	STRTBL_ENTRY(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16),
	STRTBL_ENTRY(G16B16G16R16_422_UNORM),
	STRTBL_ENTRY(B16G16R16G16_422_UNORM),
	STRTBL_ENTRY(G16_B16_R16_3PLANE_420_UNORM),
	STRTBL_ENTRY(G16_B16R16_2PLANE_420_UNORM),
	STRTBL_ENTRY(G16_B16_R16_3PLANE_422_UNORM),
	STRTBL_ENTRY(G16_B16R16_2PLANE_422_UNORM),
	STRTBL_ENTRY(G16_B16_R16_3PLANE_444_UNORM),

	{0, nullptr}
};

/** VkEnumStrings **/

/**
 * Look up a Vulkan VkFormat enum string.
 * @param vkFormat	[in] vkFormat enum
 * @return String, or nullptr if not found.
 */
const char *VkEnumStrings::lookup_vkFormat(unsigned int vkFormat)
{
	// Do a binary search.
	const VkEnumStringsPrivate::StrTbl_t key = {vkFormat, nullptr};
	const VkEnumStringsPrivate::StrTbl_t *res =
		static_cast<const VkEnumStringsPrivate::StrTbl_t*>(bsearch(&key,
			VkEnumStringsPrivate::vkFormat_tbl,
			ARRAY_SIZE(VkEnumStringsPrivate::vkFormat_tbl)-1,
			sizeof(VkEnumStringsPrivate::StrTbl_t),
			VkEnumStringsPrivate::compar));

	return (res ? res->str : 0);
}

}

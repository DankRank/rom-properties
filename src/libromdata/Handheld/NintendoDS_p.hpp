/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * NintendoDS.hpp: Nintendo DS(i) ROM reader. (Private class)              *
 *                                                                         *
 * Copyright (c) 2016-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_NINTENDODS_P_HPP__
#define __ROMPROPERTIES_LIBROMDATA_NINTENDODS_P_HPP__

#include "common.h"
#include "nds_structs.h"

// C++ includes.
#include <vector>

// librpbase, librpfile, librptexture
#include "librpbase/RomData_p.hpp"
#include "librpbase/RomFields.hpp"
#include "librptexture/img/rp_image.hpp"

namespace LibRpBase {
	struct IconAnimData;
}
namespace LibRpFile {
	class IRpFile;
}

namespace LibRomData {

class NintendoDSPrivate final : public LibRpBase::RomDataPrivate
{
	public:
		NintendoDSPrivate(NintendoDS *q, LibRpFile::IRpFile *file, bool cia);
		virtual ~NintendoDSPrivate();

	private:
		typedef LibRpBase::RomDataPrivate super;
		RP_DISABLE_COPY(NintendoDSPrivate)

	public:
		/** RomDataInfo **/
		static const char *const exts[];
		static const char *const mimeTypes[];
		static const LibRpBase::RomDataInfo romDataInfo;

	public:
		// Animated icon data.
		// This class owns all of the icons in here, so we
		// must delete all of them.
		LibRpBase::IconAnimData *iconAnimData;

		// Pointer to the first frame in iconAnimData.
		// Used when showing a static icon.
		const LibRpTexture::rp_image *icon_first_frame;

	public:
		/** RomFields **/

		// Hardware type. (RFT_BITFIELD)
		enum NDS_HWType {
			DS_HW_DS	= (1U << 0),
			DS_HW_DSi	= (1U << 1),
		};

		// DS region. (RFT_BITFIELD)
		enum NDS_Region {
			NDS_REGION_FREE		= (1U << 0),
			NDS_REGION_SKOREA	= (1U << 1),
			NDS_REGION_CHINA	= (1U << 2),
		};

		// Security data.
		enum NDS_SecurityData {
			// Blowfish tables. Based on the game code.
			NDS_SECDATA_BLOWFISH	= (1U << 0),

			// Static data.
			NDS_SECDATA_STATIC	= (1U << 1),

			// "Random" data. Algorithm is unknown.
			NDS_SECDATA_RANDOM	= (1U << 2),
		};

		// Secure Area type.
		enum NDS_SecureArea {
			NDS_SECAREA_UNKNOWN	= 0,	// Unknown
			NDS_SECAREA_HOMEBREW	= 1,	// No secure area
			NDS_SECAREA_MULTIBOOT	= 2,	// Multiboot
			NDS_SECAREA_DECRYPTED	= 3,	// Decrypted
			NDS_SECAREA_ENCRYPTED	= 4,	// Encrypted
		};

	public:
		// ROM type.
		enum class RomType {
			Unknown	= -1,

			NDS		= 0,	// Nintendo DS ROM
			NDS_Slot2	= 1,	// Nintendo DS ROM (Slot-2)
			DSi_Enhanced	= 2,	// Nintendo DSi-enhanced ROM
			DSi_Exclusive	= 3,	// Nintendo DSi-exclusive ROM

			Max
		};
		RomType romType;

		// ROM header.
		// NOTE: Must be byteswapped on access.
		NDS_RomHeader romHeader;

		// Cached ROM size to determine trimmed or untrimmed.
		off64_t romSize;

		// Secure Area status.
		uint32_t secData;
		NDS_SecureArea secArea;

		// Icon/title data from the ROM header.
		// NOTE: Must be byteswapped on access.
		NDS_IconTitleData nds_icon_title;
		bool nds_icon_title_loaded;

		// If true, this is an SRL in a 3DS CIA.
		// Some fields shouldn't be displayed.
		bool cia;

		// Field indexes for ROM operations.
		int fieldIdx_secData;	// "Security Data" (RFT_BITFIELD)
		int fieldIdx_secArea;	// "Secure Area" (RFT_STRING)

		/**
		 * Load the icon/title data.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		int loadIconTitleData(void);

		/**
		 * Load the ROM image's icon.
		 * @return Icon, or nullptr on error.
		 */
		const LibRpTexture::rp_image *loadIcon(void);

		/**
		 * Get the title index.
		 * The title that most closely matches the
		 * host system language will be selected.
		 * @return Title index, or -1 on error.
		 */
		int getTitleIndex(void) const;

		/**
		 * Get the total used ROM size as indicated from the ROM header.
		 * @return Total used ROM size, in bytes.
		 */
		inline uint32_t totalUsedRomSize(void) const
		{
			if (likely(romType < RomType::DSi_Enhanced)) {
				// NDS ROM. Return the NDS total used ROM size.
				// NOTE: 0x88 is added for the "cloneplay" RSA key.
				// References:
				// - https://github.com/d0k3/GodMode9/issues/721
				// - https://github.com/DS-Homebrew/GodMode9i/commit/43f440c9fa449ac953ad27798df5b31b2b903157
				// - https://github.com/DS-Homebrew/nds-bootstrap/commit/24243ff4ad6a9bf9c47c16b3e285dc85266b9372
				// - https://github.com/DS-Homebrew/nds-bootstrap/releases/tag/v0.44.2
				const uint32_t nds_rom_size = le32_to_cpu(romHeader.total_used_rom_size) + 0x88;
				return ((off_t)nds_rom_size < this->romSize
					? nds_rom_size
					: static_cast<uint32_t>(this->romSize));
			}

			// DSi ROM. Return the DSi total used ROM size.
			// NOTE: "cloneplay" RSA key is included in here.
			return le32_to_cpu(romHeader.dsi.total_used_rom_size);
		}

		/**
		 * Is the ROM trimmed?
		 * @return True if trimmed; false if not trimmed.
		 */
		inline bool isRomTrimmed(void) const
		{
			return !(totalUsedRomSize() < this->romSize);
		}

		/**
		 * Check the NDS security data.
		 *
		 * $1000-$3FFF is normally unreadable on hardware, so this
		 * area is usually blank in dumped ROMs. However, this area
		 * normally has precomputed Blowfish tables and other data,
		 * which are used as part of the NDS security system.
		 * DSiWare and Wii U VC SRLs, as well as SRLs generated by
		 * the DS SDK, will have actual data here.
		 *
		 * @return NDS security data flags.
		 */
		uint32_t checkNDSSecurityData(void);

		/**
		 * Check the NDS Secure Area type.
		 * This reads from the ROM, so the ROM must be open.
		 * @return Secure area type.
		 */
		NDS_SecureArea checkNDSSecureArea(void);

		/**
		 * Get the localized string identifying the NDS Secure Area type.
		 * This uses the cached secArea value.
		 * @return NDS Secure Area type string.
		 */
		const char *getNDSSecureAreaString(void);

		/**
		 * Convert a Nintendo DS(i) region value to a GameTDB language code.
		 * @param ndsRegion Nintendo DS region.
		 * @param dsiRegion Nintendo DSi region.
		 * @param idRegion Game ID region.
		 *
		 * NOTE: Mulitple GameTDB language codes may be returned, including:
		 * - User-specified fallback language code for PAL.
		 * - General fallback language code.
		 *
		 * @return GameTDB language code(s), or empty vector if the region value is invalid.
		 * NOTE: The language code may need to be converted to uppercase!
		 */
		static std::vector<uint16_t> ndsRegionToGameTDB(
			uint8_t ndsRegion, uint32_t dsiRegion, char idRegion);

		/**
		 * Get the DSi flags string vector.
		 * @return DSi flags string vector.
		 */
		static LibRpBase::RomFields::ListData_t *getDSiFlagsStringVector(void);

		/**
		 * Get the maximum supported language for an icon/title version.
		 * @param version Icon/title version.
		 * @return Maximum supported language.
		 */
		static NDS_Language_ID getMaxSupportedLanguage(uint16_t version);

		/**
		 * Get the language ID to use for the title fields.
		 * @return NDS language ID.
		 */
		NDS_Language_ID getLanguageID(void) const;

		/**
		 * Get the default language code for the multi-string fields.
		 * @return Language code, e.g. 'en' or 'es'.
		 */
		inline uint32_t getDefaultLC(void) const;
};

}

#endif /* __ROMPROPERTIES_LIBROMDATA_NINTENDODS_P_HPP__ */

/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * NESMappers.cpp: NES mapper data.                                        *
 *                                                                         *
 * Copyright (c) 2016-2021 by David Korth.                                 *
 * Copyright (c) 2016-2018 by Egor.                                        *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "NESMappers.hpp"

namespace LibRomData {

/**
 * References:
 * - https://wiki.nesdev.com/w/index.php/Mapper
 * - https://wiki.nesdev.com/w/index.php/NES_2.0_submappers
 */

// Mirroring behaviors for different mappers
enum NESMirroringType {
	MIRRORING_UNKNOWN = 0,		// When submapper has this value, it inherits mapper's value
	// NOTE: for all of these we assume that if 4-Screen bit is set it means that the mapper's
	// logic gets ignored, and there's simply 4K of SRAM at $2000. For more complicated mappers
	// (MMC5) this would actually be a downgrade, and maybe even impossible, but iNES format
	// applies the same logic to all mappers (except 30 and 218, see below)
	// Reference: http://wiki.nesdev.com/w/index.php/NES_2.0#Hard-Wired_Mirroring
	// NOTE: H/V/A/B refers to CIRAM A10 being connected to PPU A11/A10/Vss/Vdd respectively
	// NOTE: boards that only ever existed in H or V configuration still use the H/V bit.
	MIRRORING_HEADER,		// fixed H/V (the default)
	MIRRORING_MAPPER,		// Mapper-controlled (unspecified)
	MIRRORING_MAPPER_HVAB,		// - switchable H/V/A/B (e.g. MMC1)
	MIRRORING_MAPPER_HV,		// - switchable H/V     (e.g. MMC3)
	MIRRORING_MAPPER_AB,		// - switchable A/B     (e.g. AxROM)
	MIRRORING_MAPPER_MMC5,		// - arbitrary configuration with 3 NTs and fill mode
	MIRRORING_MAPPER_NAMCO163,	// - arbitrary configuration with 2 RAM and 224 ROM NTs
	MIRRORING_MAPPER_VRC6,		// - it's complicated (Konami games only use H/V/A/B)
	MIRRORING_MAPPER_JY,		// - J.Y. Company ASIC mapper (also complicated)
	MIRRORING_MAPPER_SUNSOFT4,	// - switchable H/V/A/B with 2 RAM and 128 ROM NTs
	MIRRORING_MAPPER_NAMCOT3425,	// - H but you can select how PPU A11 maps to CIRAM A10
					//   (effectively it's selectable H/A/B/swapped-H)
	MIRRORING_MAPPER_GTROM,		// - paged 4 screen RAM
	MIRRORING_MAPPER_TxSROM,	// - arbitrary configuration with 2 NTs
	MIRRORING_MAPPER_SACHEN8259,	// - switchable H/V/A/L-shaped (A10 or A11)
	MIRRORING_MAPPER_SACHEN74LS374N,// - switchable H/V/A/L-shaped (A10 and A11)
	MIRRORING_MAPPER_DIS23C01,	// - switchable H/V. A on reset.
	MIRRORING_MAPPER_233,		// - switchable H/V/B/L-shaped (A10 and A11)
	MIRRORING_MAPPER_235,		// - switchable H/V/A
	MIRRORING_1SCREEN_A,		// fixed A
	MIRRORING_1SCREEN_B,		// fixed B
					// (the distinction is only relevant for Magic Floor)
	MIRRORING_4SCREEN,		// 4 screen regardless of header (e.g. Vs. System)
	// The following mappers interpret the header bits differently
	MIRRORING_UNROM512,		// fixed H/V/4 or switchable A/B (mapper 30)
	MIRRORING_BANDAI_FAMILYTRAINER,	// fixed H/V or switchable A/B (mapper 70) (see note below)
	MIRRORING_MAGICFLOOR,		// fixed H/V/A/B (mapper 218)

	// NOTE: fwNES describes mappers 70 and 78 as using 4 Screen bit to specify switchable A/B
	// - 70 normally has fixed H/V. For switchable A/B, 152 should be used instead.
	// - 78 has either switchable H/V or switchable A/B. Emulators default to one of those,
	// and use checksumming to detect the other. Submappers should be used instead.
};

class NESMappersPrivate
{
	private:
		// Static class.
		NESMappersPrivate();
		~NESMappersPrivate();
		RP_DISABLE_COPY(NESMappersPrivate)

	public:

		// iNES mapper list.
		struct MapperEntry {
			const char *name;		// Name of the board. (If unknown, nullptr.)
			const char *manufacturer;	// Manufacturer. (If unknown, nullptr.)
			NESMirroringType mirroring;	// Mirroring behavior.
		};
		static const MapperEntry mappers_plane0[];
		static const MapperEntry mappers_plane1[];
		static const MapperEntry mappers_plane2[];

		/**
		 * NES 2.0 submapper information.
		 *
		 * `deprecated` can be one of the following:
		 * - 0: Not deprecated.
		 * - >0: Deprecated, and this is the replacement mapper number.
		 * - 0xFFFF: Deprecated, no replacement.
		 *
		 * A submapper may be deprecated in favor of submapper 0, in which
		 * case the `deprecated` value is equal to that mapper's number.
		 * It is assumed that the replacement mapper always uses submapper 0.
		 */
		struct SubmapperInfo {
			uint8_t submapper;		// Submapper number.
			uint8_t reserved;
			uint16_t deprecated;
			const char *desc;		// Description.
			NESMirroringType mirroring;	// Mirroring behavior.
		};

		// Submappers.
		static const struct SubmapperInfo mmc1_submappers[];		// 001
		static const struct SubmapperInfo discrete_logic_submappers[];	// 002, 003, 007
		static const struct SubmapperInfo mmc3_submappers[];		// 004
		static const struct SubmapperInfo bandai_fcgx_submappers[];	// 016
		static const struct SubmapperInfo namco_129_164_submappers[];	// 019
		static const struct SubmapperInfo vrc4a_vrc4c_submappers[];	// 021
		static const struct SubmapperInfo vrc4ef_vrc2b_submappers[];	// 023
		static const struct SubmapperInfo vrc4bd_vrc2c_submappers[];	// 025
		static const struct SubmapperInfo irem_g101_submappers[];	// 032
		static const struct SubmapperInfo bnrom_nina001_submappers[];	// 034
		static const struct SubmapperInfo sunsoft4_submappers[];	// 068
		static const struct SubmapperInfo codemasters_submappers[];	// 071
		static const struct SubmapperInfo mapper078_submappers[];	// 078
		static const struct SubmapperInfo cony_yoko_submappers[];	// 083
		static const struct SubmapperInfo mapper114_submappers[];	// 114
		static const struct SubmapperInfo mapper197_submappers[];	// 197
		static const struct SubmapperInfo namcot_175_340_submappers[];	// 210
		static const struct SubmapperInfo sugar_softec_submappers[];	// 215
		static const struct SubmapperInfo quattro_submappers[];		// 232
		static const struct SubmapperInfo onebus_submappers[];		// 256
		static const struct SubmapperInfo smd132_smd133_submappers[];	// 268
		static const struct SubmapperInfo mapper313_submappers[];	// 313

		/**
		 * NES 2.0 submapper list.
		 *
		 * NOTE: Submapper 0 is optional, since it's the default behavior.
		 * It may be included if NES 2.0 submapper 0 acts differently from
		 * iNES mappers.
		 */
		struct SubmapperEntry {
			uint16_t mapper;		// Mapper number.
			uint16_t info_size;		// Number of entries in info.
			const SubmapperInfo *info;	// Submapper information.
		};
		static const SubmapperEntry submappers[];

		/**
		 * bsearch() comparison function for SubmapperInfo.
		 * @param a
		 * @param b
		 * @return
		 */
		static int RP_C_API SubmapperInfo_compar(const void *a, const void *b);

		/**
		 * bsearch() comparison function for SubmapperEntry.
		 * @param a
		 * @param b
		 * @return
		 */
		static int RP_C_API SubmapperEntry_compar(const void *a, const void *b);
};

/**
 * Mappers: NES 2.0 Plane 0 [000-255] (iNES 1.0)
 * TODO: Add more fields:
 * - Programmable mirroring
 * - Extra VRAM for 4 screens
 */
const NESMappersPrivate::MapperEntry NESMappersPrivate::mappers_plane0[] = {
	/** NES 2.0 Plane 0 [0-255] (iNES 1.0) **/

	// Mappers 000-009
	{"NROM",			"Nintendo",		MIRRORING_HEADER},
	{"SxROM (MMC1)",		"Nintendo",		MIRRORING_MAPPER_HVAB},
	{"UxROM",			"Nintendo",		MIRRORING_HEADER},
	{"CNROM",			"Nintendo",		MIRRORING_HEADER},
	{"TxROM (MMC3), HKROM (MMC6)",	"Nintendo",		MIRRORING_MAPPER_HV},
	{"ExROM (MMC5)",		"Nintendo",		MIRRORING_MAPPER_MMC5},
	{"Game Doctor Mode 1",		"Bung/FFE",		MIRRORING_MAPPER_HVAB},
	{"AxROM",			"Nintendo",		MIRRORING_MAPPER_AB},
	{"Game Doctor Mode 4 (GxROM)",	"Bung/FFE",		MIRRORING_MAPPER_HVAB},
	{"PxROM, PEEOROM (MMC2)",	"Nintendo",		MIRRORING_MAPPER_HV},

	// Mappers 010-019
	{"FxROM (MMC4)",		"Nintendo",		MIRRORING_MAPPER_HV},
	{"Color Dreams",		"Color Dreams",		MIRRORING_HEADER},
	{"MMC3 variant",		"FFE",			MIRRORING_MAPPER_HV},
	{"NES-CPROM",			"Nintendo",		MIRRORING_HEADER},
	{"SL-1632 (MMC3/VRC2 clone)",	"Nintendo",		MIRRORING_MAPPER_HVAB},
	{"K-1029 (multicart)",		nullptr,		MIRRORING_MAPPER_HV},
	{"FCG-x",			"Bandai",		MIRRORING_MAPPER_HVAB},
	{"FFE #17",			"FFE",			MIRRORING_MAPPER_HVAB},
	{"SS 88006",			"Jaleco",		MIRRORING_MAPPER_HVAB},
	{"Namco 129/163",		"Namco",		MIRRORING_MAPPER_NAMCO163},	// TODO: Namcot-106?

	// Mappers 020-029
	{"Famicom Disk System",		"Nintendo",		MIRRORING_MAPPER_HV}, // this isn't actually used, as FDS roms are stored in their own format.
	{"VRC4a, VRC4c",		"Konami",		MIRRORING_MAPPER_HVAB},
	{"VRC2a",			"Konami",		MIRRORING_MAPPER_HVAB},
	{"VRC4e, VRC4f, VRC2b",		"Konami",		MIRRORING_MAPPER_HVAB},
	{"VRC6a",			"Konami",		MIRRORING_MAPPER_VRC6},
	{"VRC4b, VRC4d, VRC2c",		"Konami",		MIRRORING_MAPPER_HVAB},
	{"VRC6b",			"Konami",		MIRRORING_MAPPER_VRC6},
	{"VRC4 variant",		nullptr,		MIRRORING_MAPPER_HVAB}, //investigate
	{"Action 53",			"Homebrew",		MIRRORING_MAPPER_HVAB},
	{"RET-CUFROM",			"Sealie Computing",	MIRRORING_HEADER},	// Homebrew

	// Mappers 030-039
	{"UNROM 512",			"RetroUSB",		MIRRORING_UNROM512},	// Homebrew
	{"NSF Music Compilation",	"Homebrew",		MIRRORING_HEADER},
	{"Irem G-101",			"Irem",			MIRRORING_MAPPER_HV /* see submapper */},
	{"Taito TC0190",		"Taito",		MIRRORING_MAPPER_HV},
	{"BNROM, NINA-001",		nullptr,		MIRRORING_HEADER},
	{"J.Y. Company ASIC (8 KiB WRAM)", "J.Y. Company",	MIRRORING_MAPPER_JY},
	{"TXC PCB 01-22000-400",	"TXC",			MIRRORING_MAPPER_HV},
	{"MMC3 multicart",		"Nintendo",		MIRRORING_MAPPER_HV},
	{"GNROM variant",		"Bit Corp.",		MIRRORING_HEADER},
	{"BNROM variant",		nullptr,		MIRRORING_HEADER},

	// Mappers 040-049
	{"NTDEC 2722 (FDS conversion)",	"NTDEC",		MIRRORING_HEADER},
	{"Caltron 6-in-1",		"Caltron",		MIRRORING_MAPPER_HV},
	{"FDS conversion",		nullptr,		MIRRORING_MAPPER_HV},
	{"TONY-I, YS-612 (FDS conversion)", nullptr,		MIRRORING_HEADER},
	{"MMC3 multicart",		nullptr,		MIRRORING_MAPPER_HV},
	{"MMC3 multicart (GA23C)",	nullptr,		MIRRORING_MAPPER_HV},
	{"Rumble Station 15-in-1",	"Color Dreams",		MIRRORING_HEADER},	// NES-on-a-Chip
	{"MMC3 multicart",		"Nintendo",		MIRRORING_MAPPER_HV},
	{"Taito TC0690",		"Taito",		MIRRORING_MAPPER_HV},	// TODO: Taito-TC190V?
	{"MMC3 multicart",		nullptr,		MIRRORING_MAPPER_HV},

	// Mappers 050-059
	{"PCB 761214 (FDS conversion)",	"N-32",			MIRRORING_HEADER},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"MMC3 multicart",		nullptr,		MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Novel Diamond 9999999-in-1",	nullptr,		MIRRORING_UNKNOWN},	// conflicting information
	{"BTL-MARIO1-MALEE2",		nullptr,		MIRRORING_HEADER},	// From UNIF
	{"KS202 (unlicensed SMB3 reproduction)", nullptr,	MIRRORING_MAPPER_HV},	// Some SMB3 unlicensed reproduction
	{"Multicart",			nullptr,		MIRRORING_MAPPER_HV},
	{"(C)NROM-based multicart",	nullptr,		MIRRORING_MAPPER_HV},
	{"BMC-T3H53/BMC-D1038 multicart", nullptr,		MIRRORING_MAPPER_HV},	// From UNIF

	// Mappers 060-069
	{"Reset-based NROM-128 4-in-1 multicart", nullptr,	MIRRORING_HEADER},
	{"20-in-1 multicart",		nullptr,		MIRRORING_MAPPER_HV},
	{"Super 700-in-1 multicart",	nullptr,		MIRRORING_MAPPER_HV},
	{"Powerful 250-in-1 multicart",	"NTDEC",		MIRRORING_MAPPER_HV},
	{"Tengen RAMBO-1",		"Tengen",		MIRRORING_MAPPER_HV},
	{"Irem H3001",			"Irem",			MIRRORING_MAPPER_HV},
	{"GxROM, MHROM",		"Nintendo",		MIRRORING_HEADER},
	{"Sunsoft-3",			"Sunsoft",		MIRRORING_MAPPER_HVAB},
	{"Sunsoft-4",			"Sunsoft",		MIRRORING_MAPPER_SUNSOFT4},
	{"Sunsoft FME-7",		"Sunsoft",		MIRRORING_MAPPER_HVAB},

	// Mappers 070-079
	{"Family Trainer",		"Bandai",		MIRRORING_HEADER /* see wiki for a caveat */},
	{"Codemasters (UNROM clone)",	"Codemasters",		MIRRORING_HEADER /* see submapper */},
	{"Jaleco JF-17",		"Jaleco",		MIRRORING_HEADER},	// TODO: Jaleco-2?
	{"VRC3",			"Konami",		MIRRORING_HEADER},
	{"43-393/860908C (MMC3 clone)",	"Waixing",		MIRRORING_MAPPER_HV},
	{"VRC1",			"Konami",		MIRRORING_MAPPER_HV},
	{"NAMCOT-3446 (Namcot 108 variant)",	"Namco",	MIRRORING_HEADER},	// TODO: Namco-109?
	{"Napoleon Senki",		"Lenar",		MIRRORING_4SCREEN},	// TODO: Irem-1? 
	{"Holy Diver; Uchuusen - Cosmo Carrier", nullptr,	MIRRORING_MAPPER /* see submapper */},	// TODO: Irem-74HC161?
	{"NINA-03, NINA-06",		"American Video Entertainment", MIRRORING_HEADER},

	// Mappers 080-089
	{"Taito X1-005",		"Taito",		MIRRORING_MAPPER_HV},
	{"Super Gun",			"NTDEC",		MIRRORING_HEADER},
	{"Taito X1-017 (incorrect PRG ROM bank ordering)", "Taito", MIRRORING_MAPPER_HV},
	{"Cony/Yoko",			"Cony/Yoko",		MIRRORING_MAPPER_HVAB},
	{"PC-SMB2J",			nullptr,		MIRRORING_UNKNOWN},
	{"VRC7",			"Konami",		MIRRORING_MAPPER_HVAB},
	{"Jaleco JF-13",		"Jaleco",		MIRRORING_HEADER},	// TODO: Jaleco-4?
	{"CNROM variant",		nullptr,		MIRRORING_HEADER},	// TODO: Jaleco-1?
	{"Namcot 118 variant",		nullptr,		MIRRORING_HEADER},	// TODO: Namco-118?
	{"Sunsoft-2 (Sunsoft-3 board)",	"Sunsoft",		MIRRORING_MAPPER_AB},

	// Mappers 090-099
	{"J.Y. Company (simple nametable control)", "J.Y. Company", MIRRORING_MAPPER_HVAB},
	{"J.Y. Company (Super Fighter III)", "J.Y. Company",	MIRRORING_MAPPER_HV /* see submapper */},
	{"Moero!! Pro",			"Jaleco",		MIRRORING_HEADER},	// TODO: Jaleco-3?
	{"Sunsoft-2 (Sunsoft-3R board)", "Sunsoft",		MIRRORING_HEADER},	// TODO: 74161A?
	{"HVC-UN1ROM",			"Nintendo",		MIRRORING_HEADER},	// TODO: 74161B?
	{"NAMCOT-3425",			"Namco",		MIRRORING_MAPPER_NAMCOT3425},	// TODO: Namcot?
	{"Oeka Kids",			"Bandai",		MIRRORING_HEADER},
	{"Irem TAM-S1",			"Irem",			MIRRORING_MAPPER_HV},	// TODO: Irem-2?
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"CNROM (Vs. System)",		"Nintendo",		MIRRORING_4SCREEN},

	// Mappers 100-109
	{"MMC3 variant (hacked ROMs)",	nullptr,		MIRRORING_MAPPER_HV},	// Also used for UNIF
	{"Jaleco JF-10 (misdump)",	"Jaleceo",		MIRRORING_HEADER},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Doki Doki Panic (FDS conversion)", nullptr,		MIRRORING_MAPPER_HV},
	{"PEGASUS 5 IN 1",		nullptr,		MIRRORING_HEADER},
	{"NES-EVENT (MMC1 variant) (Nintendo World Championships 1990)", "Nintendo", MIRRORING_MAPPER_HVAB},
	{"Super Mario Bros. 3 (bootleg)", nullptr,		MIRRORING_MAPPER_HV},
	{"Magic Dragon",		"Magicseries",		MIRRORING_HEADER},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},

	// Mappers 110-119
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Cheapocabra GTROM 512k flash board", "Membler Industries", MIRRORING_MAPPER_GTROM},	// Homebrew
	{"Namcot 118 variant",		nullptr,		MIRRORING_MAPPER_HV},
	{"NINA-03/06 multicart",	nullptr,		MIRRORING_MAPPER_HV},
	{"MMC3 clone (scrambled registers)", nullptr,		MIRRORING_MAPPER_HV},
	{"Kǎshèng SFC-02B/-03/-004 (MMC3 clone)", "Kǎshèng",	MIRRORING_MAPPER_HV},
	{"SOMARI-P (Huang-1/Huang-2)",	"Gouder",		MIRRORING_MAPPER_HVAB},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"TxSROM",			"Nintendo",		MIRRORING_MAPPER_TxSROM},	// TODO: MMC-3+TLS?
	{"TQROM",			"Nintendo",		MIRRORING_MAPPER_HV},

	// Mappers 120-129
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Kǎshèng A9711 and A9713 (MMC3 clone)", "Kǎshèng",	MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Kǎshèng H2288 (MMC3 clone)",	"Kǎshèng",		MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Monty no Doki Doki Daisassō (FDS conversion)", "Whirlwind Manu", MIRRORING_HEADER},
	{nullptr,			nullptr,		MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},

	// Mappers 130-139
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"TXC 05-00002-010 ASIC",	"TXC",			MIRRORING_HEADER},
	{"Jovial Race",			"Sachen",		MIRRORING_HEADER},
	{"T4A54A, WX-KB4K, BS-5652 (MMC3 clone)", nullptr,	MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Sachen 3011",			"Sachen",		MIRRORING_HEADER},
	{"Sachen 8259D",		"Sachen",		MIRRORING_MAPPER_SACHEN8259},
	{"Sachen 8259B",		"Sachen",		MIRRORING_MAPPER_SACHEN8259},
	{"Sachen 8259C",		"Sachen",		MIRRORING_MAPPER_SACHEN8259},

	// Mappers 140-149
	{"Jaleco JF-11, JF-14 (GNROM variant)", "Jaleco",	MIRRORING_HEADER},
	{"Sachen 8259A",		"Sachen",		MIRRORING_MAPPER_SACHEN8259},
	{"Kaiser KS202 (FDS conversions)", "Kaiser",		MIRRORING_HEADER},
	{"Copy-protected NROM",		nullptr,		MIRRORING_HEADER},
	{"Death Race (Color Dreams variant)", "American Game Cartridges", MIRRORING_HEADER},
	{"Sidewinder (CNROM clone)",	"Sachen",		MIRRORING_HEADER},
	{"Galactic Crusader (NINA-06 clone)", nullptr,		MIRRORING_HEADER},
	{"Sachen 3018",			"Sachen",		MIRRORING_HEADER},
	{"Sachen SA-008-A, Tengen 800008",	"Sachen / Tengen", MIRRORING_HEADER},
	{"SA-0036 (CNROM clone)",	"Sachen",		MIRRORING_HEADER},

	// Mappers 150-159
	{"Sachen SA-015, SA-630",	"Sachen",		MIRRORING_MAPPER_SACHEN74LS374N},
	{"VRC1 (Vs. System)",		"Konami",		MIRRORING_4SCREEN},
	{"Kaiser KS202 (FDS conversion)", "Kaiser",		MIRRORING_MAPPER_AB},
	{"Bandai FCG: LZ93D50 with SRAM", "Bandai",		MIRRORING_MAPPER_HVAB},
	{"NAMCOT-3453",			"Namco",		MIRRORING_MAPPER_AB},
	{"MMC1A",			"Nintendo",		MIRRORING_MAPPER_HVAB},
	{"DIS23C01",			"Daou Infosys",		MIRRORING_MAPPER_DIS23C01},
	{"Datach Joint ROM System",	"Bandai",		MIRRORING_MAPPER_HVAB},
	{"Tengen 800037",		"Tengen",		MIRRORING_MAPPER_TxSROM},
	{"Bandai LZ93D50 with 24C01",	"Bandai",		MIRRORING_MAPPER_HVAB},

	// Mappers 160-169
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Nanjing",			"Nanjing",		MIRRORING_HEADER},
	{"Waixing (unlicensed)",	"Waixing",		MIRRORING_HEADER},
	{"Fire Emblem (unlicensed) (MMC2+MMC3 hybrid)", nullptr, MIRRORING_MAPPER_HV},
	{"Subor (variant 1)",		"Subor",		MIRRORING_HEADER},
	{"Subor (variant 2)",		"Subor",		MIRRORING_HEADER},
	{"Racermate Challenge 2",	"Racermate, Inc.",	MIRRORING_HEADER},
	{"Yuxing",			"Yuxing",		MIRRORING_UNKNOWN},

	// Mappers 170-179
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Kaiser KS-7058",		"Kaiser",		MIRRORING_HEADER},
	{"Super Mega P-4040",		nullptr,		MIRRORING_MAPPER_HV},
	{"Idea-Tek ET-xx",		"Idea-Tek",		MIRRORING_HEADER},
	{"Multicart",			nullptr,		MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Waixing multicart (MMC3 clone)", "Waixing",		MIRRORING_MAPPER_HVAB},
	{"BNROM variant",		"Hénggé Diànzǐ",	MIRRORING_MAPPER_HV},
	{"Waixing / Nanjing / Jncota / Henge Dianzi / GameStar", "Waixing / Nanjing / Jncota / Henge Dianzi / GameStar", MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},

	// Mappers 180-189
	{"Crazy Climber (UNROM clone)",	"Nichibutsu",		MIRRORING_HEADER},
	{"Seicross v2 (FCEUX hack)",	"Nichibutsu",		MIRRORING_HEADER},
	{"MMC3 clone (scrambled registers) (same as 114)", nullptr, MIRRORING_MAPPER_HV},
	{"Suikan Pipe (VRC4e clone)",	nullptr,		MIRRORING_MAPPER_HVAB},
	{"Sunsoft-1",			"Sunsoft",		MIRRORING_HEADER},
	{"CNROM with weak copy protection", nullptr,		MIRRORING_HEADER},	// Submapper field indicates required value for CHR banking. (TODO: VROM-disable?)
	{"Study Box",			"Fukutake Shoten",	MIRRORING_HEADER},
	{"Kǎshèng A98402 (MMC3 clone)",	"Kǎshèng",		MIRRORING_MAPPER_HV},
	{"Bandai Karaoke Studio",	"Bandai",		MIRRORING_MAPPER_HV},
	{"Thunder Warrior (MMC3 clone)", nullptr,		MIRRORING_MAPPER_HV},

	// Mappers 190-199
	{"Magic Kid GooGoo",		nullptr,		MIRRORING_HEADER},
	{"MMC3 clone",			nullptr,		MIRRORING_MAPPER_HV},
	{"MMC3 clone",			nullptr,		MIRRORING_MAPPER_HV},
	{"NTDEC TC-112",		"NTDEC",		MIRRORING_MAPPER_HV},
	{"MMC3 clone",			nullptr,		MIRRORING_MAPPER_HV},
	{"Waixing FS303 (MMC3 clone)",	"Waixing",		MIRRORING_MAPPER_HV},
	{"Mario bootleg (MMC3 clone)",	nullptr,		MIRRORING_MAPPER_HV},
	{"Kǎshèng (MMC3 clone)",	"Kǎshèng",		MIRRORING_MAPPER_HV /* not sure */},
	{"Tūnshí Tiāndì - Sānguó Wàizhuàn", nullptr,		MIRRORING_MAPPER_HV},
	{"Waixing (clone of either Mapper 004 or 176)", "Waixing", MIRRORING_MAPPER_HVAB},

	// Mappers 200-209
	{"Multicart",			nullptr,		MIRRORING_MAPPER_HV},
	{"NROM-256 multicart",		nullptr,		MIRRORING_HEADER},
	{"150-in-1 multicart",		nullptr,		MIRRORING_MAPPER_HV},
	{"35-in-1 multicart",		nullptr,		MIRRORING_HEADER},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"MMC3 multicart",		nullptr,		MIRRORING_MAPPER_HV},
	{"DxROM (Tengen MIMIC-1, Namcot 118)", "Nintendo",	MIRRORING_HEADER},
	{"Fudou Myouou Den",		"Taito",		MIRRORING_MAPPER_NAMCOT3425},
	{"Street Fighter IV (unlicensed) (MMC3 clone)", nullptr, MIRRORING_MAPPER_HV},
	{"J.Y. Company (MMC2/MMC4 clone)", "J.Y. Company",	MIRRORING_MAPPER_JY},

	// Mappers 210-219
	{"Namcot 175, 340",		"Namco",		MIRRORING_MAPPER_HVAB /* see submapper */},
	{"J.Y. Company (extended nametable control)", "J.Y. Company", MIRRORING_MAPPER_JY},
	{"BMC Super HiK 300-in-1",	nullptr,		MIRROIRNG_MAPPER_HV},
	{"(C)NROM-based multicart (same as 058)", nullptr,	MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Sugar Softec (MMC3 clone)",	"Sugar Softec",		MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Magic Floor",			"Homebrew",		MIRRORING_MAGICFLOOR},
	{"Kǎshèng A9461 (MMC3 clone)",	"Kǎshèng",		MIRRORING_MAPPER_HV},

	// Mappers 220-229
	{"Summer Carnival '92 - Recca",	"Naxat Soft",		MIRRORING_UNKNOWN /* TODO */},
	{"NTDEC N625092",		"NTDEC",		MIRRORING_MAPPER_HV},
	{"CTC-31 (VRC2 + 74xx)",	nullptr,		MIRRORING_MAPPER_HVAB},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Jncota KT-008",		"Jncota",		MIRRORING_UNKNOWN /* TODO */},
	{"Multicart",			nullptr,		MIRRORING_MAPPER_HV},
	{"Multicart",			nullptr,		MIRRORING_MAPPER_HV},
	{"Multicart",			nullptr,		MIRRORING_MAPPER_HV},
	{"Active Enterprises",		"Active Enterprises",	MIRRORING_MAPPER_HV},
	{"BMC 31-IN-1",			nullptr,		MIRRORING_MAPPER_HV},

	// Mappers 230-239
	{"Multicart",			nullptr,		MIRRORING_MAPPER_HV},
	{"Multicart",			nullptr,		MIRRORING_MAPPER_HV},
	{"Codemasters Quattro",		"Codemasters",		MIRRORING_HEADER},
	{"Multicart",			nullptr,		MIRRORING_MAPPER_233},
	{"Maxi 15 multicart",		nullptr,		MIRRORING_MAPPER_HV},
	{"Golden Game 150-in-1 multicart", nullptr,		MIRRORING_MAPPER_235},
	{"Realtec 8155",		"Realtec",		MIRRORING_MAPPER_HV},
	{"Teletubbies 420-in-1 multicart", nullptr		MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},

	// Mappers 240-249
	{"Multicart",			nullptr,		MIRRORING_HEADER},
	{"BNROM variant (similar to 034)", nullptr,		MIRRORING_HEADER},
	{"Unlicensed",			nullptr,		MIRRORING_MAPPER_HV},
	{"Sachen SA-020A",		"Sachen",		MIRRORING_MAPPER_SACHEN74LS374N},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"MMC3 clone",			nullptr,		MIRRORING_MAPPER_HV},
	{"Fēngshénbǎng: Fúmó Sān Tàizǐ (C&E)", "C&E",		MIRRORING_HEADER},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Kǎshèng SFC-02B/-03/-004 (MMC3 clone) (incorrect assignment; should be 115)", "Kǎshèng", MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},

	// Mappers 250-255
	{"Nitra (MMC3 clone)",		"Nitra",		MIRRORING_MAPPER_HV},
	{nullptr,			nullptr,		MIRRORING_UNKNOWN},
	{"Waixing - Sangokushi",	"Waixing",		MIRRORING_MAPPER_HVAB},
	{"Dragon Ball Z: Kyōshū! Saiya-jin (VRC4 clone)", "Waixing", MIRRORING_MAPPER_HVAB},
	{"Pikachu Y2K of crypted ROMs",	nullptr,		MIRRORING_MAPPER_HV},
	{"110-in-1 multicart (same as 225)", nullptr,		MIRRORING_MAPPER_HV},
};

/**
 * Mappers: NES 2.0 Plane 1 [256-511]
 * TODO: Add more fields:
 * - Programmable mirroring
 * - Extra VRAM for 4 screens
 */
const NESMappersPrivate::MapperEntry NESMappersPrivate::mappers_plane1[] = {
	// Mappers 256-259
	{"OneBus Famiclone",		nullptr},
	{"UNIF PEC-586",		nullptr},	// From UNIF; reserved by FCEUX developers
	{"UNIF 158B",			nullptr},	// From UNIF; reserved by FCEUX developers
	{"UNIF F-15 (MMC3 multicart)",	nullptr},	// From UNIF; reserved by FCEUX developers

	// Mappers 260-269
	{"HP10xx/HP20xx multicart",	nullptr},
	{"200-in-1 Elfland multicart",	nullptr},
	{"Street Heroes (MMC3 clone)",	"Sachen"},
	{"King of Fighters '97 (MMC3 clone)", nullptr},
	{"Cony/Yoko Fighting Games",	"Cony/Yoko"},
	{"T-262 multicart",		nullptr},
	{"City Fighter IV",		nullptr},	// Hack of Master Fighter II
	{"8-in-1 JY-119 multicart (MMC3 clone)", "J.Y. Company"},
	{"SMD132/SMD133 (MMC3 clone)",	nullptr},
	{"Multicart (MMC3 clone)",	nullptr},

	// Mappers 270-279
	{"Game Prince RS-16",		nullptr},
	{"TXC 4-in-1 multicart (MGC-026)", "TXC"},
	{"Akumajō Special: Boku Dracula-kun (bootleg)", nullptr},
	{"Gremlins 2 (bootleg)",	nullptr},
	{"Cartridge Story multicart",	"RCM Group"},
	{nullptr,			nullptr},
	{nullptr,			nullptr},
	{nullptr,			nullptr},
	{nullptr,			nullptr},
	{nullptr,			nullptr},

	// Mappers 280-289
	{nullptr,			nullptr},
	{"J.Y. Company Super HiK 3/4/5-in-1 multicart", "J.Y. Company"},
	{"J.Y. Company multicart",	"J.Y. Company"},
	{"Block Family 6-in-1/7-in-1 multicart", nullptr},
	{"Drip",			"Homebrew"},
	{"A65AS multicart",		nullptr},
	{"Benshieng multicart",		"Benshieng"},
	{"4-in-1 multicart (411120-C, 811120-C)", nullptr},
	{"GKCX1 21-in-1 multicart",	nullptr},	// GoodNES 3.23b sets this to Mapper 133, which is wrong.
	{"BMC-60311C",			nullptr},	// From UNIF

	// Mappers 290-299
	{"Asder 20-in-1 multicart",	"Asder"},
	{"Kǎshèng 2-in-1 multicart (MK6)", "Kǎshèng"},
	{"Dragon Fighter (unlicensed)",	nullptr},
	{"NewStar 12-in-1/76-in-1 multicart", nullptr},
	{"T4A54A, WX-KB4K, BS-5652 (MMC3 clone) (same as 134)", nullptr},
	{"J.Y. Company 13-in-1 multicart", "J.Y. Company"},
	{"FC Pocket RS-20 / dreamGEAR My Arcade Gamer V", nullptr},
	{"TXC 01-22110-000 multicart",	"TXC"},
	{"Lethal Weapon (unlicensed) (VRC4 clone)", nullptr},
	{"TXC 6-in-1 multicart (MGC-023)", "TXC"},

	// Mappers 300-309
	{"Golden 190-in-1 multicart",	nullptr},
	{"GG1 multicart",		nullptr},
	{"Gyruss (FDS conversion)",	"Kaiser"},
	{"Almana no Kiseki (FDS conversion)", "Kaiser"},
	{"FDS conversion",		"Whirlwind Manu"},
	{"Dracula II: Noroi no Fūin (FDS conversion)", "Kaiser"},
	{"Exciting Basket (FDS conversion)", "Kaiser"},
	{"Metroid (FDS conversion)",	"Kaiser"},
	{"Batman (Sunsoft) (bootleg) (VRC2 clone)", nullptr},
	{"Ai Senshi Nicol (FDS conversion)", "Whirlwind Manu"},

	// Mappers 310-319
	{"Monty no Doki Doki Daisassō (FDS conversion) (same as 125)", "Whirlwind Manu"},
	{nullptr,			nullptr},
	{"Highway Star (bootleg)",	"Kaiser"},
	{"Reset-based multicart (MMC3)", nullptr},
	{"Y2K multicart",		nullptr},
	{"820732C- or 830134C- multicart", nullptr},
	{nullptr,			nullptr},
	{nullptr,			nullptr},
	{nullptr,			nullptr},
	{"HP-898F, KD-7/9-E multicart",	nullptr},

	// Mappers 320-329
	{"Super HiK 6-in-1 A-030 multicart", nullptr},
	{nullptr,			nullptr},
	{"35-in-1 (K-3033) multicart",	nullptr},
	{"Farid's homebrew 8-in-1 SLROM multicart", nullptr},	// Homebrew
	{"Farid's homebrew 8-in-1 UNROM multicart", nullptr},	// Homebrew
	{"Super Mali Splash Bomb (bootleg)", nullptr},
	{"Contra/Gryzor (bootleg)",	nullptr},
	{"6-in-1 multicart",		nullptr},
	{"Test Ver. 1.01 Dlya Proverki TV Pristavok test cartridge", nullptr},
	{"Education Computer 2000",	nullptr},

	// Mappers 330-339
	{"Sangokushi II: Haō no Tairiku (bootleg)", nullptr},
	{"7-in-1 (NS03) multicart",	nullptr},
	{"Super 40-in-1 multicart",	nullptr},
	{"New Star Super 8-in-1 multicart", "New Star"},
	{"5/20-in-1 1993 Copyright multicart", nullptr},
	{"10-in-1 multicart",		nullptr},
	{"11-in-1 multicart",		nullptr},
	{"12-in-1 Game Card multicart",	nullptr},
	{"16-in-1, 200/300/600/1000-in-1 multicart", nullptr},
	{"21-in-1 multicart",		nullptr},

	// Mappers 340-349
	{"35-in-1 multicart",		nullptr},
	{"Simple 4-in-1 multicart",	nullptr},
	{"COOLGIRL multicart (Homebrew)", "Homebrew",		MIRRORING_UNKNOWN /* FIXME: supports HVAB4 */},	// Homebrew
	{nullptr,			nullptr},
	{"Kuai Da Jin Ka Zhong Ji Tiao Zhan 3-in-1 multicart", nullptr},
	{"New Star 6-in-1 Game Cartridge multicart", "New Star"},
	{"Zanac (FDS conversion)",	"Kaiser"},
	{"Yume Koujou: Doki Doki Panic (FDS conversion)", "Kaiser"},
	{"830118C",			nullptr},
	{"1994 Super HIK 14-in-1 (G-136) multicart", nullptr},

	// Mappers 350-359
	{"Super 15-in-1 Game Card multicart", nullptr},
	{"9-in-1 multicart",		"J.Y. Company / Techline"},
	{nullptr,			nullptr},
	{"92 Super Mario Family multicart", nullptr},
	{"250-in-1 multicart",		nullptr},
	{"黃信維 3D-BLOCK",		nullptr},
	{"7-in-1 Rockman (JY-208)",	"J.Y. Company"},
	{"4-in-1 (4602) multicart",	"Bit Corp."},
	{"J.Y. Company multicart",	"J.Y. Company"},
	{"SB-5013 / GCL8050 / 841242C multicart", nullptr},

	// Mappers 360-369
	{"31-in-1 (3150) multicart",	"Bit Corp."},
	{"YY841101C multicart (MMC3 clone)", "J.Y. Company"},
	{"830506C multicart (VRC4f clone)", "J.Y. Company"},
	{"J.Y. Company multicart",	"J.Y. Company"},
	{"JY830832C multicart",		"J.Y. Company"},
	{"Asder PC-95 educational computer", "Asder"},
	{"GN-45 multicart (MMC3 clone)", nullptr},
	{"7-in-1 multicart",		nullptr},
	{"Super Mario Bros. 2 (J) (FDS conversion)", "YUNG-08"},
	{"N49C-300",			nullptr},

	// Mappers 370-379
	{"F600",			nullptr},
	{"Spanish PEC-586 home computer cartridge", "Dongda"},
	{"Rockman 1-6 (SFC-12) multicart", nullptr},
	{"Super 4-in-1 (SFC-13) multicart", nullptr},
	{"Reset-based MMC1 multicart",	nullptr},
	{"135-in-1 (U)NROM multicart",	nullptr},
	{"YY841155C multicart",		"J.Y. Company"},
	{"8-in-1 AxROM/UNROM multicart", nullptr},
	{"35-in-1 NROM multicart",	nullptr},

	// Mappers 380-389
	{"970630C",			nullptr},
	{"KN-42",			nullptr},
	{"830928C",			nullptr},
	{"YY840708C (MMC3 clone)",	"J.Y. Company"},
	{"L1A16 (VRC4e clone)",		nullptr},
	{"NTDEC 2779",			"NTDEC"},
	{"YY860729C",			"J.Y. Company"},
	{"YY850735C / YY850817C",	"J.Y. Company"},
	{"YY841145C / YY850835C",	"J.Y. Company"},
	{"Caltron 9-in-1 multicart",	"Caltron"},

	// Mappers 390-391
	{"Realtec 8031",		"Realtec"},
	{"NC7000M (MMC3 clone)",	nullptr},
};

/**
 * Mappers: NES 2.0 Plane 2 [512-767]
 * TODO: Add more fields:
 * - Programmable mirroring
 * - Extra VRAM for 4 screens
 */
const NESMappersPrivate::MapperEntry NESMappersPrivate::mappers_plane2[] = {
	// Mappers 512-519
	{"Zhōngguó Dàhēng",		"Sachen"},
	{"Měi Shàonǚ Mèng Gōngchǎng III", "Sachen"},
	{"Subor Karaoke",		"Subor"},
	{"Family Noraebang",		nullptr},
	{"Brilliant Com Cocoma Pack",	"EduBank"},
	{"Kkachi-wa Nolae Chingu",	nullptr},
	{"Subor multicart",		"Subor"},
	{"UNL-EH8813A",			nullptr},

	// Mappers 520-529
	{"2-in-1 Datach multicart (VRC4e clone)", nullptr},
	{"Korean Igo",			nullptr},
	{"Fūun Shōrinken (FDS conversion)", "Whirlwind Manu"},
	{"Fēngshénbǎng: Fúmó Sān Tàizǐ (Jncota)", "Jncota"},
	{"The Lord of King (Jaleco) (bootleg)", nullptr},
	{"UNL-KS7021A (VRC2b clone)",	"Kaiser"},
	{"Sangokushi: Chūgen no Hasha (bootleg)", nullptr},
	{"Fudō Myōō Den (bootleg) (VRC2b clone)", nullptr},
	{"1995 New Series Super 2-in-1 multicart", nullptr},
	{"Datach Dragon Ball Z (bootleg) (VRC4e clone)", nullptr},

	// Mappers 530-539
	{"Super Mario Bros. Pocker Mali (VRC4f clone)", nullptr},
	{nullptr,			nullptr},
	{nullptr,			nullptr},
	{"Sachen 3014",			"Sachen"},
	{"2-in-1 Sudoku/Gomoku (NJ064) (MMC3 clone)", nullptr},
	{"Nazo no Murasamejō (FDS conversion)", "Whirlwind Manu"},
	{"Waixing FS303 (MMC3 clone) (same as 195)",	"Waixing"},
	{"Waixing FS303 (MMC3 clone) (same as 195)",	"Waixing"},
	{"60-1064-16L",			nullptr},
	{"Kid Icarus (FDS conversion)",	nullptr},

	// Mappers 540-549
	{"Master Fighter VI' hack (variant of 359)", nullptr},
	{"LittleCom 160-in-1 multicart", nullptr},	// Is LittleCom the company name?
	{"World Hero hack (VRC4 clone)", nullptr},
	{"5-in-1 (CH-501) multicart (MMC1 clone)", nullptr},
	{"Waixing FS306",		"Waixing"},
	{nullptr,			nullptr},
	{nullptr,			nullptr},
	{"Konami QTa adapter (VRC5)",	"Konami"},
	{"CTC-15",			"Co Tung Co."},
	{nullptr,			nullptr},

	// Mappers 550-552
	{nullptr,			nullptr},
	{"Jncota RPG re-release (variant of 178)", "Jncota"},
	{"Taito X1-017 (correct PRG ROM bank ordering)", "Taito"},
};

/** Submappers. **/

// Mapper 001: MMC1
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::mmc1_submappers[] = {
	{1, 0,   1, "SUROM"},
	{2, 0,   1, "SOROM"},
	{3, 0, 155, "MMC1A"},
	{4, 0,   1, "SXROM"},
	{5, 0,   0, "SEROM, SHROM, SH1ROM"},
};

// Discrete logic mappers: UxROM (002), CNROM (003), AxROM (007)
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::discrete_logic_submappers[] = {
	{0, 0,   0, "Bus conflicts are unspecified"},
	{1, 0,   0, "Bus conflicts do not occur"},
	{2, 0,   0, "Bus conflicts occur, resulting in: bus AND rom"},
};

// Mapper 004: MMC3
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::mmc3_submappers[] = {
	{0, 0,      0, "MMC3C"},
	{1, 0,      0, "MMC6"},
	{2, 0, 0xFFFF, "MMC3C with hard-wired mirroring"},
	{3, 0,      0, "MC-ACC"},
	{4, 0,      0, "MMC3A"},
};

// Mapper 016: Bandai FCG-x
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::bandai_fcgx_submappers[] = {
	{1, 0, 159, "LZ93D50 with 24C01"},
	{2, 0, 157, "Datach Joint ROM System"},
	{3, 0, 153, "8 KiB of WRAM instead of serial EEPROM"},
	{4, 0,   0, "FCG-1/2"},
	{5, 0,   0, "LZ93D50 with optional 24C02"},
};

// Mapper 019: Namco 129, 163
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::namco_129_164_submappers[] = {
	{0, 0,   0, "Expansion sound volume unspecified"},
	{1, 0,  19, "Internal RAM battery-backed; no expansion sound"},
	{2, 0,   0, "No expansion sound"},
	{3, 0,   0, "N163 expansion sound: 11.0-13.0 dB louder than NES APU"},
	{4, 0,   0, "N163 expansion sound: 16.0-17.0 dB louder than NES APU"},
	{5, 0,   0, "N163 expansion sound: 18.0-19.5 dB louder than NES APU"},
};

// Mapper 021: Konami VRC4c, VRC4c
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::vrc4a_vrc4c_submappers[] = {
	{1, 0,   0, "VRC4a"},
	{2, 0,   0, "VRC4c"},
};

// Mapper 023: Konami VRC4e, VRC4f, VRC2b
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::vrc4ef_vrc2b_submappers[] = {
	{1, 0,   0, "VRC4f"},
	{2, 0,   0, "VRC4e"},
	{3, 0,   0, "VRC2b"},
};

// Mapper 025: Konami VRC4b, VRC4d, VRC2c
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::vrc4bd_vrc2c_submappers[] = {
	{1, 0,   0, "VRC4b"},
	{2, 0,   0, "VRC4d"},
	{3, 0,   0, "VRC2c"},
};

// Mapper 032: Irem G101
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::irem_g101_submappers[] = {
	{0, 0,   0, "Programmable mirroring"},
	{1, 0,   0, "Fixed one-screen mirroring", MIRRORING_1SCREEN_B},
};

// Mapper 034: BNROM / NINA-001
// TODO: Distinguish between these two for iNES ROMs.
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::bnrom_nina001_submappers[] = {
	{1, 0,   0, "NINA-001"},
	{2, 0,   0, "BNROM"},
};

// Mapper 068: Sunsoft-4
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::sunsoft4_submappers[] = {
	{1, 0,   0, "Dual Cartridge System (NTB-ROM)"},
};

// Mapper 071: Codemasters
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::codemasters_submappers[] = {
	{1, 0,   0, "Programmable one-screen mirroring (Fire Hawk)", MIRRORING_MAPPER_AB},
};

// Mapper 078: Cosmo Carrier / Holy Diver
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::mapper078_submappers[] = {
	{1, 0,      0, "Programmable one-screen mirroring (Uchuusen: Cosmo Carrier)", MIRRORING_MAPPER_AB},
	{2, 0, 0xFFFF,  "Fixed vertical mirroring + WRAM"},
	{3, 0,      0, "Programmable H/V mirroring (Holy Diver)", MIRRORING_MAPPER_HV},
};

// Mapper 083: Cony/Yoko
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::cony_yoko_submappers[] = {
	{0, 0,   0, "1 KiB CHR-ROM banking, no WRAM"},
	{1, 0,   0, "2 KiB CHR-ROM banking, no WRAM"},
	{2, 0,   0, "1 KiB CHR-ROM banking, 32 KiB banked WRAM"},
};

// Mapper 114: Sugar Softec/Hosenkan
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::mapper114_submappers[] = {
	{0, 0,   0, "MMC3 registers: 0,3,1,5,6,7,2,4"},
	{1, 0,   0, "MMC3 registers: 0,2,5,3,6,1,7,4"},
};

// Mapper 197: Kǎshèng (MMC3 clone)
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::mapper197_submappers[] = {
	{0, 0,   0, "Super Fighter III (PRG-ROM CRC32 0xC333F621)"},
	{1, 0,   0, "Super Fighter III (PRG-ROM CRC32 0x2091BEB2)"},
	{2, 0,   0, "Mortal Kombat III Special"},
	{3, 0,   0, "1995 Super 2-in-1"},
};

// Mapper 210: Namcot 175, 340
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::namcot_175_340_submappers[] = {
	{1, 0,   0, "Namcot 175 (fixed mirroring)",        MIRRORING_HEADER},
	{2, 0,   0, "Namcot 340 (programmable mirroring)", MIRRORING_MAPPER_HVAB},
};

// Mapper 215: Sugar Softec
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::sugar_softec_submappers[] = {
	{0, 0,   0, "UNL-8237"},
	{1, 0,   0, "UNL-8237A"},
};

// Mapper 232: Codemasters Quattro
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::quattro_submappers[] = {
	{1, 0,   0, "Aladdin Deck Enhancer"},
};

// Mapper 256: OneBus Famiclones
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::onebus_submappers[] = {
	{ 1, 0,   0, "Waixing VT03"},
	{ 2, 0,   0, "Power Joy Supermax"},
	{ 3, 0,   0, "Zechess/Hummer Team"},
	{ 4, 0,   0, "Sports Game 69-in-1"},
	{ 5, 0,   0, "Waixing VT02"},
	{14, 0,   0, "Karaoto"},
	{15, 0,   0, "Jungletac"},
};

// Mapper 268: SMD132/SMD133
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::smd132_smd133_submappers[] = {
	{0, 0,   0, "COOLBOY ($6000-$7FFF)"},
	{1, 0,   0, "MINDKIDS ($5000-$5FFF)"},
};

// Mapper 313: Reset-based multicart (MMC3)
const struct NESMappersPrivate::SubmapperInfo NESMappersPrivate::mapper313_submappers[] = {
	{0, 0,   0, "Game size: 128 KiB PRG, 128 KiB CHR"},
	{1, 0,   0, "Game size: 256 KiB PRG, 128 KiB CHR"},
	{2, 0,   0, "Game size: 128 KiB PRG, 256 KiB CHR"},
	{3, 0,   0, "Game size: 256 KiB PRG, 256 KiB CHR"},
	{4, 0,   0, "Game size: 256 KiB PRG (first game); 128 KiB PRG (other games); 128 KiB CHR"},
};

/**
 * NES 2.0 submapper list.
 *
 * NOTE: Submapper 0 is optional, since it's the default behavior.
 * It may be included if NES 2.0 submapper 0 acts differently from
 * iNES mappers.
 */
#define NES2_SUBMAPPER(num, arr) {num, (uint16_t)ARRAY_SIZE(arr), arr}
const NESMappersPrivate::SubmapperEntry NESMappersPrivate::submappers[] = {
	NES2_SUBMAPPER(  1, mmc1_submappers),			// MMC1
	NES2_SUBMAPPER(  2, discrete_logic_submappers),		// UxROM
	NES2_SUBMAPPER(  3, discrete_logic_submappers),		// CNROM
	NES2_SUBMAPPER(  4, mmc3_submappers),			// MMC3
	NES2_SUBMAPPER(  7, discrete_logic_submappers),		// AxROM
	NES2_SUBMAPPER( 16, bandai_fcgx_submappers),		// FCG-x
	NES2_SUBMAPPER( 19, namco_129_164_submappers),		// Namco 129/164
	NES2_SUBMAPPER( 21, vrc4a_vrc4c_submappers),		// Konami VRC4a, VRC4c
	NES2_SUBMAPPER( 23, vrc4ef_vrc2b_submappers),		// Konami VRC4e, VRC4f, VRC2b
	NES2_SUBMAPPER( 25, vrc4bd_vrc2c_submappers),		// Konami VRC4b, VRC4d, VRC2c
	NES2_SUBMAPPER( 32, irem_g101_submappers),		// Irem G101
	NES2_SUBMAPPER( 34, bnrom_nina001_submappers),		// BNROM / NINA-001
	NES2_SUBMAPPER( 68, sunsoft4_submappers),		// Sunsoft-4
	NES2_SUBMAPPER( 71, codemasters_submappers),		// Codemasters
	NES2_SUBMAPPER( 78, mapper078_submappers),
	NES2_SUBMAPPER( 83, cony_yoko_submappers),		// Cony/Yoko
	NES2_SUBMAPPER(114, mapper114_submappers),
	NES2_SUBMAPPER(197, mapper197_submappers),		// Kǎshèng (MMC3 clone)
	NES2_SUBMAPPER(210, namcot_175_340_submappers),		// Namcot 175, 340
	NES2_SUBMAPPER(215, sugar_softec_submappers),		// Sugar Softec (MMC3 clone)
	NES2_SUBMAPPER(232, quattro_submappers),		// Codemasters Quattro
	NES2_SUBMAPPER(256, onebus_submappers),			// OneBus Famiclones
	NES2_SUBMAPPER(268, smd132_smd133_submappers),		// SMD132/SMD133
	NES2_SUBMAPPER(313, mapper313_submappers),		// Reset-based multicart (MMC3)

	{0, 0, nullptr}
};

/**
 * bsearch() comparison function for SubmapperInfo.
 * @param a
 * @param b
 * @return
 */
int RP_C_API NESMappersPrivate::SubmapperInfo_compar(const void *a, const void *b)
{
	uint8_t submapper1 = static_cast<const SubmapperInfo*>(a)->submapper;
	uint8_t submapper2 = static_cast<const SubmapperInfo*>(b)->submapper;
	if (submapper1 < submapper2) return -1;
	if (submapper1 > submapper2) return 1;
	return 0;
}

/**
 * bsearch() comparison function for SubmapperEntry.
 * @param a
 * @param b
 * @return
 */
int RP_C_API NESMappersPrivate::SubmapperEntry_compar(const void *a, const void *b)
{
	uint16_t mapper1 = static_cast<const SubmapperEntry*>(a)->mapper;
	uint16_t mapper2 = static_cast<const SubmapperEntry*>(b)->mapper;
	if (mapper1 < mapper2) return -1;
	if (mapper1 > mapper2) return 1;
	return 0;
}

/** NESMappers **/

/**
 * Look up an iNES mapper number.
 * @param mapper Mapper number.
 * @return Mapper name, or nullptr if not found.
 */
const char *NESMappers::lookup_ines(int mapper)
{
	assert(mapper >= 0);
	if (mapper < 0) {
		// Mapper number is out of range.
		return nullptr;
	}

	if (mapper < 256) {
		// NES 2.0 Plane 0 [000-255] (iNES 1.0)
		static_assert(sizeof(NESMappersPrivate::mappers_plane0) == (256 * sizeof(NESMappersPrivate::MapperEntry)),
			"NESMappersPrivate::mappers_plane0[] doesn't have 256 entries.");
		return NESMappersPrivate::mappers_plane0[mapper].name;
	} else if (mapper < 512) {
		// NES 2.0 Plane 1 [256-511]
		mapper -= 256;
		if (mapper >= ARRAY_SIZE_I(NESMappersPrivate::mappers_plane1)) {
			// Mapper number is out of range for plane 1.
			return nullptr;
		}
		return NESMappersPrivate::mappers_plane1[mapper].name;
	} else if (mapper < 768) {
		// NES 2.0 Plane 2 [512-767]
		mapper -= 512;
		if (mapper >= ARRAY_SIZE_I(NESMappersPrivate::mappers_plane2)) {
			// Mapper number is out of range for plane 2.
			return nullptr;
		}
		return NESMappersPrivate::mappers_plane2[mapper].name;
	}

	// Invalid mapper number.
	return nullptr;
}

/**
 * Convert a TNES mapper number to iNES.
 * @param tnes_mapper TNES mapper number.
 * @return iNES mapper number, or -1 if unknown.
 */
int NESMappers::tnesMapperToInesMapper(int tnes_mapper)
{
	// 255 == not supported
	static const uint8_t ines_mappers[] = {
		// 0
		0,	// NROM
		1,	// SxROM (MMC1)
		9,	// PxROM (MMC2)
		4,	// TxROM (MMC3)
		10,	// FxROM (MMC4)
		5,	// ExROM (MMC5)
		2,	// UxROM
		3,	// CNROM
		66,	// GNROM
		7,	// AxROM

		// 10
		184,	// Sunsoft-1
		89,	// Sunsoft-2
		67,	// Sunsoft-3
		68,	// Sunsoft-4
		69,	// Sunsoft-5
		70,	// Bandai
		75,	// Konami VRC1
		22,	// Konami VRC2A
		23,	// Konami VRC2B
		73,	// Konami VRC3

		// 20
		21,	// Konami VRC4A
		25,	// Konami VRC4B
		255,	// Konami VRC4C (FIXME: Submapper?)
		255,	// Konami VRC4D (FIXME: Submapper?)
		255,	// Konami VRC4E (FIXME: Submapper?)
		24,	// Konami VRC6A
		26,	// Konami VRC6B
		85,	// Konami VRC7
		87,	// Jaleco-1
		48,	// Jaleco-2

		// 30
		92,	// Jaleco-3
		86,	// Jaleco-4
		18,	// Jaleco-SS8806
		93,	// 74161A
		94,	// 74161B
		95,	// Namcot
		19,	// Namcot-106
		76,	// Namco-109
		88,	// Namco-118
		118,	// MMC-3+TLS

		// 40
		33,	// Taito-TC0190
		255,	// Taito-TC0350 (FIXME: Submapper?)
		48,	// Taito-TC190V
		80,	// Taito-X-005
		82,	// Taito-X1-17
		77,	// Irem-1
		97,	// Irem-2
		78,	// Irem-74HC161
		255,	// Irem-74HC32 (FIXME: Submapper?)
		32,	// Irem-G-101

		// 50
		65,	// Irem-H-3001
		185,	// VROM-disable
	};

	if (tnes_mapper < 0 || tnes_mapper >= ARRAY_SIZE_I(ines_mappers)) {
		// Undefined TNES mapper.
		return -1;
	} else if (ines_mappers[tnes_mapper] == 255) {
		// Not supported.
		return -1;
	}

	return ines_mappers[tnes_mapper];
}

/**
 * Look up an NES 2.0 submapper number.
 * TODO: Return the "depcrecated" value?
 * @param mapper Mapper number.
 * @param submapper Submapper number.
 * @return Submapper name, or nullptr if not found.
 */
const char *NESMappers::lookup_nes2_submapper(int mapper, int submapper)
{
	assert(mapper >= 0);
	assert(submapper >= 0);
	assert(submapper < 256);
	if (mapper < 0 || submapper < 0 || submapper >= 256) {
		// Mapper or submapper number is out of range.
		return nullptr;
	}

	// Do a binary search in submappers[].
	const NESMappersPrivate::SubmapperEntry key = { static_cast<uint16_t>(mapper), 0, nullptr };
	const NESMappersPrivate::SubmapperEntry *res =
		static_cast<const NESMappersPrivate::SubmapperEntry*>(bsearch(&key,
			NESMappersPrivate::submappers,
			ARRAY_SIZE(NESMappersPrivate::submappers)-1,
			sizeof(NESMappersPrivate::SubmapperEntry),
			NESMappersPrivate::SubmapperEntry_compar));
	if (!res || !res->info || res->info_size == 0)
		return nullptr;

	// Do a binary search in res->info.
	const NESMappersPrivate::SubmapperInfo key2 = { static_cast<uint8_t>(submapper), 0, 0, nullptr };
	const NESMappersPrivate::SubmapperInfo *res2 =
		static_cast<const NESMappersPrivate::SubmapperInfo*>(bsearch(&key2,
			res->info, res->info_size,
			sizeof(NESMappersPrivate::SubmapperInfo),
			NESMappersPrivate::SubmapperInfo_compar));
	// TODO: Return the "deprecated" value?
	return (res2 ? res2->desc : nullptr);
}

const char *NESMappers::lookup_ines_mirroring(int mapper, int submapper, bool vert, bool four)
{
	int mirror = MIRRORING_UNKNOWN; // TODO: fetch this from table
	int submapper_mirror = MIRRORING_UNKNOWN; // TODO: fetch this from table

	if (submapper_mirror != MIRRORING_UNKNOWN) // Override mapper's value
		mirror = submapper_mirror;

	// Handle some special cases
	switch (mirror) {
		case MIRRORING_UNKNOWN:
			// Default to showing the header flags
			mirror = MIRRORING_HEADER;
			break;
		case MIRRORING_UNROM512:
			// xxxx0xx0 - Horizontal mirroring
			// xxxx0xx1 - Vertical mirroring
			// xxxx1xx0 - Mapper-controlled, single screen
			// xxxx1xx1 - Four screens
			mirror = four && !vert ? MIRRORING_MAPPER_AB : MIRRORING_HEADER;
			break;
		case MIRRORING_BANDAI_FAMILYTRAINER:
			// Mapper 152 should be used instead of setting 4sc on this mapper (70).
			mirror = four ? MIRRORING_MAPPER_AB : MIRRORING_HEADER;
		case MIRRORING_MAGICFLOOR:
			// Magic Floor maps CIRAM across the entire PPU address space
			// CIRAM A10:   A10  A11  A12  A13
			// iNES flag6: 0xx1 0xx0 1xx0 1xx1
			//       $0000   aB   aa   aa   aa < pattern table 1
			//       $0800   aB   BB   aa   aa
			//       $1000   aB   aa   BB   aa < pattern table 2
			//       $1800   aB   BB   BB   aa
			//       $2000   aB   aa   aa   BB < nametables
			//       $2800   aB   BB   aa   BB
			//       $3000   aB   aa   BB   BB < unused memory
			//       $3800   aB   BB   BB   BB
			// Mirroring:  Vert Hori 1scA 1scB
			//
			// It's important to differentiate between 1scA and 1scB as it affects
			// pattern table mapping.
			mirror = four ? (vert ? MIRRORING_1SCREEN_B : MIRRORING_1SCREEN_A)
				      : MIRRORING_HEADER;
			break;
	}

	if (mirror == MIRRORING_1SCREEN_A)
		return C_("NES|Mirroring", "Single Screen (A)");
	if (mirror == MIRRORING_1SCREEN_B)
		return C_("NES|Mirroring", "Single Screen (B)");
	if (four || mirror == MIRRORING_4SCREEN)
		return C_("NES|Mirroring", "Four Screeens");

	if (mirror == MIRRORING_HEADER)
		return vert ? C_("NES|Mirroring", "Vertical") : C_("NES|Mirroring", "Horizontal");

	// TODO:
}

}

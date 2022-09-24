/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * EXEData.cpp: DOS/Windows executable data.                               *
 *                                                                         *
 * Copyright (c) 2016-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "EXEData.hpp"
#include "Other/exe_structs.h"

namespace LibRomData { namespace EXEData {

struct MachineType {
	uint16_t cpu;
	const char *name;
};

// PE machine types.
// NOTE: The cpu field *must* be sorted in ascending order.
static const MachineType machineTypes_PE[] = {
	{IMAGE_FILE_MACHINE_I386,	"Intel i386"},
	{IMAGE_FILE_MACHINE_R3000_BE,	"MIPS R3000 (big-endian)"},
	{IMAGE_FILE_MACHINE_R3000,	"MIPS R3000"},
	{IMAGE_FILE_MACHINE_R4000,	"MIPS R4000"},
	{IMAGE_FILE_MACHINE_R10000,	"MIPS R10000"},
	{IMAGE_FILE_MACHINE_WCEMIPSV2,	"MIPS (WCE v2)"},
	{IMAGE_FILE_MACHINE_ALPHA,	"DEC Alpha AXP"},
	{IMAGE_FILE_MACHINE_SH3,	"Hitachi SH3"},
	{IMAGE_FILE_MACHINE_SH3DSP,	"Hitachi SH3 DSP"},
	{IMAGE_FILE_MACHINE_SH3E,	"Hitachi SH3E"},
	{IMAGE_FILE_MACHINE_SH4,	"Hitachi SH4"},
	{IMAGE_FILE_MACHINE_SH5,	"Hitachi SH5"},
	{IMAGE_FILE_MACHINE_ARM,	"ARM"},
	{IMAGE_FILE_MACHINE_THUMB,	"ARM Thumb"},
	{IMAGE_FILE_MACHINE_ARMNT,	"ARM Thumb-2"},
	{IMAGE_FILE_MACHINE_AM33,	"Matsushita AM33"},
	{IMAGE_FILE_MACHINE_POWERPC,	"PowerPC"},
	{IMAGE_FILE_MACHINE_POWERPCFP,	"PowerPC with FPU"},
	{IMAGE_FILE_MACHINE_POWERPCBE,	"PowerPC (big-endian)"},
	{IMAGE_FILE_MACHINE_IA64,	"Intel Itanium"},
	{IMAGE_FILE_MACHINE_MIPS16,	"MIPS16"},
	{IMAGE_FILE_MACHINE_M68K,	"Motorola 68000"},
	{IMAGE_FILE_MACHINE_PA_RISC,	"PA-RISC"},
	{IMAGE_FILE_MACHINE_ALPHA64,	"DEC Alpha AXP (64-bit)"},
	{IMAGE_FILE_MACHINE_MIPSFPU,	"MIPS with FPU"},
	{IMAGE_FILE_MACHINE_MIPSFPU16,	"MIPS16 with FPU"},
	{IMAGE_FILE_MACHINE_TRICORE,	"Infineon TriCore"},
	{IMAGE_FILE_MACHINE_CEF,	"Common Executable Format"},
	{IMAGE_FILE_MACHINE_EBC,	"EFI Byte Code"},
	{IMAGE_FILE_MACHINE_RISCV32,	"RISC-V (32-bit address space)"},
	{IMAGE_FILE_MACHINE_RISCV64,	"RISC-V (64-bit address space)"},
	{IMAGE_FILE_MACHINE_RISCV128,	"RISC-V (128-bit address space)"},
	{IMAGE_FILE_MACHINE_AMD64,	"AMD64"},
	{IMAGE_FILE_MACHINE_M32R,	"Mitsubishi M32R"},
	{IMAGE_FILE_MACHINE_ARM64,	"ARM (64-bit)"},
	{IMAGE_FILE_MACHINE_CEE,	"MSIL"},
};

// LE machine types.
// NOTE: The cpu field *must* be sorted in ascending order.
static const MachineType machineTypes_LE[] = {
	{LE_CPU_80286,		"Intel i286"},
	{LE_CPU_80386,		"Intel i386"},
	{LE_CPU_80486,		"Intel i486"},
	{LE_CPU_80586,		"Intel Pentium"},
	{LE_CPU_i860_N10,	"Intel i860 XR (N10)"},
	{LE_CPU_i860_N11,	"Intel i860 XP (N11)"},
	{LE_CPU_MIPS_I,		"MIPS Mark I (R2000, R3000"},
	{LE_CPU_MIPS_II,	"MIPS Mark II (R6000)"},
	{LE_CPU_MIPS_III,	"MIPS Mark III (R4000)"},
};

/** Public functions **/

/**
 * Look up a PE machine type. (CPU)
 * @param cpu PE machine type.
 * @return Machine type name, or nullptr if not found.
 */
const char *lookup_pe_cpu(uint16_t cpu)
{
	// Do a binary search.
	static const MachineType *const pMachineTypes_PE_end =
		&machineTypes_PE[ARRAY_SIZE(machineTypes_PE)];
	auto pPE = std::lower_bound(machineTypes_PE, pMachineTypes_PE_end, cpu,
		[](const MachineType &pe, uint16_t cpu) {
			return (pe.cpu < cpu);
		});
	if (pPE == pMachineTypes_PE_end || pPE->cpu != cpu) {
		return nullptr;
	}
	return pPE->name;
}

/**
 * Look up an LE machine type. (CPU)
 * @param cpu LE machine type.
 * @return Machine type name, or nullptr if not found.
 */
const char *lookup_le_cpu(uint16_t cpu)
{
	// Do a binary search.
	static const MachineType *const pMachineTypes_LE_end =
		&machineTypes_LE[ARRAY_SIZE(machineTypes_LE)];
	auto pLE = std::lower_bound(machineTypes_LE, pMachineTypes_LE_end, cpu,
		[](const MachineType &pe, uint16_t cpu) {
			return (pe.cpu < cpu);
		});
	if (pLE == pMachineTypes_LE_end || pLE->cpu != cpu) {
		return nullptr;
	}
	return pLE->name;
}

} }

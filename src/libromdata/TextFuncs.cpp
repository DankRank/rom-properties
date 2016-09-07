/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * TextFuncs.cpp: Text encoding functions.                                 *
 *                                                                         *
 * Copyright (c) 2009-2016 by David Korth.                                 *
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

#include "TextFuncs.hpp"
#include "config.libromdata.h"

// Determine which character set decoder to use.
#if defined(_WIN32)
# include <windows.h>
#elif defined(HAVE_ICONV)
# include <iconv.h>
# if defined(RP_UTF8)
#  define RP_ICONV_ENCODING "UTF-8"
# elif defined(RP_UTF16)
#  include <byteorder.h>
#  if SYS_BYTEORDER == SYS_BIG_ENDIAN
#   define RP_ICONV_ENCODING "UTF-16BE"
#  else
#   define RP_ICONV_ENCODING "UTF-16LE"
#  endif
# endif
#endif

// C++ includes.
#include <string>
using std::string;
using std::u16string;
using std::wstring;

// C includes. (C++ namespace)
#include <cassert>
#include <cstdlib>
#include <cstring>

// TODO: Use std::auto_ptr<>?

namespace LibRomData {

#if defined(_WIN32)
/**
 * Convert a null-terminated multibyte string to UTF-16.
 * @param mbs		[in] Multibyte string. (null-terminated)
 * @param codepage	[in] mbs codepage.
 * @param dwFlags	[in, opt] Conversion flags.
 * @return Allocated UTF-16 string, or NULL on error. (Must be free()'d after use!)
 */
static char16_t *W32U_mbs_to_UTF16(const char *mbs, unsigned int codepage, DWORD dwFlags = 0)
{
	static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t is not 16-bit!");
	int cchWcs = MultiByteToWideChar(codepage, dwFlags, mbs, -1, nullptr, 0);
	if (cchWcs <= 0)
		return nullptr;

	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(codepage, dwFlags, mbs, -1, wcs, cchWcs);
	return reinterpret_cast<char16_t*>(wcs);
}

/**
 * Convert a multibyte string to UTF-16.
 * @param mbs		[in] Multibyte string.
 * @param cbMbs		[in] Length of mbs, in bytes.
 * @param codepage	[in] mbs codepage.
 * @param cchWcs_ret	[out, opt] Number of characters in the returned string.
 * @param dwFlags	[in, opt] Conversion flags.
 * @return Allocated UTF-16 string, or NULL on error. (Must be free()'d after use!)
 * NOTE: Returned string might NOT be NULL-terminated!
 */
static char16_t *W32U_mbs_to_UTF16(const char *mbs, int cbMbs,
		unsigned int codepage, int *cchWcs_ret, DWORD dwFlags = 0)
{
	int cchWcs = MultiByteToWideChar(codepage, dwFlags, mbs, cbMbs, nullptr, 0);
	if (cchWcs <= 0)
		return nullptr;

	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(codepage, dwFlags, mbs, cbMbs, wcs, cchWcs);

	if (cchWcs_ret)
		*cchWcs_ret = cchWcs;
	return reinterpret_cast<char16_t*>(wcs);
}

/**
 * Convert a null-terminated UTF-16 string to multibyte.
 * @param wcs		[in] UTF-16 string. (null-terminated)
 * @param codepage	[in] mbs codepage.
 * @return Allocated multibyte string, or NULL on error. (Must be free()'d after use!)
 */
static char *W32U_UTF16_to_mbs(const char16_t *wcs, unsigned int codepage)
{
	int cbMbs = WideCharToMultiByte(codepage, 0, reinterpret_cast<const wchar_t*>(wcs), -1, nullptr, 0, nullptr, nullptr);
	if (cbMbs <= 0)
		return nullptr;
 
	char *mbs = (char*)malloc(cbMbs);
	WideCharToMultiByte(codepage, 0, reinterpret_cast<const wchar_t*>(wcs), -1, mbs, cbMbs, nullptr, nullptr);
	return mbs;
}

/**
 * Convert a UTF-16 string to multibyte.
 * @param wcs		[in] UTF-16 string.
 * @param cchWcs	[in] Length of wcs, in characters.
 * @param codepage	[in] mbs codepage.
 * @param cbMbs_ret	[out, opt] Number of bytes in the returned string.
 * @return Allocated multibyte string, or NULL on error. (Must be free()'d after use!)
 * NOTE: Returned string might NOT be NULL-terminated!
 */
static char *W32U_UTF16_to_mbs(const char16_t *wcs, int cchWcs,
		unsigned int codepage, int *cbMbs_ret)
{
	int cbMbs = WideCharToMultiByte(codepage, 0, reinterpret_cast<const wchar_t*>(wcs), cchWcs, nullptr, 0, nullptr, nullptr);
	if (cbMbs <= 0)
		return nullptr;

	char *mbs = (char*)malloc(cbMbs);
	WideCharToMultiByte(codepage, 0, reinterpret_cast<const wchar_t*>(wcs), cchWcs, mbs, cbMbs, nullptr, nullptr);

	if (cbMbs_ret)
		*cbMbs_ret = cbMbs;
	return mbs;
}

#elif defined(HAVE_ICONV)

/**
 * Convert a string from one character set to another.
 * @param src 		[in] Source string.
 * @param src_bytes_len [in] Source length, in bytes.
 * @param src_charset	[in] Source character set.
 * @param dest_charset	[in] Destination character set.
 * @return malloc()'d UTF-8 string, or nullptr on error.
 */
static char *rp_iconv(const char *src, size_t src_bytes_len,
			const char *src_charset, const char *dest_charset)
{
	if (!src || src_bytes_len == 0)
		return nullptr;

	if (!src_charset)
		src_charset = "";
	if (!dest_charset)
		dest_charset = "";

	// Based on examples from:
	// * http://www.delorie.com/gnu/docs/glibc/libc_101.html
	// * http://www.codase.com/search/call?name=iconv

	// Open an iconv descriptor.
	iconv_t cd;
	cd = iconv_open(dest_charset, src_charset);
	if (cd == (iconv_t)(-1)) {
		// Error opening iconv.
		return nullptr;
	}

	// Allocate the output buffer.
	// UTF-8 is variable length, and the largest UTF-8 character is 4 bytes long.
	const size_t out_bytes_len = (src_bytes_len * 4) + 4;
	size_t out_bytes_remaining = out_bytes_len;
	char *outbuf = (char*)malloc(out_bytes_len);

	// Input and output pointers.
	char *inptr = (char*)(src);	// Input pointer.
	char *outptr = &outbuf[0];	// Output pointer.

	int success = 1;

	while (src_bytes_len > 0) {
		if (iconv(cd, &inptr, &src_bytes_len, &outptr, &out_bytes_remaining) == (size_t)(-1)) {
			// An error occurred while converting the string.
			if (outptr == &outbuf[0]) {
				// No bytes were converted.
				success = 0;
			} else {
				// Some bytes were converted.
				// Accept the string up to this point.
				// Madou Monogatari I has a broken Shift-JIS sequence
				// at position 9, which resulted in no conversion.
				// (Reported by andlabs.)
				success = 1;
			}
			break;
		}
	}

	// Close the iconv descriptor.
	iconv_close(cd);

	if (success) {
		// The string was converted successfully.

		// Make sure the string is null-terminated.
		size_t null_bytes = (out_bytes_remaining > 4 ? 4 : out_bytes_remaining);
		for (size_t i = null_bytes; i > 0; i--) {
			*outptr++ = 0x00;
		}

		// Return the output buffer.
		return outbuf;
	}

	// The string was not converted successfully.
	free(outbuf);
	return nullptr;
}
#endif /* HAVE_ICONV */

/**
 * Convert cp1252 or Shift-JIS text to UTF-8.
 * @param str cp1252 or Shift-JIS text.
 * @param len Length of str, in bytes.
 * @return UTF-8 string.
 */
std::string cp1252_sjis_to_utf8(const char *str, size_t len)
{
#if defined(_WIN32)
	// Win32 version.
	// Attempt to convert from Shift-JIS to UTF-16.
	int cchWcs;
	char16_t *wcs = W32U_mbs_to_UTF16(str, (int)len, 932, &cchWcs, MB_ERR_INVALID_CHARS);
	if (!wcs) {
		// Shift-JIS conversion failed.
		// Fall back to cp1252.
		wcs = W32U_mbs_to_UTF16(str, (int)len, 1252, &cchWcs);
	}

	if (wcs) {
		// Convert the UTF-16 to UTF-8.
		int cbMbs;
		char *mbs = W32U_UTF16_to_mbs(wcs, cchWcs, CP_UTF8, &cbMbs);
		free(wcs);
		if (!mbs)
			return string();
		string ret(mbs, cbMbs);
		free(mbs);
		return ret;
	}
#elif defined(HAVE_ICONV)
	// iconv version.
	// Try Shift-JIS first.
	char *mbs = rp_iconv((char*)str, len, "SHIFT-JIS", "UTF-8");
	if (mbs) {
		string ret(mbs);
		free(mbs);
		return ret;
	}

	// Try cp1252.
	mbs = rp_iconv((char*)str, len, "CP1252", "UTF-8");
	if (mbs) {
		string ret(mbs);
		free(mbs);
		return ret;
	}
#else
#error Text conversion is not available on this system.
#endif

	// Unable to convert the string.
	return string();
}

/**
 * Convert cp1252 or Shift-JIS text to UTF-16.
 * @param str cp1252 or Shift-JIS text.
 * @param len Length of str, in bytes.
 * @return UTF-16 string.
 */
u16string cp1252_sjis_to_utf16(const char *str, size_t len)
{
#ifdef RP_WIS16
	static_assert(sizeof(wchar_t) == sizeof(char16_t), "RP_WIS16 is defined, but wchar_t is not 16-bit!");
#else /* !RP_WIS16 */
	static_assert(sizeof(wchar_t) != sizeof(char16_t), "RP_WIS16 is not defined, but wchar_t is 16-bit!");
#endif /* RP_WIS16 */

	// Attempt to convert str from Shift-JIS to UTF-16.
#if defined(_WIN32)
	// Win32 version.
	int cchWcs;
	char16_t *wcs = W32U_mbs_to_UTF16(str, (int)len, 932, &cchWcs, MB_ERR_INVALID_CHARS);
	if (!wcs) {
		// Shift-JIS conversion failed.
		// Fall back to cp1252.
		wcs = W32U_mbs_to_UTF16(str, (int)len, 1252, &cchWcs);
	}

	if (wcs) {
		// Return the UTF-16 string.
		u16string ret(reinterpret_cast<const char16_t*>(wcs), cchWcs);
		free(wcs);
		return ret;
	}
#elif defined(HAVE_ICONV)
	// iconv version.
	// Try Shift-JIS first.
	char16_t *wcs = (char16_t*)rp_iconv((char*)str, len, "SHIFT-JIS", RP_ICONV_ENCODING);
	if (wcs) {
		u16string ret(wcs);
		free(wcs);
		return ret;
	}

	// Try cp1252.
	wcs = (char16_t*)rp_iconv((char*)str, len, "CP1252", RP_ICONV_ENCODING);
	if (wcs) {
		u16string ret(wcs);
		free(wcs);
		return ret;
	}
#else
#error Text conversion is not available on this system.
#endif

	// Unable to convert the string.
	return u16string();
}

/**
 * Convert UTF-8 text to UTF-16.
 * @param str UTF-8 text.
 * @param len Length of str, in bytes.
 * @return UTF-16 string.
 */
u16string utf8_to_utf16(const char *str, size_t len)
{
#if defined(_WIN32)
	// Win32 version.
	int cchWcs;
	char16_t *wcs = W32U_mbs_to_UTF16(str, (int)len, CP_UTF8, &cchWcs);
	if (wcs) {
		u16string ret(wcs, cchWcs);
		free(wcs);
		return ret;
	}
#elif defined(HAVE_ICONV)
	// iconv version.
	char16_t *wcs = (char16_t*)rp_iconv((char*)str, len, "UTF-8", RP_ICONV_ENCODING);
	if (wcs) {
		u16string ret(wcs);
		free(wcs);
		return ret;
	}
#else
#error Text conversion is not available on this system.
#endif

	// Unable to convert the string.
	return u16string();
}

/**
 * Convert UTF-16 text to UTF-8.
 * @param str UTF-16 text.
 * @param len Length of str, in characters.
 * @return UTF-8 string.
 */
string utf16_to_utf8(const char16_t *str, size_t len)
{
#if defined(_WIN32)
	// Win32 version.
	int cbMbs;
	char *mbs = W32U_UTF16_to_mbs(str, (int)len, CP_UTF8, &cbMbs);
	if (mbs) {
		string ret(mbs, cbMbs);
		free(mbs);
		return ret;
	}
#elif defined(HAVE_ICONV)
	// iconv version.
	char *mbs = (char*)rp_iconv((char*)str, len*sizeof(*str), RP_ICONV_ENCODING, "UTF-8");
	if (mbs) {
		string ret(mbs);
		free(mbs);
		return ret;
	}
#else
#error Text conversion is not available on this system.
#endif

	// Unable to convert the string.
	return string();
}

/**
 * Convert Latin-1 (ISO-8859-1) text to UTF-16.
 * NOTE: 0x80-0x9F (cp1252) is converted to '?'.
 * @param str Latin-1 text.
 * @param len Length of str, in bytes.
 * @return UTF-16 string.
 */
u16string latin1_to_utf16(const char *str, size_t len)
{
	// Convert from Latin-1 to UTF-16.
	u16string wcs;
	wcs.reserve(len);
	for (; len > 0; len--, str++) {
		if ((*str & 0xE0) == 0x80) {
			// Characters 0x80-0x9F. Replace with '?'.
			wcs.push_back(_RP_CHR('?'));
		} else {
			// Other character.
			wcs.push_back(*str);
		}
	}
	return wcs;
}

#ifndef RP_WIS16
/**
 * char16_t strlen().
 * @param wcs 16-bit string.
 * @return Length of str, in characters.
 */
size_t u16_strlen(const char16_t *wcs)
{
	size_t len = 0;
	while (*wcs++)
		len++;
	return len;
}

/**
 * char16_t strdup().
 * @param wcs 16-bit string.
 * @return Copy of wcs.
 */
char16_t *u16_strdup(const char16_t *wcs)
{
	size_t len = u16_strlen(wcs)+1;	// includes terminator
	char16_t *ret = (char16_t*)malloc(len*sizeof(*wcs));
	memcpy(ret, wcs, len*sizeof(*wcs));
	return ret;
}
#endif /* !RP_WIS16 */

}

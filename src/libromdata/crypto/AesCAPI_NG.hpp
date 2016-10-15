/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * AesCAPI_NG.hpp: AES decryption class using Win32 CryptoAPI NG.          *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_CRYPTO_AESCAPI_NG_HPP__
#define __ROMPROPERTIES_LIBROMDATA_CRYPTO_AESCAPI_NG_HPP__

#include "IAesCipher.hpp"

namespace LibRomData {

class AesCAPI_NG_Private;
class AesCAPI_NG : public IAesCipher
{
	public:
		AesCAPI_NG();
		virtual ~AesCAPI_NG();

	private:
		typedef IAesCipher super;
		AesCAPI_NG(const AesCAPI_NG &other);
		AesCAPI_NG &operator=(const AesCAPI_NG &other);
	private:
		friend class AesCAPI_NG_Private;
		AesCAPI_NG_Private *const d;

	public:
		/**
		 * Is CryptoAPI NG usable on this system?
		 *
		 * If CryptoAPI NG is usable, this function will load
		 * bcrypt.dll and all required function pointers.
		 *
		 * @return True if this system supports CryptoAPI NG.
		 */
		static bool isUsable(void);

	public:
		/**
		 * Has the cipher been initialized properly?
		 * @return True if initialized; false if not.
		 */
		virtual bool isInit(void) const final;

		/**
		 * Set the encryption key.
		 * @param key Key data.
		 * @param len Key length, in bytes.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int setKey(const uint8_t *key, unsigned int len) final;

		/**
		 * Set the cipher chaining mode.
		 * @param mode Cipher chaining mode.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int setChainingMode(ChainingMode mode) final;

		/**
		 * Set the IV.
		 * @param iv IV data.
		 * @param len IV length, in bytes.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int setIV(const uint8_t *iv, unsigned int len) final;

		/**
		 * Decrypt a block of data.
		 * @param data Data block.
		 * @param data_len Length of data block.
		 * @return Number of bytes decrypted on success; 0 on error.
		 */
		virtual unsigned int decrypt(uint8_t *data, unsigned int data_len) final;

		/**
		 * Decrypt a block of data using the specified IV.
		 * @param data Data block.
		 * @param data_len Length of data block.
		 * @param iv IV for the data block.
		 * @param iv_len Length of the IV.
		 * @return Number of bytes decrypted on success; 0 on error.
		 */
		virtual unsigned int decrypt(uint8_t *data, unsigned int data_len,
			const uint8_t *iv, unsigned int iv_len) final;
};

}

#endif /* __ROMPROPERTIES_LIBROMDATA_CRYPTO_AESCAPI_NG_HPP__ */

/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * RomDataDecl.hpp: Macros for declaring a RomData subclass.               *
 *                                                                         *
 * Copyright (c) 2018 by Egor.                                             *
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

#ifndef __ROMPPROPERTIES_LIBROMDATA_ROMDATADECL_HPP__
#define __ROMPPROPERTIES_LIBROMDATA_ROMDATADECL_HPP__

#include "librpbase/RomData.hpp"
#include "librpbase/file/RpFile.hpp"

#define ROMDATA_DECL_BEGIN(cname) \
    class cname##Private; \
	class cname : public LibRpBase::RomData { \
		public: \
			explicit cname(LibRpBase::IRpFile* file); \
		protected: \
			virtual ~cname() { } \
		private: \
			typedef RomData super; \
			friend class cname##Private; \
			RP_DISABLE_COPY(cname); \
		public: \
			static int isRomSupported_static(const DetectInfo *info); \
			virtual int isRomSupported(const DetectInfo *info) const override final; \
			virtual const char *systemName(unsigned int type) const override final; \
			static const char *const *supportedFileExtensions_static(void); \
			virtual const char *const *supportedFileExtensions(void) const override final; \
		protected: \
			virtual int loadFieldData(void) override final;

#define ROMDATA_DECL_IMGSUPPORT() \
		public: \
			static uint32_t supportedImageTypes_static(void); \
			virtual uint32_t supportedImageTypes(void) const override final; \
			static std::vector<RomData::ImageSizeDef> supportedImageSizes_static(ImageType imageType); \
			virtual std::vector<RomData::ImageSizeDef> supportedImageSizes(ImageType imageType) const override final; 

#define ROMDATA_DECL_IMGPF() \
		public: \
			virtual uint32_t imgpf(ImageType imageType) const override final;

#define ROMDATA_DECL_IMGINT() \
		protected: \
			virtual int loadInternalImage(ImageType imageType, const LibRpBase::rp_image **pImage) override final;

#define ROMDATA_DECL_IMGEXT() \
		public: \
			virtual int extURLs(ImageType imageType, std::vector<ExtURL> *pExtURLs, int size = IMAGE_SIZE_DEFAULT) const override final;

#define ROMDATA_DECL_ICONANIM() \
		public: \
			virtual const LibRpBase::IconAnimData *iconAnimData(void) const override final;

#define ROMDATA_DECL_CLOSE() \
		public: \
			virtual void close(void) override final;

#define ROMDATA_DECL_END() \
	};

namespace LibRomData {
    template<typename T>
    LibRpBase::RomData* RomData_ctor(LibRpBase::IRpFile* file);
    template<typename T>
    int RomData_isRomSupported(const LibRpBase::RomData::DetectInfo *info);
    template<typename T>
    const char *const *RomData_supportedFileExtensions(void);
    template<typename T>
    uint32_t RomData_supportedImageTypes(void);
    template<typename T>
    std::vector<LibRpBase::RomData::ImageSizeDef> RomData_supportedImageSizes(LibRpBase::RomData::ImageType imageType);
}

#define ROMDATA_IMPL(cname) \
	template<> \
	LibRpBase::RomData* RomData_ctor<cname>(LibRpBase::IRpFile *file) { \
		return new cname(file); \
	} \
	template<> \
	int RomData_isRomSupported<cname>(const LibRpBase::RomData::DetectInfo *info) { \
		return cname::isRomSupported_static(info); \
	} \
	template<> \
	const char *const *RomData_supportedFileExtensions<cname>(void) { \
		return cname::supportedFileExtensions_static(); \
	}
#define ROMDATA_IMPL_IMG(cname) \
	template<> \
	uint32_t RomData_supportedImageTypes<cname>(void) { \
		return cname::supportedImageTypes_static(); \
	}

    // not all classes implement supportedImageSizes_static, and it seems like it's not used by anything
#if 0
	template <>
	std::vector<LibRpBase::RomData::ImageSizeDef> RomData_supportedImageSizes<cname>(LibRpBase::RomData::ImageType imageType) {
		return cname::supportedImageSizes_static(imageType);
	}
#endif

#endif

/***************************************************************************
 * ROM Properties Page shell extension. (KDE)                              *
 * ExtractorPlugin.cpp: KFileMetaData extractor plugin.                    *
 *                                                                         *
 * Qt's plugin system prevents a single shared library from exporting      *
 * multiple plugins, so this file acts as a KFileMetaData ExtractorPlugin, *
 * and then forwards the request to the main library.                      *
 *                                                                         *
 * Copyright (c) 2018-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "check-uid.hpp"
#include "ExtractorPlugin.hpp"

// librpbase, librpfile
using namespace LibRpBase;
using LibRpFile::IRpFile;

// libromdata
#include "libromdata/RomDataFactory.hpp"
using LibRomData::RomDataFactory;

// C++ STL classes.
using std::string;
using std::vector;

// KDE includes
// NOTE: kfilemetadata_version.h was added in 5.94.0, so we can't use it.
// Using kcoreaddons_version.h instead.
#include <kfileitem.h>
//#include <kfilemetadata_version.h>
#include <kcoreaddons_version.h>
#include <kfilemetadata/extractorplugin.h>
#include <kfilemetadata/properties.h>
using KFileMetaData::ExtractorPlugin;
using KFileMetaData::ExtractionResult;
using namespace KFileMetaData::Property;

/**
 * Factory method.
 * NOTE: Unlike the ThumbCreator version, this one is specific to
 * rom-properties, and is called by a forwarder library.
 */
extern "C" {
	Q_DECL_EXPORT RomPropertiesKDE::ExtractorPlugin *PFN_CREATEEXTRACTORPLUGINKDE_FN(QObject *parent)
	{
		CHECK_UID_RET(nullptr);
		return new RomPropertiesKDE::ExtractorPlugin(parent);
	}
}

namespace RomPropertiesKDE {

ExtractorPlugin::ExtractorPlugin(QObject *parent)
	: super(parent)
{ }

QStringList ExtractorPlugin::mimetypes(void) const
{
	// Get the MIME types from RomDataFactory.
	const vector<const char*> &vec_mimeTypes = RomDataFactory::supportedMimeTypes();

	// Convert to QStringList.
	QStringList mimeTypes;
	mimeTypes.reserve(static_cast<int>(vec_mimeTypes.size()));
	for (const char *mimeType : vec_mimeTypes) {
		mimeTypes += QLatin1String(mimeType);
	}
	return mimeTypes;
}

void ExtractorPlugin::extract(ExtractionResult *result)
{
	// Attempt to open the ROM file.
	IRpFile *const file = openQUrl(QUrl(result->inputUrl()), false);
	if (!file) {
		// Could not open the file.
		return;
	}

	// Get the appropriate RomData class for this ROM.
	// file is dup()'d by RomData.
	RomData *const romData = RomDataFactory::create(file, RomDataFactory::RDA_HAS_METADATA);
	file->unref();	// file is ref()'d by RomData.
	if (!romData) {
		// ROM is not supported.
		return;
	}

	// Get the metadata properties.
	const RomMetaData *const metaData = romData->metaData();
	if (!metaData || metaData->empty()) {
		// No metadata properties.
		romData->unref();
		return;
	}

	// Process the metadata.
	const int count = metaData->count();
	for (int i = 0; i < count; i++) {
		const RomMetaData::MetaData *const prop = metaData->prop(i);
		assert(prop != nullptr);
		if (!prop)
			continue;

		// RomMetaData's property indexes match KFileMetaData.
		// No conversion is necessary.
		switch (prop->type) {
			case PropertyType::Integer: {
				int ivalue = prop->data.ivalue;
				switch (prop->name) {
					case LibRpBase::Property::Duration:
						// Duration needs to be converted from ms to seconds.
						ivalue /= 1000;
						break;
					case LibRpBase::Property::Rating:
						// Rating is [0,100]; needs to be converted to [0,10].
						ivalue /= 10;
						break;
					default:
						break;
				}
				result->add(static_cast<KFileMetaData::Property::Property>(prop->name), ivalue);
				break;
			}

			case PropertyType::UnsignedInteger: {
				result->add(static_cast<KFileMetaData::Property::Property>(prop->name),
					    prop->data.uvalue);
				break;
			}

			case PropertyType::String: {
				LibRpBase::Property prop_name = prop->name;
				// NOTE: kfilemetadata_version.h was added in KF5 5.94.0.
				// Using kcoreaddons_version.h instead.
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5,53,0)
				if (prop_name == LibRpBase::Property::Description) {
					// KF5 5.53 added Description.
					// Fall back to Subject since Description isn't available.
					prop_name = LibRpBase::Property::Subject;
				}
#endif /* KCOREADDONS_VERSION < QT_VERSION_CHECK(5,53,0) */

				const string *str = prop->data.str;
				if (str) {
					result->add(static_cast<KFileMetaData::Property::Property>(prop_name),
						QString::fromUtf8(str->data(), static_cast<int>(str->size())));
				}
				break;
			}

			case PropertyType::Timestamp: {
				// TODO: Verify timezone handling.
				// NOTE: fromMSecsSinceEpoch() with TZ spec was added in Qt 5.2.
				// Maybe write a wrapper function? (RomDataView uses this, too.)
				// NOTE: Some properties might need the full QDateTime.
				// CreationDate seems to work fine with just QDate.
				QDateTime dateTime;
				dateTime.setTimeSpec(Qt::UTC);
				dateTime.setMSecsSinceEpoch((qint64)prop->data.timestamp * 1000);
				result->add(static_cast<KFileMetaData::Property::Property>(prop->name),
					dateTime.date());
				break;
			}

			case PropertyType::Double: {
				result->add(static_cast<KFileMetaData::Property::Property>(prop->name),
					    prop->data.dvalue);
				break;
			}

			default:
				// ERROR!
				assert(!"Unsupported RomMetaData PropertyType.");
				break;
		}
	}

	// Finished extracting metadata.
	romData->unref();
}

}

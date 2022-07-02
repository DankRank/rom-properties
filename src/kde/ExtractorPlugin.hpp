/***************************************************************************
 * ROM Properties Page shell extension. (KDE)                              *
 * ExtractorPlugin.hpp: KFileMetaData extractor plugin.                    *
 *                                                                         *
 * Qt's plugin system prevents a single shared library from exporting      *
 * multiple plugins, so this file acts as a KFileMetaData ExtractorPlugin, *
 * and then forwards the request to the main library.                      *
 *                                                                         *
 * Copyright (c) 2018-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_KDE_EXTRACTORPLUGIN_HPP__
#define __ROMPROPERTIES_KDE_EXTRACTORPLUGIN_HPP__

#include <QtCore/qglobal.h>
#include <kfilemetadata/extractorplugin.h>

#if QT_VERSION >= QT_VERSION_CHECK(7,0,0)
#  error Needs updating for Qt7
#elif QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#  define PFN_CREATEEXTRACTORPLUGINKDE_FN createExtractorPluginKF6
#  define PFN_CREATEEXTRACTORPLUGINKDE_NAME "createExtractorPluginKF6"
#  define RomPropertiesKDE RomPropertiesKF6
#elif QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#  define PFN_CREATEEXTRACTORPLUGINKDE_FN createExtractorPluginKF5
#  define PFN_CREATEEXTRACTORPLUGINKDE_NAME "createExtractorPluginKF5"
#  define RomPropertiesKDE RomPropertiesKF5
#elif QT_VERSION >= QT_VERSION_CHECK(4,0,0)
#  define PFN_CREATEEXTRACTORPLUGINKDE_FN createExtractorPluginKDE4
#  define PFN_CREATEEXTRACTORPLUGINKDE_NAME "createExtractorPluginKDE4"
#  define RomPropertiesKDE RomPropertiesKDE4
#else /* QT_VERSION < QT_VERSION_CHECK(4,0,0) */
#  error Qt version is too old
#endif

namespace RomPropertiesKF5 {

class ExtractorPlugin final : public ::KFileMetaData::ExtractorPlugin
{
	Q_OBJECT
	Q_INTERFACES(KFileMetaData::ExtractorPlugin)

	public:
		explicit ExtractorPlugin(QObject *parent = nullptr);

	private:
		typedef KFileMetaData::ExtractorPlugin super;
		Q_DISABLE_COPY(ExtractorPlugin);

	public:
		QStringList mimetypes(void) const final;
		void extract(KFileMetaData::ExtractionResult *result) final;
};

// Exported function pointer to create a new RpExtractorPlugin.
typedef ExtractorPlugin* (*pfn_createExtractorPluginKDE_t)(QObject *parent);

}

#endif /* __ROMPROPERTIES_KDE_EXTRACTORPLUGIN_HPP__ */

/***************************************************************************
 * ROM Properties Page shell extension. (KDE)                              *
 * stub-export.cpp: Exported function for the rp-config stub.              *
 *                                                                         *
 * Copyright (c) 2016-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "config.kde.h"
#include "check-uid.hpp"

#include "ConfigDialog.hpp"
#include "RomDataView.hpp"
#include "xattr/XAttrView.hpp"

// Program version
#include "librpbase/config/AboutTabText.hpp"
using namespace LibRpBase;

// Other rom-properties libraries
#include "libromdata/RomDataFactory.hpp"
#include "librptexture/img/rp_image.hpp"
using namespace LibRomData;
using namespace LibRpTexture;

#include "RpQImageBackend.hpp"
#include "AchQtDBus.hpp"

// i18n
#ifdef ENABLE_NLS
#  include "../GettextTranslator.hpp"
#endif

/**
 * Initialize the QApplication.
 * @param argc
 * @param argv
 * @return QApplication
 */
static QApplication *initQApp(int &argc, char *argv[])
{
	QApplication *app = qApp;
	if (app) {
		// QApplication is already initialized.

		// Initialize base i18n.
		// TODO: Install the translator even if we're reusing the QApplication?
		rp_i18n_init();

		return app;
	}

	// Set high-DPI mode on Qt 5. (not needed on Qt 6)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0) && QT_VERSION < QT_VERSION_CHECK(6,0,0)
	// Enable High DPI.
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
	// Enable High DPI pixmaps.
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#else
	// Hardcode the value in case the user upgrades to Qt 5.6 later.
	// http://doc.qt.io/qt-5/qt.html#ApplicationAttribute-enum
	QApplication::setAttribute(static_cast<Qt::ApplicationAttribute>(13), true);
#endif /* QT_VERSION >= QT_VERSION_CHECK(5,6,0) */
#endif /* QT_VERSION >= QT_VERSION_CHECK(5,0,0) && QT_VERSION < QT_VERSION_CHECK(6,0,0) */
	// Create the QApplication.
	app = new QApplication(argc, argv);

#ifdef ENABLE_NLS
	// Install the translator for Gettext translations.
	rp_i18n_init();
	app->installTranslator(new GettextTranslator());
#endif /* ENABLE_NLS */

	// Set the application information.
	// TODO: Different info for the RomDataView test program?
	app->setApplicationName(QLatin1String("rp-config"));
	app->setOrganizationDomain(QLatin1String("gerbilsoft.com"));
	app->setOrganizationName(QLatin1String("GerbilSoft"));
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
	app->setApplicationDisplayName(QCoreApplication::translate(
			"ConfigDialog", "ROM Properties Page configuration", nullptr));
#  if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
	app->setDesktopFileName(QLatin1String("com.gerbilsoft.rom-properties.rp-config"));
#  endif /* QT_VERSION >= QT_VERSION_CHECK(5,7,0) */
#endif /* QT_VERSION >= QT_VERSION_CHECK(5,0,0) */

	const char *const programVersion =
		AboutTabText::getProgramInfoString(AboutTabText::ProgramInfoStringID::ProgramVersion);
	assert(programVersion != nullptr);
	if (programVersion) {
		app->setApplicationVersion(QLatin1String(programVersion));
	}

	return app;
}

/**
 * Exported function for the rp-config stub.
 * @param argc
 * @param argv
 * @return 0 on success; non-zero on error.
 */
extern "C"
Q_DECL_EXPORT int RP_C_API rp_show_config_dialog(int argc, char *argv[])
{
	CHECK_UID_RET(EXIT_FAILURE);

	QApplication *const app = initQApp(argc, argv);

	// Create and run the ConfigDialog.
	// TODO: Get the return value?
	ConfigDialog *const configDialog = new ConfigDialog();
	configDialog->setObjectName(QLatin1String("configDialog"));
	configDialog->show();

	// Run the Qt UI.
	// FIXME: May need changes if the main loop is already running.
	return app->exec();
}

/**
 * Exported function for the RomDataView test program stub.
 * @param argc
 * @param argv
 * @return 0 on success; non-zero on error.
 */
extern "C"
Q_DECL_EXPORT int RP_C_API rp_show_RomDataView_dialog(int argc, char *argv[])
{
	CHECK_UID_RET(EXIT_FAILURE);

	// TODO: argv[] needs to be updated such that [0] == argv[0] and [1] == URI.
	// For now, assuming the last element is the URI.
	if (argc < 2) {
		// Not enough parameters...
		fputs("*** " RP_KDE_UPPER " rp_show_RomDataView_dialog(): ERROR: No URI specified.\n", stderr);
		return EXIT_FAILURE;
	}
	const char *const uri = argv[argc-1];

	QApplication *const app = initQApp(argc, argv);

	// Register RpQImageBackend and AchQtDBus.
	rp_image::setBackendCreatorFn(RpQImageBackend::creator_fn);
#if defined(ENABLE_ACHIEVEMENTS) && defined(HAVE_QtDBus_NOTIFY)
	AchQtDBus::instance();
#endif /* ENABLE_ACHIEVEMENTS && HAVE_QtDBus_NOTIFY */

	// Create a QDialog.
	QDialog *const dialog = new QDialog(nullptr,
		Qt::Dialog |
		Qt::CustomizeWindowHint |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowCloseButtonHint);
	dialog->setObjectName(QLatin1String("dialog"));
	dialog->show();

	// Create a QVBoxLayout for the dialog.
	QVBoxLayout *const vboxDialog = new QVBoxLayout(dialog);
	vboxDialog->setObjectName(QLatin1String("vboxDialog"));

	// Create a QTabWidget to simulate KDE Dolphin.
	QTabWidget *const tabWidget = new QTabWidget(dialog);
	tabWidget->setObjectName(QLatin1String("tabWidget"));
	vboxDialog->addWidget(tabWidget);

	// Create a button box for the standard buttons. (and RomDataView's "Options" button)
	QDialogButtonBox *const buttonBox = new QDialogButtonBox(dialog);
	buttonBox->setObjectName(QLatin1String("buttonBox"));
	buttonBox->setOrientation(Qt::Horizontal);
	buttonBox->setStandardButtons(QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
	vboxDialog->addWidget(buttonBox);
	QObject::connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

	// Open a RomData object.
	// TODO: Support URLs? Currently filenames only for testing.
	fprintf(stderr, "*** " RP_KDE_UPPER " rp_show_RomDataView_dialog(): Opening URI: '%s'\n", uri);
	RomDataPtr romData = RomDataFactory::create(argv[argc-1]);
	if (romData) {
		// Create a RomDataView object.
		RomDataView *const romDataView = new RomDataView(romData, dialog);
		romDataView->setObjectName(QLatin1String("romDataView"));
		tabWidget->addTab(romDataView, QLatin1String("ROM Properties"));
	} else {
		fputs("*** " RP_KDE_UPPER " rp_show_RomDataView_dialog(): RomData object could not be created for this URI.\n", stderr);
	}

#if 0
	// Create an XAttrView object.
	// FIXME: Need to reference the XAttrView plugin?
	XAttrView *const xattrView = new XAttrView(QUrl(QString::fromUtf8(uri)), dialog);
	if (xattrView->hasAttributes()) {
		xattrView->setObjectName(QLatin1String("xattrView"));
		tabWidget->addTab(romDataView, QLatin1String("xattrs"));
	} else {
		fputs("*** " RP_KDE_UPPER " rp_show_RomDataView_dialog(): No extended attributes found; not showing xattrs tab.\n", stderr);
		delete xattrView;
	}
#endif

	// Make sure we have at least one tab.
	if (tabWidget->count() < 1) {
		fputs("*** " RP_KDE_UPPER " rp_show_RomDataView_dialog(): No tabs were created; exiting.\n", stderr);
		return EXIT_FAILURE;
	}

	// Run the Qt UI.
	// FIXME: May need changes if the main loop is already running.
	fputs("*** " RP_KDE_UPPER " rp_show_RomDataView_dialog(): Starting main loop.\n", stderr);
	return app->exec();
}

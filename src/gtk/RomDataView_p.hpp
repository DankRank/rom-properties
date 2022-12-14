/***************************************************************************
 * ROM Properties Page shell extension. (GTK+ common)                      *
 * RomDataView.cpp: RomData viewer widget. (Private functions)             *
 *                                                                         *
 * Copyright (c) 2017-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_GTK_ROMDATAVIEW_P_HPP__
#define __ROMPROPERTIES_GTK_ROMDATAVIEW_P_HPP__

#include "config.gtk.h"
#include <gtk/gtk.h>

#include "OptionsMenuButton.hpp"

// librpbase
namespace LibRpBase {
	class RomData;
	class RomFields;
}

// C++ includes
#include <vector>

#if GTK_CHECK_VERSION(3,0,0)
typedef GtkBoxClass superclass;
typedef GtkBox super;
#define GTK_TYPE_SUPER GTK_TYPE_BOX
#define USE_GTK_GRID 1	// Use GtkGrid instead of GtkTable.
#else /* !GTK_CHECK_VERSION(3,0,0) */
typedef GtkVBoxClass superclass;
typedef GtkVBox super;
#define GTK_TYPE_SUPER GTK_TYPE_VBOX
#endif /* GTK_CHECK_VERSION(3,0,0) */

// GTK+ property page class
struct _RpRomDataViewClass {
	superclass __parent__;
};

// Multi-language stuff
typedef std::pair<GtkLabel*, const LibRpBase::RomFields::Field*> Data_StringMulti_t;

struct Data_ListDataMulti_t {
	GtkListStore *listStore;
	GtkTreeView *treeView;
	const LibRpBase::RomFields::Field *field;

	Data_ListDataMulti_t(
		GtkListStore *listStore,
		GtkTreeView *treeView,
		const LibRpBase::RomFields::Field *field)
		: listStore(listStore)
		, treeView(treeView)
		, field(field) { }
};

// C++ objects
struct _RpRomDataViewCxx {
	struct tab {
		GtkWidget	*vbox;		// Either parent page or a GtkVBox/GtkBox.
		GtkWidget	*table;		// GtkTable (2.x); GtkGrid (3.x)
		GtkWidget	*lblCredits;

		tab() : vbox(nullptr), table(nullptr), lblCredits(nullptr) { }
	};
	std::vector<tab> tabs;

	// Description labels
	std::vector<GtkLabel*> vecDescLabels;

	// RFT_STRING_MULTI value labels
	std::vector<Data_StringMulti_t> vecStringMulti;

	// RFT_LISTDATA_MULTI value GtkListStores
	std::vector<Data_ListDataMulti_t> vecListDataMulti;

	// Default language code for multi-language.
	uint32_t def_lc;
};

// GTK+ property page instance
struct _RpRomDataView {
	super __parent__;

	_RpRomDataViewCxx	*cxx;		// C++ objects
	LibRpBase::RomData	*romData;	// ROM data
	gchar			*uri;		// URI (GVfs)

	// Header row.
	GtkWidget	*hboxHeaderRow_outer;
	GtkWidget	*hboxHeaderRow;
	GtkWidget	*lblSysInfo;
	GtkWidget	*imgIcon;
	GtkWidget	*imgBanner;

	// Tab layout.
	GtkWidget	*tabWidget;
	// Tabs moved to: cxx->tabs

	/** ROM operations **/

	// "Options" button. (RpOptionsMenuButton)
	GtkWidget	*btnOptions;
	gchar		*prevExportDir;

	// MessageWidget for ROM operation notifications.
	GtkWidget	*messageWidget;

	/** Multi-language **/

	// Multi-language combo box.
	GtkWidget	*cboLanguage;

	/* Timeouts */
	guint		changed_idle;

	/** Other **/

	// Description label format type.
	RpDescFormatType	desc_format_type;

	// Inhibit checkbox toggling for RFT_BITFIELD while updating.
	bool inhibit_checkbox_no_toggle;
	// Have we checked for achievements?
	bool hasCheckedAchievements;
};

G_BEGIN_DECLS

static GQuark RFT_BITFIELD_value_quark;
static GQuark RFT_LISTDATA_rows_visible_quark;
static GQuark RFT_fieldIdx_quark;
static GQuark RFT_STRING_warning_quark;
static GQuark RomDataView_romOp_quark;

int	rp_rom_data_view_update_field		(RpRomDataView		*page,
						 int			 fieldIdx);

void	btnOptions_triggered_signal_handler	(RpOptionsMenuButton	*menuButton,
						 gint		 	 id,
						 RpRomDataView		*page);

G_END_DECLS

#endif /* __ROMPROPERTIES_GTK_ROMDATAVIEW_P_HPP__ */

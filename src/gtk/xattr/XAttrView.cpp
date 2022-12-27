/***************************************************************************
 * ROM Properties Page shell extension. (GTK+ common)                      *
 * XAttrView.cpp: MS-DOS file system attribute viewer widget.              *
 *                                                                         *
 * Copyright (c) 2017-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "XAttrView.hpp"

// XAttrReader
#include "librpfile/xattr/XAttrReader.hpp"
using LibRpFile::XAttrReader;

// Attribute viewer widgets
#include "LinuxAttrView.h"
#include "DosAttrView.h"

/* Property identifiers */
typedef enum {
	PROP_0,

	PROP_URI,

	PROP_LAST
} RpXAttrViewPropID;

static void	rp_xattr_view_set_property(GObject	*object,
					   guint	 prop_id,
					   const GValue	*value,
					   GParamSpec	*pspec);
static void	rp_xattr_view_get_property(GObject	*object,
					   guint	 prop_id,
					   GValue	*value,
					   GParamSpec	*pspec);
static void	rp_xattr_view_finalize    (GObject	*object);

/** Update display **/
static int	rp_xattr_view_load_attributes(RpXAttrView *widget);
static void	rp_xattr_view_clear_display_widgets(RpXAttrView *widget);

static GParamSpec *props[PROP_LAST];

/* Column identifiers */
typedef enum {
	XATTR_COL_NAME,
	XATTR_COL_VALUE,

	XATTR_COL_MAX
} XAttrColumns;

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

// XAttrView class
struct _RpXAttrViewClass {
	superclass __parent__;
};

// XAttrView instance
struct _RpXAttrView {
	super __parent__;

	gchar *uri;
	XAttrReader *xattrReader;
	gboolean has_attributes;

	GtkWidget *fraLinuxAttributes;
	GtkWidget *linuxAttrView;

	GtkWidget *fraDosAttributes;
	GtkWidget *dosAttrView;

	GtkWidget	*fraXAttr;
	GtkListStore	*listStore;
	GtkWidget	*treeView;
};

// NOTE: G_DEFINE_TYPE() doesn't work in C++ mode with gcc-6.2
// due to an implicit int to GTypeFlags conversion.
G_DEFINE_TYPE_EXTENDED(RpXAttrView, rp_xattr_view,
	GTK_TYPE_SUPER, (GTypeFlags)0, {});

static void
rp_xattr_view_class_init(RpXAttrViewClass *klass)
{
	GObjectClass *const gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->set_property = rp_xattr_view_set_property;
	gobject_class->get_property = rp_xattr_view_get_property;
	gobject_class->finalize = rp_xattr_view_finalize;

	/** Properties **/

	props[PROP_URI] = g_param_spec_string(
		"uri", "URI", "URI of the file being displayed.",
		nullptr,
		(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	// Install the properties.
	g_object_class_install_properties(gobject_class, PROP_LAST, props);
}

static void
rp_xattr_view_init(RpXAttrView *widget)
{
#if GTK_CHECK_VERSION(3,0,0)
	// Make this a VBox.
	gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_VERTICAL);
#endif /* GTK_CHECK_VERSION(3,0,0) */

	// Linux attributes
	widget->fraLinuxAttributes = gtk_frame_new(C_("XAttrView", "Linux Attributes"));
	gtk_widget_set_name(widget->fraLinuxAttributes, "fraLinuxAttributes");
	GtkWidget *const vboxLinuxAttributes = rp_gtk_vbox_new(0);
	gtk_widget_set_name(vboxLinuxAttributes, "vboxLinuxAttributes");
	widget->linuxAttrView = rp_linux_attr_view_new();
	gtk_widget_set_name(widget->linuxAttrView, "linuxAttrView");

	// MS-DOS attributes
	widget->fraDosAttributes = gtk_frame_new(C_("XAttrView", "MS-DOS Attributes"));
	gtk_widget_set_name(widget->fraDosAttributes, "fraDosAttributes");
	GtkWidget *const vboxDosAttributes = rp_gtk_vbox_new(0);
	gtk_widget_set_name(vboxDosAttributes, "vboxDosAttributes");
	widget->dosAttrView = rp_dos_attr_view_new();
	gtk_widget_set_name(widget->dosAttrView, "dosAttrView");

	// Extended attributes
	widget->fraXAttr = gtk_frame_new(C_("XAttrView", "Extended Attributes"));
	gtk_widget_set_name(widget->fraXAttr, "fraXAttr");

	// Scroll area for the GtkTreeView.
#if GTK_CHECK_VERSION(4,0,0)
	GtkWidget *const scrlXAttr = gtk_scrolled_window_new();
	gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scrlXAttr), true);
#else /* !GTK_CHECK_VERSION(4,0,0) */
	GtkWidget *const scrlXAttr = gtk_scrolled_window_new(nullptr, nullptr);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrlXAttr), GTK_SHADOW_IN);
#endif /* GTK_CHECK_VERSION */
	gtk_widget_set_name(scrlXAttr, "scrlXAttr");
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlXAttr),
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
#if GTK_CHECK_VERSION(2,91,1)
	gtk_widget_set_halign(scrlXAttr, GTK_ALIGN_FILL);
	gtk_widget_set_valign(scrlXAttr, GTK_ALIGN_FILL);
	gtk_widget_set_hexpand(scrlXAttr, TRUE);
	gtk_widget_set_vexpand(scrlXAttr, TRUE);
	gtk_widget_set_margin(scrlXAttr, 6);	// TODO: GTK2 version
#endif /* GTK_CHECK_VERSION(2,91,1) */

	// Column titles
	static const char *const column_titles[XATTR_COL_MAX] = {
		NOP_C_("XAttrView", "Name"),
		NOP_C_("XAttrView", "Value"),
	};

	// TODO: GtkListView version for GTK4

	// Create the GtkListStore and GtkTreeView.
	widget->listStore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	widget->treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(widget->listStore));
	gtk_widget_set_name(widget->treeView, "treeView");
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget->treeView), TRUE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrlXAttr), widget->treeView);

#if !GTK_CHECK_VERSION(3,0,0)
	// GTK+ 2.x: Use the "rules hint" for alternating row colors.
	// Deprecated in GTK+ 3.14 (and removed in GTK4), but it doesn't
	// work with GTK+ 3.x anyway.
	// TODO: GTK4's GtkListView might have a similar function.
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(widget->treeView), true);
#endif /* !GTK_CHECK_VERSION(3,0,0) */

	// Create the columns.
	// NOTE: Unlock Time is stored as a string, not as a GDateTime or Unix timestamp.
	for (int i = 0; i < XATTR_COL_MAX; i++) {
		GtkTreeViewColumn *const column = gtk_tree_view_column_new();
		gtk_tree_view_column_set_title(column,
			dpgettext_expr(RP_I18N_DOMAIN, "XAttrView", column_titles[i]));
		gtk_tree_view_column_set_resizable(column, TRUE);

		GtkCellRenderer *const renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(column, renderer, FALSE);
		gtk_tree_view_column_add_attribute(column, renderer, "text", i);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget->treeView), column);
	}

#if GTK_CHECK_VERSION(4,0,0)
	gtk_box_append(GTK_BOX(vboxLinuxAttributes), widget->linuxAttrView);
	gtk_box_append(GTK_BOX(vboxDosAttributes), widget->dosAttrView);
#else /* !GTK_CHECK_VERSION(4,0,0) */
	gtk_box_pack_start(GTK_BOX(vboxLinuxAttributes), widget->linuxAttrView, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vboxDosAttributes), widget->dosAttrView, FALSE, FALSE, 0);
#endif /* GTK_CHECK_VERSION(4,0,0) */

#if GTK_CHECK_VERSION(2,91,0)
	gtk_widget_set_margin(widget->fraLinuxAttributes, 6);
	gtk_widget_set_margin(widget->fraDosAttributes, 6);
	gtk_widget_set_margin(widget->fraXAttr, 6);
	gtk_widget_set_margin(vboxLinuxAttributes, 6);
	gtk_widget_set_margin(vboxDosAttributes, 6);
	gtk_frame_set_child(GTK_FRAME(widget->fraLinuxAttributes), vboxLinuxAttributes);
	gtk_frame_set_child(GTK_FRAME(widget->fraDosAttributes), vboxDosAttributes);
	gtk_frame_set_child(GTK_FRAME(widget->fraXAttr), scrlXAttr);
#else /* !GTK_CHECK_VERSION(2,91,0) */
	// NOTE: The extra alignments outside the frame reduce the frame widths.
	// This only affects GTK2, so meh.
	GtkWidget *const alignVboxLinuxAttributes = gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f);
	gtk_widget_set_name(alignVboxLinuxAttributes, "alignVboxLinuxAttributes");
	gtk_widget_show(alignVboxLinuxAttributes);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignVboxLinuxAttributes), 6, 6, 6, 6);
	gtk_container_add(GTK_CONTAINER(alignVboxLinuxAttributes), vboxLinuxAttributes);
	gtk_frame_set_child(GTK_FRAME(widget->fraLinuxAttributes), alignVboxLinuxAttributes);

	GtkWidget *const alignFraLinuxAttributes = gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f);
	gtk_widget_set_name(alignFraLinuxAttributes, "alignFraLinuxAttributes");
	gtk_widget_show(alignFraLinuxAttributes);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignFraLinuxAttributes), 6, 6, 6, 6);
	gtk_container_add(GTK_CONTAINER(alignFraLinuxAttributes), widget->fraLinuxAttributes);

	GtkWidget *const alignVboxDosAttributes = gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f);
	gtk_widget_set_name(alignVboxDosAttributes, "alignVboxDosAttributes");
	gtk_widget_show(alignVboxDosAttributes);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignVboxDosAttributes), 6, 6, 6, 6);
	gtk_container_add(GTK_CONTAINER(alignVboxDosAttributes), vboxDosAttributes);
	gtk_frame_set_child(GTK_FRAME(widget->fraDosAttributes), alignVboxDosAttributes);

	GtkWidget *const alignFraDosAttributes = gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f);
	gtk_widget_set_name(alignFraDosAttributes, "alignFraDosAttributes");
	gtk_widget_show(alignFraDosAttributes);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignFraDosAttributes), 6, 6, 6, 6);
	gtk_container_add(GTK_CONTAINER(alignFraDosAttributes), widget->fraDosAttributes);

	GtkWidget *const alignScrlXAttr = gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f);
	gtk_widget_set_name(alignScrlXAttr, "alignScrlXAttr");
	gtk_widget_show(alignScrlXAttr);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignScrlXAttr), 6, 6, 6, 6);
	gtk_container_add(GTK_CONTAINER(alignScrlXAttr), scrlXAttr);
	gtk_frame_set_child(GTK_FRAME(widget->fraXAttr), alignScrlXAttr);

	GtkWidget *const alignFraXAttr = gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f);
	gtk_widget_set_name(alignFraXAttr, "alignFraXAttr");
	gtk_widget_show(alignFraXAttr);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignFraXAttr), 6, 6, 6, 6);
	gtk_container_add(GTK_CONTAINER(alignFraXAttr), widget->fraXAttr);
#endif /* GTK_CHECK_VERSION(2,91,0) */

#if GTK_CHECK_VERSION(4,0,0)
	gtk_box_append(GTK_BOX(widget), widget->fraLinuxAttributes);
	gtk_box_append(GTK_BOX(widget), widget->fraDosAttributes);
	gtk_box_append(GTK_BOX(widget), widget->fraXAttr);	// TODO: Expand?
#else /* !GTK_CHECK_VERSION(4,0,0) */
#  if GTK_CHECK_VERSION(2,91,0)
	gtk_box_pack_start(GTK_BOX(widget), widget->fraLinuxAttributes, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(widget), widget->fraDosAttributes, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(widget), widget->fraXAttr, TRUE, TRUE, 0);
#  else /* !GTK_CHECK_VERSION(2,91,0) */
	gtk_box_pack_start(GTK_BOX(widget), alignFraLinuxAttributes, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(widget), alignFraDosAttributes, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(widget), alignFraXAttr, TRUE, TRUE, 0);	// FIXME: Expand isn't working here.
#  endif /* GTK_CHECK_VERSION(2,91,0) */
	gtk_widget_show(widget->fraLinuxAttributes);
	gtk_widget_show(widget->fraDosAttributes);
	gtk_widget_show(widget->fraXAttr);
	gtk_widget_show(vboxLinuxAttributes);
	gtk_widget_show(vboxDosAttributes);
	gtk_widget_show(widget->linuxAttrView);
	gtk_widget_show(widget->dosAttrView);
	gtk_widget_show(scrlXAttr);
	gtk_widget_show(widget->treeView);
#endif /* GTK_CHECK_VERSION(4,0,0) */
}

/**
 * Create a new XAttrView widget.
 * @param uri URI
 * @return XAttrView
 */
GtkWidget*
rp_xattr_view_new(const gchar *uri)
{
	return (GtkWidget*)g_object_new(RP_TYPE_XATTR_VIEW, "uri", uri, NULL);
}

/** Properties **/

static void
rp_xattr_view_set_property(GObject	*object,
			   guint	 prop_id,
			   const GValue	*value,
			   GParamSpec	*pspec)
{
	RpXAttrView *const widget = RP_XATTR_VIEW(object);

	switch (prop_id) {
		case PROP_URI: {
			const gchar *uri = g_value_get_string(value);
			if (g_strcmp0(widget->uri, uri) != 0) {
				// URI has changed.
				g_free(widget->uri);
				widget->uri = g_strdup(uri);
				rp_xattr_view_load_attributes(widget);
			}
			break;
		}

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
rp_xattr_view_get_property(GObject		*object,
				guint		 prop_id,
				GValue		*value,
				GParamSpec	*pspec)
{
	RpXAttrView *const widget = RP_XATTR_VIEW(object);

	switch (prop_id) {
		case PROP_URI:
			g_value_set_string(value, widget->uri);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

/** Dispose / Finalize **/

static void
rp_xattr_view_finalize(GObject *object)
{
	RpXAttrView *const widget = RP_XATTR_VIEW(object);

	g_free(widget->uri);
	delete widget->xattrReader;

	// Call the superclass finalize() function.
	G_OBJECT_CLASS(rp_xattr_view_parent_class)->finalize(object);
}

/** Update display **/

/**
 * Load Linux attributes, if available.
 * @param widget XAttrView
 * @return 0 on success; negative POSIX error code on error.
 */
static int
rp_xattr_view_load_linux_attrs(RpXAttrView *widget)
{
	// Hide by default.
	// If we do have attributes, we'll show the widgets there.
	gtk_widget_hide(widget->fraLinuxAttributes);

	if (!widget->xattrReader->hasLinuxAttributes()) {
		// No Linux attributes.
		return -ENOENT;
	}

	// We have Linux attributes.
	rp_linux_attr_view_set_flags(RP_LINUX_ATTR_VIEW(widget->linuxAttrView),
		widget->xattrReader->linuxAttributes());
	gtk_widget_show(widget->fraLinuxAttributes);
	return 0;
}

/**
 * Load MS-DOS attributes, if available.
 * @param widget XAttrView
 * @return 0 on success; negative POSIX error code on error.
 */
static int
rp_xattr_view_load_dos_attrs(RpXAttrView *widget)
{
	// Hide by default.
	// If we do have attributes, we'll show the widgets there.
	gtk_widget_hide(widget->fraDosAttributes);

	if (!widget->xattrReader->hasDosAttributes()) {
		// No Linux attributes.
		return -ENOENT;
	}

	// We have Linux attributes.
	rp_dos_attr_view_set_attrs(RP_DOS_ATTR_VIEW(widget->dosAttrView),
		widget->xattrReader->dosAttributes());
	gtk_widget_show(widget->fraDosAttributes);
	return 0;
}

/**
 * Load POSIX xattrs, if available.
 * @param widget XAttrView
 * @return 0 on success; negative POSIX error code on error.
 */
static int
rp_xattr_view_load_posix_xattrs(RpXAttrView *widget)
{
	// Hide by default.
	// If we do have attributes, we'll show the widgets there.
	gtk_widget_hide(widget->fraXAttr);

	if (!widget->xattrReader->hasGenericXAttrs()) {
		// No generic attributes.
		return -ENOENT;
	}

	gtk_list_store_clear(widget->listStore);
	const XAttrReader::XAttrList &xattrList = widget->xattrReader->genericXAttrs();
	for (const auto &xattr : xattrList) {
		GtkTreeIter treeIter;
		gtk_list_store_append(widget->listStore, &treeIter);
		gtk_list_store_set(widget->listStore, &treeIter,
			0, xattr.first.c_str(), 1, xattr.second.c_str(), -1);
	}

	// Resize the columns to fit the contents.
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(widget->treeView));

	// Extended attributes retrieved.
	gtk_widget_show(widget->fraXAttr);
	return 0;
}

/**
 * Load the attributes from the specified file.
 * The attributes will be loaded into the display widgets.
 * @param widget XAttrView
 * @return 0 on success; negative POSIX error code on error.
 */
static int
rp_xattr_view_load_attributes(RpXAttrView *widget)
{
	// Attempt to open the file.
	gchar *const filename = g_filename_from_uri(widget->uri, nullptr, nullptr);
	if (!filename) {
		// Not a local file.
		widget->has_attributes = false;
		delete widget->xattrReader;
		widget->xattrReader = nullptr;
		return -EIO;
	}

	// Close the XAttrReader if it's already open.
	delete widget->xattrReader;

	// Open an XAttrReader.
	widget->xattrReader = new XAttrReader(filename);
	g_free(filename);
	int err = widget->xattrReader->lastError();
	if (err != 0) {
		// Error reading attributes.
		delete widget->xattrReader;
		widget->xattrReader = nullptr;
		return err;
	}

	// Load the attributes.
	bool hasAnyAttrs = false;
	int ret = rp_xattr_view_load_linux_attrs(widget);
	if (ret == 0) {
		hasAnyAttrs = true;
	}
	ret = rp_xattr_view_load_dos_attrs(widget);
	if (ret == 0) {
		hasAnyAttrs = true;
	}
	ret = rp_xattr_view_load_posix_xattrs(widget);
	if (ret == 0) {
		hasAnyAttrs = true;
	}

	// If we have attributes, great!
	// If not, clear the display widgets.
	if (hasAnyAttrs) {
		widget->has_attributes = true;
	} else {
		widget->has_attributes = false;
		rp_xattr_view_clear_display_widgets(widget);
	}
	return 0;
}

/**
 * Clear the display widgets.
 * @param widget XAttrView
 */
static void
rp_xattr_view_clear_display_widgets(RpXAttrView *widget)
{
	rp_linux_attr_view_clear_flags(RP_LINUX_ATTR_VIEW(widget->linuxAttrView));
	rp_dos_attr_view_clear_attrs(RP_DOS_ATTR_VIEW(widget->dosAttrView));
}

/** Property accessors / mutators **/

/**
 * Set the current URI.
 * @param widget XAttrView
 * @param uri URI
 */
void
rp_xattr_view_set_uri(RpXAttrView *widget, const gchar *uri)
{
	g_return_if_fail(RP_IS_XATTR_VIEW(widget));
	if (g_strcmp0(widget->uri, uri) != 0) {
		g_free(widget->uri),
		widget->uri = g_strdup(uri);
		rp_xattr_view_load_attributes(widget);
		g_object_notify_by_pspec(G_OBJECT(widget), props[PROP_URI]);
	}
}

/**
 * Get the current URI.
 * @param widget XAttrView
 * @return URI
 */
const gchar*
rp_xattr_view_get_uri(RpXAttrView *widget)
{
	g_return_val_if_fail(RP_IS_XATTR_VIEW(widget), 0);
	return widget->uri;
}

/**
 * Are attributes loaded from the current URI?
 * @param widget RpXAttrView
 * @return True if we have attributes; false if not.
 */
gboolean
rp_xattr_view_has_attributes(RpXAttrView *widget)
{
	g_return_val_if_fail(RP_IS_XATTR_VIEW(widget), FALSE);
	return widget->has_attributes;
}

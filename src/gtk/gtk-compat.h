/***************************************************************************
 * ROM Properties Page shell extension. (GTK+ common)                      *
 * gtk-compat.h: GTK+ compatibility functions.                             *
 *                                                                         *
 * Copyright (c) 2017-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_GTK_GTK_COMPAT_H__
#define __ROMPROPERTIES_GTK_GTK_COMPAT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(2,14,0)
static inline GdkWindow*
gtk_widget_get_window(GtkWidget *widget)
{
	g_return_val_if_fail(widget != NULL, NULL);
	return widget->window;
}
#endif /* GTK_CHECK_VERSION(2,14,0) */

#if !GTK_CHECK_VERSION(3,0,0)
static inline GtkWidget*
gtk_tree_view_column_get_button(GtkTreeViewColumn *tree_column)
{
	// Reference: https://github.com/kynesim/wireshark/blob/master/ui/gtk/old-gtk-compat.h
	g_return_val_if_fail(tree_column != NULL, NULL);

	/* This is too late, see https://bugzilla.gnome.org/show_bug.cgi?id=641089
	 * According to
	 * http://ftp.acc.umu.se/pub/GNOME/sources/gtk+/2.13/gtk+-2.13.4.changes
	 * access to the button element was sealed during 2.13. They also admit that
	 * they missed a use case and thus failed to provide an accessor function:
	 * http://mail.gnome.org/archives/commits-list/2010-December/msg00578.html
	 * An accessor function was finally added in 3.0.
	 */
#  if (GTK_CHECK_VERSION(2,14,0) && defined(GSEAL_ENABLE))
	return tree_column->_g_sealed__button;
#  else
	return tree_column->button;
#  endif
}
#endif /* !GTK_CHECK_VERSION(3,0,0) */

#if !GTK_CHECK_VERSION(3,2,0)
static inline gboolean
gdk_event_get_button(const GdkEvent *event, guint *button)
{
	// from GTK+ source
	gboolean fetched = TRUE;
	guint number = 0;

	g_return_val_if_fail (event != NULL, FALSE);

	switch ((guint) event->any.type)
	{
		case GDK_BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:
			number = event->button.button;
			break;
		default:
			fetched = FALSE;
			break;
	}

	if (button)
		*button = number;

	return fetched;
}
#endif /* !GTK_CHECK_VERSION(3,2,0) */

#if !GTK_CHECK_VERSION(3,10,0)
static inline GdkEventType
gdk_event_get_event_type (const GdkEvent *event)
{
	g_return_val_if_fail(event != NULL, GDK_NOTHING);

	return event->any.type;
}
#endif /* !GTK_CHECK_VERSION(3,10,0) */

#if !GTK_CHECK_VERSION(4,0,0)
static inline void
gtk_scrolled_window_set_child(GtkScrolledWindow *scrolled_window, GtkWidget *child)
{
	// TODO: Remove the exisitng child widget?
	gtk_container_add(GTK_CONTAINER(scrolled_window), child);
}
#endif /* GTK_CHECK_VERSION(4,0,0) */

#if GTK_CHECK_VERSION(4,0,0)
#  define GTK_CSS_PROVIDER_LOAD_FROM_DATA(provider, data, length) \
	gtk_css_provider_load_from_data((provider), (data), (length))
#else /* !GTK_CHECK_VERSION(4,0,0) */
#  define GTK_CSS_PROVIDER_LOAD_FROM_DATA(provider, data, length) \
	gtk_css_provider_load_from_data((provider), (data), (length), NULL)
#endif /* GTK_CHECK_VERSION(4,0,0) */

// Clipboard
#if GTK_CHECK_VERSION(4,0,0)
static inline void
rp_gtk_main_clipboard_set_text(const char *text)
{
	GValue value = G_VALUE_INIT;
	g_value_init(&value, G_TYPE_STRING);
	g_value_set_string(&value, text);

	GdkDisplay *const display = gdk_display_get_default();
	GdkClipboard *const clipboard = gdk_display_get_clipboard(display);
	gdk_clipboard_set_value(clipboard, &value);

	g_value_unset(&value);
}
#else /* !GTK_CHECK_VERSION(4,0,0) */
static inline void
rp_gtk_main_clipboard_set_text(const char *text)
{
	GtkClipboard *const clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(clipboard, text, -1);
}
#endif /* GTK_CHECK_VERSION(4,0,0) */

#if GTK_CHECK_VERSION(4,0,0)
#define GTK_WIDET_GET_TOPLEVEL_FN(name, type, macro) \
static inline type* \
gtk_widget_get_toplevel_##name(GtkWidget *widget) \
{ \
	return macro(gtk_widget_get_root(widget)); \
}
#else /* !GTK_CHECK_VERSION(4,0,0) */
#define GTK_WIDET_GET_TOPLEVEL_FN(name, type, macro) \
static inline type* \
gtk_widget_get_toplevel_##name(GtkWidget *widget) \
{ \
	return macro(gtk_widget_get_toplevel(widget)); \
}
#endif /* GTK_CHECK_VERSION(4,0,0) */

GTK_WIDET_GET_TOPLEVEL_FN(widget, GtkWidget, GTK_WIDGET)
GTK_WIDET_GET_TOPLEVEL_FN(window, GtkWindow, GTK_WINDOW)
GTK_WIDET_GET_TOPLEVEL_FN(dialog, GtkDialog, GTK_DIALOG)

#if !GTK_CHECK_VERSION(4,0,0)
static inline GtkWidget*
gtk_widget_get_first_child(GtkWidget *widget)
{
	GtkWidget *ret = NULL;

	// Assuming this is a GtkContainer.
	assert(GTK_IS_CONTAINER(widget));
	GList *const widgetList = gtk_container_get_children(GTK_CONTAINER(widget));
	if (!widgetList)
		return ret;
	// NOTE: First widget in the list matches the first widget in the
	// UI file, contrary to the bitfield stuff in RomDataView...
	GList *const widgetIter = g_list_first(widgetList);
	assert(widgetIter != NULL);
	if (widgetIter)
		ret = GTK_WIDGET(widgetIter->data);
	g_list_free(widgetList);
	return ret;
}
#endif /* !GTK_CHECK_VERSION(4,0,0) */

#if !GTK_CHECK_VERSION(4,0,0)
static inline void gtk_widget_class_set_activate_signal(GtkWidgetClass *widget_class, guint signal_id)
{
	widget_class->activate_signal = signal_id;
}
static inline guint gtk_widget_class_get_activate_signal(GtkWidgetClass *widget_class)
{
	return widget_class->activate_signal;
}
#endif /* !GTK_CHECK_VERSION(4,0,0) */

#if !GTK_CHECK_VERSION(4,0,0)
static inline void gtk_frame_set_child(GtkFrame *frame, GtkWidget *child)
{
	gtk_container_add(GTK_CONTAINER(frame), child);
}
#endif /* !GTK_CHECK_VERSION(4,0,0) */

static inline GtkWidget*
RP_gtk_vbox_new(gint spacing)
{
#if GTK_CHECK_VERSION(3,0,0)
	return gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
#else /* GTK_CHECK_VERSION(3,0,0) */
	return gtk_vbox_new(FALSE, spacing);
#endif
}

static inline GtkWidget*
RP_gtk_hbox_new(gint spacing)
{
#if GTK_CHECK_VERSION(3,0,0)
	return gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
#else /* GTK_CHECK_VERSION(3,0,0) */
	return gtk_hbox_new(FALSE, spacing);
#endif
}

#if GTK_CHECK_VERSION(3,0,0)
#  define GTK_WIDGET_HALIGN_LEFT(widget)	gtk_widget_set_halign((widget), GTK_ALIGN_START)
#  define GTK_WIDGET_HALIGN_CENTER(widget)	gtk_widget_set_halign((widget), GTK_ALIGN_CENTER)
#  define GTK_WIDGET_HALIGN_RIGHT(widget)	gtk_widget_set_halign((widget), GTK_ALIGN_END)
#else
#  define GTK_WIDGET_HALIGN_LEFT(widget)	gtk_misc_set_alignment(GTK_MISC(widget), 0.0f, 0.0f)
#  define GTK_WIDGET_HALIGN_CENTER(widget)	gtk_misc_set_alignment(GTK_MISC(widget), 0.5f, 0.0f)
#  define GTK_WIDGET_HALIGN_RIGHT(widget)	gtk_misc_set_alignment(GTK_MISC(widget), 1.0f, 0.0f)
#endif

#if GTK_CHECK_VERSION(3,16,0)
#  define GTK_LABEL_XALIGN_LEFT(label)		gtk_label_set_xalign(GTK_LABEL(label), 0.0f)
#  define GTK_LABEL_XALIGN_CENTER(label)	gtk_label_set_xalign(GTK_LABEL(label), 0.5f)
#  define GTK_LABEL_XALIGN_RIGHT(label)		gtk_label_set_xalign(GTK_LABEL(label), 1.0f)
#else
#  define GTK_LABEL_XALIGN_LEFT(label)		gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.0f)
#  define GTK_LABEL_XALIGN_CENTER(label)	gtk_misc_set_alignment(GTK_MISC(label), 0.5f, 0.0f)
#  define GTK_LABEL_XALIGN_RIGHT(label)		gtk_misc_set_alignment(GTK_MISC(label), 1.0f, 0.0f)
#endif

/**
 * Set margin for all four sides.
 * @param widget GtkWidget
 */
static inline void
gtk_widget_set_margin(GtkWidget *widget, gint margin)
{
	// TODO: GTK2 version.
#if GTK_CHECK_VERSION(3,12,0)
	gtk_widget_set_margin_start(widget, margin);
	gtk_widget_set_margin_end(widget, margin);
	gtk_widget_set_margin_top(widget, margin);
	gtk_widget_set_margin_bottom(widget, margin);
#elif GTK_CHECK_VERSION(3,0,0)
	gtk_widget_set_margin_left(widget, margin);
	gtk_widget_set_margin_right(widget, margin);
	gtk_widget_set_margin_top(widget, margin);
	gtk_widget_set_margin_bottom(widget, margin);
#endif
}

G_END_DECLS

#endif /* __ROMPROPERTIES_GTK_GTK_COMPAT_H__ */

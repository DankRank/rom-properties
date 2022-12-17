/***************************************************************************
 * ROM Properties Page shell extension. (GTK+ common)                      *
 * AchievementItem.cpp: Achievement ComboBox Item (for GtkDropDown)        *
 *                                                                         *
 * Copyright (c) 2017-2022 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "AchievementItem.hpp"

// librpbase
using namespace LibRpBase;

// C++ STL classes
#include <limits>
using std::string;

/* Property identifiers */
typedef enum {
	PROP_0,

	PROP_ICON,
	PROP_DESCRIPTION,
	PROP_UNLOCK_TIME,

	PROP_LAST
} RpAchievementPropID;

static void	rp_achievement_item_set_property(GObject		*object,
							guint		 prop_id,
							const GValue	*value,
							GParamSpec	*pspec);
static void	rp_achievement_item_get_property(GObject		*object,
							guint		 prop_id,
							GValue		*value,
							GParamSpec	*pspec);

static void	rp_achievement_item_dispose     (GObject		*object);
static void	rp_achievement_item_finalize    (GObject		*object);

static GParamSpec *props[PROP_LAST];

// AchievementItem class
struct _RpAchievementItemClass {
	GObjectClass __parent__;
};

// AchievementItem instance
struct _RpAchievementItem {
	GObject __parent__;

	PIMGTYPE	 icon;
	char		*description;

	// NOTE: GObject's property system doesn't appear to support GDateTime.
	// The property will be gint64 without GDateTime support.
	gint64		unlock_time;
};

// NOTE: G_DEFINE_TYPE() doesn't work in C++ mode with gcc-6.2
// due to an implicit int to GTypeFlags conversion.
G_DEFINE_TYPE_EXTENDED(RpAchievementItem, rp_achievement_item,
	G_TYPE_OBJECT, static_cast<GTypeFlags>(0), {});

static void
rp_achievement_item_class_init(RpAchievementItemClass *klass)
{
	GObjectClass *const gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->set_property = rp_achievement_item_set_property;
	gobject_class->get_property = rp_achievement_item_get_property;
	gobject_class->dispose = rp_achievement_item_dispose;
	gobject_class->finalize = rp_achievement_item_finalize;

	/** Properties **/

	props[PROP_ICON] = g_param_spec_object(
		"icon", "Icon", "Icon representing this achievement",
		PIMGTYPE_GOBJECT_TYPE,
		(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	props[PROP_DESCRIPTION] = g_param_spec_string(
		"description", "Descrpition", "Achievement description",
		"",
		(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	props[PROP_UNLOCK_TIME] = g_param_spec_int64(
		"unlock-time", "Unlock Time", "Timestamp when this achievement was unlocked",
		static_cast<gint64>(std::numeric_limits<gint64>::min()),
		static_cast<gint64>(std::numeric_limits<gint64>::max()),
		static_cast<gint64>(-1LL),
		(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	// Install the properties.
	g_object_class_install_properties(gobject_class, PROP_LAST, props);
}

static void
rp_achievement_item_init(RpAchievementItem *item)
{
	RP_UNUSED(item);
}

RpAchievementItem*
rp_achievement_item_new(PIMGTYPE icon, const char *description, time_t unlock_time)
{
	return static_cast<RpAchievementItem*>(
		g_object_new(RP_TYPE_ACHIEVEMENT_ITEM,
			"icon", icon,
			"description", description,
			"unlock-time", static_cast<gint64>(unlock_time),
			nullptr));
}

/** Properties **/

static void
rp_achievement_item_set_property(GObject	*object,
				 guint		 prop_id,
				 const GValue	*value,
				 GParamSpec	*pspec)
{
	RpAchievementItem *const item = RP_ACHIEVEMENT_ITEM(object);

	switch (prop_id) {
		case PROP_ICON: {
			PIMGTYPE const icon = PIMGTYPE_CAST(g_value_get_object(value));
			if (icon) {
				if (item->icon) {
					PIMGTYPE_unref(item->icon);
					item->icon = nullptr;
				}
				item->icon = PIMGTYPE_ref(icon);
			}
			break;
		}

		case PROP_DESCRIPTION:
			if (item->description) {
				g_free(item->description);
			}
			item->description = g_strdup(g_value_get_string(value));
			break;

		case PROP_UNLOCK_TIME:
			item->unlock_time = g_value_get_int64(value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}

	// TODO: Notification signals for GListModel?
}

static void
rp_achievement_item_get_property(GObject	*object,
				 guint		 prop_id,
				 GValue		*value,
				 GParamSpec	*pspec)
{
	RpAchievementItem *const item = RP_ACHIEVEMENT_ITEM(object);

	switch (prop_id) {
		case PROP_ICON:
			if (item->icon) {
				// Caller must take a reference.
				g_value_set_object(value, item->icon);
			}
			break;

		case PROP_DESCRIPTION:
			g_value_set_string(value, item->description);
			break;

		case PROP_UNLOCK_TIME:
			g_value_set_int64(value, item->unlock_time);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

/** Dispose / Finalize **/

static void
rp_achievement_item_dispose(GObject *object)
{
	RpAchievementItem *const item = RP_ACHIEVEMENT_ITEM(object);

	if (item->icon) {
		PIMGTYPE_unref(item->icon);
		item->icon = nullptr;
	}

	// Call the superclass dispose() function.
	G_OBJECT_CLASS(rp_achievement_item_parent_class)->dispose(object);
}


static void
rp_achievement_item_finalize(GObject *object)
{
	RpAchievementItem *const item = RP_ACHIEVEMENT_ITEM(object);

	if (item->description) {
		g_free(item->description);
		item->description = nullptr;
	}

	// Call the superclass finalize() function.
	G_OBJECT_CLASS(rp_achievement_item_parent_class)->finalize(object);
}

/** Property accessors / mutators **/

void
rp_achievement_item_set_icon(RpAchievementItem *item, PIMGTYPE icon)
{
	g_return_if_fail(RP_IS_ACHIEVEMENT_ITEM(item));

	if (item->icon) {
		PIMGTYPE_unref(item->icon);
	}
	item->icon = PIMGTYPE_ref(icon);

	g_object_notify_by_pspec(G_OBJECT(item), props[PROP_ICON]);

}

PIMGTYPE
rp_achievement_item_get_icon(RpAchievementItem *item)
{
	g_return_val_if_fail(RP_IS_ACHIEVEMENT_ITEM(item), nullptr);

	// Caller must take a reference to the icon.
	return item->icon;
}

void
rp_achievement_item_set_description(RpAchievementItem *item, const char *description)
{
	g_return_if_fail(RP_IS_ACHIEVEMENT_ITEM(item));

	if (item->description) {
		g_free(item->description);
	}
	item->description = g_strdup(description);

	g_object_notify_by_pspec(G_OBJECT(item), props[PROP_DESCRIPTION]);

}

const char*
rp_achievement_item_get_description(RpAchievementItem *item)
{
	g_return_val_if_fail(RP_IS_ACHIEVEMENT_ITEM(item), nullptr);

	// String is owned by this instance.
	return item->description;
}

void
rp_achievement_item_set_unlock_time(RpAchievementItem *item, time_t unlock_time)
{
	g_return_if_fail(RP_IS_ACHIEVEMENT_ITEM(item));

	item->unlock_time = static_cast<gint64>(unlock_time);
	g_object_notify_by_pspec(G_OBJECT(item), props[PROP_UNLOCK_TIME]);

}

time_t
rp_achievement_item_get_unlock_time(RpAchievementItem *item)
{
	g_return_val_if_fail(RP_IS_ACHIEVEMENT_ITEM(item), -1);
	return item->unlock_time;
}

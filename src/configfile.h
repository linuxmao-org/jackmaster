/*
 *  Read and write windows style .ini files
 *
 *  Copyright (C) 2010 Lutz Moehrmann <moehre _AT_ 69b.org>
 *
 *  This file is part of JackMaster.
 *  mostly "borrowed" from:
 *
 *  XMMS - Cross-platform multimedia player
 *  Copyright (C) 1998-2000  Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the
 *  Free Software Foundation, Inc.,
 *  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LIBCONFIGFILE_H
#define LIBCONFIGFILE_H

#include <glib.h>

typedef struct
{
	gchar *key;
	gchar *value;
}
ConfigLine;

typedef struct
{
	gchar *name;
	GList *lines;
}
ConfigSection;

typedef struct
{
	GList *sections;
}
ConfigFile;

#ifdef __cplusplus
extern "C" {
#endif

ConfigFile *libcfg_new(void);
ConfigFile *libcfg_open_file(gchar * filename);
gboolean libcfg_write_file(ConfigFile * cfg, gchar * filename);
void libcfg_free(ConfigFile * cfg);
ConfigFile *libcfg_open_default_file(gchar * program);
gboolean libcfg_write_default_file(ConfigFile * cfg, gchar * program);

gboolean libcfg_string(char mode, ConfigFile * cfg, gchar * section, gchar * key, gchar ** value);
gboolean libcfg_int(char mode, ConfigFile * cfg, gchar * section, gchar * key, gint * value);
gboolean libcfg_boolean(char mode, ConfigFile * cfg, gchar * section, gchar * key, gboolean * value);
gboolean libcfg_float(char mode, ConfigFile * cfg, gchar * section, gchar * key, gfloat * value);
gboolean libcfg_double(char mode, ConfigFile * cfg, gchar * section, gchar * key, gdouble * value);

void libcfg_remove_key(ConfigFile * cfg, gchar * section, gchar * key);

#ifdef __cplusplus
};
#endif

#endif

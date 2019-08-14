/*
 *  Windows style .ini files reader/writer
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

#ifndef CONFIGFILE_INCLUDED

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#include "configfile.h"

#endif

static ConfigSection *libcfg_create_section(ConfigFile * cfg, char * name);
static ConfigLine *libcfg_create_string(ConfigSection * section, char * key, char * value);
static ConfigSection *libcfg_find_section(ConfigFile * cfg, char * name);
static ConfigLine *libcfg_find_string(ConfigSection * section, char * key);

ConfigFile *libcfg_new(void)
{
	ConfigFile *cfg;

	cfg = (ConfigFile *)g_malloc0(sizeof (ConfigFile));

	return cfg;
}

ConfigFile *libcfg_open_file(gchar * filename)
{
	ConfigFile *cfg;

	FILE *file;
	char *buffer, **lines, *tmp;
	int i;
	struct stat stats;
	ConfigSection *section = NULL;

	g_return_val_if_fail(filename != NULL, FALSE);

	if (lstat(filename, &stats) == -1)
		return NULL;
	if (!(file = fopen(filename, "r")))
		return NULL;

	buffer = (char *)g_malloc(stats.st_size + 1);
	if ((int)fread(buffer, 1, stats.st_size, file) != stats.st_size)
	{
		g_free(buffer);
		fclose(file);
		return NULL;
	}
	fclose(file);
	buffer[stats.st_size] = '\0';

	cfg = (ConfigFile *)g_malloc0(sizeof (ConfigFile));
	lines = g_strsplit(buffer, "\n", 0);
	g_free(buffer);
	i = 0;
	while (lines[i])
	{
		if (lines[i][0] == '[')
		{
			if ((tmp = strchr(lines[i], ']')))
			{
				*tmp = '\0';
				section = libcfg_create_section(cfg, &lines[i][1]);
			}
		}
		else if (lines[i][0] != '#' && section)
		{
			if ((tmp = strchr(lines[i], '=')))
			{
				*tmp = '\0';
				tmp++;
				libcfg_create_string(section, lines[i], tmp);
			}
		}
		i++;
	}
	g_strfreev(lines);
	return cfg;
}

gchar * libcfg_get_default_filename(gchar * program)
{
	static char *filename = NULL;
	if (!filename)
		filename = g_strconcat(g_get_home_dir(), "/.etc/", program, ".ini", NULL);
	return filename;
}

ConfigFile * libcfg_open_default_file(gchar * program)
{
	ConfigFile *ret;

	ret = libcfg_open_file(libcfg_get_default_filename(program));
	if (!ret)
		ret = libcfg_new();
	return ret;
}

gboolean libcfg_write_file(ConfigFile * cfg, gchar * filename)
{
	FILE *file;
	GList *section_list, *line_list;
	ConfigSection *section;
	ConfigLine *line;

	g_return_val_if_fail(cfg != NULL, FALSE);
	g_return_val_if_fail(filename != NULL, FALSE);

	if (!(file = fopen(filename, "w")))
		return FALSE;

	section_list = cfg->sections;
	while (section_list)
	{
		section = (ConfigSection *) section_list->data;
		if (section->lines)
		{
			fprintf(file, "[%s]\n", section->name);
			line_list = section->lines;
			while (line_list)
			{
				line = (ConfigLine *) line_list->data;
				fprintf(file, "%s=%s\n", line->key, line->value);
				line_list = g_list_next(line_list);
			}
			fprintf(file, "\n");
		}
		section_list = g_list_next(section_list);
	}
	fclose(file);
	return TRUE;
}

gboolean libcfg_write_default_file(ConfigFile * cfg, gchar * program)
{
	return libcfg_write_file(cfg, libcfg_get_default_filename(program));
}

static gboolean libcfg_read_string(ConfigFile * cfg, gchar * section, gchar * key, gchar ** value)
{
	ConfigSection *sect;
	ConfigLine *line;

	g_return_val_if_fail(cfg != NULL, FALSE);
	g_return_val_if_fail(section != NULL, FALSE);
	g_return_val_if_fail(key != NULL, FALSE);
	g_return_val_if_fail(value != NULL, FALSE);

	if (!(sect = libcfg_find_section(cfg, section)))
		return FALSE;
	if (!(line = libcfg_find_string(sect, key)))
		return FALSE;
	*value = g_strdup(line->value);
	return TRUE;
}

static gboolean libcfg_read_int(ConfigFile * cfg, gchar * section, gchar * key, gint * value)
{
	char *str;

	g_return_val_if_fail(cfg != NULL, FALSE);
	g_return_val_if_fail(section != NULL, FALSE);
	g_return_val_if_fail(key != NULL, FALSE);
	g_return_val_if_fail(value != NULL, FALSE);

	if (!libcfg_read_string(cfg, section, key, &str))
		return FALSE;
	*value = atoi(str);
	g_free(str);

	return TRUE;
}

static gboolean libcfg_read_boolean(ConfigFile * cfg, gchar * section, gchar * key, gboolean * value)
{
	char *str;

	g_return_val_if_fail(cfg != NULL, FALSE);
	g_return_val_if_fail(section != NULL, FALSE);
	g_return_val_if_fail(key != NULL, FALSE);
	g_return_val_if_fail(value != NULL, FALSE);

	if (!libcfg_read_string(cfg, section, key, &str))
		return FALSE;
	if (!strcasecmp(str, "TRUE"))
		*value = TRUE;
	else
		*value = FALSE;
	g_free(str);
	return TRUE;
}

static gboolean libcfg_read_float(ConfigFile * cfg, gchar * section, gchar * key, gfloat * value)
{
	char *str, *locale;

	g_return_val_if_fail(cfg != NULL, FALSE);
	g_return_val_if_fail(section != NULL, FALSE);
	g_return_val_if_fail(key != NULL, FALSE);
	g_return_val_if_fail(value != NULL, FALSE);

	if (!libcfg_read_string(cfg, section, key, &str))
		return FALSE;

	locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	*value = strtod(str, NULL);
	setlocale(LC_NUMERIC, locale);
	g_free(locale);
	g_free(str);

	return TRUE;
}

static gboolean libcfg_read_double(ConfigFile * cfg, gchar * section, gchar * key, gdouble * value)
{
	char *str, *locale;

	g_return_val_if_fail(cfg != NULL, FALSE);
	g_return_val_if_fail(section != NULL, FALSE);
	g_return_val_if_fail(key != NULL, FALSE);
	g_return_val_if_fail(value != NULL, FALSE);

	if (!libcfg_read_string(cfg, section, key, &str))
		return FALSE;

	locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	*value = strtod(str, NULL);
	setlocale(LC_NUMERIC, locale);
	g_free(locale);
	g_free(str);

	return TRUE;
}

static void libcfg_write_string(ConfigFile * cfg, gchar * section, gchar * key, gchar * value)
{
	ConfigSection *sect;
	ConfigLine *line;

	g_return_if_fail(cfg != NULL);
	g_return_if_fail(section != NULL);
	g_return_if_fail(key != NULL);
	g_return_if_fail(value != NULL);
	
	sect = libcfg_find_section(cfg, section);
	if (!sect)
		sect = libcfg_create_section(cfg, section);
	if ((line = libcfg_find_string(sect, key)))
	{
		g_free(line->value);
		line->value = g_strchug(g_strchomp(g_strdup(value)));
	}
	else
		libcfg_create_string(sect, key, value);
}

static void libcfg_write_int(ConfigFile * cfg, gchar * section, gchar * key, gint value)
{
	char *strvalue;

	g_return_if_fail(cfg != NULL);
	g_return_if_fail(section != NULL);
	g_return_if_fail(key != NULL);

	strvalue = g_strdup_printf("%d", value);
	libcfg_write_string(cfg, section, key, strvalue);
	g_free(strvalue);
}

static void libcfg_write_boolean(ConfigFile * cfg, gchar * section, gchar * key, gboolean value)
{
	g_return_if_fail(cfg != NULL);
	g_return_if_fail(section != NULL);
	g_return_if_fail(key != NULL);

	if (value)
		libcfg_write_string(cfg, section, key, "TRUE");
	else
		libcfg_write_string(cfg, section, key, "FALSE");
}

static void libcfg_write_float(ConfigFile * cfg, gchar * section, gchar * key, gfloat value)
{
	char *strvalue, *locale;

	g_return_if_fail(cfg != NULL);
	g_return_if_fail(section != NULL);
	g_return_if_fail(key != NULL);

	locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	strvalue = g_strdup_printf("%g", value);
	setlocale(LC_NUMERIC, locale);
	libcfg_write_string(cfg, section, key, strvalue);
	g_free(locale);
	g_free(strvalue);
}

static void libcfg_write_double(ConfigFile * cfg, gchar * section, gchar * key, gdouble value)
{
	char *strvalue, *locale;

	g_return_if_fail(cfg != NULL);
	g_return_if_fail(section != NULL);
	g_return_if_fail(key != NULL);

	locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	strvalue = g_strdup_printf("%g", value);
	setlocale(LC_NUMERIC, locale);
	libcfg_write_string(cfg, section, key, strvalue);
	g_free(locale);
	g_free(strvalue);
}

void libcfg_remove_key(ConfigFile * cfg, gchar * section, gchar * key)
{
	ConfigSection *sect;
	ConfigLine *line;

	g_return_if_fail(cfg != NULL);
	g_return_if_fail(section != NULL);
	g_return_if_fail(key != NULL);

	if ((sect = libcfg_find_section(cfg, section)) != NULL)
	{
		if ((line = libcfg_find_string(sect, key)) != NULL)
		{
			g_free(line->key);
			g_free(line->value);
			g_free(line);
			sect->lines = g_list_remove(sect->lines, line);
		}
	}
}

void libcfg_free(ConfigFile * cfg)
{
	ConfigSection *section;
	ConfigLine *line;
	GList *section_list, *line_list;

	if (cfg == NULL)
		return;

	section_list = cfg->sections;
	while (section_list)
	{
		section = (ConfigSection *) section_list->data;
		g_free(section->name);

		line_list = section->lines;
		while (line_list)
		{
			line = (ConfigLine *) line_list->data;
			g_free(line->key);
			g_free(line->value);
			g_free(line);
			line_list = g_list_next(line_list);
		}
		g_list_free(section->lines);
		g_free(section);

		section_list = g_list_next(section_list);
	}
	g_list_free(cfg->sections);
	g_free(cfg);
}

static ConfigSection *libcfg_create_section(ConfigFile * cfg, char * name)
{
	ConfigSection *section;

	section = (ConfigSection *)g_malloc0(sizeof(ConfigSection));
	section->name = g_strdup(name);
	cfg->sections = g_list_append(cfg->sections, section);

	return section;
}

static ConfigLine *libcfg_create_string(ConfigSection * section, char * key, char * value)
{
	ConfigLine *line;

	line = (ConfigLine *)g_malloc0(sizeof (ConfigLine));
	line->key = g_strchug(g_strchomp(g_strdup(key)));
	line->value = g_strchug(g_strchomp(g_strdup(value)));
	section->lines = g_list_append(section->lines, line);

	return line;
}

static ConfigSection *libcfg_find_section(ConfigFile * cfg, char * name)
{
	ConfigSection *section;
	GList *list;

	list = cfg->sections;
	while (list)
	{
		section = (ConfigSection *) list->data;
		if (!strcasecmp(section->name, name))
			return section;
		list = g_list_next(list);
	}
	return NULL;
}

static ConfigLine *libcfg_find_string(ConfigSection * section, char * key)
{
	ConfigLine *line;
	GList *list;

	list = section->lines;
	while (list)
	{
		line = (ConfigLine *) list->data;
		if (!strcasecmp(line->key, key))
			return line;
		list = g_list_next(list);
	}
	return NULL;
}


#define LIBCFG_CAT(pref, name)             pref ## name

#define LIBCFG_SOMETHING(name,type,value) \
gboolean LIBCFG_CAT(libcfg_, name)(char mode, ConfigFile * cfg, gchar * section, gchar * key, type * value) \
{ if (mode=='w') \
  { LIBCFG_CAT(libcfg_write_, name)(cfg, section, key, *value); return 0; } \
else \
  return LIBCFG_CAT(libcfg_read_, name)(cfg, section, key, value); \
}

LIBCFG_SOMETHING(string, char *, value)
LIBCFG_SOMETHING(int, int, value);
LIBCFG_SOMETHING(boolean, int, value);
LIBCFG_SOMETHING(float, float, value);
LIBCFG_SOMETHING(double, double, value);

#undef LIBCFG_CAT
#undef LIBCFG_SOMETHING

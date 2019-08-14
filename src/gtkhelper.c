/*
 * GTK helper
 *
 * Copyright (C) 2010 Lutz Moehrmann <moehre _AT_ 69b.org>
 *
 * This file is part of JackMaster.
 *
 * JackMaster is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JackMaster is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JackMaster. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
		     
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <gtk/gtk.h>
#include <gdk/gdkprivate.h>

#include <gtkhelper.h>


#ifndef USE_GTK_2_0

/***************************\
 *                         *
 *  xlib helper functions  *
 *                         *
\***************************/

Display *
get_xdisplay (GtkWidget * win)
{
  return ((GdkWindowPrivate *)win->window)->xdisplay;
}

Drawable
get_xwindow (GtkWidget * win)
{
  return ((GdkWindowPrivate *)win->window)->xwindow;
}


/********************************\
 *                              *
 *  functions missing in gtk 1  *
 *                              *
\********************************/

#define gtk_container_get_children(widget) gtk_container_children(widget)

void
gtk_window_set_gravity (GtkWindow * win, GdkGravity gravity)
{ GdkWindowPrivate * wp = (GdkWindowPrivate *)GTK_WIDGET(win)->window;
  XSizeHints sh;
  long supplied_return;

  if (wp == NULL) return;
  XGetWMNormalHints(wp->xdisplay, wp->xwindow, &sh, &supplied_return);
  sh.flags |= PWinGravity;
  sh.win_gravity= gravity;
  XSetWMNormalHints(wp->xdisplay, wp->xwindow, &sh);
}

GtkWidget *
gtk_widget_get_parent (GtkWidget * widget)
{
  return widget->parent;
}

void        
gtk_widget_modify_fg (GtkWidget * widget, GtkStateType state, GdkColor * color)
{ GtkStyle * style;

  style= gtk_widget_get_style(widget);
  style->fg[state]= *color;
  gtk_widget_set_style(widget, style);
}

void        
gtk_widget_modify_bg (GtkWidget * widget, GtkStateType state, GdkColor * color)
{ GtkStyle * style;

  style= gtk_widget_get_style(widget);
  style->bg[state]= *color;
  gtk_widget_set_style(widget, style);
}

void        
gtk_widget_modify_font (GtkWidget * widget, GdkFont * font)
{ GtkStyle * style;

  style= gtk_widget_get_style(widget);
  style->font= font;
  gtk_widget_set_style(widget, style);
}

gdouble
gtk_adjustment_get_value (GtkAdjustment * adjustment)
{
  return adjustment->value;
}

/* there seems to be no official way of getting the size of a bitmap... */
typedef struct possibly_gdk_bitmap {
  gint32 dunno[5];
  guint16 width;
  guint16 height;
} possibly_gdk_bitmap;

void
gdk_drawable_get_size (GdkDrawable * drawable, gint * width, gint * height)
{
  possibly_gdk_bitmap * b= (possibly_gdk_bitmap *)drawable;

  *width=  b->width;
  *height= b->height;
}

#endif /* not USE_GTK_2_0 */


/********************************************\
 *                                          *
 *  these should do the same in gtk 1 or 2  *
 *                                          *
\********************************************/

void
set_win_pos (GtkWidget * win, gint x, gint y)
{
#ifdef USE_GTK_2_0
  gtk_window_move(GTK_WINDOW(win), x, y);
#else
  if (win->window==NULL) return;
//  DBG("(x=%d y=%d)\n", x, y);
  gdk_window_move(win->window, x, y);
#endif
}

void
get_win_pos (GtkWidget * win, gint * x, gint * y)
{
#ifdef USE_GTK_2_0
  gtk_window_get_position(GTK_WINDOW(win), x, y);
#else
  if (win->window==NULL) return;
  gdk_window_get_origin(win->window, x, y);
#endif
//  DBG("get_win_pos x=%d y=%d\n", *x, *y);
}


void
set_win_size (GtkWidget * win, gint w, gint h)
{
//#ifdef USE_GTK_2_0
//  gtk_window_resize(GTK_WINDOW(win), w, h);
//#else
  if (win->window==NULL) return;
  gdk_window_resize(win->window, w, h);
//#endif
}

void
get_win_size (GtkWidget * win, gint * w, gint * h)
{
//#ifdef USE_GTK_2_0
//  gtk_window_get_size(GTK_WINDOW(win), w, h);
//#else
  if (win->window==NULL) return;
  gdk_window_get_size(win->window, w, h);
//#endif
//  DBG("get_win_size x=%d y=%d\n", *w, *h);
}


/* let gtk finish all queued (drawing-) operations */
void
iterate ()
{
#ifdef USE_GTK_2_0
  while (g_main_iteration(FALSE));
#else
  while (!gtk_main_iteration_do(FALSE));
#endif
}


GtkWidget *
container_get_first_child (GtkContainer * widget)
{ GList * children;
  GtkWidget * r= NULL;

  children= gtk_container_get_children(GTK_CONTAINER(widget));
  if (children!=NULL) r= children->data;
  g_list_free(children);
  return r;
}

GtkWidget *
container_get_child_by_type (GtkContainer * widget, GtkType widget_type)
{ GList * children, * c;
  GtkWidget * r= NULL;

  children= gtk_container_get_children(GTK_CONTAINER(widget));
  for (c=children; c!=NULL; c=c->next)
    if (widget_type==0 || GTK_OBJECT_TYPE(c->data)==widget_type)
      { r= c->data; break; }
  g_list_free(children);
  return r;
}

gint
container_get_child_index (GtkContainer * widget, GtkWidget * child)
{ GList * children, * c;
  gint x= 0;
  gint r= -1;

  children= gtk_container_get_children(GTK_CONTAINER(widget));
  for (c=children; c!=NULL; c=c->next) {
    if (c->data==child) { r= x; break; }
    x++;
  }
  g_list_free(children);
  return r;
}


int
string_width (GtkStyle * style, gchar * text)
{
#ifdef USE_GTK_2_0
/*
  struct PangoFontMetrics * m;
  m= pango_font_get_metrics(PANGOFONT(style->font_desc), NULL);
  return m->approximate_char_width*strlen(text);
*/
  /* yeah, i know, its 'deprecated' (and its just guesswork), but i couldn't
     get this pango thingie right... */
  return
    gdk_string_width(gdk_font_from_description(style->font_desc), text);
#else
  return gdk_string_width(style->font, text);
#endif
}

int
string_height (GtkStyle * style, gchar * text)
{
#ifdef USE_GTK_2_0
/*
  struct PangoFontMetrics * m;
  m= pango_font_get_metrics(PANGOFONT(style->font_desc), NULL);
  return m->approximate_char_width*strlen(text);
*/
  /* yeah, i know, its 'deprecated' (and its just guesswork), but i couldn't
     get this pango thingie right... */
  return
    gdk_string_height(gdk_font_from_description(style->font_desc), text);
#else
  return gdk_string_height(style->font, text)+2;
#endif
}


static GList * pixmaps_directories= NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
add_pixmap_directory (const gchar * directory)
{
  DBG("('%s')\n", directory);
  pixmaps_directories= g_list_prepend(pixmaps_directories, g_strdup(directory));
}

/* This is an internally used function to check if a pixmap file exists. */
static gchar *
check_file_exists (const gchar * directory, const gchar * filename)
{
  gchar * full_filename;
  struct stat s;
  gint status;

  full_filename= g_strdup_printf("%s" G_DIR_SEPARATOR_S "%s", directory, filename);
  status= stat(full_filename, &s);
  if (status == 0 && S_ISREG(s.st_mode)) return full_filename;
  g_free(full_filename);
  return NULL;
}

/* This is an internally used function to create gdkpixmaps. */
static xpm_t *
create_gdkpixmap (GtkWidget * widget, const gchar * filename)
{
  gchar * found_filename= NULL;
  GdkColormap * colormap;
  GList * elem;
  xpm_t xpm, * r;

  if (!filename || !filename[0])
      return NULL;

  /* We first try any pixmaps directories set by the application. */
  elem= pixmaps_directories;
  while (elem) {
    found_filename= check_file_exists((gchar*)elem->data, filename);
    if (found_filename) break;
    elem= elem->next;
  }

  if (!found_filename) {
    g_warning("Couldn't find pixmap file: %s", filename);
    return NULL;
  }

  colormap= gtk_widget_get_colormap(widget);
  xpm.pixmap= gdk_pixmap_colormap_create_from_xpm
                    (NULL, colormap, &xpm.mask, NULL, found_filename);
  if (xpm.pixmap == NULL) {
    g_warning("Error loading pixmap file: %s", found_filename);
    g_free(found_filename);
    return NULL;
  }

  g_free(found_filename);
  r= g_malloc(sizeof(xpm_t)); r->pixmap= xpm.pixmap; r->mask= xpm.mask;
  return r;
}

void
set_icon (GtkWidget * window, const gchar * filename, const gchar * iconame)
{
  xpm_t * xpm= create_gdkpixmap(window, filename);
  if (xpm) {
    gdk_window_set_icon(window->window, NULL, xpm->pixmap, xpm->mask);
    gdk_window_set_icon_name(window->window, iconame);
//    free(xpm);
  }
}

/* eof */

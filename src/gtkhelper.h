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

typedef struct xpm_t {
  GdkPixmap * pixmap;
  GdkBitmap * mask;
} xpm_t;

#ifndef USE_GTK_2_0
  /* some defs missing in gtk1 */

  typedef enum {
    GTK_MESSAGE_INFO		= 1,
    GTK_MESSAGE_WARNING		= 2,
    GTK_MESSAGE_QUESTION	= 3,
    GTK_MESSAGE_ERROR		= 4
  } GtkMessageType;

  typedef enum {
    GTK_DIALOG_MODAL               = 1 << 0, /* call gtk_window_set_modal (win, TRUE) */
    GTK_DIALOG_DESTROY_WITH_PARENT = 1 << 1, /* call gtk_window_set_destroy_with_parent () */
    GTK_DIALOG_NO_SEPARATOR        = 1 << 2  /* no separator bar above buttons */
  } GtkDialogFlags;
  
  typedef enum {
    GTK_BUTTONS_NONE		= 1,
    GTK_BUTTONS_OK		= 2,
    GTK_BUTTONS_CLOSE		= 3,
    GTK_BUTTONS_CANCEL		= 4,
    GTK_BUTTONS_YES_NO		= 5,
    GTK_BUTTONS_OK_CANCEL	= 6
  } GtkButtonsType;

  /* Currently, these are the same values numerically as in the
   * X protocol. If you change that, gdkwindow-x11.c/gdk_window_set_geometry_hints()
   * will need fixing.
   */
  typedef enum
  {
    GDK_GRAVITY_NORTH_WEST = 1,
    GDK_GRAVITY_NORTH,
    GDK_GRAVITY_NORTH_EAST,
    GDK_GRAVITY_WEST,
    GDK_GRAVITY_CENTER,
    GDK_GRAVITY_EAST,
    GDK_GRAVITY_SOUTH_WEST,
    GDK_GRAVITY_SOUTH,
    GDK_GRAVITY_SOUTH_EAST,
    GDK_GRAVITY_STATIC
  } GdkGravity;
  
#  define gdk_draw_drawable gdk_draw_pixmap

extern Display * xdisplay;
extern Drawable xwindow;

/* for toplevel windows */
Display *
get_xdisplay (GtkWidget * win);

Drawable
get_xwindow (GtkWidget * win);

gboolean
gtk_widget_translate_coordinates
  (GtkWidget * src_widget, GtkWidget * dest_widget,
   gint src_x, gint src_y, gint * dest_x, gint * dest_y);

#endif /* not USE_GTK_2_0 */


void
set_win_pos (GtkWidget * win, gint x, gint y);

void
get_win_pos (GtkWidget * win, gint * x, gint * y);

void
set_win_size (GtkWidget * win, gint w, gint h);

void
get_win_size (GtkWidget * win, gint * w, gint * h);

/* for normal widgets */
//void
//set_min_width (GtkWidget * widget, gint w);

void
set_widget_size (GtkWidget * widget, gint w, gint h);

void
iterate ();


#ifndef USE_GTK_2_0

void
gtk_window_set_gravity (GtkWindow * win, GdkGravity gravity);

void
gtk_widget_popup (GtkWidget * widget, gint x, gint y);

GtkWidget *
gtk_message_dialog_new (GtkWindow * parent, GtkDialogFlags flags,
                        GtkMessageType type, GtkButtonsType buttons,
			const gchar * message_format, ...);

void
gtk_dialog_run (GtkDialog * dialog);

GtkWidget *
gtk_widget_get_parent (GtkWidget * widget);

void        
gtk_widget_modify_fg (GtkWidget * widget, GtkStateType state, GdkColor * color);

void        
gtk_widget_modify_bg (GtkWidget * widget, GtkStateType state, GdkColor * color);

void        
gtk_widget_modify_font (GtkWidget * widget, GdkFont * font);

gdouble
gtk_adjustment_get_value (GtkAdjustment * adjustment);

void
gdk_drawable_get_size (GdkDrawable * drawable, gint * width, gint * height);

#endif


GtkWidget *
container_get_first_child (GtkContainer * widget);

GtkWidget *
container_get_child_by_type (GtkContainer * widget, GtkType widget_type);

gint
container_get_child_index (GtkContainer * widget, GtkWidget * child);


int
string_width (GtkStyle * style, gchar * text);

int
string_height (GtkStyle * style, gchar * text);


void
add_pixmap_directory (const gchar * directory);

void
set_icon (GtkWidget * window, const gchar * filename, const gchar * iconame);

/* eof */

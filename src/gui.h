/*
 * GUI
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

/*
void
on_mainwin_destroy (GtkObject * object, gpointer data);

gboolean
on_meter_button_event
  (GtkWidget * widget, GdkEventButton * event, gpointer data);

gboolean
on_route_button_event
  (GtkWidget * widget, GdkEventButton * event, gpointer data);

#if SHOW_MUTE == 1
gboolean
on_mute_value_changed (GtkWidget * widget, gpointer data);
#endif

#if SHOW_SOLO == 1
gboolean
on_solo_value_changed (GtkWidget * widget, gpointer data);
#endif

gboolean
on_vscale_value_changed (GtkWidget * widget, gpointer data);

gboolean
on_vscale_button_event
  (GtkWidget * widget, GdkEventButton * event, gpointer data);
*/

extern GtkTooltips * tips;

extern GtkWidget * mainwin;

extern GtkWidget * meter_l[MAX_FADERS];
extern GtkWidget * meter_r[MAX_FADERS];
#if SHOW_MUTE == 1
extern GtkWidget * mutes[MAX_FADERS];
#endif
#if SHOW_SOLO == 1
extern GtkWidget * solos[MAX_FADERS];
#endif
#if SHOW_DISPLAY == 1
extern GtkWidget * entries[MAX_FADERS];
#endif
#if SHOW_LABEL == 1
extern GtkWidget * labels[MAX_FADERS];
#endif
extern GtkObject * adjs[MAX_FADERS];
extern GtkWidget * vscales[MAX_FADERS];


extern void create_mainwin ();

/* eof */

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <jack/jack.h>

#include "main.h"
#include "gui.h"
#include "jack_if.h"
#include "alsa_if.h"

#include "gtkhelper.h"

#include "gtkmeter.h"
//#include "gtkmeterscale.h"


GtkTooltips * tips;

int value_width;
GtkWidget * value_win;
GtkWidget * value_lbl;

GtkWidget * mainwin;

#if SHOW_METERS == 1
GtkWidget * meter_l[MAX_FADERS];
GtkWidget * meter_r[MAX_FADERS];
#endif
#if SHOW_MUTE == 1
GtkWidget * mutes[MAX_FADERS];
#endif
#if SHOW_SOLO == 1
GtkWidget * solos[MAX_FADERS];
#endif
#if SHOW_ROUTING == 1
#endif
#if SHOW_DISPLAY == 1
GtkWidget * entries[MAX_FADERS];
#endif
#if SHOW_LABEL == 1
GtkWidget * labels[MAX_FADERS];
#endif

GtkObject * adjs[MAX_FADERS];
GtkWidget * vscales[MAX_FADERS];


/*** callbacks ***************************************************************/


static void
on_mainwin_destroy (GtkObject * object, gpointer udata)
{
  gtk_main_quit();
}

static void
on_mainwin_unmap_ev (GtkObject * object, gpointer udata)
{
  visible= FALSE;
}

static void
on_mainwin_map_ev (GtkObject * object, gpointer udata)
{
  visible= TRUE;
}


#if SHOW_METERS == 1
static gboolean
on_meter_button_event
  (GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  DBG("(data=%d)\n", P2I(data));
  gtk_meter_reset_peak(GTK_METER(meter_l[P2I(data)]));
  gtk_meter_update(GTK_METER(meter_l[P2I(data)]));
//  gtk_adjustment_value_changed(gtk_meter_get_adjustment(GTK_METER(meter_l[P2I(data)])));

  gtk_meter_reset_peak(GTK_METER(meter_r[P2I(data)]));
  gtk_meter_update(GTK_METER(meter_r[P2I(data)]));
//  gtk_adjustment_value_changed(gtk_meter_get_adjustment(GTK_METER(meter_r[P2I(data)])));

  return FALSE;
}
#endif


#if SHOW_ROUTING == 1
static void
route_add_value (int * dest, int value)
{
  *dest += value;
  if ( *dest < ins )
    *dest= master0;
  if ( *dest >= (master0+masters) )
    *dest= ins;
}

static gboolean
on_route_button_event
  (GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  char s[10];
  int d= real_dest[P2I(data)];
  int v= event->button==1 ? 1 : event->button==3 ? -1 : 0;
  DBG("(data=%d event->button=%d)\n", P2I(data), event->button);
  
  if (v==0) return FALSE;
  
  route_add_value(&d, v);
  if ( d == P2I(data) ) route_add_value(&d, v);

  real_dest[P2I(data)]= d;
  if (!solo[P2I(data)]) jm->dest[P2I(data)]= d;
  jm_status_change();
  get_channel_label(s, 10, d);
  gtk_label_set_text(GTK_LABEL(container_get_first_child(GTK_CONTAINER(widget))), s);

  return FALSE;
}
#endif


#if SHOW_MUTE == 1
static gboolean
on_mute_value_changed (GtkWidget * widget, gpointer data)
{
  gboolean v= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  DBG("() value=%d\n", v);

  mute[P2I(data)]= v;
  if (!active) return FALSE;
  jm->gain[IL][P2I(data)]= jm->gain[IR][P2I(data)]=
    v ? 0
#if SHOW_SOLO == 1
      : P2I(data)<master0 && solo_on && !solo[P2I(data)] ? 0
#endif
      : db2lin(fader[P2I(data)]);
  return FALSE;
}
#endif


#if SHOW_SOLO == 1
static gboolean
on_solo_value_changed (GtkWidget * widget, gpointer data)
{
  solo[P2I(data)]= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  jm->dest[P2I(data)]= solo[P2I(data)] ? master0 : real_dest[P2I(data)];

  DBG("(data=%d) value=%d\n", P2I(data), solo[P2I(data)]);
  if (!active) return FALSE;
  solo_changed();

  return FALSE;
}
#endif


static void
update_fader_value (int channel)
{
  gdouble v= get_fader_val(channel);
  gchar s[10];
  
//  DBG("(channel=%d) x=%d y=%d w=%d h=%d\n",
//      channel, widget->allocation.x, widget->allocation.y, widget->allocation.width, widget->allocation.height);

  snprintf(s, 10, "%5.2f dB", v);
  gtk_label_set_text(GTK_LABEL(value_lbl), s);
}


static gboolean
on_vscale_value_changed (GtkWidget * widget, gpointer data)
{
  gdouble v= get_fader_val(P2I(data));

#if SHOW_DISPLAY == 1
  char s[10];
  sprintf(s, "%3.0f", v*100);
  gtk_entry_set_text(GTK_ENTRY(entries[P2I(data)]), s);
#endif

//  DBG("(data=%d) v=%5.3f db=%5.3f p=%5.3f\n",
//      P2I(data), gtk_adjustment_get_value(GTK_ADJUSTMENT(adjs[P2I(data)])), v, p);

  fader[P2I(data)]= v;
#ifdef HAVE_ALSA
  ctr_send(P2I(data), v);
#endif
  update_fader_value(P2I(data));

  if (!active) return FALSE;

  if (
#if SHOW_MUTE == 1
      !mute[P2I(data)]
#else
      TRUE
#endif
      &&
#if SHOW_SOLO == 1
      (P2I(data)>=master0 || !solo_on || solo[P2I(data)])
#else
      TRUE
#endif
     )
    jm->gain[IL][P2I(data)]= jm->gain[IR][P2I(data)]= db2lin(fader[P2I(data)]);
  return FALSE;
}


static void
show_fader_value (int channel)
{
  GtkWidget * widget= vscales[channel];
  gint x, y, x1= 0, y1= 0;
  
  get_win_pos(mainwin, &x, &y);
#ifdef USE_GTK_2_0
  gtk_widget_translate_coordinates(widget, mainwin, 0,0, &x1, &y1);
#else
  x1= widget->allocation.x; y1= widget->allocation.y;
#endif
  x += x1 + (widget->allocation.width/2) - (value_width/2);
  y += y1 + widget->allocation.height;
  set_win_pos(value_win, x, y);
  gtk_widget_show(value_win);
  set_win_pos(value_win, x, y);
}


static gboolean
on_vscale_button_event
  (GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  if (event->type==GDK_BUTTON_PRESS) {
    update_fader_value(P2I(data));
    show_fader_value(P2I(data));
  } else {
    gtk_widget_hide(value_win);
  }
  return FALSE;
}


/*** init ********************************************************************/


static GtkWidget *
create_main_table ()
{
  int n, x, y;
#if SHOW_LABEL == 1
  char s[10];
#endif
  GtkWidget * table1;
#if SHOW_METERS == 1
  GtkWidget * event;
  GtkWidget * hbox;
  GtkObject * m_adj;
//  GtkWidget * meterscale;
#endif
#if SHOW_FX == 1
  GtkWidget * button;
#endif
#if SHOW_ROUTING == 1
  GtkWidget * r_frame;
  GtkWidget * r_event;
  GtkWidget * r_label;
#endif
//  GtkWidget * vscale;
  GtkWidget * lbl;

  n= SHOW_METERS+SHOW_FX+SHOW_MUTE+SHOW_SOLO+SHOW_PFL+SHOW_ROUTING+SHOW_DISPLAY+1+SHOW_LABEL;
  DBG(": rows=%d\n", n);
  table1= gtk_table_new(n, faders, FALSE);
  gtk_widget_ref(table1);
  gtk_object_set_data_full(GTK_OBJECT(mainwin), "", table1,
        (GtkDestroyNotify)gtk_widget_unref);
  gtk_widget_show(table1);

  x= 0;
  for (n=0; n<faders; n++) {
    y= 0; //padx= n<(channels+subs) ? 0 : 2;

    if ( (n==ins) || (n==(ins+subs)) ) {
      lbl= gtk_label_new("");
      gtk_widget_set_usize(lbl, SPACER_WIDTH, -2);
      gtk_widget_show(lbl);
      gtk_table_attach(GTK_TABLE(table1), lbl, x, x+1, y, y+1,
          GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
      x++;
    }

#if SHOW_METERS == 1
    event= gtk_event_box_new();
    gtk_signal_connect(GTK_OBJECT(event), "button_press_event",
        GTK_SIGNAL_FUNC(on_meter_button_event), I2P(n));
    gtk_widget_show(event);
    gtk_table_attach(GTK_TABLE(table1), event, x, x+1, y, y+1,
        GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 0); y++;

    hbox= gtk_hbox_new(FALSE, 0);
    gtk_widget_ref(hbox);
    gtk_object_set_data_full(GTK_OBJECT(mainwin), "", hbox,
        (GtkDestroyNotify)gtk_widget_unref);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(event), hbox);

    m_adj= gtk_adjustment_new(METER_INIT, METER_MIN, METER_MAX, 0.0, 0.0, 0.0);
    meter_l[n]= gtk_meter_new(GTK_ADJUSTMENT(m_adj), GTK_METER_UP);
    gtk_widget_set_usize(meter_l[n], METER_WIDTH, METER_HEIGHT);
//    GTK_WIDGET_UNSET_FLAGS(meter_l[n], GTK_CAN_FOCUS);
//    GTK_WIDGET_UNSET_FLAGS(meter, GTK_CAN_DEFAULT);
    gtk_widget_show(meter_l[n]);
    gtk_box_pack_start(GTK_BOX(hbox), meter_l[n], TRUE, TRUE, 0);
/*
    meterscale= gtk_meterscale_new(GTK_METERSCALE_TOP, METER_MIN, METER_MAX);
    gtk_widget_set_usize(meterscale, 10, -2);
    gtk_widget_show(meterscale);
    gtk_box_pack_start(GTK_BOX(hbox), meterscale, TRUE, TRUE, 1);
*/
    m_adj= gtk_adjustment_new(METER_INIT, METER_MIN, METER_MAX, 0.0, 0.0, 0.0);
    meter_r[n]= gtk_meter_new(GTK_ADJUSTMENT(m_adj), GTK_METER_UP);
    gtk_widget_set_usize(meter_r[n], METER_WIDTH, METER_HEIGHT);
//    gtk_signal_connect(GTK_OBJECT(meter_r[n]), "button_press_event",
//        GTK_SIGNAL_FUNC(on_meter_button_event), I2P(n));
    gtk_widget_show(meter_r[n]);
    gtk_box_pack_start(GTK_BOX(hbox), meter_r[n], TRUE, TRUE, 0);
#endif

#if SHOW_FX == 1
    button= gtk_button_new_with_label("fx");
    gtk_widget_ref(button);
    gtk_object_set_data_full(GTK_OBJECT(mainwin), "", button,
        (GtkDestroyNotify) gtk_widget_unref);
    GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);
    gtk_widget_set_usize(button, -2, 20);
    gtk_widget_show(button);
    gtk_table_attach(GTK_TABLE(table1), button, x, x+1, y, y+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
	(GtkAttachOptions)(0), 0, 0); y++;
#endif

#if SHOW_MUTE == 1
    mutes[n]= gtk_toggle_button_new_with_label("M");
    gtk_widget_ref(mutes[n]);
    gtk_object_set_data_full(GTK_OBJECT(mainwin), "", mutes[n],
        (GtkDestroyNotify) gtk_widget_unref);
    GTK_WIDGET_UNSET_FLAGS(mutes[n], GTK_CAN_FOCUS);
    gtk_signal_connect(GTK_OBJECT(mutes[n]), "toggled",
        GTK_SIGNAL_FUNC(on_mute_value_changed), I2P(n));
    gtk_widget_show(mutes[n]);
    gtk_table_attach(GTK_TABLE(table1), mutes[n], x, x+1, y, y+1,
        GTK_EXPAND | GTK_FILL, 0, 0, 0); y++;
#endif

#if SHOW_SOLO == 1
    if ( n < (ins) ) {
      solos[n]= gtk_toggle_button_new_with_label("S");
      gtk_widget_ref(solos[n]);
      gtk_object_set_data_full(GTK_OBJECT(mainwin), "", solos[n],
          (GtkDestroyNotify) gtk_widget_unref);
      GTK_WIDGET_UNSET_FLAGS(solos[n], GTK_CAN_FOCUS);
      gtk_signal_connect(GTK_OBJECT(solos[n]), "toggled",
          GTK_SIGNAL_FUNC(on_solo_value_changed), I2P(n));
      gtk_widget_show(solos[n]);
      gtk_table_attach(GTK_TABLE(table1), solos[n], x, x+1, y, y+1,
          GTK_EXPAND | GTK_FILL, 0, 0, 0);
    }
    y++;
#endif

#if SHOW_ROUTING == 1
    if ( n < (ins+subs) ) {
      r_frame= gtk_frame_new(NULL);
      gtk_frame_set_shadow_type(GTK_FRAME(r_frame), GTK_SHADOW_OUT);
      GTK_WIDGET_UNSET_FLAGS(r_frame, GTK_CAN_FOCUS);
      gtk_widget_show(r_frame);
      gtk_table_attach(GTK_TABLE(table1), r_frame, x, x+1, y, y+1,
          (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(0), 0, 0);

      r_event= gtk_event_box_new();
      gtk_widget_show(r_event);
      gtk_signal_connect(GTK_OBJECT(r_event), "button_press_event",
          GTK_SIGNAL_FUNC(on_route_button_event), I2P(n));
      gtk_container_add(GTK_CONTAINER(r_frame), r_event);

      r_label= gtk_label_new(NULL);
      get_channel_label(s, 10, real_dest[n]);
      gtk_label_set_text(GTK_LABEL(r_label), s);
      gtk_widget_set_usize(r_label, CHANNEL_WIDTH-2, -2);
      gtk_widget_show(r_label);
      gtk_container_add(GTK_CONTAINER(r_event), r_label);
    }
    y++;
#endif

#if SHOW_DISPLAY == 1
    entries[n]= gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entries[x]), "0");
    gtk_widget_ref(entries[n]);
    gtk_object_set_data_full(GTK_OBJECT(mainwin), "", entries[x],
        (GtkDestroyNotify)gtk_widget_unref);
    GTK_WIDGET_UNSET_FLAGS(entries[n], GTK_CAN_FOCUS);
//    gtk_widget_set_usize(entries[n], CHANNEL_MIN_WIDTH, -2);
    gtk_widget_show(entries[n]);
    gtk_table_attach(GTK_TABLE(table1), entries[n], x, x+1, y, y+1,
        (GTK_EXPAND | GTK_FILL), 0, 0, 0); y++;
#endif

    adjs[n]= gtk_adjustment_new(FADER_MIN, FADER_MIN, FADER_MAX+FADER_SIZE, 0.01,  1.0, FADER_SIZE);
    vscales[n]= gtk_vscale_new(GTK_ADJUSTMENT(adjs[n]));
//    gtk_widget_ref(vscale);
//    gtk_object_set_data_full(GTK_OBJECT(mainwin), "", vscale,
//        (GtkDestroyNotify)gtk_widget_unref);
//    gtk_range_set_inverted(GTK_RANGE(vscales[n]), TRUE);
#ifdef USE_GTK_2_0
    GTK_RANGE(vscales[n])->slider_size_fixed= FALSE;
    GTK_RANGE(vscales[n])->min_slider_size= 20;
#endif
    gtk_widget_set_usize(vscales[n], CHANNEL_WIDTH, FADER_HEIGHT);
    GTK_WIDGET_UNSET_FLAGS(vscales[n], GTK_CAN_FOCUS);
//    gtk_scale_set_value_pos(GTK_SCALE(vscales[n]), GTK_POS_BOTTOM);
//    gtk_scale_set_digits(GTK_SCALE(vscales[n]), 1);
    gtk_scale_set_draw_value(GTK_SCALE(vscales[n]), FALSE);
    gtk_signal_connect(GTK_OBJECT(adjs[n]), "value_changed",
        GTK_SIGNAL_FUNC(on_vscale_value_changed), I2P(n));
    gtk_signal_connect(GTK_OBJECT(vscales[n]), "button_press_event",
        GTK_SIGNAL_FUNC(on_vscale_button_event), I2P(n));
    gtk_signal_connect(GTK_OBJECT(vscales[n]), "button_release_event",
        GTK_SIGNAL_FUNC(on_vscale_button_event), I2P(n));
    gtk_widget_show(vscales[n]);
    gtk_table_attach(GTK_TABLE(table1), vscales[n], x, x+1, y, y+1,
        (GTK_EXPAND | GTK_FILL | GTK_SHRINK), (GTK_EXPAND | GTK_FILL | GTK_SHRINK), 0, 0); y++;

#if SHOW_LABEL == 1
    get_channel_label(s, 10, n);
#ifdef USE_GTK_2_0
    labels[n]= gtk_entry_new();
    gtk_entry_set_text(labels[n], s);
    gtk_entry_set_has_frame(labels[n], FALSE);
#else
    labels[n]= gtk_label_new(s);
#endif
    gtk_widget_ref(labels[n]);
    gtk_object_set_data_full(GTK_OBJECT(mainwin), "", labels[n],
        (GtkDestroyNotify)gtk_widget_unref);
//    GTK_WIDGET_UNSET_FLAGS(label, GTK_CAN_FOCUS);
    gtk_widget_set_usize(labels[n], CHANNEL_WIDTH, -2);
    gtk_widget_show(labels[n]);
    gtk_table_attach(GTK_TABLE(table1), labels[n], x, x+1, y, y+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(0), 0, 0); y++;
#endif

    x++;
  }
  return table1;
}


void
create_mainwin ()
{
  GtkWidget * frame;
  GtkWidget * vbox;
  GtkWidget * label;
  GtkWidget * table1;

  tips= gtk_tooltips_new();
  gtk_tooltips_set_delay(tips, TOOLTIPS_DELAY);

  mainwin= gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data(GTK_OBJECT(mainwin), "", mainwin);
  gtk_window_set_title(GTK_WINDOW(mainwin), PROGLABEL);

  vbox= gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(vbox);
  gtk_object_set_data_full(GTK_OBJECT(mainwin), "", vbox,
        (GtkDestroyNotify)gtk_widget_unref);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(mainwin), vbox);

  label= gtk_label_new(NULL);
  gtk_widget_set_usize(label, -2, 2);
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

  table1= create_main_table();
  gtk_box_pack_start(GTK_BOX(vbox), table1, TRUE, TRUE, 0);
//  gtk_container_add(GTK_CONTAINER(mainwin), table1);

#if SHOW_LABEL != 1
  label= gtk_label_new(NULL);
  gtk_widget_set_usize(label, -2, 2);
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
#endif

  gtk_signal_connect(GTK_OBJECT(mainwin), "destroy",
      GTK_SIGNAL_FUNC(on_mainwin_destroy), NULL);
  gtk_signal_connect(GTK_OBJECT(mainwin), "unmap_event",
      GTK_SIGNAL_FUNC(on_mainwin_unmap_ev), NULL);
  gtk_signal_connect(GTK_OBJECT(mainwin), "map_event",
      GTK_SIGNAL_FUNC(on_mainwin_map_ev), NULL);

  value_width= string_width(mainwin->style, "-99.99 dB");
#ifndef USE_GTK_2_0
  value_width+= 4;
#endif
  value_win= gtk_window_new(GTK_WINDOW_POPUP);
  frame= gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
  GTK_WIDGET_UNSET_FLAGS(frame, GTK_CAN_FOCUS);
  gtk_widget_show(frame);
  gtk_container_add(GTK_CONTAINER(value_win), frame);
  value_lbl= gtk_label_new(NULL);
  gtk_widget_set_usize(value_lbl, value_width, -2);
  gtk_widget_show(value_lbl);
  gtk_container_add(GTK_CONTAINER(frame), value_lbl);

}

/* eof */

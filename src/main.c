/*
 * JackMaster - a "Master Console" for the jack-audio-connection-kit
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#include <X11/Xlib.h>

#include <glib.h>
#include <gtk/gtk.h>

#include <jack/jack.h>

#ifdef HAVE_LASH
#  include <lash/lash.h>
#endif

#include "gtkmeter.h"

#include "main.h"
#include "configfile.h"
#include "jack_if.h"

#ifdef HAVE_ALSA
#  include "alsa_if.h"
#endif

#include "gtkhelper.h"
#include "gui.h"


char progname[10+1+5+1];

#ifdef HAVE_LASH
lash_client_t * lash_client;
#endif

//GtkWidget * mainwin;

int active= FALSE;
int visible= FALSE;
//int visible= TRUE;

int ins= NO_OF_INS;
int subs= NO_OF_SUBS;
int masters= NO_OF_MASTERS;

//#define sub_idx ins
int master0= NO_OF_INS+NO_OF_SUBS;
int faders= NO_OF_INS+NO_OF_SUBS+NO_OF_MASTERS;

float fader[MAX_FADERS];
#if SHOW_MUTE == 1
int mute[MAX_FADERS];
#endif
#if SHOW_SOLO == 1
int solo[MAX_FADERS];
int solo_on= FALSE;
#endif
int real_dest[MAX_FADERS];

#ifdef HAVE_ALSA
unsigned int ctr[MAX_FADERS];
#endif

#if SAVE_CONNS > 0
GPtrArray * conn_now[MAX_CHANNELS][MAX_FADERS];
GPtrArray * conn_todo[MAX_CHANNELS][MAX_FADERS];
int todo_cnt= 0;
#endif

static int cfg_px= -1;
static int cfg_py= -1;
static int cfg_sx= -1;
static int cfg_sy= -1;


void
perrorf (char * fmt, ...)
{
  va_list args;

  fprintf(stderr, "%s: ", PROGNAME);
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}


#if SAVE_CONNS > 0
static int
find_conn (const char ** array, char * name)
{
  int z;

  DBG("(name='%s')", name);
  for (z=0; array[z]; z++) {
    DBC(" %d='%s'", z, array[z]);
    if (strcmp(array[z], name) == 0)
      { DBC(" : %d\n", z); return z; }
  }

  DBC(" : -1\n");
  return -1;
}

void
store_todo (int channel, int fader, char * connect_to)
{
  g_ptr_array_add(conn_todo[channel][fader], connect_to);
  todo_cnt++;
  DBG("(channel=%d, fader=%d, connect_to='%s') todo_cnt=%d\n", channel, fader, connect_to, todo_cnt);
}

void
clear_todo (int channel, int fader, int index)
{
  g_ptr_array_remove_index_fast(conn_todo[channel][fader], index);
  todo_cnt--;
  DBG("(channel=%d, fader=%d, index=%d) todo_cnt=%d\n", channel, fader, index, todo_cnt);
}

static void
cfg_conns (char mode, ConfigFile * cfg, char * section, int no)
{
  int y, z;
  char v[3];
  char * c;
  gchar ** d;
  GString * s;

  if (mode == 'r')

    for (y=0; y<MAX_CHANNELS; y++) {
      sprintf(v, "c%d", y+1);
      libcfg_string(mode, cfg, section, v, &c);
      if (c) {
        d= g_strsplit(c, "\t", 0);
        for (z=0; d[z]; z++) if (*d[z]) store_todo(y, no, d[z]);
        free(d); //g_strfreev(d);
      }
    }

  else

    for (y=0; y<MAX_CHANNELS; y++) {
      s= g_string_new(NULL);
      for (z=0; z<conn_now[y][no]->len; z++) {
        g_string_append(s, g_ptr_array_index(conn_now[y][no], z));
        g_string_append_c(s, '\t');
      }
      for (z=0; z<conn_todo[y][no]->len; z++) {
        g_string_append(s, g_ptr_array_index(conn_todo[y][no], z));
        g_string_append_c(s, '\t');
      }
      sprintf(v, "c%d", y+1);
      libcfg_string(mode, cfg, section, v, &s->str);
      g_string_free(s, TRUE);
    }

}
#endif


static void
config (char mode, ConfigFile * cfg)
{
  int x;
  char lbl[82]; /* enaugh even for 256-bit-systems */

  libcfg_int(mode, cfg, PROGNAME, "pos_x", &cfg_px);
  libcfg_int(mode, cfg, PROGNAME, "pos_y", &cfg_py);
  libcfg_int(mode, cfg, PROGNAME, "size_x", &cfg_sx);
  libcfg_int(mode, cfg, PROGNAME, "size_y", &cfg_sy);
  libcfg_float(mode, cfg, PROGNAME, "gain_lp", &jm->gain_factor);
#if SHOW_METERS == 1
  libcfg_float(mode, cfg, PROGNAME, "peak_fall", &jm->peak_factor);
#endif

  for (x=0; x<faders; x++) {
//    get_channel_label(lbl, 31, n);
    if (x < ins)
      sprintf(lbl, "%d", x+1);
    else if (x < master0)
      sprintf(lbl, "s%d", x-ins+1);
    else
      sprintf(lbl, "m%d", x-master0+1);
#if SAVE_CONNS > 0
    cfg_conns(mode, cfg, lbl, x);
#endif
#if SHOW_MUTE == 1
    libcfg_int(mode, cfg, lbl, "m", &mute[x]);
#endif
    if (x < master0) {
      libcfg_int(mode, cfg, lbl, "d", &real_dest[x]);
      if (real_dest[x]>=faders || real_dest[x]<ins)
        real_dest[x]= master0;
#if SHOW_SOLO == 1
      if (x < ins)
        libcfg_int(mode, cfg, lbl, "s", &solo[x]);
#endif
    }
    libcfg_float(mode, cfg, lbl, "v", &fader[x]);
  }
}


static void
cfg_load (const char * fn)
{
  ConfigFile * cfg;

  cfg= libcfg_open_file((void *)fn);
  if (cfg) {
    config('r', cfg);
    libcfg_free(cfg);
  }
}


static void
cfg_save (const char * fn)
{
  ConfigFile * cfg= libcfg_new();

  config('w', cfg);
  libcfg_write_file(cfg, (void *)fn);
  libcfg_free(cfg);
}


static void
jm_cfg_init ()
{
  char rcfile[PATH_MAX];
  int fd;

  snprintf(rcfile, PATH_MAX, "%s/.etc/%s", getenv("HOME"), PROGNAME ".gtkrc");
  if ((fd = open(rcfile, O_RDONLY)) >= 0) {
    close(fd);
    DBG("() Using rc file: %s\n", rcfile);
    gtk_rc_parse(rcfile);
  }

  snprintf(rcfile, PATH_MAX, "%s/.etc/%s", getenv("HOME"), RC_FILE_NAME);
  cfg_load(rcfile);
}


static void
jm_cfg_exit ()
{
  char rcfile[PATH_MAX];

  snprintf(rcfile, PATH_MAX, "%s/.etc/%s", getenv("HOME"), RC_FILE_NAME);
  cfg_save(rcfile);
}


static void
jm_ui_init ()
{
  int x;

//  DBG(": gain[0][%d]=%5.3f\n", ins+subs, gain[IL][ins+subs]);
  DBG("()\n");

  for (x=0; x<faders; x++) {
    if (x < master0) {
      jm->dest[x]= real_dest[x];
#if SHOW_SOLO == 1
      if (x < ins)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(solos[x]), solo[x]);
#endif
    }
#if SHOW_MUTE == 1
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mutes[x]), mute[x]);
#endif
  }
#if SHOW_SOLO == 1
  solo_changed();
#endif
  active= TRUE;
  for (x=0; x<faders; x++)
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adjs[x]), FADER_MIN+FADER_MAX - fader[x]);
//  DBG(": gain[0][%d]=%5.3f\n", ins+subs, gain[IL][ins+subs]);
}


void
get_channel_label (char * buf, size_t size, int channel)
{
  if (channel < ins)
    snprintf(buf, size, "%c", channel+'a');
  else if (channel < master0)
    snprintf(buf, size, "S-%c", channel-ins+'a');
  else
    snprintf(buf, size, "MA");
}


#if SHOW_SOLO == 1
void
solo_changed ()
{
  int x;

  solo_on= FALSE;
  for (x=0; x<master0; x++)
    if (solo[x]) { solo_on= TRUE; break; }
  for (x=0; x<master0; x++)
    jm->gain[IL][x]= jm->gain[IR][x]=
      mute[x] ? 0 :
      solo[x] ? db2lin(fader[x]) :
      solo_on ? 0 : db2lin(fader[x]);
}
#endif


#if SHOW_METERS == 1
static gboolean
update_meters (gpointer data)
{
  int x;
  
  if (!visible) return TRUE;

  gdk_threads_enter();
  for (x=0; x<(ins+subs+masters); x++) {
    gtk_adjustment_set_value(
      gtk_meter_get_adjustment( GTK_METER(meter_l[x])), lin2db(jm->peak[IL][x]) );
    gtk_adjustment_set_value(
      gtk_meter_get_adjustment( GTK_METER(meter_r[x])), lin2db(jm->peak[IR][x]) );
  }
  gdk_threads_leave();

  return TRUE;
}

static void
get_display_label (char * buf, size_t size, int channel)
{
  int l, w;
  char * t;

  DBG("(size=%zu channel=%d)", size, channel);
  if (jm->conns[0][channel]) {
    strncpy(buf, jm->conns[0][channel][0], size); buf[size]= 0;
    t= strchr(buf, ':'); if (t) *t= 0;
    l= strlen(buf); w= labels[channel]->allocation.width;
    DBC(" l=%d w=%d", l, w);
    gdk_threads_enter();
    while (l && string_width(mainwin->style, buf)>w)
      buf[--l]= 0;
    gdk_threads_leave();
    DBC("  l=%d", l);
  } else
    get_channel_label(buf, size, channel);
  DBC(" : buf='%s'\n", buf);
}
#endif

#define LABEL_MAX_LEN 16

static gboolean
slow_background (gpointer data)
{
#if SAVE_CONNS > 0
  static int gc0= -1, saved= 0;
  int c, x, y, z, gc1;
  char * p;
# if SHOW_LABEL == 1
  char s[LABEL_MAX_LEN+1];
# endif
#endif

  gdk_threads_enter();
  get_win_pos(mainwin, &cfg_px, &cfg_py);
  get_win_size(mainwin, &cfg_sx, &cfg_sy);
  gdk_threads_leave();

#if SAVE_CONNS > 0
  if (jm->jack_up) goto loop;

  if (saved) goto nosave;
  DBG(": saving connections... todo_cnt= %d\n", todo_cnt);
  saved++; todo_cnt= 0;
  for (x=0; x<faders; x++)
    for (y=0; y<MAX_CHANNELS; y++) {
      for (z=0; z<conn_now[y][x]->len; z++)
	store_todo(y, x, g_ptr_array_index(conn_now[y][x], z));
      g_ptr_array_free(conn_now[y][x], FALSE);
      conn_now[y][x]= g_ptr_array_new();
    }
  DBG(": saving connections done. todo_cnt= %d\n", todo_cnt);

nosave:  
  if (jm_jack_start(1)) return TRUE;

loop:
//  DBG(": loop:\n");
  gc1= jm->graph_order_cnt;
  if (gc1 == gc0) return TRUE;
  gc0= gc1;

  if (todo_cnt<=0) goto norest;
  DBG(": restoring connections... todo_cnt= %d\n", todo_cnt);
  for (x=0; x<faders; x++)
    for (y=0; y<MAX_CHANNELS; y++)
      for (z=0; z<conn_todo[y][x]->len; z++)
        if ( jm_jack_conn(y, x, g_ptr_array_index(conn_todo[y][x], z)) == 0 )
	  clear_todo(y, x, z);
  DBG(": restoring connections done. todo_cnt= %d\n", todo_cnt);

norest:
  DBG(": getting connections...\n");
  for (x=0; x<faders; x++)
    for (y=0; y<MAX_CHANNELS; y++) {
      if (gc1 != jm->graph_order_cnt) goto loop;

/*
      DBG(": starting for-loop\n");
      for (z=0; z<conn_now[y][x]->len; z++)
	DBG(": z=%d\n", z);
	p= g_ptr_array_index(conn_now[y][x], z);
	DBG(": finding '%s'.\n", p);
        if (find_conn(jm->conns[y][x], p) < 0) {
	  DBG(": connection %d,%d,%d <-> '%s' removed.\n", y,x,z, p);
//	  c= find_conn((void *)conn_todo[y][x]->pdata, p); if (c<0) continue;
//	  clear_todo(y, x, c);
	  DBG(": todo-list #%d cleared, todo_cnt= %d \n", c, todo_cnt);
	}
*/
      g_ptr_array_free(conn_now[y][x], TRUE);
      conn_now[y][x]= g_ptr_array_new();
#if SHOW_LABEL == 1
      if (x<ins && y==0) {
	get_display_label(s, LABEL_MAX_LEN, x);
//        DBG(": label[%d] = '%s'\n", x, s);
        gdk_threads_enter();
        gtk_label_set_text(GTK_LABEL(labels[x]), s);
        gdk_threads_leave();
      }
#endif
      if (!jm->conns[y][x]) continue;
      DBG(": conn_now[%d][%d] =", y, x);
      for (z=0; jm->conns[y][x][z]; z++) {
        g_ptr_array_add(conn_now[y][x], g_strdup(jm->conns[y][x][z]));
        DBC(" '%s'", jm->conns[y][x][z]);
      }
      DBC("\n");
    }
  DBG(": getting connections done.\n");

  goto loop;

#else
  return TRUE;
#endif
}


#ifdef HAVE_LASH
static gboolean
update_lash (gpointer data)
{
  lash_event_t * event;

  while ((event= lash_get_event(lash_client))) {
    switch (lash_event_get_type(event)) {
      case LASH_Save_File:
		cfg_save(lash_get_fqn(lash_event_get_string(event), RC_FILE_NAME));
		lash_send_event(lash_client, event);
		break;
      case LASH_Restore_File:
		cfg_load(lash_get_fqn(lash_event_get_string(event), RC_FILE_NAME));
		gdk_threads_enter();
		jm_ui_init();
		if (cfg_px>=0) set_win_pos(mainwin, cfg_px, cfg_py);
		if (cfg_sx>=0) set_win_size(mainwin, cfg_sx, cfg_sy);
		gdk_threads_leave();
		lash_send_event(lash_client, event);
		break;
      case LASH_Quit:
		gtk_main_quit();
		lash_event_destroy(event);
		break;
      case LASH_Server_Lost:
		fprintf(stderr, PROGNAME ": the lash server disconnected!\n");
		return FALSE;
      default:  break;		/* make gcc -Wall the fuck shut up */
    }
  }

  return TRUE;
}
#endif


#ifdef HAVE_ALSA
void
ctr_rcv_cb (unsigned int channel, unsigned int param, unsigned int value)
{
  float v= (float)value/MIDI_MULT * (FADER_MAX-FADER_MIN) + FADER_MIN;
  unsigned int p= MAX_FADERS-(param-MIDI_OFFSET)-1;

  if (p<MAX_FADERS) {
    ctr[p]= value;
    gdk_threads_enter();
    set_fader_val(p, v);
    gdk_threads_leave();
    DBG("(channel=%d param=%d value=%d) p=%d v=%3.2f\n", channel, param, value, p, v);
  }
}


void
ctr_send (unsigned int param, float value)
{
  unsigned int v= 0.5 + (value-FADER_MIN) / (FADER_MAX-FADER_MIN) * MIDI_MULT;
  unsigned int p= MAX_FADERS-param-1 + MIDI_OFFSET;

  if (ctr[param]!=v) {
    ctr[param]= v;
    jm_send_ctr(MIDI_CHANNEL-1, p, v);
    DBG("(param=%d value=%3.2f) p=%d v=%d\n", param, value, p, v);
  }
}
#endif


static void
jm_main_init ()
{
  int x;
#if SAVE_CONNS > 0
  int y;
#endif

  for (x=0; x<MAX_FADERS; x++) {
#if SHOW_MUTE == 1
    mute[x]= FALSE;
#endif
#if SHOW_SOLO == 1
    solo[x]= FALSE;
#endif
    jm->dest[x]= real_dest[x]= master0;
#ifdef HAVE_ALSA
    ctr[x]= -1;
#endif
#if SAVE_CONNS > 0
    for (y=0; y<MAX_CHANNELS; y++) {
      conn_now[y][x]= g_ptr_array_new();
      conn_todo[y][x]= g_ptr_array_new();
    }
#endif
  }
#if SHOW_METERS == 1
  jm->peak_factor= (90.0 + PEAK_FALL) / 100.0;
#endif
  jm->gain_factor= GAIN_CTR_LP;

//snprintf(progname, 17, PROGNAME "_%d", getpid());
  snprintf(progname, 17, PROGNAME);
}


static void
jm_gtk_init (int argc, char * argv[])
{
#ifdef HAVE_ALSA
# ifdef USE_GTK_2_0
  gdk_threads_init();
# endif
//  gdk_threads_enter();
#endif

#ifdef ENABLE_NLS
  bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain(PACKAGE);
#endif

  gtk_set_locale();
  gtk_init(&argc, &argv);

  add_pixmap_directory(DATA_ROOT_DIR "/icons");
}


#ifdef HAVE_LASH
static void
jm_lash_init (int argc, char * argv[])
{
  lash_args_t * lash_args;
  lash_event_t * lash_event;

  lash_args= lash_extract_args(&argc, &argv);
  lash_client= lash_init(lash_args, PROGNAME, LASH_Config_File, LASH_PROTOCOL(2,0));
  fprintf(stderr, PROGNAME ": %s.\n",
	  lash_client ? "lash connection established" : "could not initialise lash");
  if (lash_client) {
    DBG(": setting LASH CLIENT name to '%s'.\n", progname);

    lash_event= lash_event_new_with_type(LASH_Client_Name);
    lash_event_set_string(lash_event, progname);
    lash_send_event(lash_client, lash_event);

    lash_event= lash_event_new_with_type(LASH_Jack_Client_Name);
    lash_event_set_string(lash_event, progname);
    lash_send_event(lash_client, lash_event);
/*
    lash_jack_client_name(lash_client, progname);
*/
  }
}
#endif


int
main (int argc, char * argv[])
{
  jm_jack_init();
  jm_main_init();
  jm_cfg_init();

  if (jm_jack_start(0)) exit(EXIT_FAILURE);

  jm_gtk_init(argc, argv);
#ifdef HAVE_ALSA
  jm_alsa_init(progname, ctr_rcv_cb);
#endif
#ifdef HAVE_LASH
  jm_lash_init(argc, argv);
#endif

  create_mainwin();
  DBG("() setting pos x=%d y=%d.\n", cfg_px, cfg_py);
  if (cfg_px>=0) set_win_pos(mainwin, cfg_px, cfg_py);
  DBG("() setting size w=%d h=%d.\n", cfg_sx, cfg_sy);
  if (cfg_sx>=0) gtk_window_set_default_size(GTK_WINDOW(mainwin), cfg_sx, cfg_sy);

  jm_ui_init();

  gtk_widget_show(mainwin);
#ifndef USE_GTK_2_0
  gtk_window_set_gravity(GTK_WINDOW(mainwin), GDK_GRAVITY_STATIC);
#endif
  /* unfortunately, this has to be done again, 'cause mainwin->window
     is NULL before gtk_widget_show and those lousy window managers (may
     some deity damn 'em) tend to place windows the way they like (but
     they honor the move request afterwards) */
  if (cfg_px>=0) set_win_pos(mainwin, cfg_px, cfg_py);

  set_icon(mainwin, PROGNAME "16x16.xpm", progname);

#if SHOW_METERS == 1
  gtk_timeout_add(1000/METER_UPDATE_HZ, update_meters, NULL);
#endif
#ifdef HAVE_LASH
  if (lash_client) gtk_timeout_add(100, update_lash, NULL);
#endif
  gtk_timeout_add(250, slow_background, NULL);

  gtk_main();

#ifdef HAVE_ALSA
//  gdk_threads_leave();
#endif

  jm_jack_exit();
  jm_cfg_exit();

  return 0;
}

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

extern char progname[];

extern int active;
extern int visible;

extern int ins;
extern int subs;
extern int masters;

extern int master0;
extern int faders;

extern float fader[MAX_FADERS];
#if SHOW_MUTE == 1
extern int mute[MAX_FADERS];
#endif
#if SHOW_SOLO == 1
extern int solo[MAX_FADERS];
extern int solo_on;
#endif
extern int real_dest[MAX_FADERS];
#ifdef HAVE_ALSA
extern unsigned int ctr[MAX_FADERS];
#endif


extern void
perrorf (char * fmt, ...);

#if SHOW_SOLO == 1
void
solo_changed();
#endif

void
get_channel_label (char * buf, size_t size, int channel);

#ifdef HAVE_ALSA
void
ctr_send (unsigned int param, float value);
#endif

#define get_fader_val(channel) \
  (FADER_MIN+FADER_MAX - gtk_adjustment_get_value(GTK_ADJUSTMENT(adjs[channel])))

#define set_fader_val(channel, value) \
  (gtk_adjustment_set_value(GTK_ADJUSTMENT(adjs[channel]), FADER_MIN+FADER_MAX - value))


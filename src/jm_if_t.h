/*
 * Interface to the JACK Server
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

typedef jack_default_audio_sample_t sample_t;

typedef struct jm_if_t {
  int ins;
  int subs;
  int masters;
  int faders;
  int master0;

  int jack_up;
  int quiet;

#ifdef INTERNAL_CLIENT
  int status_change_cnt;
#endif

  sample_t gain_factor;
  sample_t gain[MAX_CHANNELS][MAX_FADERS];

  int dest[MAX_FADERS];

#if SHOW_METERS == 1
  sample_t peak_factor;
  sample_t peak[MAX_CHANNELS][MAX_FADERS];
#endif

#if SAVE_CONNS > 0
  int graph_order_cnt;
  const char ** conns[MAX_CHANNELS][MAX_FADERS];
#endif
} jm_if_t;


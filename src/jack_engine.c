/*
 * The heart of it
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <jack/jack.h>

#include "jack_if.h"
#include "jack_process.h"
#include "jack_engine.h"


int
jm_process_cb (jack_nframes_t nframes, void * arg)
{
  sample_t * buf[MAX_CHANNELS][jm->ins + jm->subs + jm->masters];
  sample_t v;
#if SHOW_METERS == 1
  sample_t w;
#endif
  int d, x, y, z;
#ifdef JACK_BUG_FIX
  static int firstrun= 0;
#endif

#ifdef INTERNAL_CLIENT
  static int sc0= 0; int sc1;

  for ( ; (sc1=jm->status_change_cnt)!=sc0; sc0=sc1)
    jm_status_change();
#endif

//  DBG("(nframes=%d)\n", nframes);

  for (x=0; x<jm->faders; x++) {
    for (y=0; y<MAX_CHANNELS; y++) {
      int_gain[y][x]= int_gain[y][x] * (1.0f - jm->gain_factor) + jm->gain[y][x] * jm->gain_factor;
#if SHOW_METERS == 1
      jm->peak[y][x] *= jm->peak_factor;
#endif
    }

    if (!status[x]) continue;

    for (y=0; y<MAX_CHANNELS; y++) {
      buf[y][x]= jack_port_get_buffer(ports[y][x], nframes);
      if (buf[y][x]==NULL)
        { printf("buffer for port #%d not found\n", x); exit(1); }
    }
    
#ifdef JACK_BUG_FIX
    if (!firstrun) {
      firstrun++;
      for (y=0; y<MAX_CHANNELS; y++)
        for (z=0; z<nframes; z++) buf[y][x][z]= 0;
    }
#endif
  }

  for (x=jm->ins; x<jm->faders; x++) {
    if (!status[x]) continue;
    for (y=0; y<MAX_CHANNELS; y++) {
#ifdef FAST_FLOAT_0
      memset(buf[y][x], 0, nframes*sizeof(sample_t));
#else
      for (z=0; z<nframes; z++) buf[y][x][z]= 0;
#endif
    }
  }

  for (x=0; x<jm->master0; x++) {
    if (!status[x]) continue;
    d= jm->dest[x];
    for (y=0; y<MAX_CHANNELS; y++)
      for (z=0; z<nframes; z++) {
        v= buf[y][x][z] * int_gain[y][x]; buf[y][d][z] += v;
#if SHOW_METERS == 1
        w= fabsf(v); if (w > jm->peak[y][x]) jm->peak[y][x] = w;
#endif
      }
  }

  for (x=jm->master0; x<jm->faders; x++) {
    if (!status[x]) continue;
    for (y=0; y<MAX_CHANNELS; y++)
      for (z=0; z<nframes; z++) {
        v= buf[y][x][z]= buf[y][x][z] * int_gain[y][x];
#if SHOW_METERS == 1
        w= fabsf(v); if (w > jm->peak[y][x]) jm->peak[y][x] = w;
#endif
      }
  }

  return 0;
}


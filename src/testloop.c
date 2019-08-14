/*
 * loop to find best optimizations
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
#include <fcntl.h>
#include <math.h>
#include <time.h>

#include <jack/jack.h>

#include "jm_if_t.h"
#include "jack_engine.h"

typedef sample_t fake_port_t;

jm_if_t * jm= NULL;

sample_t int_gain[MAX_CHANNELS][MAX_FADERS];
fake_port_t * ports[MAX_CHANNELS][MAX_FADERS+AUTO_PORTS];

int status[MAX_FADERS];

int avg= 0;


inline void *
jack_port_get_buffer (jack_port_t * port, jack_nframes_t size)  
{
  return port;
}


void
testloop (int bufsize)
{
  int x, y, z, count= 0;
  time_t t, u;

  for (x=0; x<jm->faders; x++)
    for (y=0; y<MAX_CHANNELS; y++) {
      int_gain[y][x]= 0.0f;
#if SHOW_METERS == 1
      jm->peak[y][x]= 0.0f;
#endif
      for (z=0; z<bufsize/2; z++)
        ports[y][x][z]= (float)z / (float)bufsize * 4.0f - 1.0f;
      for (z=bufsize/2; z<bufsize; z++)
        ports[y][x][z]= 0.0f;
    }
  t= time(NULL);
  while ((u=time(NULL))==t) {}

  do {
    jm_process_cb(bufsize, NULL);
    count++;
  } while (u==time(NULL));

//  printf("%d\t%d\t%d\n", bufsize, count, bufsize*count);
  printf("%d\t", bufsize*count);
  avg += bufsize*count;
}


int
main (int argc, char * argv[])
{
  jm_if_t jmm;
  int x, y;

  jm= &jmm;
  jm->ins= NO_OF_INS;
  jm->subs= NO_OF_SUBS;
  jm->masters= NO_OF_MASTERS;
  jm->faders= NO_OF_INS+NO_OF_SUBS+NO_OF_MASTERS;
  jm->master0= NO_OF_INS+NO_OF_SUBS;
  jm->jack_up= 0;
  jm->gain_factor= 0.15f;
#ifdef INTERNAL_CLIENT
  jm->status_change_cnt= 0;
#endif
#if SAVE_CONNS > 0
  jm->graph_order_cnt= 0;
#endif
#if SHOW_METERS == 1
  jm->peak_factor= 0.9f;
#endif

  for (x=0; x<jm->ins; x++)
    jm->dest[x]= (x&2) ? jm->ins : jm->master0;
  for (x=jm->ins; x<jm->master0; x++)
    jm->dest[x]= jm->master0;

  for (x=0; x<jm->faders; x++) {
    status[x]= (x>=jm->ins) ? 1 : (x&1);
    for (y=0; y<MAX_CHANNELS; y++) {
      ports[y][x]= malloc(4096*sizeof(sample_t));
      jm->gain[y][x]= 1.0f;
    }
  }

//  fprintf(stderr, "buf:\truns:\tnorm:\n");
//  testloop(16); testloop(256); testloop(4096);
  for (x=1; argv[x]; x++)
    testloop(atoi(argv[x]));

  printf("%d\t", avg/x);

  return 0;
}

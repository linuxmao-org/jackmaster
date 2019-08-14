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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <glib.h>

#include <jack/jack.h>

#include "jm_if_t.h"
#include "jack_process.h"
#include "jack_engine.h"

#ifdef DODBG
#  undef DBG
#  define DBG(format, args...) fprintf(stderr, "%s.%s" format, PROGNAME "_so", __func__ , ##args)
#endif

extern jm_if_t * jm ATT_HIDDEN;

void perrorf (char * fmt, ...) ATT_HIDDEN;


jm_if_t * jm= NULL;

void
perrorf (char * fmt, ...)
{
  va_list args;

  fprintf(stderr, "%s: ", PROGNAME "_so");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

#include "jack_process.c"

int
jack_initialize (jack_client_t * so_client, const char * so_data)  
{
  int jm_shm= atoi(so_data);

  DBG("(so_data='%s')\n", so_data);
  client= so_client;

  if ( (jm= shmat(jm_shm, 0, 0)) == (void *)-1 ) {
    perrorf("shmat error");
    return -1;
  }

  return jm_jack_process_start(jm->quiet);
}

void
jack_finish ()
{
  DBG("()\n");

  jack_deactivate(client);
  jack_client_close(client);
  return;
}

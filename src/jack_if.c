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
#include <jack/intclient.h>

#include "main.h"
#include "jack_if.h"
#ifndef INTERNAL_CLIENT
#  include "jack_process.h"
#  include "jack_engine.h"
#endif


jm_if_t * jm= NULL;

#ifdef INTERNAL_CLIENT

static jack_client_t * client;
static jack_intclient_t intclient;
static int jm_shm;

void jm_status_change (void)
{
  jm->status_change_cnt++;
}

/* #if SAVE_CONNS > 0
int  jm_jack_conn (int channel, int fader, char * connect_to)
{
  return 0;
}
#endif */

#else /* INTERNAL_CLIENT */

#include "jack_process.c"

#endif /* INTERNAL_CLIENT */


void
jm_jack_init ()
{
#if SAVE_CONNS > 0
  int x, y;
#endif

#ifdef INTERNAL_CLIENT
  if ( (jm_shm= shmget(IPC_PRIVATE, sizeof(jm_if_t), 0600)) < 0 ) {
    perrorf("shmget error");
    exit(EXIT_FAILURE);
  }
  if ( (jm= shmat(jm_shm, 0, 0)) == (void *)-1 ) {
    perrorf("shmat error");
    shmctl(jm_shm, IPC_RMID, 0);
    exit(EXIT_FAILURE);
  }
#else
  jm= malloc(sizeof(jm_if_t));
#endif

  jm->ins= ins;
  jm->subs= subs;
  jm->masters= masters;
  jm->faders= faders;
  jm->master0= master0;
  jm->jack_up= 0;
#ifdef INTERNAL_CLIENT
  jm->status_change_cnt= 0;
#endif
#if SAVE_CONNS > 0
  jm->graph_order_cnt= 0;

  for (x=0; x<faders; x++)
    for (y=0; y<MAX_CHANNELS; y++)
      jm->conns[y][x]= NULL;
#endif
}


//#ifndef INTERNAL_CLIENT
static const char *
jm_get_jack_status_message (jack_status_t status)
{
  if (status & JackInvalidOption)
    return "The operation contained an invalid or unsupported option.";
  if (status & JackShmFailure)
    return "Unable to access shared memory.";
  if (status & JackVersionError)
    return "Client's protocol version does not match.";
  if (status & JackNameNotUnique)
    return "The desired client name was not unique.";

  if (status & JackLoadFailure)
    return "Unable to load internal client.";
  if (status & JackInitFailure)
    return "Unable to initialize client.";
  if (status & JackNoSuchClient)
    return "Requested client does not exist.";

  if (status & JackServerError)
    return "Communication error with the JACK server.";
  if (status & JackServerFailed)
    return "Unable to connect to the JACK server.";
  if (status & JackFailure)
    return "Overall operation failed.";

  if (status & JackServerStarted)
    return "The JACK server was started as a result of this operation.";

  return NULL;
}
//#endif


static int
jm_show_jack_msg (jack_status_t st, int quiet)
{
  const char * msg;

  msg= jm_get_jack_status_message(st);
  if (st!=0 && st!=JackServerStarted) {
    if (quiet) return -1;
    if (msg)
      perrorf("%s: %s", "Could not connect to jackd", msg);
    else
      perrorf("%s: Unknown status %d", "Could not connect to jackd", st);
    return -1;
  }
  if (msg && !quiet) perrorf("%s", msg);
  return 0;
}


int
jm_jack_start (int quiet)
{
  jack_status_t st= 0;
#ifdef INTERNAL_CLIENT
  GString * shm;
# define CLIENT_NAME PROGNAME "_ctr"
#else  
# define CLIENT_NAME PROGNAME
#endif

  DBG("()\n");

  /* try to become a client of the JACK server */
  client= jack_client_open(CLIENT_NAME, JM_JACK_CLIENT_OPTIONS, &st);
  DBG(": jack_client_open(%s, %d) = %d\n", CLIENT_NAME, JM_JACK_CLIENT_OPTIONS, st);
  if (jm_show_jack_msg(st, quiet) || client==NULL) return -1;
#ifdef INTERNAL_CLIENT
  shm= g_string_new(NULL); g_string_sprintf(shm, "%d", jm_shm);
  jm->quiet= quiet;
  intclient= jack_internal_client_load
	       (client, PROGNAME, JackLoadName|JackLoadInit|JM_JACK_INTCLIENT_OPTIONS,
	        &st, PROGNAME, shm->str);
  jm_show_jack_msg(st, quiet);
  g_string_free(shm, TRUE);
  return st;
#else  
  return jm_jack_process_start(quiet);
#endif
}

int
jm_jack_exit ()
{
  DBG("()\n");
  
#ifdef INTERNAL_CLIENT
  jack_internal_client_unload(client, intclient);
  shmctl(jm_shm, IPC_RMID, 0);
//  return 0;
#else  
  jack_deactivate(client);
//  free(jm);
#endif
  return jack_client_close(client);
}

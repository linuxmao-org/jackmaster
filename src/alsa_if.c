/*
 * Alsa Interface
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

#ifdef HAVE_ALSA

#include <stdio.h>

#include <pthread.h>

#include <alsa/asoundlib.h>

#include "main.h"
#include "alsa_if.h"


/* global ALSA connection */
static snd_seq_t * alsa= NULL;

/* used to control the looping thread */
static int run= 1;

static int port;
static snd_seq_event_t oevent;

jm_rcv_cb_t * jm_rcv_ctr;


/* Wait for midi events and then trigger them. */
static void *
midi_loop (void * ctlname)
{
  snd_seq_event_t * event;

  while (run) {
    snd_seq_event_input(alsa, &event);

    if (event->type == MIDI_CTR) {
//    DBG(": controller channel=%d param=%d value=%d\n", channel+1, param, value);
      jm_rcv_ctr(event->data.control.channel, event->data.control.param,
	         event->data.control.value);
    } else {
      DBG(": type=%d\n", event->type);
    }

    snd_seq_free_event(event);
  }

//  snd_seq_delete_simple_port(alsa, iport);
  DBG(": midi control exiting.");
  return 0;
}


void
jm_send_ctr (unsigned int channel, unsigned int param, unsigned int value)
{
//  DBG("(channel=%d param=%d value=%d)\n", channel, param, value);

  oevent.data.control.channel= channel;
  oevent.data.control.param= param;
  oevent.data.control.value= value;
  snd_seq_event_output(alsa, &oevent);
  snd_seq_drain_output(alsa);
}


/* Set up the alsa connection. */
void
jm_alsa_init (char * progname, jm_rcv_cb_t * callback)
{
  int r;
  pthread_t midi_tid;

  r= snd_seq_open(&alsa, "default", SND_SEQ_OPEN_DUPLEX, 0);
  if (r < 0) {
    perrorf("Couldn't open ALSA sequencer");
  } else {
    snd_seq_set_client_name(alsa, progname);
    port= snd_seq_create_simple_port(alsa, progname,
	   SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE
	   | SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
	   MIDI_TYPE);

    jm_rcv_ctr= callback;
    pthread_create(&midi_tid, NULL, midi_loop, NULL);

    snd_seq_ev_clear(&oevent);
    snd_seq_ev_set_fixed(&oevent);
    snd_seq_ev_set_direct(&oevent);
    snd_seq_ev_set_source(&oevent, port);
    snd_seq_ev_set_subs(&oevent);
    oevent.type= MIDI_CTR;
  }
}


/* tell the midi control thread to stop */
void
jm_alsa_stop (void)
{
  run= 0;
}

#endif /* HAVE_ALSA */

/* eof */

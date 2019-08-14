/*
 * Configuration file to customize JackMaster
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

#include <ac_cfg.h>

/* Channels */
#define NO_OF_INS	16
#define NO_OF_SUBS	4
#define NO_OF_MASTERS	1 /* This one may not be changed yet */

//#define NO_OF_AUX_SENDS	0 not yet
//#define NO_OF_AUX_RETS	0 not yet


/* UI-widgets to show */
#define SHOW_METERS	1
//#define SHOW_FX	0 not yet
//#define SHOW_PAN	0 not yet
#define SHOW_MUTE	1
#define SHOW_SOLO	1
//#define SHOW_PFL	0 not yet
#define SHOW_ROUTING	1
//#define SHOW_DISPLAY	0 obsolete
#define SHOW_LABEL	1

#define MAX_LABEL_CHARS	4


/* Let the engine run as a jack-server-internal-client. This should reduces
   the CPU-load, but also (currently) disables "SAVE_CONNS" below.
   Furthermore, it seems to slightly INCREASE cpu-load  */
//#define INTERNAL_CLIENT


/* Save connections made to our ports, so they can automatically
   be restored after the server or jackmaster comes up again. The
   advantage this has over LASH is that you don't have to "close
   project" and then "file/open project" it again. But if you do that
   anyway (because, for instance, your other proggies don't survive a
   server down), there's no point in using this. */
#define SAVE_CONNS	1


/* Any connections made to the "auto"-ports will automatically be reconnected
   to the first free "in"-port (or just the first one if all are used) */
/* This stupid thing crashes the jack server (as well as qjackctrl!) here. 
   Dunno why, perhaps you may not call jack_connect() or jack_disconnect()
   from a graph_order_callback??? Well, for now i have more important things
   to attend to... */
#define AUTO_PORTS	0


/* Meter range in dB */
#define METER_MIN	-70
#define METER_MAX	6
/* Set this to 0 for the side-effect of having all peak indicators at 0 dB */
#define METER_INIT	METER_MIN


/* Fader range in dB */
#define FADER_MIN	-70
#define FADER_MAX	6
/* Fader knob size in pixels */
#define FADER_SIZE	25


/* Minimum size in pixels of some widgets */
#define CHANNEL_WIDTH	24
#define SPACER_WIDTH	4
#define METER_WIDTH	10
#define METER_HEIGHT	60
#define FADER_HEIGHT	100


/* much higher frequencies *really* eat up cpu */
#define METER_UPDATE_HZ 12


/* range: 0 (=fast) .. 9 (=slow) */
#define PEAK_FALL	0


/* VCA gain control lowpass factor (>0.0=slow 1.0=fast) */
#define GAIN_CTR_LP	0.15f


/* Not yet */
#define TOOLTIPS_DELAY	1


/* Kind of a device type for alsa. Seems to have no effect though... */
#define MIDI_TYPE	SND_SEQ_PORT_TYPE_MIDI_GENERIC

/* MIDI channel to send/receive automation data (1-16) */
#define MIDI_CHANNEL	1

/* Uncomment this to use 14 bit NRPN controllers for automation instead of
   the 7 bit ones. Unfortunately, sequencer programs on linux seem to ignore
   NRPN controllers (at least, i couldn't convince neither Rosegarden4 nor
   MusE to record 'em. And it gets worse: MusE once (0.6.3, i think) had at
   least some NRPN-support in the gui which dissapeared in 0.7.0, while
   0.7.2pre5 even lacks most of the standard 7 bit controllers). */
#define MIDI_NRPN

/* Controller # for the master fader (0-16383 if MIDI_NRPN is set, 1-99
   otherwise). The other faders are assigned from right to left. This is
   done to make sure the master fader always gets the same controller #,
   even if you change the number of ins/subs */
#define MIDI_OFFSET_7BIT	7
#define MIDI_OFFSET_NRPN	0


/* Do not automatically start the JACK server when it is not already running.
   This option is always selected if $JACK_NO_START_SERVER is defined in the
   process environment. */
#define JM_JACK_CLIENT_OPTIONS JackNoStartServer

/* Use the exact client name requested. Otherwise, JACK automatically
   generates a unique one, if needed. */
//#define JM_JACK_CLIENT_OPTIONS JackUseExactName

//#define JM_JACK_CLIENT_OPTIONS (JackNoStartServer | JackUseExactName)

/* The internal client only has this option: */
//#define JM_JACK_INTCLIENT_OPTIONS JackUseExactName


/* Sometimes, unconnectd inputs get noise from jack. To fix that, one could
   overwrite the *input* buffer with zeros. This "fix" does just that on
   application start. */
#define JACK_BUG_FIX


/* Produce lots of ugly debugging output */
#define DODBG


/* */
#define ATT_HIDDEN  __attribute__ ((visibility ("hidden")))


/*****************************************************************************/


/* Name to announce to jackd */
#define PROGNAME	"jackmaster"

/* Name to advertise for X */
#define PROGLABEL	"JackMaster"

#define RC_FILE_NAME	PROGNAME ".rc"

#define MAX_FADERS	NO_OF_INS+NO_OF_SUBS+NO_OF_MASTERS

/* Set this to 2 for stereo. Other values require code changes */
#define MAX_CHANNELS	2
/* Indices for left and right */
#define IL		0
#define IR		1


#define P2I(p)		GPOINTER_TO_INT(p)
#define I2P(i)		GINT_TO_POINTER(i)

#define db2lin(g)	(powf(10.0f, (g) * 0.05f))

#define lin2db(v)	(20.0f * log10f(v))

#ifndef AUTO_PORTS
#  define AUTO_PORTS	0
#endif

#ifndef SAVE_CONNS
#  define SAVE_CONNS	0
#endif

#ifdef INTERNAL_CLIENT
#  undef SAVE_CONNS
#  define SAVE_CONNS	0
#endif

#ifndef SHOW_METERS
#  define SHOW_METERS	0
#endif
#ifndef SHOW_FX
#  define SHOW_FX	0
#endif
#ifndef SHOW_MUTE
#  define SHOW_MUTE	0
#endif
#ifndef SHOW_SOLO
#  define SHOW_SOLO	0
#endif
#ifndef SHOW_PFL
#  define SHOW_PFL	0
#endif
#ifndef SHOW_ROUTING
#  define SHOW_ROUTING	0
#endif
#ifndef SHOW_DISPLAY
#  define SHOW_DISPLAY	0
#endif
#ifndef SHOW_LABEL
#  define SHOW_LABEL	0
#endif

#ifndef JM_JACK_CLIENT_OPTIONS
   /* Null value to use when no option bits are needed. */
#  define JM_JACK_CLIENT_OPTIONS JackNullOption
#endif

#ifndef JM_JACK_INTCLIENT_OPTIONS
#  define JM_JACK_INTCLIENT_OPTIONS JackNullOption
#endif

#ifdef MIDI_NRPN
#  define MIDI_CTR	SND_SEQ_EVENT_NONREGPARAM
#  define MIDI_MULT	16383
#  define MIDI_OFFSET	MIDI_OFFSET_NRPN
#else
#  define MIDI_CTR	SND_SEQ_EVENT_CONTROLLER
#  define MIDI_MULT	127
#  define MIDI_OFFSET	MIDI_OFFSET_7BIT
#endif

#ifdef DODBG
#  define DBG(format, args...) fprintf(stderr, "%s.%s" format, PROGNAME, __func__ , ##args)
#  define DBC(format, args...) fprintf(stderr, format, ##args)
#else
#  define DBG(format, args...) {}
#  define DBC(format, args...) {}
#endif

/* eof */

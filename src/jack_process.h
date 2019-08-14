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

extern int jack_warm ATT_HIDDEN;

extern sample_t int_gain[MAX_CHANNELS][MAX_FADERS] ATT_HIDDEN;
extern jack_port_t * ports[MAX_CHANNELS][MAX_FADERS+AUTO_PORTS] ATT_HIDDEN;

extern jack_client_t * client ATT_HIDDEN;
extern jack_nframes_t jack_bufsize ATT_HIDDEN;

extern int status[MAX_FADERS] ATT_HIDDEN;


void set_port_status (int channel, int index) ATT_HIDDEN;
void jm_status_change () ATT_HIDDEN;
#if AUTO_PORTS == 1
void check_auto (int index) ATT_HIDDEN;
#endif
int  jm_graph_order_cb (void * arg) ATT_HIDDEN;
void jm_shutdown_cb (void * arg) ATT_HIDDEN;
#if SAVE_CONNS > 0
int  jm_jack_conn (int channel, int fader, char * connect_to) ATT_HIDDEN;
#endif
jack_port_t * register_port (unsigned long flags, char * format, void * param) ATT_HIDDEN;
void register_ports (int channel, unsigned long flags) ATT_HIDDEN;
int  jm_jack_process_start (int quiet) ATT_HIDDEN;



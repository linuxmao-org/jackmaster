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

int jack_warm= 0;

sample_t int_gain[MAX_CHANNELS][MAX_FADERS];
jack_port_t * ports[MAX_CHANNELS][MAX_FADERS+AUTO_PORTS];

jack_client_t * client;
jack_nframes_t jack_bufsize;

int status[MAX_FADERS];


void
set_port_status (int channel, int index)
{
  int connected= jack_port_connected(ports[index][channel]);
  int bit= 1 << index;

//  DBG("(channel=%d bit=%x connected=%d) dest=%d\n", channel, bit, connected, dest[channel]);

  if (connected) {
    status[channel] |= bit;
    status[jm->dest[channel]] |= bit;
  } else {
    status[channel] &= ~bit;
  }
}


void
jm_status_change ()
{
  int x;

//  DBG("port=%p conns=%d)\n", port, conns);
  
/*  
#if AUTO_PORTS == 1
  check_auto(master_idx+jm->masters);
  check_auto(master_idx+jm->masters);
#endif
*/

  DBG(": ");

  for (x=jm->ins; x<jm->faders; x++)
    status[x]= 0;

  for (x=0; x<jm->ins; x++) {
    set_port_status(x, IL); set_port_status(x, IR);
    DBC("%x", status[x]);
  }

  DBC(" ");

  for (x=jm->ins; x<jm->faders; x++) {
    if (x==jm->master0) DBC(" ");
    status[jm->dest[x]] |= status[x];
    DBC("%x", status[x]);
  }

  DBC("\n");
}


#if AUTO_PORTS == 1
void
check_auto (int index)
{
  int e, x, fi= 0;
  const char ** portlist= jack_port_get_connections(ports[index][jm->faders]);

  DBG("(index=%d)\n", index);

  if (portlist) {
    for (x=0; x<jm->ins; x++)
      if ( (status[x] & (index+1)) == 0)
        { fi= x; break; }
    DBG(": fi=%d\n", fi);
    for (x=0; portlist[x]; x++) {
      DBG(": disconnecting '%s' => '%s'\n", portlist[x], jack_port_name(ports[index][jm->faders]));
      e= jack_disconnect(client, portlist[x], jack_port_name(ports[index][jm->faders]));
      DBG(": result= %d\n", e);
      DBG(": connecting    '%s' => '%s'\n", portlist[x], jack_port_name(ports[index][fi]));
      e= jack_connect(client, portlist[x], jack_port_name(ports[index][fi]));
      DBG(": result= %d\n", e);
    }
    free(portlist);
  }
}
#endif


int
jm_graph_order_cb (void * arg)
{
#if SAVE_CONNS > 0
  int x, y, z;
#endif

  jm_status_change();

#if SAVE_CONNS > 0
  for (x=0; x<jm->faders; x++)
    for (y=0; y<MAX_CHANNELS; y++) {
      free(jm->conns[y][x]);
      jm->conns[y][x]= jack_port_get_connections(ports[y][x]);
      if (!jm->conns[y][x]) continue;

      DBG(": conns[%d][%d]=", y, x);
      for (z=0; jm->conns[y][x][z]; z++)
        DBC(" '%s'", jm->conns[y][x][z]);
      DBC("\n");
//      p= jack_port_get_connections(ports[y][x]);      conns[y][x]= p;

    }

  jm->graph_order_cnt++;
#endif

  return 0;
}


 void
jm_shutdown_cb (void * arg)
{

  DBG("()\n");
/*
  for (i=0; i<jm->faders; i++)
    for (j=0; j<MAX_CHANNELS; j++)
      free(conns[j][i]);
*/
  jm->jack_up= 0;
}


#if SAVE_CONNS > 0
int
jm_jack_conn (int channel, int fader, char * connect_to)
{
  char * source, * dest;
  
  DBG("(channel=%d, fader=%d, connect_to='%s')\n", channel, fader, connect_to);

  if (fader < jm->ins)
    { source= connect_to; dest= (char *)jack_port_name(ports[channel][fader]); }
  else
    { source= (char *)jack_port_name(ports[channel][fader]); dest= connect_to; }

  return jack_connect(client, source, dest);
}
#endif


jack_port_t *
register_port (unsigned long flags, char * format, void * param)
{
  char port_name[32];
  jack_port_t * port;

  snprintf(port_name, 31, format, param);
  port= jack_port_register
      (client, port_name, JACK_DEFAULT_AUDIO_TYPE, flags, jack_bufsize);
  if (!port) {
    perrorf("Cannot register jack port '%s'", port_name);
    exit(EXIT_FAILURE);
  }
  return port;
}


void
register_ports (int channel, unsigned long flags)
{
  char s[32];

#if AUTO_PORTS == 1
  if (channel >= jm->faders)
    { sprintf(s, "auto"); flags |= JackPortIsInput; }
  else
#endif
  if (channel < jm->ins)
    { sprintf(s, "in-%c", channel+'a'); flags |= JackPortIsInput; }
  else
  if (channel < jm->master0)
    { sprintf(s, "sub-%c", channel - jm->ins + 'a'); flags |= JackPortIsOutput; }
  else
    { sprintf(s, "master"); flags |= JackPortIsOutput; }
  ports[IL][channel]= register_port(flags, "%s-l", s);
  ports[IR][channel]= register_port(flags, "%s-r", s);
  DBG(": registering port #%d: l=%p r=%p\n", channel, ports[IL][channel], ports[IR][channel]);
}


int
jm_jack_process_start (int quiet)
{
  int x;

  DBG("()\n");

  for (x=0; x<jm->faders; x++) status[x]= 0;

  jack_bufsize= jack_get_buffer_size(client);
  DBG(": buffersize=%" PRIu32 "\n", jack_bufsize);

#if AUTO_PORTS == 1
  register_ports(jm->faders, 0);
#endif
  for (x=jm->master0; x<jm->faders; x++)
    register_ports(x, 0);
  for (x=0; x<jm->master0; x++)
    register_ports(x, 0);

  if (jack_set_process_callback(client, jm_process_cb, 0))
    { if (!quiet) perrorf("Could not register process callback."); return -1; }

  if (jack_set_graph_order_callback(client, jm_graph_order_cb, NULL))
    if (!quiet) perrorf("Could not register graph callback.");

  jack_on_shutdown(client, jm_shutdown_cb, NULL);

  DBG(": callbacks installed.\n");

  /* tell the JACK server that we are ready to rock */
  if (jack_activate(client))
    { if (!quiet) perrorf("Cannot activate jack client."); return -1; }

  jack_warm= jm->jack_up= 1;
  DBG(": finished.\n");
  return 0;
}


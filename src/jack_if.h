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

#include "jm_if_t.h"

extern jm_if_t * jm;

void jm_jack_init ();
int  jm_jack_start (int quiet);
int  jm_jack_exit ();

void jm_status_change (void);
#if SAVE_CONNS > 0
int  jm_jack_conn (int channel, int fader, char * connect_to);
#endif

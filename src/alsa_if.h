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

#ifdef HAVE_ALSA

typedef void (jm_rcv_cb_t)
  (unsigned int channel, unsigned int param, unsigned int value);

extern jm_rcv_cb_t * jm_rcv_ctr;

void
jm_send_ctr (unsigned int channel, unsigned int param, unsigned int value);

void
jm_alsa_init (char * progname, jm_rcv_cb_t * callback);

void
jm_alsa_stop (void);

#endif /* HAVE_ALSA */

/* eof */

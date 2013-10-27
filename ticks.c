/*
    Copyright (C) 2012 Lauri Kasanen
    
    ! Forked from radeontop and modified  by ldsh on 27 october 2013 !

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "radeonload.h"
#include <pthread.h>

struct bits_t *results = NULL;

static void *collector(void *arg) {

	const unsigned int ticks = * ((unsigned int *) arg);

	struct bits_t res[2];

	// Save one second's worth of history
	struct bits_t *history = calloc(ticks, sizeof(struct bits_t));
	unsigned int cur = 0, curres = 0;

	const useconds_t sleeptime = 1e6 / ticks;

	while (1) {
		unsigned int stat = readgrbm();

		memset(&history[cur], 0, sizeof(struct bits_t));

		if (stat & bits.ee) history[cur].ee = 1;
		if (stat & bits.vgt) history[cur].vgt = 1;
		if (stat & bits.gui) history[cur].gui = 1;
		if (stat & bits.ta) history[cur].ta = 1;
		if (stat & bits.tc) history[cur].tc = 1;
		if (stat & bits.sx) history[cur].sx = 1;
		if (stat & bits.sh) history[cur].sh = 1;
		if (stat & bits.spi) history[cur].spi = 1;
		if (stat & bits.smx) history[cur].smx = 1;
		if (stat & bits.sc) history[cur].sc = 1;
		if (stat & bits.pa) history[cur].pa = 1;
		if (stat & bits.db) history[cur].db = 1;
		if (stat & bits.cr) history[cur].cr = 1;
		if (stat & bits.cb) history[cur].cb = 1;


		usleep(sleeptime);
		cur++;
		cur %= ticks;

		// One second has passed, we have one sec's worth of data
		if (cur == 0) {
			unsigned int i;

			memset(&res[curres], 0, sizeof(struct bits_t));

			for (i = 0; i < ticks; i++) {
				res[curres].ee += history[i].ee;
				res[curres].vgt += history[i].vgt;
				res[curres].gui += history[i].gui;
				res[curres].ta += history[i].ta;
				res[curres].tc += history[i].tc;
				res[curres].sx += history[i].sx;
				res[curres].sh += history[i].sh;
				res[curres].spi += history[i].spi;
				res[curres].smx += history[i].smx;
				res[curres].sc += history[i].sc;
				res[curres].pa += history[i].pa;
				res[curres].db += history[i].db;
				res[curres].cb += history[i].cb;
				res[curres].cr += history[i].cr;
			}

			// Atomically write it to the pointer
			__sync_bool_compare_and_swap(&results, results, &res[curres]);

			curres++;
			curres %= 2;
		}
	}

	return NULL;
}

void collect(unsigned int *ticks) {

	// Start a thread collecting data
	pthread_t tid;
	pthread_attr_t attr;

	// We don't care to join this thread
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&tid, &attr, collector, ticks);
}

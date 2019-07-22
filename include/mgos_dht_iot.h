/*
 * Remote Fan Controller
 * Copyright (c) 2019 Michiel Fokke
 * All rights reserved
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
#ifndef MGOS_DHT_IOT_H
#define MGOS_DHT_IOT_H

/* C++ guard */
#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

struct history {
	double *value;
	int size;
	int pointer;
};

struct rpc_args {
	struct history *h1;
	struct history *h2;
};

struct sample_args {
	struct mgos_dht *dht;
	struct history *h1;
	struct history *h2;
};

struct history *history_init(double value);

void history_update(struct history *hist, double value);

double average(struct history *hist);

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif // MGOS_DHT_IOT_H

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

#define MAX_SENSOR_COUNT 1
#define DHT22_INIT_DELAY 2100

struct mgos_sensor_set {
	int count;
	int max_count;
	struct mgos_sensor **sensors;
};

struct mgos_sensor {
	int pin;
	struct mgos_dht *dht;
	struct mgos_history *temp_history;
	struct mgos_history *humidity_history;
};

struct mgos_history {
	double *value;
	int size;
	int pointer;
};

struct rpc_args {
	struct mgos_history *h1;
	struct mgos_history *h2;
};

struct sample_args {
	struct mgos_dht *dht;
	struct mgos_history *h1;
	struct mgos_history *h2;
};

struct mgos_sensor_set *sensor_set_init(int max_count);

bool sensor_set_add_sensor(struct mgos_sensor_set *sensor_set, int pin);

struct mgos_sensor *sensor_init(int pin);

struct mgos_history *history_init(double value);

bool history_update(struct mgos_history *hist, double value);

double average(struct mgos_history *hist);

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif // MGOS_DHT_IOT_H

/*
 * Remote Fan Controller
 * Copyright (c) 2019 Michiel Fokke <github@fokke.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

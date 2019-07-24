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

#include "mgos.h"
#include "mgos_dht.h"
#include "mgos_rpc.h"
#include "mgos_dht_iot.h"

struct mgos_sensor_set *sensor_set_init(int max_count)
{
	struct mgos_sensor_set *sensor_set;
	sensor_set = (struct mgos_sensor_set *) malloc(sizeof(*sensor_set));
	sensor_set->sensors = (struct mgos_sensor **) malloc(max_count*sizeof(sensor_set->sensors));
	sensor_set->max_count = max_count;
	sensor_set->count = 0;
	return sensor_set;
}

bool sensor_set_add_sensor(struct mgos_sensor_set *sensor_set, int pin)
{
	if (sensor_set->count < sensor_set->max_count) {
		struct mgos_sensor *s = sensor_init(pin);
		if (s != NULL) {
			LOG(LL_INFO, ("Sensor on GPIO pin %d initialized", pin));
			mgos_msleep(DHT22_INIT_DELAY); // wait to prevent nan results from DHT

			s->temp_history = history_init(mgos_dht_get_temp(s->dht));
			LOG(LL_INFO, ("Allocated temp history (size: %d)", s->temp_history->size));

			s->humidity_history = history_init(mgos_dht_get_humidity(s->dht));
			LOG(LL_INFO, ("Allocated humidity history (size: %d)", s->humidity_history->size));

			sensor_set->sensors[sensor_set->count] = s;
			sensor_set->count++;
			return true;
		} else {
			LOG(LL_ERROR, ("Sensor on pin %d not initialized", pin));
		}
	}
	LOG(LL_ERROR, ("Can't add sensor. Maximum count is %d", sensor_set->max_count));
	return false;
}

struct mgos_sensor *sensor_init(int pin)
{
	struct mgos_sensor *sensor;
	sensor = (struct mgos_sensor *) malloc(sizeof(*sensor));

	sensor->pin = pin;
	sensor->dht = mgos_dht_create(pin, DHT22);
	if (sensor->dht == NULL) {
		free(sensor);
		return NULL;
	}
	return sensor;
}

struct mgos_history *history_init(double value)
{
	struct mgos_history *hist;
	hist = (struct mgos_history *) malloc(sizeof(*hist));
	hist->size = mgos_sys_config_get_dht_iot_history_size();
	hist->value = (double *) malloc(hist->size * sizeof(double));

	for (int i = 0; i < hist->size; i++)
		hist->value[i] = value;
	hist->pointer = hist->size;
	return hist;
}

bool history_update(struct mgos_history *hist, double value)
{
	if (value == value) { // exclude nan values
		hist->pointer = (hist->pointer + 1) % hist->size;		
		hist->value[hist->pointer] = value;
		return true;
	} else {
		return false;
	}
}

double average(struct mgos_history *hist)
{
	double sum = 0;
	for (int i = 0; i < hist->size; i++)
		sum += hist->value[i];
	return round(10 * sum / hist->size) / 10; // round to one decimal
}

static void rpc_cb(struct mg_rpc_request_info *ri, void *cb_arg,
	struct mg_rpc_frame_info *fi, struct mg_str args)
{
	struct mgos_sensor_set *ss = (struct mgos_sensor_set *) cb_arg;
	struct mgos_sensor *s = ss->sensors[0]; 
	struct mgos_history *temp_hist = s->temp_history;
	struct mgos_history *humidity_hist = s->humidity_history;

	mg_rpc_send_responsef(ri, "{temp: %.1lf, humidity: %.1lf}", \
		average(temp_hist), average(humidity_hist));
	(void) fi;
	(void) args;
}

static void dht_iot_sample_cb(void *cb_arg)
{
	struct mgos_sensor_set *ss = (struct mgos_sensor_set *) cb_arg;
	for (int i=0; i<ss->count; i++) {
		struct mgos_sensor *s = ss->sensors[i]; 
		struct mgos_history *temp_hist = s->temp_history;
		struct mgos_history *humidity_hist = s->humidity_history;
		struct mgos_dht *dht = s->dht;

		double temp = mgos_dht_get_temp(dht);
		double humidity = mgos_dht_get_humidity(dht);

		history_update(temp_hist, temp);
		history_update(humidity_hist, humidity);

		LOG(LL_INFO, ("Sensor %d", i));
		LOG(LL_INFO, ("\tTemperature: %f (avg %.1f)", temp, average(temp_hist)));
		LOG(LL_INFO, ("\tHumidity:    %f (avg %.1f)", humidity, average(humidity_hist)));
	}
}

bool mgos_dht_iot_init(void)
{
	struct mgos_sensor_set *sensor_set = sensor_set_init(MAX_SENSOR_COUNT);
	int pin = mgos_sys_config_get_dht_iot_dht_pin();
	if (pin >= 0) {
		if (sensor_set_add_sensor(sensor_set, pin) == true) {
			int sample_interval = mgos_sys_config_get_dht_iot_sample_interval();
			mgos_set_timer(sample_interval, true, dht_iot_sample_cb, (void *)sensor_set);
			mg_rpc_add_handler(mgos_rpc_get_global(), "Dht.Read", "", rpc_cb, (void *)sensor_set);
		}
	}
	LOG(LL_INFO, ("DHT-IOT library loaded."));
	return true;
}
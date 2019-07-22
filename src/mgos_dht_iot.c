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

struct history *history_init(double value)
{
	struct history *hist;
	hist = (struct history *) malloc(sizeof(struct history));
	hist->size = mgos_sys_config_get_dht_iot_history_size();
	hist->value = (double *) malloc(hist->size * sizeof(double));

	for (int i = 0; i < hist->size; i++)
		hist->value[i] = value;
	hist->pointer = hist->size;
	return hist;
}

void history_update(struct history *hist, double value)
{
	if (value == value) { // exclude nan values
		hist->pointer = (hist->pointer + 1) % hist->size;		
		hist->value[hist->pointer] = value;
	}
}

double average(struct history *hist)
{
	double sum = 0;
	for (int i = 0; i < hist->size; i++)
		sum += hist->value[i];
	return round(10 * sum / hist->size) / 10; // round to one decimal
}

static void dht_iot_sample_cb(void *cb_arg)
{
	struct mgos_dht *dht = (struct mgos_dht *)((struct sample_args *) cb_arg)->dht;
	struct history *temp_hist = (struct history *)((struct sample_args *) cb_arg)->h1;
	struct history *humidity_hist = (struct history *)((struct sample_args *) cb_arg)->h2;
	double temp = mgos_dht_get_temp(dht);
	double humidity = mgos_dht_get_humidity(dht);
	history_update(temp_hist, temp);
	history_update(humidity_hist, humidity);
	LOG(LL_INFO, ("Temperature: %f (avg %.1f)", temp, average(temp_hist)));
	LOG(LL_INFO, ("Humidity:    %f (avg %.1f)", humidity, average(humidity_hist)));
}

static void rpc_cb(struct mg_rpc_request_info *ri, void *cb_arg,
                   struct mg_rpc_frame_info *fi, struct mg_str args)
{
	struct history *h1 = (struct history *)((struct rpc_args *) cb_arg)->h1;
	struct history *h2 = (struct history *)((struct rpc_args *) cb_arg)->h2;

	mg_rpc_send_responsef(ri, "{temp: %.1lf, humidity: %.1lf}", \
  		average(h1), average(h2));
  (void) fi;
  (void) args;
}

void dht_iot_rpc_init(struct history *temp_hist, struct history *humidity_hist)
{
		struct rpc_args *cb_arg;
		cb_arg = (struct rpc_args *) malloc(sizeof(*cb_arg));
		cb_arg->h1 = temp_hist;
		cb_arg->h2 = humidity_hist;
		mg_rpc_add_handler(mgos_rpc_get_global(), "Dht.Read", "", \
			rpc_cb, (void *)cb_arg);
}

void dht_iot_sample_init(struct mgos_dht *dht, struct history *temp_hist,\
	struct history *humidity_hist)
{
	struct sample_args *cb_arg;
	cb_arg = (struct sample_args *) malloc(sizeof(*cb_arg));
	cb_arg->dht = dht;
	cb_arg->h1 = temp_hist;
	cb_arg->h2 = humidity_hist;
	int sample_interval = mgos_sys_config_get_dht_iot_sample_interval();
	mgos_set_timer(sample_interval, true, dht_iot_sample_cb, cb_arg);
	LOG(LL_INFO, ("Started sample timer (interval %d ms)", sample_interval));
}

bool mgos_dht_iot_init(void)
{
	int pin = mgos_sys_config_get_dht_iot_dht_pin();

	struct mgos_dht *dht = mgos_dht_create(pin, DHT22);
	if (dht == NULL) {
		LOG(LL_WARN, ("Error configuring DHT22 sensor on pin %d.", pin));
	} else {
		LOG(LL_INFO, ("DHT22 sensor configured on pin %d.", pin));
		int sample_interval = mgos_sys_config_get_dht_iot_sample_interval();
		mgos_msleep(sample_interval); // wait to prevent nan results from DHT

		struct history *temp_hist = history_init(mgos_dht_get_temp(dht));
		LOG(LL_INFO, ("Allocated temp history (size: %d)", temp_hist->size));

		struct history *humidity_hist = history_init(mgos_dht_get_humidity(dht));
		LOG(LL_INFO, ("Allocated humidity history (size: %d)", humidity_hist->size));

		dht_iot_rpc_init(temp_hist, humidity_hist);
		dht_iot_sample_init(dht, temp_hist, humidity_hist);
	}
	LOG(LL_INFO, ("DHT-IOT library loaded."));
	return true;
}
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

static void timer_cb(void *dht) {
	LOG(LL_INFO, ("Temperature: %f", mgos_dht_get_temp(dht)));
	LOG(LL_INFO, ("Humidity:    %f", mgos_dht_get_humidity(dht)));
	void mgos_dht_close(struct mgos_dht *dht);
}

bool mgos_dht_iot_init(void) {
	int pin = mgos_sys_config_get_dht_iot_dht_pin();
	LOG(LL_INFO, ("DHT-IOT library loaded."));

	struct mgos_dht *dht = mgos_dht_create(pin, DHT22);
	if (dht == NULL) {
		LOG(LL_WARN, ("Error configuring DHT22 sensor on pin %d.", pin));
	} else {
		LOG(LL_INFO, ("DHT22 sensor configured on pin %d.", pin));
		mgos_set_timer(2000, false, timer_cb, dht);
	}
  return true;
}
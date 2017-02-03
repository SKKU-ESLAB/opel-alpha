/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Eunsoo Park<esevan.park@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CMFW_BT_H__
#define __CMFW_BT_H__
#include <cmfw_base.h>

typedef enum{
	CMFW_BT_DISCON,
	CMFW_BT_CONNECTED
} cmfw_bt_stat_e;

void bt_init();
int bt_open( cmfw_port_e bt_port );
void bt_close( cmfw_port_e bt_port );
void bt_try_connect( void *main_loop, ActionListener *action_listener );
int bt_connect(int cli_sample, cmfw_port_e port);
bool bt_connection_req( cmfw_port_e bt_port );
int bt_accept(cmfw_port_e port);
int bt_sock_get(cmfw_port_e port);
cmfw_bt_stat_e bt_sock_stat(cmfw_port_e port);

#endif //?__CMFW_BT_H__



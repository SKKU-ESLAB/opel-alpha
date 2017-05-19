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

#ifndef __CMFW_WFD_H__
#define __CMFW_WFD_H__
#include <cmfw_base.h>

enum {
	WFD_ERR_NONE = 0,
	WFD_SOCKET_FAIL = -1,
	WFD_BIND_FAIL = -2,
	WFD_LISTEN_FAIL = -3,
	WFD_ACCEPT_FAIL = -4
};
void wfd_init();
void wfd_deinit();

//This listeners should be freed in after_work_cb
void wfd_reset(void);
void wfd_on(cmfw_port_e port);
bool wfd_is_on(void);
int wfd_open(cmfw_port_e port);
void wfd_close(cmfw_port_e port);
int wfd_accept(cmfw_port_e port);

//void wfd_on( void *main_loop, ActionListener *action_listener );
//void wfd_off();
//void wfd_accept( void *main_loop, AcceptListener *accept_listener);

//void wfd_read( void *main_loop, int sock, ReadListener *read_listener );

#endif //?__CMFW_WFD_H__

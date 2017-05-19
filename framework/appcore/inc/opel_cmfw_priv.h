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

#ifndef __OPEL_CMFW_PRIV_H__
#define __OPEL_CMFW_PRIV_H__

typedef struct{
	unsigned char header_id;
	unsigned char header_flag;
	unsigned char *buf;
	unsigned short len; // payload len
	unsigned int offset;//data offset
} cmfw_queue_node_s;

typedef struct{
	int bt_sock;
	int wfd_sock;
	int port_stat;
} cmfw_sock_pair_s;

typedef enum{
	CMFW_CMD_WFD_ON = 1,
	CMFW_CMD_WFD_ON_ACK = 2,
	CMFW_CMD_WFD_OFF = 3,
	CMFW_CMD_WFD_OFF_ACK = 4
} cmfw_cmd_e;

int __cmfw_send_cmd(cmfw_port_e port, cmfw_cmd_e cmd);

#endif

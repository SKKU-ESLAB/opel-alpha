/* Copyright (c) 2015-2016 CISS, and contributors. All rights reserved.
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

#ifndef __OPEL_CMFW_H__
#include <cmfw_base.h>


typedef enum{
	CMFW_DISCON,
	CMFW_CONNECTED,
	CMFW_WFD_CONNECTED
} cmfw_stat_e;

typedef enum{
	CMFW_E_NONE = 0,
	CMFW_E_FAIL = -1,
	CMFW_E_OUT_OF_BOUND = -2,
	CMFW_E_DISCON = -3,
	CMFW_E_NO_FILE = -4,
	CMFW_E_INVALID_PARAM = -5
} cmfw_err_e;

typedef struct{
	unsigned file_size;
	unsigned curr_offset;
	unsigned curr_len;
	char src_fname[256];
	char dest_fname[256];
} cmfw_file_info_s;

void cmfw_init();
bool cmfw_open(cmfw_port_e port);
void cmfw_close(cmfw_port_e port);

int cmfw_accept(cmfw_port_e port);
int cmfw_connect(cmfw_port_e port);
cmfw_stat_e cmfw_get_stat();

/*
cmfw_recv_msg : Blocking operaiton
   @ return - The bytes of received data. or cmfw_err_e when error occurs.
   @ port (IN) Port enumueration number from which the message is received.
   @ buf (OUT) Message buffer to which the message is copied.
   @ len (IN) size of buf
   ? Note: If param len is shorter than message size, it would fail to recv msg.
  */
int cmfw_recv_msg(cmfw_port_e port, char *buf, int len);

/*
cmfw_recv_file : Blocking operation
   @ return - The bytes of received data. or cmfw_err_e when error occurs.
   @ port (IN) Port enumeration number from which the message is received.
   ? Note: This method does not guarantee the whole file transfer, but this is invoked whenever every file data arrived instead.
   */
int cmfw_recv_file(cmfw_port_e port, char *dest_dir);

/*
   
   */
int cmfw_send_msg(cmfw_port_e port, const char *buf, int len);
int cmfw_send_file(cmfw_port_e port, char *fname);
bool cmfw_wfd_on(cmfw_port_e port);

#endif //?__OPEL_CMFW_H__

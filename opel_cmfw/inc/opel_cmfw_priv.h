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

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



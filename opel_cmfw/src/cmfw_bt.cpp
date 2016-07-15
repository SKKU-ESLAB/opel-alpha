#include <string.h>
#include <sys/socket.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <cmfw_log.h>
#include <cmfw_bt.h>

static int bt_sock[DEFINED_NUM_PORTS];
static int bt_cli_sock[DEFINED_NUM_PORTS];
static sdp_session_t *sessions[DEFINED_NUM_PORTS];
static cmfw_bt_stat_e bt_stat[DEFINED_NUM_PORTS];
static char port_uuid[DEFINED_NUM_PORTS][16] = {\
	{0x0a,0x1b,0x2c,0x3d,0x4e,0x5f,0x6a,0x1c,0x2d,0x0e,0x1f,0x2a, 0x3b,0x4c,0x5d,0x6d},\
	{0x0a,0x1b,0x2c,0x3d,0x4e,0x5f,0x6a,0x1c,0x2d,0x0e,0x1f,0x2a, 0x3b,0x4c,0x5d,0x6e},\
	{0x0a,0x1b,0x2c,0x3d,0x4e,0x5f,0x6a,0x1c,0x2d,0x0e,0x1f,0x2a, 0x3b,0x4c,0x5d,0x6f},\
	{0x0a,0x1b,0x2c,0x3d,0x4e,0x5f,0x6a,0x1c,0x2d,0x0e,0x1f,0x2a, 0x3b,0x4c,0x5d,0x6a},\
	{0x0a,0x1b,0x2c,0x3d,0x4e,0x5f,0x6a,0x1c,0x2d,0x0e,0x1f,0x2a, 0x3b,0x4c,0x5d,0x6b}\
};

static sdp_session_t *bt_register_service( cmfw_port_e bt_port, int port )
{
	__ENTER__;
	char service_name[256];
	char service_dsc[1024];
	char service_prov[256];
	sdp_session_t *t_session = 0;
	char *uuid_char_arr;
	int i;

	uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid;
	sdp_list_t *l2cap_list = 0,
			   *rfcomm_list = 0,
			   *root_list = 0,
			   *proto_list = 0,
			   *access_proto_list = 0;
	sdp_data_t *channel = 0;

	if( bt_port == CMFW_DEFAULT_PORT ){
		strcpy( service_name, "OPEL_DEF_PORT" );
		strcpy( service_dsc, "OPEL Default Port" );
		strcpy( service_prov, "OPEL Default Port" );
		uuid_char_arr = port_uuid[bt_port];
	}
	else if( bt_port == CMFW_CONTROL_PORT ){
		strcpy( service_name, "OPEL_CTRL_PORT");
		strcpy( service_dsc, "OPEL Control Port" );
		strcpy( service_prov, "OPEL Control Port");
		uuid_char_arr= port_uuid[bt_port];
	}
	else {
		cmfw_log( "Other port" );
		strcpy( service_name, "OPEL_OTHER_PORT");
		strcpy( service_dsc, "OPEL Other Port" );
		strcpy( service_prov, "OPEL Other Port");
		uuid_char_arr= port_uuid[bt_port];
	}

	sdp_record_t *record = sdp_record_alloc();

	// set the general service ID

	sdp_uuid128_create( &svc_uuid, uuid_char_arr );

	sdp_set_service_id( record, svc_uuid );

	// make the service record publicly browsable
	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root_list = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups( record, root_list );

	// set l2cap information
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append( 0, &l2cap_uuid );
	proto_list = sdp_list_append( 0, l2cap_list );

	// set rfcomm information
	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	channel = sdp_data_alloc(SDP_UINT8, &port);
	rfcomm_list = sdp_list_append( 0, &rfcomm_uuid );
	sdp_list_append( rfcomm_list, channel );
	sdp_list_append( proto_list, rfcomm_list );

	// attach protocol information to service record
	access_proto_list = sdp_list_append( 0, proto_list );
	sdp_set_access_protos( record, access_proto_list );

	// set the name, provider, and description
	sdp_set_info_attr(record, service_name, service_prov, service_dsc);

	// connect to the local SDP server, register the service record, and disconnect
	bdaddr_t my_bdaddr_any = {0, 0, 0, 0, 0, 0};
	bdaddr_t my_bdaddr_local = {0, 0, 0, 0xff, 0xff, 0xff};
	t_session = sdp_connect( &my_bdaddr_any, &my_bdaddr_local, SDP_RETRY_IF_BUSY );
	if(NULL == t_session){
		sdp_data_free( channel );
		sdp_list_free( l2cap_list, 0 );
		sdp_list_free( rfcomm_list, 0 );
		sdp_list_free( root_list, 0 );
		sdp_list_free( access_proto_list, 0 );
		sdp_record_free( record ); //Let me check later
		__EXIT__;
		return NULL;
	}
	int err = sdp_record_register(t_session, record, 0);

	sdp_data_free( channel );
	sdp_list_free( l2cap_list, 0 );
	sdp_list_free( rfcomm_list, 0 );
	sdp_list_free( root_list, 0 );
	sdp_list_free( access_proto_list, 0 );
	sdp_record_free( record ); //Let me check later
	__EXIT__;

	return t_session;

}

static int bt_dynamic_bind_rc( int sock )
{
	int err;
	int tmp_port;
	struct sockaddr_rc sockaddr;

	bdaddr_t my_baddr_any = { 0, 0, 0, 0, 0, 0 };
	sockaddr.rc_family = AF_BLUETOOTH;
	sockaddr.rc_bdaddr = my_baddr_any;
	sockaddr.rc_channel = (unsigned char) 0;

	for ( tmp_port = 1; tmp_port < 31; tmp_port++ ){
		sockaddr.rc_channel = tmp_port;
		err = bind(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_rc));
		if( !err ){
			return tmp_port;
		}

		if( errno == EINVAL ){
			break;
		}
	}

	if( tmp_port == 31 ){
		err = -1;
		return -1;
	}
	return err;
}

void bt_init()
{
	int i;
	for(i=0; i<DEFINED_NUM_PORTS; i++){
		bt_sock[i] = -1;
		sessions[i] = NULL;
	}
}

int bt_open( cmfw_port_e bt_port )
{
	int bt_socket;
	sdp_session_t *session;

	if( bt_sock[bt_port] >= 0 ){
		if( sessions[bt_port] != NULL )
			return bt_sock[bt_port];
		else{
			close( bt_sock[bt_port] );
			bt_sock[bt_port] = -1;
		}
	}
	else if( sessions[bt_port] != NULL ){
		sdp_close( sessions[bt_port] );
		sessions[bt_port] = NULL;
	}

	bt_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if( bt_socket < 0 ){
		cmfw_log( "%s", strerror(errno) );
		return -1;
	}

	int port = bt_dynamic_bind_rc(bt_socket);
	if( port < 1 || port >30 ){
		cmfw_log( "dynamic binding failed");
		return -1;
	}
	else
		cmfw_log("Bound socket %d to port %d", bt_socket, port);

	session = bt_register_service( bt_port, port );

	if( NULL == session ){
		cmfw_log( "session creation failed" );
		return -1;
	}

	if( listen( bt_socket, 1 ) < 0 ) {
		cmfw_log("Listening failed");
		return -1;
	}

	sessions[bt_port] = session;
	bt_sock[bt_port] = bt_socket;

	return bt_socket;
}

void bt_close( cmfw_port_e port )
{
	if( NULL != sessions[port]){
		sdp_close(sessions[port]);
		sessions[port] = NULL;
	}
	if( bt_sock[port] >= 0){
		close(bt_sock[port]);
		bt_sock[port] = -1;
	}

	bt_stat[port] = CMFW_BT_DISCON;
}


void bt_try_connect( void *main_loop, ActionListener *action_listener)
{

}

bool bt_connection_req( cmfw_port_e bt_port )
{

}
int bt_getpeername(int sock, void *sock_addr)
{
	struct sockaddr_rc *cli_addr = (struct sockaddr_rc *)sock_addr;
	socklen_t len = sizeof(struct sockaddr_rc);
	return getpeername(sock, (struct sockaddr *)cli_addr, &len);
}
int bt_connect(int cli, cmfw_port_e port)
{
	struct sockaddr_rc cli_addr, addr;
	socklen_t len = sizeof(cli_addr);

	sdp_session_t *session;
	bdaddr_t my_bdaddr_any = {0,0,0,0,0,0};
	char bdad[256];
	sdp_list_t *response_list = NULL, *search_list, *attrid_list;
	unsigned int range = 0x0000ffff;
	unsigned char rfcomm_channel = 0;
	int status;
	int sock;
	uuid_t svc_uuid;

	sdp_uuid128_create( &svc_uuid, port_uuid[port] );

	//Get peer android info
	int res = getpeername(cli, (struct sockaddr *)&cli_addr, &len);
	if(res < 0)
		return res;

	ba2str(&(cli_addr.rc_bdaddr), bdad);
	cmfw_log("Peer to connect : %s(%d)", bdad, cli_addr.rc_channel);


	//Get service info
	session = sdp_connect(&my_bdaddr_any, &(cli_addr.rc_bdaddr), SDP_RETRY_IF_BUSY);
	if( NULL == session ){
		cmfw_log("sdp connection failed");
		return -1;
	}
	cmfw_log("Android SDP Session connected");
	search_list = sdp_list_append(NULL, &svc_uuid);
	attrid_list = sdp_list_append(NULL, &range);

	res = sdp_service_search_attr_req(session, search_list, \
			SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
	sdp_list_t *r = response_list;

	for(; r; r= r->next){
		sdp_record_t *rec = (sdp_record_t *) r->data;
		sdp_list_t *proto_list;

		if (sdp_get_access_protos(rec, &proto_list) == 0) {
			sdp_list_t *p = proto_list;

			// go through each protocol sequence
			for( ; p ; p = p->next ) {
				sdp_list_t *pds = (sdp_list_t*)p->data;

				// go through each protocol list of the protocol sequence
				for( ; pds ; pds = pds->next ) {

					// check the protocol attributes
					sdp_data_t *d = (sdp_data_t*)pds->data;
					int proto = 0;
					for( ; d; d = d->next ) {
						switch( d->dtd ) {
							case SDP_UUID16:
							case SDP_UUID32:
							case SDP_UUID128:
								proto = sdp_uuid_to_proto( &d->val.uuid );
								break;
							case SDP_UINT8:
								if( proto == RFCOMM_UUID ) {
									rfcomm_channel = d->val.int8;
								}
								break;
						}
					}
				}
				sdp_list_free( (sdp_list_t*)p->data, 0 );
			}
			sdp_list_free( proto_list, 0);
		}

		sdp_record_free( rec );
		if(rfcomm_channel > 0)
			break;
	}

	sdp_close(session);
	sdp_list_free( search_list, 0);
	sdp_list_free( attrid_list, 0);
	sdp_list_free( response_list, 0);

	if(rfcomm_channel <= 0){
		cmfw_log("Cannot find service");
		return -1;
	}

	//Making socket and connect
	sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = rfcomm_channel;
	memcpy(&(addr.rc_bdaddr), &(cli_addr.rc_bdaddr), sizeof(addr.rc_bdaddr));

	res = connect(sock, (struct sockaddr *)&addr, sizeof(addr));

	if( res < 0 )
		return res;

	return sock;
}
int bt_accept( cmfw_port_e port)
{
	struct sockaddr_rc cli_addr = {0, };
	socklen_t opt = sizeof(cli_addr);
	int new_cli_fd = -1;

	if(bt_sock[port] < 0){
		cmfw_log( "Port is not opened yet" );
		return -1;
	}
	new_cli_fd = accept(bt_sock[port], (struct sockaddr *)&cli_addr, &opt);

	if(new_cli_fd < 0){
		cmfw_log("Accept Failed");
		bt_close(port);
		return -1;
	}

	bt_stat[port] = CMFW_BT_CONNECTED;

	return new_cli_fd;
}

int bt_sock_get(cmfw_port_e port)
{
	return bt_sock[port];
}

cmfw_bt_stat_e bt_sock_stat(cmfw_port_e port)
{
	return bt_stat[port];
}

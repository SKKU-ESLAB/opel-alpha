#include <stdint.h>
#include <cstdio>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <comm_communicator.h>

OPEL_Communicator::OPEL_Communicator()
{
	bt_scan_list = NULL;

	// Bluetooth Part
	ScanBD();	
}

OPEL_Communicator::~OPEL_Communicator()
{
	if(NULL != bt_scan_list);
	free( bt_scan_list );
}

OPEL_Communicator::COMM_Advertise()
{
}

int OPEL_Communicator::ScanBD()
{
	inquiry_info *ii = NULL;
	int max_rsp;
	int dev_id, hci_sock, len, flags;
	char addr[19] = {0};
	inquiry_info *tmp_bt_scan_list;

	dev_id = hci_get_route(NULL);
	sock = hci_open_dev(dev_id);
	if ( dev_id < 0 || sock < 0 ) {
		comm_log("Opening HCI Socket Error");
		return COMM_E_FAIL;
	}


	len = MAX_BT_SCAN_LEN;
	max_rsp = MAX_BT_SCAN_RSP;
	flags = IREQ_CACHE_FLUSH;
	tmp_bt_scan_list = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
	num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
	if( num_rsp < 0 ){
		comm_log("HCI inquiry < 0");
		return COMM_E_FAIL;
	}

	tmp_bt_scan_list = (inquiry_info*)malloc(num_rsp * sizeof(inquiry_info));
	memcpy(tmp_bt_scan_list, ii, num_rsp * sizeof(inquiry_info));

	if(NULL != bt_scan_list){
		free(bt_scan_list);
		bt_scan_list = tmp_bt_scan_list; 
	}

	free( ii );
	close( sock );

	return COMM_S_OK;
}

int OPEL_Communicator::ScanWiFiDirect()
{
	return COMM_S_OK;
}

int OPEL_Communicator::Scan(OUT opel_bd_name *name_array, IN bool refresh = 0)
{
	int i, res;

	if(refresh){
		if(COMM_S_OK != ScanBD(name_array)){
			comm_log("Failed to connect bluetooth");
		}
	}

	for( i = 0 ; i < num_rsp ; i++ ) {
		ba2str(*(bt_scan_list+i)->bdaddr, addr);
		memset(name_array[i], 0, sizeof(name));
		if ( hci_read_remote_name(sock, &(bt_scan_list+i)->bdaddr, sizeof(name_array[i]), name_array[i], 0) < 0)
			strcpy(name_array[i], "[unknown]");
		printf("%s %s\n", addr, name_array[i]);
	}

	return 0;
}

int OPEL_Communicator::OpenChannel(IN const char *intf_name, OUT OPEL_Server **opel_server, IN void (*server_handler)(IN OPEL_Socket *client, IN OPEL_MSG *msg) = NULL)
{	
	if(NULL == opel_server){
		comm_log("Opel server has no container (Second argument is null)");
		return COMM_E_POINTER;
	}
	
	*opel_server = new OPEL_Server(intf_name);
	(*opel_server)->SetServerhandler(server_handler);

	return COMM_S_OK;
}

int OPEL_Communicator::AccessChannel(IN const char *intf_name, OUT OPEL_Client **opel_client, IN void (*client_handler)(IN OPEL_MSG *msg) = NULL)
{
	if(NULL == opel_client){
		comm_log("Opel client (Sec Argument is null) has no container");
		return COMM_E_POINTER;
	}

	*opel_client = new OPEL_Client(intf_name);
	(*opel_client)->SetClientHandler(client_handler);

	return COMM_S_OK;
}

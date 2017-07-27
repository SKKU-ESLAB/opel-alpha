#include <comm_util.h>
#include <comm_queue.h>


class OPEL_Communicator
{
	private:
		int BD_Stat, WD_Stat;

		/* Bluetooth Configuration Properties */
		bdaddr_t target;
		inquiry_info *bt_scan_list;
		int num_rsp;
		void SetTarget(char *bdaddr_str);

		/* Wifi Direct Configuration Properties */


	public:
		OPEL_Communicator();
		~OPEL_Communicator();

		// Advertise this device as a opel device

		int Scan(OUT opel_bd_name *name_array, IN bool refresh = 0 );
		int ScanWiFiDirect();
		int ScanBD();

		/* This address is MAC-like 48 bit address with char array representation */
		int Connect(IN const char *addr);

		/*
		   Open RFCOMM Channel and advertise it as a service into the pico net
		   intf_name (IN) - This should be allocated before method call.
		   opel_server (OUT) - This method returns OPEL_Server class as a pointer by allocating the new instance of it.
		 */
		int OpenChannel(IN const char *intf_name, OUT OPEL_Server **opel_server, IN void (*server_handler)(IN OPEL_Socket *client, IN OPEL_MSG *msg) = NULL);

		/*
		   Access the channel which is opened by the other piconet node
		   intf_name (IN) - This should be allocated before method call.
		   opel_client (OUT) - This method returns OPEL_client class as a pointer by allocating the new instance of it.
		 */
		int AccessChannel(IN const char *intf_name, OUT OPEL_Client **opel_client, IN void (*client_handler)(IN OPEL_MSG *msg) = NULL);

};

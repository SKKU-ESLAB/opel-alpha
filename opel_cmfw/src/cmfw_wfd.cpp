#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <cmfw_log.h>
#include <cmfw_base.h>
#include <cmfw_wfd.h>
#include <tmp_control.h>
static int wfd_sock[DEFINED_NUM_PORTS];
static int wfd_port_nums[DEFINED_NUM_PORTS];

void wfd_init()
{
	__ENTER__;
	static bool initialized;
	int i;

	if(initialized)
		return;
	initialized = true;
	for(i=0; i<DEFINED_NUM_PORTS; i++){
		wfd_sock[i] = -1;
		wfd_port_nums[i] = 10001+i;
		cmfw_log("wfd_port:%d-%d", i, wfd_port_nums[i]);
	}

	char buf[256];
	tmpc_get("wifi/wifi-direct/init", buf, 256);
	int value = atoi(buf);
	if( value == 1 ){
		cmfw_log("P2p already inited");
		return;
	}



	system( "./bin/p2p_setup.sh init" );
	
	cmfw_log("Wifi direct init done");
	__EXIT__;
}

void wfd_deinit()
{
}

static bool started;
void wfd_on()
{
	__ENTER__;

	char buf[256];

	tmpc_get("wifi/wifi-direct/wfd_stat", buf, 256);
	int value = atoi(buf);
	if(value == 1){
		cmfw_log("P2p already started!");
		return;
	}
	else{
	 //system("./bin/p2p_setup.sh start");
		cmfw_log("P2p start!");
	}
	started = true;
	__EXIT__;
}

int wfd_open(cmfw_port_e port)
{
	int res = WFD_ERR_NONE, listen_fd;
	struct sockaddr_in saddr;

	if(wfd_sock[port] >= 0)
		wfd_close(port);

	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		res = WFD_SOCKET_FAIL;
		return res;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr("192.168.49.1");
	saddr.sin_port = htons(wfd_port_nums[port]);

	int reuse = 1;
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1){
		res = WFD_SOCKET_FAIL;
		return res;
	}

	if(bind(listen_fd, (struct sockaddr *)&saddr,\
				sizeof(saddr)) < 0){
		cmfw_log("bind error:%s", strerror(errno));
		res = WFD_BIND_FAIL;
		return res;
	}

	if(listen(listen_fd, 5) < 0){
		res = WFD_LISTEN_FAIL;
		return res;
	}
	
	cmfw_log("Open success");
	wfd_sock[port] = listen_fd;

	return listen_fd;
}

void wfd_close(cmfw_port_e port)
{
	if(wfd_sock[port] >= 0)
		close(wfd_sock[port]);

	wfd_sock[port] = -1;

	//system("./bin/p2p_setup.sh stop");

	/*
	int i;
	for(i=0; i<DEFINED_NUM_PORTS; i++){
		if(wfd_sock[i] > 0)
			break;
	}
	*/
	
}
int wfd_accept(cmfw_port_e port)
{
	__ENTER__;

	int res = WFD_ERR_NONE, cli_fd, caddr_len;
	struct sockaddr_in caddr;
	struct hostent *h;
	
	if((res = wfd_open(port)) < 0)
		return res;

	caddr_len = sizeof(caddr);
	cmfw_log("WFD Server(PORT:%d) now accepting...", wfd_port_nums[port]);
	if((cli_fd = accept(wfd_sock[port], (struct sockaddr *)&caddr,\
					(socklen_t *)&caddr_len)) < 0){
		cmfw_log("accept_err:%s", strerror(errno));
		res = WFD_ACCEPT_FAIL;
		return res;
	}
	cmfw_log("Accepted!");


	cmfw_log("Server connected to (%s) \n",\
			inet_ntoa(*(struct in_addr *)&caddr.sin_addr));

	wfd_close(port);


	res = cli_fd;
	__EXIT__;

	return res;

}
//// wfd_accept

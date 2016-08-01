#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <map>
#include <list>
#include <mutex>
#include <fcntl.h>
#include <sys/time.h>

#include <cmfw_base.h>
#include <opel_cmfw.h>
#include <opel_cmfw_priv.h>
#include <cmfw_log.h>
#include <cmfw_wfd.h>
#include <cmfw_bt.h>
#include <tmp_control.h>

using namespace std;

static mutex cli_sock_mutex[DEFINED_NUM_PORTS];
static cmfw_sock_pair_s cli_socks[DEFINED_NUM_PORTS];
static unsigned char header_id[DEFINED_NUM_PORTS];

//If bug occurs, locks for stores should be added
static map<unsigned char, cmfw_file_header_s *> file_header_store[DEFINED_NUM_PORTS];
static map<unsigned char, cmfw_msg_header_s *> msg_header_store[DEFINED_NUM_PORTS];

static mutex msg_recv_lock[DEFINED_NUM_PORTS];
static mutex file_recv_lock[DEFINED_NUM_PORTS];
static list<cmfw_queue_node_s *> msg_recv_queue[DEFINED_NUM_PORTS];
static list<cmfw_queue_node_s *> file_recv_queue[DEFINED_NUM_PORTS];

void cmfw_init()
{
	int i;

	bt_init();
	wfd_init();

	for(i=0; i<DEFINED_NUM_PORTS; i++){
		cli_socks[i].bt_sock = -1;
		cli_socks[i].wfd_sock = -1;
		header_id[i] = 1;
	}
}
bool cmfw_open(cmfw_port_e port)
{	
	if( port == CMFW_DEFAULT_PORT ){
		bt_open( CMFW_CONTROL_PORT );
		bt_open( CMFW_RFS_PORT );
	}
	return (bt_open(port) >= 0);
}

void cmfw_close(cmfw_port_e port)
{
	if( port == CMFW_DEFAULT_PORT ){
		bt_close( CMFW_CONTROL_PORT );
		bt_close( CMFW_RFS_PORT );
	}
	bt_close(port);
	wfd_close(port);
}

int cmfw_accept(cmfw_port_e port)
{
	cli_socks[port].bt_sock = bt_accept(port);
	if(cli_socks[port].bt_sock > 0)
		cmfw_log("Client accepted to BT");
	return cli_socks[port].bt_sock;
}

cmfw_stat_e cmfw_get_stat(cmfw_port_e port)
{
	if( bt_sock_stat(port) == CMFW_BT_DISCON )
		return CMFW_DISCON;
	else if( cli_socks[port].wfd_sock > 0  )
		return CMFW_WFD_CONNECTED;
	else
		return CMFW_CONNECTED;
}

static int read_fully(int fd, void *buf, size_t len)
{
	int bytes = 0;
	int res;
	while( bytes < len ){
		res = read(fd, (char *)buf+bytes, len-bytes);
		if(res < 0 ){
			return res;
		}
		else if(res == 0){
			cmfw_log("Client socket closed");
			return -1;
		}
		bytes += res;
	}
}

int __cmfw_recv_msg_priv(int *res, cmfw_port_e port, char *buf, int len,\
		unsigned char header_id_local,\
		unsigned char header_flag_local,\
		unsigned char *payload_data,\
		unsigned short payload_size_local,\
		unsigned int offset_local)
{
	if( _is_header(header_flag_local) ){
		//If the node includes header
		map<unsigned char, cmfw_msg_header_s *>::iterator it_msg_header = msg_header_store[port].find( header_id_local );
		if( it_msg_header != msg_header_store[port].end() ){
			//If header already exists, free it
			cmfw_log("Header already exists;error");
			cmfw_msg_header_s *msg_header = it_msg_header->second;
			if( msg_header->data != NULL )
				free(msg_header->data);
			msg_header_store[port].erase( it_msg_header );
			free(msg_header);
		}
		cmfw_msg_header_s *msg_header = (cmfw_msg_header_s *)malloc(sizeof(cmfw_msg_header_s));
		int total_data_size = *((int *)payload_data);
		msg_header->total_data_size = ntohl(total_data_size);
		msg_header->data = (unsigned char *)malloc(msg_header->total_data_size);

		msg_header_store[port][header_id_local] = msg_header;
	}
	else{
		map<unsigned char, cmfw_msg_header_s *>::iterator it_msg_header = msg_header_store[port].find( header_id_local );
		if( it_msg_header == msg_header_store[port].end() ){
			//If header does not exists, it's error
			cmfw_log("Header does not exist; error");
			exit(1);
		}

		cmfw_msg_header_s *msg_header = it_msg_header->second;

		memcpy( &(msg_header->data[offset_local]), payload_data, payload_size_local );
		if( offset_local + payload_size_local == msg_header->total_data_size ){
			msg_header_store[port].erase(it_msg_header);
			if( len < msg_header->total_data_size ){
				free(msg_header->data);
				free(msg_header);
				*res = CMFW_E_OUT_OF_BOUND;
				return 0;
			}
			memcpy( buf, msg_header->data, msg_header->total_data_size );
			free(msg_header->data);
			free(msg_header);

			*res = CMFW_E_NONE;
			return 0;
		}
	}

	return 1;
}

int __cmfw_get_sock(cmfw_port_e port)
{
	switch(cmfw_get_stat(port)){
		case CMFW_DISCON:
			return -1;
		case CMFW_CONNECTED:
			return cli_socks[port].bt_sock;
		case CMFW_WFD_CONNECTED:
			return cli_socks[port].wfd_sock;
	}
	return -1;
}

int __cmfw_recv_msg_node(cmfw_port_e port, char *buf, int len)
{
	int res;
	while(true){
		unsigned char header_id_local = 0;
		unsigned char header_flag_local = 0;
		unsigned short payload_size = 0;
		unsigned int offset = 0;

		//Search recv queue to construct a message.
		//If queue is empty, break.
		//If a message is completely received, return.
		if(msg_recv_queue[port].empty())
			break;
		msg_recv_lock[port].lock();
		cmfw_queue_node_s *msg_node = msg_recv_queue[port].front();
		msg_recv_queue[port].pop_front();
		msg_recv_lock[port].unlock();
		//If queue is not empty
		header_id_local = msg_node->header_id;

		int cont;
		cont = __cmfw_recv_msg_priv(&res, port, buf, len,\
				msg_node->header_id,\
				msg_node->header_flag,\
				msg_node->buf,\
				msg_node->len,\
				msg_node->offset);

		free(msg_node->buf);
		free(msg_node);

		if(!cont){
			return res;
		}
	}

	return 1;
}
int __cmfw_recv_msg_sock(cmfw_port_e port, char *buf, int len, bool is_wfd)
{
	while(true){
		//If a message is completely received, break and return.
		//If a file is received by this read, dangle the data to the file recv queue.
		unsigned char header_id;
		unsigned char header_flag;
		unsigned short payload_size;
		unsigned int curr_offset;
		unsigned char *payload_data;
		static int n = 0;

		int cli_sock = -1;
		if( is_wfd ){
			cli_sock = __cmfw_get_sock(port);
		}
		else {
			cli_sock = cli_socks[port].bt_sock;
		}
		if( cli_sock < 0 ){
			return CMFW_E_DISCON;
		}

		cli_sock_mutex[port].lock();
		int res_read;
		res_read = read_fully( cli_sock, (void *)&header_id, sizeof(header_id) );
		if( res_read <= 0 ){
			cmfw_log("read error");
			cli_sock_mutex[port].unlock();
			return CMFW_E_DISCON;
		}

		read_fully( cli_sock, (void *)&header_flag, sizeof(header_flag) );
		if( res_read <= 0 ){
			cmfw_log("read error");
			cli_sock_mutex[port].unlock();
			return CMFW_E_DISCON;
		}
		read_fully( cli_sock, &payload_size, sizeof(payload_size) );
		if( res_read <= 0 ){
			cmfw_log("read error");
			cli_sock_mutex[port].unlock();
			return CMFW_E_DISCON;
		}
		read_fully( cli_sock, &curr_offset, sizeof(curr_offset) );
		if( res_read <= 0 ){
			cmfw_log("read error");
			cli_sock_mutex[port].unlock();
			return CMFW_E_DISCON;
		}

		payload_size = ntohs(payload_size);
		curr_offset = ntohl(curr_offset);
		if( payload_size > 0 ){
			payload_data = (unsigned char *)malloc(payload_size);
			//cmfw_log("read %d", payload_size);
			res_read = read_fully(cli_sock, payload_data, payload_size);
			if( res_read <= 0){
				cmfw_log("read error");
				cli_sock_mutex[port].unlock();
				return CMFW_E_DISCON;
			}
		}
		cli_sock_mutex[port].unlock();

		if( _is_file(header_flag) ){
			//If it's file, queue data to the file queue
			cmfw_queue_node_s *queue_node = (cmfw_queue_node_s *)malloc(sizeof(cmfw_queue_node_s));
			queue_node->header_id = header_id;
			queue_node->header_flag = header_flag;
			queue_node->buf = payload_data;
			queue_node->len = payload_size;
			queue_node->offset = curr_offset;

			file_recv_lock[port].lock();
			file_recv_queue[port].push_back( queue_node );
			file_recv_lock[port].unlock();
		}
		else{
			//It's msg!
			int res, cont;
			cont = __cmfw_recv_msg_priv(&res, port, buf, len,\
					header_id,\
					header_flag,\
					payload_data,\
					payload_size,\
					curr_offset);

			free(payload_data);

			if(!cont)
				return res;
		}
	}

}
int cmfw_recv_msg(cmfw_port_e port, char *buf, int len)
{
	int res = __cmfw_recv_msg_node(port, buf, len);

	if( res <= 0)
		return res;

	return __cmfw_recv_msg_sock(port, buf, len, 0);
}

int __cmfw_recv_cmd(cmfw_port_e port)
{
	int bt_sock = cli_socks[port].bt_sock;
	unsigned char header_flag;

	int res = read(bt_sock, &header_flag, sizeof(header_flag));

	if(res <= 0)
		return res;

	if(!_is_cmd(header_flag))
		return -1;

	return _reserved_info(header_flag);
}

int cmfw_connect(cmfw_port_e port)
{
	int cli_sock, i;
	int res;


	//Getting bdaddr fo client
	for(i=0; i < DEFINED_NUM_PORTS; i++){
		cli_sock = cli_socks[i].bt_sock;
		if(cli_sock > 0)
			break;
	}

	res = bt_connect( cli_sock, port );
	return res;
}

void __get_time_interval(struct timeval *now, struct timeval *before)
	//now = now-before
{
	now->tv_usec-=before->tv_usec;
	now->tv_sec -= before->tv_sec;
	if(now->tv_usec < 0){
		now->tv_usec += 1000000;
		now->tv_sec -= 1;
	}
}
int __cmfw_process_cmd(int cmd, cmfw_port_e port)
{
	__ENTER__;
	struct timeval check, now;
	switch( cmd ){
		case CMFW_CMD_WFD_ON:
			{
				gettimeofday(&check, NULL);
				cmfw_log("WFD_on");
				wfd_on(port);

				cmfw_log("cmfw connect trial");
				int sock = cmfw_connect(CMFW_CONTROL_PORT);

				if(sock < 0){
					cmfw_log("Control port connection failed");
					return CMFW_E_FAIL;
				}
				cli_socks[CMFW_CONTROL_PORT].bt_sock = sock;
				char buf[256];
				int cmd = __cmfw_send_cmd(CMFW_CONTROL_PORT, CMFW_CMD_WFD_ON_ACK);
				cmfw_log("Connection closed");

				close(sock);
				cli_socks[CMFW_CONTROL_PORT].bt_sock = -1;
				cmfw_log("Command Sent and closed");
				gettimeofday(&now, NULL);
				__get_time_interval(&now, &check);
				gettimeofday(&check, NULL);
				cmfw_log("WFD On: %d.%d", now.tv_sec, now.tv_usec);

				if((sock = wfd_accept(port)) < 0)
					return CMFW_E_FAIL;
				gettimeofday(&now, NULL);
				__get_time_interval(&now, &check);
				cmfw_log("WFD Connected: %d.%d", now.tv_sec, now.tv_usec);

				cli_socks[port].wfd_sock = sock;

				break;
			}
		case CMFW_CMD_WFD_OFF:
			cmfw_log("Cmd wfd off");
			break;
		default:
			break;
	}

	__EXIT__;
	return CMFW_E_NONE;
}
static void _wfd_cli_close(cmfw_port_e port){
	cmfw_log("cli sock: %d", cli_socks[port].wfd_sock);
	if(cli_socks[port].wfd_sock > 0)
		close(cli_socks[port].wfd_sock);
	cli_socks[port].wfd_sock = -1;
}
static void _bt_cli_close(cmfw_port_e port){
	if(cli_socks[port].bt_sock > 0)
		close(cli_socks[port].bt_sock);
	cli_socks[port].bt_sock = -1;
}
static bool _wfd_on(cmfw_port_e port){
	bool by_wfd;

	wfd_reset();
	wfd_on(port);
	cli_socks[CMFW_CONTROL_PORT].bt_sock = bt_accept(CMFW_CONTROL_PORT);
	if((by_wfd = wfd_is_on()))
		cmfw_send_msg(CMFW_CONTROL_PORT, "on", 2);
	else
		cmfw_send_msg(CMFW_CONTROL_PORT, "off", 3);

	close(cli_socks[CMFW_CONTROL_PORT].bt_sock);
	cli_socks[CMFW_CONTROL_PORT].bt_sock = -1;

	return by_wfd;
}

int cmfw_recv_file(cmfw_port_e port, char *dest_dir)
{

	/* wfd on */
	bool by_wfd = _wfd_on(port);
	/* ? wfd on */
	if(by_wfd)
		cli_socks[port].wfd_sock = wfd_accept(port);
	else{
		cli_socks[CMFW_RFS_PORT].bt_sock = cmfw_accept(CMFW_RFS_PORT);
	}

	/* recv file */
	cmfw_log("Receiving File ...");
	unsigned char buf[CMFW_PACKET_SIZE];
	int bytes = 0;
	int res_read;
	int wfd_sock = cli_socks[port].wfd_sock;
	int rfs_cli_sock = cli_socks[CMFW_RFS_PORT].bt_sock;

	if(by_wfd && wfd_sock < 0){
		return CMFW_E_DISCON;
	}
	else if((!by_wfd) && rfs_cli_sock < 0){
		return CMFW_E_DISCON;
	}

	struct timeval start, end;

	gettimeofday(&start, NULL);

	unsigned char fname_len;
	//
	if(by_wfd)
		res_read = read(wfd_sock, &fname_len, 1);
	else
		res_read = read(rfs_cli_sock, &fname_len, 1);
	if(res_read < 0){
		if(by_wfd)
			_wfd_cli_close(port);
		else
			_bt_cli_close(CMFW_RFS_PORT);
		return CMFW_E_DISCON;
	}
	cmfw_log("fname_len:%d", fname_len);
	char *dest_file_path;
	char *to_read;
	if(dest_dir == NULL){
		int len = strlen("./");
		dest_file_path = (char *)malloc(len+fname_len+1);
		memcpy(dest_file_path, "./", len);
		to_read = dest_file_path+len;
		dest_file_path[len+fname_len] = '\0';
	}
	else{
		int len = strlen(dest_dir);
		if(dest_dir[len-1] != '/')
			len++;
		dest_file_path = (char *)malloc(len+fname_len+1);
		memcpy(dest_file_path, dest_dir, len);
		if(dest_file_path[len-1] != '/')
			dest_file_path[len-1] = '/';
		to_read = dest_file_path+len;
		dest_file_path[len+fname_len] = '\0';
	}
	//
	if(by_wfd)
		res_read = read(wfd_sock, to_read, fname_len);
	else
		res_read = read(rfs_cli_sock, to_read, fname_len);
	if(res_read < 0){
		free (dest_file_path);
		if(by_wfd)
			_wfd_cli_close(port);
		else
			_bt_cli_close(CMFW_RFS_PORT);
		return CMFW_E_DISCON;
	}

	cmfw_log("Dest File Path:%s", dest_file_path);

	int fd_file = open(dest_file_path, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	if( fd_file < 0 ){
		free(dest_file_path);
		_wfd_cli_close(port);
		return CMFW_E_NO_FILE;
	}

	int fsize;
	//
	if(by_wfd)
		res_read = read(wfd_sock, &fsize, 4);
	else
		res_read = read(rfs_cli_sock, &fsize, 4);
	if(res_read < 0){
		free(dest_file_path);
		if(by_wfd)
			_wfd_cli_close(port);
		else
			_bt_cli_close(CMFW_RFS_PORT);
		return CMFW_E_DISCON;
	}

	fsize = ntohl(fsize);
	cmfw_log("fsize: %d", fsize);

	int curr_progress = 0;
	int prev_progress = 0;
	while( bytes < fsize ){
		if(by_wfd)
			res_read = read(wfd_sock, buf, CMFW_PACKET_SIZE);
		else
			res_read = read(rfs_cli_sock, buf, CMFW_PACKET_SIZE);
		
		if(res_read <= 0){
			free(dest_file_path);
			if(by_wfd)
				_wfd_cli_close(port);
			else
				_bt_cli_close(CMFW_RFS_PORT);
			return CMFW_E_DISCON;
		}

		write(fd_file, buf, res_read);

		bytes += res_read;
		curr_progress = (bytes * 10 / fsize);
		if(curr_progress != prev_progress){
			prev_progress = curr_progress;
			cmfw_log("Receving File...%d%", curr_progress*10);
		}
	}

	if(by_wfd)
		res_read = write(wfd_sock, "1", 1);
	else
		res_read = write(rfs_cli_sock, "1", 1);


	gettimeofday(&end, NULL);

	end.tv_usec -= start.tv_usec;
	if(end.tv_usec < 0){
		end.tv_usec += 1000000;
		end.tv_sec -= 1;
	}
	end.tv_sec -= start.tv_sec;

	double tm = (double)(end.tv_sec) + (double)(end.tv_usec/1000000.0);

	double bps = fsize/tm;

	if(bps > 1024.0)
		cmfw_log("KBPS: %llf", bps/1024.0);
	else
		cmfw_log("Byte per second: %llf", bps);


	close(fd_file);
	free(dest_file_path);
	if(by_wfd)
		_wfd_cli_close(port);
	else
		_bt_cli_close(CMFW_RFS_PORT);

	/* ? recv file */


	return CMFW_E_NONE;
}

int __cmfw_send_cmd(cmfw_port_e port, cmfw_cmd_e cmd)
{
	int res = 0;
	int cli_sock = cli_socks[port].bt_sock;

	if( cli_sock < 0 )
		return CMFW_E_DISCON;


	unsigned char header_info = 0x10 | (0x0F & cmd);
	res = write(cli_sock, &header_info, 1);

	return res;
}
int cmfw_send_msg(cmfw_port_e port, char *buf, int len)
{
	int res = 0;

	int cli_sock = cli_socks[port].bt_sock;

	if( cli_sock < 0 )
		return CMFW_E_DISCON;

	unsigned char *to_write = NULL;
	unsigned char *payload_header_data = NULL;

	cmfw_header_s *payload_header = (cmfw_header_s *)malloc(\
			sizeof(cmfw_header_s));

	payload_header->header_id = header_id[port]++;
	//__file_bit:0x80, __data_bit:0x40, __end_bit:0x20
	payload_header->header_info = 0;
	payload_header->payload_size = CMFW_MSG_HEADER_SIZE;
	payload_header->curr_offset = 0;

	//Send header
	to_write = (unsigned char*)malloc(CMFW_HEADER_SIZE + payload_header->payload_size);
	memcpy(to_write, &(payload_header->header_id), 1);
	memcpy(to_write+1, &(payload_header->header_info), 1);
	unsigned short tmp_payload_size = htons(payload_header->payload_size);
	memcpy(to_write+2, &tmp_payload_size, 2);
	unsigned int tmp_offset = htonl(payload_header->curr_offset);
	memcpy(to_write+4, &tmp_offset, 4);

	int tmp_len = htonl(len);
	memcpy(to_write+8, &tmp_len, 4);

	int bytes = 0;
	while( bytes < CMFW_HEADER_SIZE + payload_header->payload_size ){
		int res_write = write(cli_sock, to_write+bytes, \
				CMFW_HEADER_SIZE + payload_header->payload_size - bytes);
		if(res_write < 0){
			free(to_write);
			free(payload_header);
			return res_write;
		}
		else if(res_write == 0){
			cmfw_log("write failed");
			free(to_write);
			free(payload_header);
			return CMFW_E_DISCON;
		}
		bytes += res_write;
	}

	free(to_write);
	to_write = NULL;

	//__file_bit, __data_bit, __end_bit
	short max_payload_size = CMFW_PACKET_SIZE - CMFW_HEADER_SIZE;

	bytes = 0;
	while ( bytes < len ){
		if( max_payload_size < len - bytes )
			payload_header->payload_size = max_payload_size;
		else
			payload_header->payload_size = len-bytes;

		payload_header->curr_offset = bytes;
		if( payload_header->curr_offset + payload_header->payload_size \
				== len ){
			payload_header->header_info = __data_bit | __end_bit;
		}
		else
			payload_header->header_info = __data_bit;

		to_write = (unsigned char *)malloc(CMFW_HEADER_SIZE + payload_header->payload_size);
		memcpy(to_write, &(payload_header->header_id), 1);
		memcpy(to_write+1, &(payload_header->header_info), 1);
		unsigned short tmp_payload_size = htons(payload_header->payload_size);
		memcpy(to_write+2, &tmp_payload_size, 2);
		unsigned int tmp_offset = htonl(payload_header->curr_offset);
		memcpy(to_write+4, &tmp_offset, 4);

		memcpy(to_write+8, buf+bytes, payload_header->payload_size);

		int w_bytes = 0;
		while( w_bytes < CMFW_HEADER_SIZE + payload_header->payload_size ){
			int res_write = write(cli_sock, to_write+w_bytes, \
					CMFW_HEADER_SIZE + payload_header->payload_size - w_bytes);
			if(res_write <= 0){
				cmfw_log("write failed");
				free(to_write);
				free(payload_header);
				return CMFW_E_DISCON;
			}
			w_bytes += res_write;
		}

		free(to_write);
		to_write = NULL;
		bytes += payload_header->payload_size;
	}

	free(payload_header);
	return CMFW_E_NONE;
}

static char *__cmfw_token_fname(char *fname)
{
	if(fname == NULL)
		return NULL;
	int len = strlen(fname);
	if(len <= 0)
		return NULL;

	int pos;
	for(pos = len-1; pos >= 0; pos--){
		if(fname[pos] == '/')
			break;
	}

	//It's directory or 0 string
	if(pos == len-1)
		return NULL;

	//Relative path from pwd
	if(pos < 0)
		return fname;


	return &fname[pos+1];
}


int cmfw_send_file(cmfw_port_e port, char *fname)
{
	/* Wifi On */
	bool by_wfd = _wfd_on(port);
	/* ?Wifi On */

	int res = CMFW_E_NONE;
	if(by_wfd)
		cli_socks[port].wfd_sock = wfd_accept(port);
	else
		cli_socks[CMFW_RFS_PORT].bt_sock = cmfw_accept(CMFW_RFS_PORT);
	/* Send file */
	cmfw_log("Sending File .. ");

	unsigned char buf[CMFW_PACKET_SIZE];
	int res_write, res_read, fname_len, flen, bytes = 0;
	int fd_file;
	char *brief_path;
	struct timeval start, end;
	int wfd_sock = cli_socks[port].wfd_sock;
	int rfs_cli_sock = cli_socks[CMFW_RFS_PORT].bt_sock;

	if(by_wfd && wfd_sock < 0){
		return CMFW_E_DISCON;
	}
	else if((!by_wfd) && rfs_cli_sock < 0){
		return CMFW_E_DISCON;
	}
	
	brief_path = __cmfw_token_fname(fname);
	if(brief_path == NULL){
		if(by_wfd)
		_wfd_cli_close(port);
		else
			_bt_cli_close(CMFW_RFS_PORT);
		return CMFW_E_INVALID_PARAM;
	}

	gettimeofday(&start, NULL);

	//
	fd_file = open(fname, O_RDONLY);
	if( fd_file < 0){
		if(by_wfd)
			_wfd_cli_close(port);
		else
			_bt_cli_close(CMFW_RFS_PORT);
		return CMFW_E_NO_FILE;
	}

	fname_len = strlen(brief_path);
	int n_len = htonl(fname_len);
	if(by_wfd)
		res_write = write(wfd_sock, &n_len, 4);
	else
		res_write = write(rfs_cli_sock, &n_len, 4);
	if(res_write < 0){
		if(by_wfd)
			_wfd_cli_close(port);
		else
			_bt_cli_close(CMFW_RFS_PORT);
		return CMFW_E_DISCON;
	}

	if(by_wfd)
		res_write = write(wfd_sock, brief_path, fname_len);
	else
		res_write = write(rfs_cli_sock, brief_path, fname_len);
	if(res_write < 0){
		if(by_wfd)
			_wfd_cli_close(port);
		else
			_bt_cli_close(CMFW_RFS_PORT);
		return CMFW_E_DISCON;
	}


	flen = lseek(fd_file, 0, SEEK_END);
	lseek(fd_file, 0, SEEK_SET);

	int n_flen = htonl(flen);
	if(by_wfd)
		res_write = write(wfd_sock, &n_flen, 4);
	else
		res_write = write(rfs_cli_sock, &n_flen, 4);

	if(res_write < 0){
		if(by_wfd)
			_wfd_cli_close(port);
		else
			_bt_cli_close(CMFW_RFS_PORT);
		return CMFW_E_DISCON;
	}
	cmfw_log("Src File Path: %s(%s:%d)[%d]", fname, brief_path, fname_len, flen);

	int curr_progress, prev_progress = 0;
	while( bytes < flen){
		res_read = read(fd_file, buf, CMFW_PACKET_SIZE);
		if(res_read < 0){
			res = CMFW_E_NO_FILE;
			break;
		}
		if(res_read == 0){
			res = CMFW_E_NONE;
			break;
		}
		
		if(by_wfd)
			res_write = write(wfd_sock, buf, res_read);
		else
			res_write = write(rfs_cli_sock, buf, res_read);
		if(res_write < 0){
			if(by_wfd)
				_wfd_cli_close(port);
			else
				_bt_cli_close(CMFW_RFS_PORT);
			res = CMFW_E_DISCON;
			break;
		}

		bytes += res_read;
		if((curr_progress = (bytes * 10 / flen)) != prev_progress ){
			cmfw_log("File sending ...%d", (curr_progress) * 10);
			prev_progress = curr_progress;
		}

	}

	if(by_wfd)
		read(wfd_sock, buf, 1);
	else
		read(rfs_cli_sock, buf, 1);

	if(res == CMFW_E_NONE){
		cmfw_log("Finished to send()");
	}
	cmfw_log("File sending... done");

	close(fd_file);
	cmfw_log("file closed");
	if(by_wfd)
		_wfd_cli_close(port);
	else
		_bt_cli_close(CMFW_RFS_PORT);
	cmfw_log("client closed");

	return res;
}

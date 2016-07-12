#ifndef __CMFW_BASE_H__
#define __CMFW_BASE_H__

#define DEFINED_NUM_PORTS 5
#define CMFW_HEADER_SIZE 8
#define CMFW_MSG_HEADER_SIZE 4
#define CMFW_PACKET_SIZE 1008

typedef enum {
	CMFW_CONTROL_PORT = 0,
	CMFW_DEFAULT_PORT = 1,
	CMFW_RFS_PORT = 2,
	CMFW_STR_PORT = 3,
	CMFW_MAR_PORT = 4
} cmfw_port_e;

typedef struct {
	void (*on_success)(void);
	void (*on_failure)(int reason);
	int res;
} ActionListener;

typedef struct {
	void (*on_success)(int sock);
	void (*on_failure)(int reason);
	int res;
} AcceptListener;

typedef struct{
	unsigned int file_size;
	unsigned char src_file_name_len;
	char *src_file_name;
	unsigned char dest_file_name_len;
	char *dest_file_name;
} cmfw_file_header_s;

typedef struct {
	unsigned int total_data_size;
	unsigned char *data;
} cmfw_msg_header_s;

typedef struct {
	unsigned char header_id;
	unsigned char header_info;
	unsigned short payload_size;
	unsigned int curr_offset;
} cmfw_header_s;

#define __file_bit 0x80
#define __data_bit 0x40
#define __end_bit 0x20
#define __cmd_bit 0x10
#define __reserved_mask 0x0F

#define _is_file(x) (((x) & __file_bit) == __file_bit)
#define _is_msg(x) (((x) & __file_bit) == 0x00)

#define _is_data(x) (((x) & __data_bit) == __data_bit)
#define _is_header(x) (((x) & __data_bit) == 0x00)

#define _is_last(x) (((x) & __end_bit) == __end_bit)

#define _is_cmd(x) (((x) & __cmd_bit) == __cmd_bit)

#define _reserved_info(x) ((x) & __reserved_mask)

typedef struct {
	void (*on_success_msg)(char *data, int len);
	void (*on_success_file)(cmfw_file_header_s *header, unsigned int curr_offset, char *data, int len);
	void (*on_failure)(int reason);
	void *__private;
} ReadListener;
#endif //?__CMFW_BASE_H__

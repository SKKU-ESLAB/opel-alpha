#include "abtMisc.h"
#include "abtGstEncoder.h"

#define MINOR_ADDR "143.248.36.16"
#define HOPE_ADDR "143.248.39.251"

#define MINOR_PORT_JETSON2MINOR "55555"
#define HOPE_PORT_JETSON2HOPE "50000"

int abtServerListen(abtServerInfo *);
int _abtServerListen(abtServerInfo *);
int abtGstEncoderListen(abtGstEncoderInfo *, abtMediaInfo *, abtFileList *, abtServerInfo *);
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data);

int main(int argc, char *argv[]) {

	abtGstEncoderInfo gstEncInfo;

	abtMediaInfo medInfo;
	abtFileList fileList;
	//abtServerInfo hopeServInfo;
	abtServerInfo minorServInfo;

	abtFileListInitialize(&fileList);
	abtFileListAdd(&fileList);

	abtMediaInfoInitialize(&medInfo, &fileList);
	printf("1");

	abtOpenVideo(&medInfo);

	abtGstEncoderConstruct(&gstEncInfo, &medInfo);
	abtGstEncoderRun(&gstEncInfo, &medInfo);
	abtGstEncoderListen(&gstEncInfo, &medInfo, &fileList, &minorServInfo);

	//abtServerInfoInitialize(&hopeServInfo);
	abtServerInfoInitialize(&minorServInfo, MINOR_ADDR, MINOR_PORT_JETSON2MINOR);
	//abtConnectToServer(&hopeServInfo);
	abtConnectToServer(&minorServInfo);
	abtServerListen(&minorServInfo);

	printf("2");

	//abtServerListen(&hopeServInfo);

	//abtMainLoop(&msg);
	while(1) { sleep(10); }

	return 0;
}


/* ---------------- following function operates asynchronous -----------------------*/

int abtServerListen(abtServerInfo *pServInfo) {

	pthread_t tid = 0;

	pthread_create(&tid,
			NULL,
			(void * (*)(void *)) _abtServerListen,
			(void *) pServInfo);

	return 0;

}


int _abtServerListen(abtServerInfo *pServInfo) {

	uint8_t data = 0x00;

	while (1) {
		//read(pServInfo->clnt_sid, &data, sizeof(uint8_t));
		switch (data) {
			case 0x00 : // restart program
				{
					//restart_caffe();
					//data = 0x00;
					//write(pServInfo->clnt_sid, &data, sizeof(uint8_t));
				} break;
			case 0x01 : // video streaming start req
				{
					//stream_on();
					//write(pServInfo->clnt_sid, &data, sizeof(uint8_t));
					// gstreamer start;
				} break;
			case 0x02 : // video streaming stop req
				{
					//stream_off();
					//write(pServInfo->clnt_sid, &data, sizeof(uint8_t));
					// gstreamer end;
				} break;
			case 0x03 : // parameter sel/download
				{
					//parameter_download();
					//write(pServInfo->clnt_sid, &data, sizeof(uint8_t));
				} break;
			case 0x04 : // parameter update
				{
					//parameter_update();
					//write(pServInfo->clnt_sid, &data, sizeof(uint8_t));
					//client_state = 0x01;
				} break;
		}
	}
	return 0;
}

int abtGstEncoderListen(abtGstEncoderInfo *pGstEncInfo,
		abtMediaInfo *pMedInfo,
		abtFileList *pFileList,
		abtServerInfo *pServInfo
		) {

	static uint64_t data[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

	data[0] = (uint64_t) pGstEncInfo;
	data[1] = (uint64_t) pMedInfo;
	data[2] = (uint64_t) pFileList;
	data[3] = (uint64_t) pServInfo;

	// gst_bus_add_watch(pGstEncInfo->bus, bus_call, pGstEncInfo->loop);
	pGstEncInfo->bus_watch_id = gst_bus_add_watch(pGstEncInfo->bus, bus_call, (void *) data);

	return 0;

}


static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {

	abtGstEncoderInfo *pGstEncInfo = ((abtGstEncoderInfo *) ((uint64_t *) data)[0]);
	abtMediaInfo *pMedInfo = ((abtMediaInfo *) ((uint64_t *) data)[1]);
	abtFileList *pFileList = ((abtFileList *) ((uint64_t *) data)[2]);
	abtServerInfo *pServInfo = ((abtServerInfo *) ((uint64_t *) data)[3]);

	// GMainLoop *loop = (GMainLoop *) data;

	switch (GST_MESSAGE_TYPE(msg)) {

		case GST_MESSAGE_EOS :
			{
				abtGstEncoderEnd(pGstEncInfo);
				printf("1\n");
				abtGstEncoderDestroy(pGstEncInfo);
				printf("1\n");
				abtFileListAdd(pFileList);
				printf("11\n");
				abtMediaInfoInitialize(pMedInfo, pFileList);
				printf("11\n");
				abtGstEncoderConstruct(pGstEncInfo, pMedInfo);
				printf("111\n");
				abtGstEncoderRun(pGstEncInfo, pMedInfo);
				printf("111\n");
				abtGstEncoderListen(pGstEncInfo, pMedInfo, pFileList, pServInfo);
				printf("1111\n");
				write(pServInfo->clnt_sid, pFileList->pFileInfo[pFileList->current_file_number].file_name, ABT_FILE_NAME_LENGTH);
				//write(pserverinfo->clnt_sid, data, sizeof(uint8_t));
				// g_main_loop_quit(loop);
				break;
			}

		case GST_MESSAGE_ERROR :
			{
				gchar *debug;
				GError *error;

				gst_message_parse_error(msg, &error, &debug);
				g_free(debug);

				g_printerr("Error: %s\n", error->message);
				g_error_free(error);

				g_main_loop_quit(pGstEncInfo->loop);

				break;
			}

		case GST_MESSAGE_UNKNOWN :
			{
				g_print("Unknown message.\n");
				break;
			}

		default :
			{
				g_print("message_type: %d\n", GST_MESSAGE_TYPE(msg));
				break;
			}
	}

	return TRUE;
}


/* uint32_t sock = 0;

uint32_t thread_arg[2] = {0, 0};
uint32_t exit_condition = 0;

thread_arg[0] = (uint32_t) (&sock);
thread_arg[1] = (uint32_t) (&exit_condition);

pthread_t tid = 0;

pthread_create(&tid, NULL, &check_exit_condition, (void *) thread_arg);

if (tid == -1) { printf("thread creation failed\n"); exit(32); }

uint8_t deco_flag = 0;

if (exit_condition == 0x01) {
	printf("Terminating DNN\n");
	exit(0);
}
}
return 0;
} */


/* int main_loop(abtMessage *pMsg) {

	while (1) {
		switch (pMsg->jetson_state) {
			case 0x00 : // NORMAL
				{
				} break;
			case 0x01 : // ERROR
				{

					exit(0);
				} break;
			case 0x02 : // EXIT
				{
				} break;
			case 0xff :
				{
				} break;

			default :
				{
				}
		}
	}

	return 0;
}





int message_from_caffe(abtMessage *pMsg) {

	while (1) {

		switch (message) {

			case 0x0000 : // event detected
				{
					message = 0x1111;
				} break;
			case 0x0001 :
				{
					message = 0x1111;
				} break;
			case 0x0002 :
				{
					message = 0x1111;
				} break;
			case 0x0003 :
				{
					message = 0x1111;
				} break;
			default :
				{
				} break;
		}
	}

	return 0;
}



void *check_exit_condition(void *arg) {

	uint8_t data[2] = {0x00, 0x00};


	while (1) {
		printf("waiting !!!\n");

		// read(*((uint32_t *) arg), data, 2);
		read(sock, data, 2);

		if ( (data[0] == 0x70) && (data[1] == 0x70) ) {

			*((uint32_t *) (((uint32_t *) arg)[1])) = 0x01;

			printf("termination signal received");
			return NULL;
		}
		printf("no termination signal received");

		sleep(5000);
	}

}
int caffe(void *arg) {


};
int gstreamer(void *arg) {
};


*/

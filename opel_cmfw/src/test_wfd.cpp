#include <tmp_control.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(){
	char buf[256] = {0,};
	while(1){
		char cmd = getchar();
		switch(cmd){
			case 's':
				system("./bin/p2p_setup.sh start");
				break;
			case 'c':
				system("./bin/p2p_setup.sh connect");
				tmpc_get("wifi/wifi-direct/dev_addr", buf, 256);
				printf("strlen:%d\n", strlen(buf));
				break;
			case 'i':
				system("./bin/p2p_setup.sh stop");
				break;

		}
	}
}

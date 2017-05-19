#include <stdio.h>
#include <tmp_control.h>
#include <string.h>

int main()
{
	FILE *fp_file;

	fp_file = fopen("./bin/dhcpd.conf", "w+");
	if(NULL == fp_file){
		printf("file open error\n");
		return -1;
	}

	char buf[256]= {0,};
	tmpc_get("wifi/wifi-direct/iface", buf, 256);

	fprintf(fp_file, "start\t192.168.49.20\n");
	fprintf(fp_file, "end\t192.168.49.40\n");
	fprintf(fp_file, "interface\t%s\n", buf);
	printf("%d\n", strlen(buf));
	fprintf(fp_file, "max_leases\t20\n");
	fprintf(fp_file, "option\tsubnet\t255.255.255.0\n");
	fprintf(fp_file, "option\trouter\t192.168.49.1\n");
	fprintf(fp_file, "option\tlease\t864000\n");
	fprintf(fp_file, "option\tbroadcast\t192.168.49.255\n");

	return 0;
}

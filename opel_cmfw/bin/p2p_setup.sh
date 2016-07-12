#!/bin/bash

WFD_DIR=/tmp/wifi/wifi-direct/
INIT_PATH=/tmp/wifi/wifi-direct/init
WFD_STAT_PATH=/tmp/wifi/wifi-direct/wfd_stat
DHCP_STAT_PATH=/tmp/wifi/wifi-direct/dhcp_stat
IFACE_PATH=/tmp/wifi/wifi-direct/iface
DEV_ADDR_PATH=/tmp/wifi/wifi-direct/dev_addr

P2P_CONF_PATH="./bin/p2p.conf"
DHCP_CONF_PATH="./bin/dhcpd.conf"


init_wf()
{
	sudo mkdir -p ${WFD_DIR}
	sudo mkdir -p ${INIT_PATH}
	sudo mkdir -p ${WFD_STAT_PATH}
	sudo mkdir -p ${DHCP_STAT_PATH}
	sudo mkdir -p ${IFACE_PATH}
	sudo mkdir -p ${DEV_ADDR_PATH}
	
	sudo touch ${INIT_PATH}/self
	sudo chmod 666 ${INIT_PATH}/self
	sudo echo 0 > ${INIT_PATH}/self
	sudo touch ${WFD_STAT_PATH}/self
	sudo chmod 666 ${WFD_STAT_PATH}/self
	sudo touch ${IFACE_PATH}/self
	sudo chmod 666 ${IFACE_PATH}/self
	sudo touch ${DHCP_STAT_PATH}/self
	sudo chmod 666 ${DHCP_STAT_PATH}/self
	sudo touch ${DEV_ADDR_PATH}/self
	sudo chmod 666 ${DEV_ADDR_PATH}/self

	sudo echo 0 > ${WFD_STAT_PATH}/self
	sudo echo 0 > ${DHCP_STAT_PATH}/self
	sudo echo 1 > ${INIT_PATH}/self

	sudo chown -R pi:pi /tmp/wifi
	
	sudo ifconfig wlan0 up
	sudo wpa_supplicant -Dnl80211 -iwlan0 -c${P2P_CONF_PATH} -Bd
	sudo wpa_cli p2p_group_add persistent=0
	P2P_IFNAME=`ifconfig | awk '/p2p/ {print $1}'`
	echo ${P2P_IFNAME} > ${IFACE_PATH}/self
	sudo ip addr add 192.168.49.1/24 brd 192.168.49.255 dev ${P2P_IFNAME}
	sudo ifconfig ${P2P_IFNAME} 192.168.49.1 up
	DEV_ADDR=`cat ${DEV_ADDR_PATH}/self`
	
	sudo wpa_cli wps_pin any 12345670
#sudo wpa_cli p2p_set ssid_postfix ub
	sudo udhcpd ${DHCP_CONF_PATH} -f &
	echo 1 > ${INIT_PATH}/self


}
deinit_wfd()
{
	P2P_IFNAME=`ifconfig | awk '/p2p/ {print $1}'`
	sudo wpa_cli p2p_flush
	sudo wpa_cli p2p_group_remove ${P2P_IFNAME}
	sudo pkill -x udhcpd
	sudo echo 0 > ${DHCP_STAT_PATH}/self
	sudo rm /var/lib/dhcpcd5/dhcpcd-p2p-wlan0-* -f

	sudo pkill -x wpa_supplicant
	sudo ifconfig wlan0 down
	sudo echo 0 > ${WFD_STAT_PATH}/self
	sudo echo 0 > ${INIT_PATH}/self
}

start_wfd()
{
		echo Started
}

stop_wfd()
{
	echo stopped
}


case $1 in
"start")
start_wfd
;;
"stop")
stop_wfd
;;
"init")
init_wf
;;
"deinit")
deinit_wfd
;;
*)
/bin/echo $0 [init/start/stop]
exit 1
;;
esac


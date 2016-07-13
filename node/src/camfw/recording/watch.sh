#/bin/sh

GOVN_COLO="\033[31m"
WARN_COLO="\033[31;47m"
INFO_COLO="\033[36m"
INIT_COLO="\033[0m"

YOUR_UID=`id -u`
if [ $YOUR_UID -ne 0 ]
then
	echo "Root privilege is required (Your uid is $YOUR_UID)"
	exit
fi

ATTR_PNAME=$1
ATTR_PID=`pidof $ATTR_PNAME | awk '{print $1}'`
if [ -z $ATTR_PID  ]
then
	echo "Usage: $0 <pid>"
	exit
fi

ATTR_DATE=`date +%H:%M:%S.%N`
savefile="./result/$ATTR_DATE"
#tempfile="./result/tempResult.txt"
echo 1 > /sys/bus/i2c/drivers/INA231/2-0045/enable
echo 1 > /sys/bus/i2c/drivers/INA231/2-0040/enable
echo 1 > /sys/bus/i2c/drivers/INA231/2-0041/enable
echo 1 > /sys/bus/i2c/drivers/INA231/2-0044/enable

#rm $savefile 
#touch $savefile

#sleep 1

echo "Date CPULoad CPUIdle Mem Freq0 Freq1 Freq2 Freq3 Freq4 Freq5 Freq6 Freq7 BigW BigA BigV LitW LitA LitV RXPack TXPack RXByte TXByte" > $savefile

while true; do

us_a=`cat /proc/stat | head -n9 | tail -n1 | awk '{print $2}'`
ni_a=`cat /proc/stat | head -n9 | tail -n1 | awk '{print $3}'`
sy_a=`cat /proc/stat | head -n9 | tail -n1 | awk '{print $4}'`
id_a=`cat /proc/stat | head -n9 | tail -n1 | awk '{print $5}'`
sum_a=$(($us_a+$ni_a+$sy_a+$id_a));

sleep 0.5

us_b=`cat /proc/stat | head -n9 | tail -n1 | awk '{print $2}'`
ni_b=`cat /proc/stat | head -n9 | tail -n1 | awk '{print $3}'`
sy_b=`cat /proc/stat | head -n9 | tail -n1 | awk '{print $4}'`
id_b=`cat /proc/stat | head -n9 | tail -n1 | awk '{print $5}'`
sum_b=$(($us_b+$ni_b+$sy_b+$id_b));

us=$(($us_b-$us_a));
ni=$(($ni_b-$ni_a));
sy=$(($sy_b-$sy_a));
id=$(($id_b-$id_a));
sum=$(($sum_b-$sum_a));

pus=$(echo $us" "$sum | awk '{print $1*100/$2}')
pni=$(echo $ni" "$sum | awk '{print $1*100/$2}')
psy=$(echo $sy" "$sum | awk '{print $1*100/$2}')
pid=$(echo $id" "$sum | awk '{print $1*100/$2}')

psum=$(echo $pus" "$pni" "$psy | awk '{print $1+$2+$3}')

echo " "
echo "------------------------------------------------------" 
ATTR_DATE=`date +%H:%M:%S.%N`
ATTR_CPU_LOAD=$psum
ATTR_CPU_IDLE=$pid
ATTR_MEM=`ps -p$ATTR_PID -o pmem=`
ATTR_CPU0_FREQ=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq`
ATTR_CPU1_FREQ=`cat /sys/devices/system/cpu/cpu1/cpufreq/scaling_cur_freq`
ATTR_CPU2_FREQ=`cat /sys/devices/system/cpu/cpu2/cpufreq/scaling_cur_freq`
ATTR_CPU3_FREQ=`cat /sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq`
ATTR_CPU4_FREQ=`cat /sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq`
ATTR_CPU5_FREQ=`cat /sys/devices/system/cpu/cpu5/cpufreq/scaling_cur_freq`
ATTR_CPU6_FREQ=`cat /sys/devices/system/cpu/cpu6/cpufreq/scaling_cur_freq`
ATTR_CPU7_FREQ=`cat /sys/devices/system/cpu/cpu7/cpufreq/scaling_cur_freq`
ATTR_POWER_BIG_W=`cat /sys/bus/i2c/drivers/INA231/2-0040/sensor_W`
ATTR_POWER_BIG_A=`cat /sys/bus/i2c/drivers/INA231/2-0040/sensor_A`
ATTR_POWER_BIG_V=`cat /sys/bus/i2c/drivers/INA231/2-0040/sensor_V`
ATTR_POWER_LITTLE_W=`cat /sys/bus/i2c/drivers/INA231/2-0045/sensor_W`
ATTR_POWER_LITTLE_A=`cat /sys/bus/i2c/drivers/INA231/2-0045/sensor_A`
ATTR_POWER_LITTLE_V=`cat /sys/bus/i2c/drivers/INA231/2-0045/sensor_V`
ATTR_NETWORK_RX_PACKETS=`ifconfig | grep "RX" | awk '{print $2}' | head -n1 | cut -d':' -f2`
ATTR_NETWORK_TX_PACKETS=`ifconfig | grep "TX" | awk '{print $2}' | head -n1 | cut -d':' -f2`
ATTR_NETWORK_RX_BYTES=`ifconfig | grep "RX" | head -n2 | tail -n1 | awk '{print $2}' | cut -d':' -f2`
ATTR_NETWORK_TX_BYTES=`ifconfig | grep "TX" | head -n2 | tail -n1 | awk '{print $6}' | cut -d':' -f2`
echo "$ATTR_DATE" \
	     "$ATTR_CPU_LOAD" \
	     "$ATTR_CPU_IDLE" \
	     "$ATTR_MEM" \
	     "$ATTR_CPU0_FREQ" \
	     "$ATTR_CPU1_FREQ" \
	     "$ATTR_CPU2_FREQ" \
	     "$ATTR_CPU3_FREQ" \
	     "$ATTR_CPU4_FREQ" \
	     "$ATTR_CPU5_FREQ" \
	     "$ATTR_CPU6_FREQ" \
	     "$ATTR_CPU7_FREQ" \
	     "$ATTR_POWER_BIG_W" \
	     "$ATTR_POWER_BIG_A" \
	     "$ATTR_POWER_BIG_V" \
	     "$ATTR_POWER_LITTLE_W" \
	     "$ATTR_POWER_LITTLE_A" \
	     "$ATTR_POWER_LITTLE_V" \
	     "$ATTR_NETWORK_RX_PACKETS" \
	     "$ATTR_NETWORK_TX_PACKETS" \
	     "$ATTR_NETWORK_RX_BYTES" \
	     "$ATTR_NETWORK_TX_BYTES" \
	     >> $savefile

echo "$ATTR_DATE" "${INFO_COLO}(ns)${INIT_COLO}"
echo "${GOVN_COLO}Governor:${INIT_COLO}" `cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
echo "${WARN_COLO}[Load]${INIT_COLO}"
echo "${INFO_COLO}CPU:${INIT_COLO} (Load) " $ATTR_CPU_LOAD "%, (Idle) " $ATTR_CPU_IDLE "%" \
"${INFO_COLO}Memory${INIT_COLO}:	" $ATTR_MEM "%"
echo "${WARN_COLO}[Frequency(Hz)]${INIT_COLO}" 
echo "${INFO_COLO}Core 0	Core 1	Core 2	Core 3	Core 4	Core 5	Core 6	Core 7${INIT_COLO}"
echo \
	     "$ATTR_CPU0_FREQ" \
	     "$ATTR_CPU1_FREQ" \
	     "$ATTR_CPU2_FREQ" \
	     "$ATTR_CPU3_FREQ" \
	     "$ATTR_CPU4_FREQ" \
	     "$ATTR_CPU5_FREQ" \
	     "$ATTR_CPU6_FREQ" \
	     "$ATTR_CPU7_FREQ"
echo " "
echo "${WARN_COLO}[Network]${INIT_COLO}"
echo "${INFO_COLO}RX:${INIT_COLO} " $ATTR_NETWORK_RX_PACKETS " packets (" $ATTR_NETWORK_RX_BYTES " bytes)" 
echo "${INFO_COLO}TX:${INIT_COLO} " $ATTR_NETWORK_TX_PACKETS " packets (" $ATTR_NETWORK_TX_BYTES " bytes)"
echo " "
echo "${WARN_COLO}[Power]${INIT_COLO}"
echo "${INFO_COLO}Big-core:	${INIT_COLO}" \
$ATTR_POWER_BIG_W "W" \
"(" $ATTR_POWER_BIG_A "A"  \
$ATTR_POWER_BIG_V "V" ")"

echo "${INFO_COLO}Little-core:	${INIT_COLO}" \
$ATTR_POWER_LITTLE_W "W" \
"(" $ATTR_POWER_LITTLE_A "A"  \
$ATTR_POWER_LITTLE_V "V" ")"

echo "${INFO_COLO}GPU:		${INIT_COLO}" \
`cat /sys/bus/i2c/drivers/INA231/2-0044/sensor_W` "W" \
"(" `cat /sys/bus/i2c/drivers/INA231/2-0044/sensor_A` "A"  \
`cat /sys/bus/i2c/drivers/INA231/2-0044/sensor_V` "V" ")"

echo "${INFO_COLO}DRAM:		${INIT_COLO}" \
`cat /sys/bus/i2c/drivers/INA231/2-0041/sensor_W` "W" \
"(" `cat /sys/bus/i2c/drivers/INA231/2-0041/sensor_A` "A"  \
`cat /sys/bus/i2c/drivers/INA231/2-0041/sensor_V` "V" ")"

echo "${WARN_COLO}[Thermal]${INIT_COLO}"
echo `cat /sys/devices/10060000.tmu/temp` 


#cat /proc/stat | grep cpu >> $tempfile

#echo "do"
sleep 0.1 
done

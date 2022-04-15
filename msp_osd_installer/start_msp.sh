#!/system/bin/sh
date >/blackbox/start_msp.log
echo "Starting msp_displayport_mux ..." >> /blackbox/start_msp.log
setprop dji.hdvt_uav_service 0
mv /dev/ttyS1 /dev/ttyS1_moved
nohup /blackbox/msp_displayport_mux 192.168.41.2 /dev/ttyS1_moved /dev/ttyS1 &

ls -l /dev/ttyS1_moved >> /blackbox/start_msp.log
ps | grep msp >> /blackbox/start_msp.log
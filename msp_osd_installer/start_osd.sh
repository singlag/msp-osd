#!/system/bin/sh
date >/blackbox/start_osd.log
echo "Starting osd_dji ..." >> /blackbox/start_osd.log
nohup /blackbox/osd_dji &

ps | grep osd_dji >> /blackbox/start_osd.log
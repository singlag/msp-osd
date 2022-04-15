@echo off
echo You can continue if your DJI device connected and ADB enabled
echo Press enter to install MSP OSD to V1 Goggles...
Pause

@echo on
adb push osd_dji /blackbox
adb shell chmod 755 /blackbox/osd_dji

adb push font.bin /blackbox

adb push start_osd.sh /blackbox
adb shell chmod 755 /blackbox/start_osd.sh

adb shell cp -p /system/bin/dji_setup_uart.sh /blackbox/dji_setup_uart.sh.org

adb shell echo "'sleep 15' >> /system/bin/dji_setup_uart.sh"
adb shell echo "'./blackbox/start_osd.sh &' >> /system/bin/dji_setup_uart.sh"


@echo off

pause
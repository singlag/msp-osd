@echo off
echo You can continue if your DJI device connected and ADB enabled
echo Press enter to install MSP OSD to Airunit/Vista...
Pause

@echo on
adb push msp_displayport_mux /blackbox
adb shell chmod 755 /blackbox/msp_displayport_mux

adb push start_msp.sh /blackbox
adb shell chmod 755 /blackbox/start_msp.sh

adb shell cp -p /system/bin/dji_setup_uart.sh /blackbox/dji_setup_uart.sh.org

adb shell echo "'sleep 15' >> /system/bin/dji_setup_uart.sh"
adb shell echo "'./blackbox/start_msp.sh &' >> /system/bin/dji_setup_uart.sh"


@echo off

pause
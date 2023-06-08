# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\Users\mafassi\Desktop\UART_LVDS_Vitis2\Software_system\_ide\scripts\debugger_software-default.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\Users\mafassi\Desktop\UART_LVDS_Vitis2\Software_system\_ide\scripts\debugger_software-default.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent Nexys Video 210276B06393B" && level==0 && jtag_device_ctx=="jsn-Nexys Video-210276B06393B-13636093-0"}
fpga -file C:/Users/mafassi/Desktop/UART_LVDS_Vitis2/Software/_ide/bitstream/Microblaze_LVDS.bit
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
loadhw -hw C:/Users/mafassi/Desktop/UART_LVDS_Vitis/LVDS_UART_PLATFORM/export/LVDS_UART_PLATFORM/hw/Microblaze_LVDS.xsa -regs
configparams mdm-detect-bscan-mask 2
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
rst -system
after 3000
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
dow C:/Users/mafassi/Desktop/UART_LVDS_Vitis2/Software/Debug/Software.elf
bpadd -addr &main

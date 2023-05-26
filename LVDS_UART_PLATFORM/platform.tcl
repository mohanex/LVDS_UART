# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\mafassi\Desktop\UART_LVDS_Vitis\LVDS_UART_PLATFORM\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\mafassi\Desktop\UART_LVDS_Vitis\LVDS_UART_PLATFORM\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {LVDS_UART_PLATFORM}\
-hw {C:\Users\mafassi\Downloads\carte_blowbox\blowbox\Microblaze5\Microblaze_LVDS.xsa}\
-proc {microblaze_0} -os {standalone} -fsbl-target {psu_cortexa53_0} -out {C:/Users/mafassi/Desktop/UART_LVDS_Vitis}

platform write
platform generate -domains 
platform active {LVDS_UART_PLATFORM}
platform generate
platform write
platform clean
platform active {LVDS_UART_PLATFORM}
platform config -updatehw {C:/Users/mafassi/Downloads/carte_blowbox/blowbox/Microblaze5/design_1_wrapper_V1.xsa}
platform generate
platform write
platform active {LVDS_UART_PLATFORM}
platform generate -domains 

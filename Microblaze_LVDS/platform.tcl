# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\mafassi\Desktop\UART_LVDS_Vitis\Microblaze_LVDS\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\mafassi\Desktop\UART_LVDS_Vitis\Microblaze_LVDS\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {Microblaze_LVDS}\
-hw {C:\Users\mafassi\Downloads\carte_blowbox\blowbox\Microblaze5\Microblaze_LVDS.xsa}\
-fsbl-target {psu_cortexa53_0} -out {C:/Users/mafassi/Desktop/UART_LVDS_Vitis}

platform write
domain create -name {standalone_microblaze_0} -display-name {standalone_microblaze_0} -os {standalone} -proc {microblaze_0} -runtime {cpp} -arch {32-bit} -support-app {hello_world}
platform generate -domains 
platform active {Microblaze_LVDS}
platform generate -quick
platform generate
platform clean
platform active {Microblaze_LVDS}
platform config -updatehw {C:/Users/mafassi/Downloads/carte_blowbox/blowbox/Microblaze5/design_1_wrapper_V1.xsa}
platform generate

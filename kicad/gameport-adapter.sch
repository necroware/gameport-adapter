EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "GamePort Adapter"
Date "2020-11-19"
Rev "0.1"
Comp "Necroware"
Comment1 "by Scorp"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector:DB15_Female CONN1
U 1 1 5FB67AC2
P 2050 2150
F 0 "CONN1" H 2204 2196 50  0000 L CNN
F 1 "GamePort" H 2204 2105 50  0000 L CNN
F 2 "Connector_Dsub:DSUB-15_Female_Horizontal_P2.77x2.84mm_EdgePinOffset7.70mm_Housed_MountingHolesOffset9.12mm" H 2050 2150 50  0001 C CNN
F 3 " ~" H 2050 2150 50  0001 C CNN
F 4 "C77835" H 2050 2150 50  0001 C CNN "LCSC Part"
	1    2050 2150
	1    0    0    -1  
$EndComp
Text GLabel 1750 1450 0    50   Input ~ 0
VCC
Text GLabel 1750 2850 0    50   Input ~ 0
VCC
Text GLabel 1750 1550 0    50   Input ~ 0
VCC
Text GLabel 1750 2750 0    50   Input ~ 0
MIDI_RXD
Text GLabel 1750 2050 0    50   Input ~ 0
GND
Text GLabel 1750 2150 0    50   Input ~ 0
MIDI_TXD
Text GLabel 1750 2250 0    50   Input ~ 0
GND
Text GLabel 1750 1650 0    50   Input ~ 0
Button_1
Text GLabel 1750 2650 0    50   Input ~ 0
Button_2
Text GLabel 1750 1750 0    50   Input ~ 0
Button_3
Text GLabel 1750 2550 0    50   Input ~ 0
Button_4
Text GLabel 1750 1850 0    50   Input ~ 0
AxisX_1
Text GLabel 1750 1950 0    50   Input ~ 0
AxisX_2
Text GLabel 1750 2450 0    50   Input ~ 0
AxisY_1
Text GLabel 1750 2350 0    50   Input ~ 0
AxisY_2
Text GLabel 4200 2700 2    50   Input ~ 0
Button_1
Text GLabel 4200 2600 2    50   Input ~ 0
Button_3
Text GLabel 3100 2600 0    50   Input ~ 0
Button_2
Text GLabel 3100 2500 0    50   Input ~ 0
Button_4
Text GLabel 4200 1900 2    50   Input ~ 0
VCC
Text GLabel 4200 2100 2    50   Input ~ 0
MIDI_TXD
Text GLabel 3100 2700 0    50   Input ~ 0
MIDI_RXD
Text GLabel 4200 2300 2    50   Input ~ 0
AxisX_1
Text GLabel 4200 2200 2    50   Input ~ 0
AxisX_2
Text GLabel 3100 2200 0    50   Input ~ 0
AxisY_2
Text GLabel 3100 2400 0    50   Input ~ 0
AxisY_1
Text GLabel 4200 1700 2    50   Input ~ 0
GND
Text GLabel 3100 1800 0    50   Input ~ 0
GND
Text GLabel 3100 1900 0    50   Input ~ 0
GND
NoConn ~ 4200 1800
NoConn ~ 4200 1600
NoConn ~ 3100 1600
NoConn ~ 3100 1700
$Comp
L Switch:SW_DIP_x03 SW1
U 1 1 5FF7E601
P 3650 3350
F 0 "SW1" H 3650 3817 50  0000 C CNN
F 1 "SW_DIP_x03" H 3650 3726 50  0000 C CNN
F 2 "Button_Switch_THT:SW_DIP_SPSTx03_Slide_9.78x9.8mm_W7.62mm_P2.54mm" H 3650 3350 50  0001 C CNN
F 3 "~" H 3650 3350 50  0001 C CNN
	1    3650 3350
	1    0    0    -1  
$EndComp
Text GLabel 3950 3150 2    50   Input ~ 0
GND
Text GLabel 3950 3250 2    50   Input ~ 0
GND
Text GLabel 3950 3350 2    50   Input ~ 0
GND
$Comp
L Necroware:Arduino_Pro_Micro U1
U 1 1 5FF74419
P 3100 1400
F 0 "U1" H 3650 1465 50  0000 C CNN
F 1 "Arduino_Pro_Micro" H 3650 1374 50  0000 C CNN
F 2 "Package_DIP:DIP-24_W15.24mm_Socket" H 3100 1400 50  0001 C CNN
F 3 "" H 3100 1400 50  0001 C CNN
	1    3100 1400
	1    0    0    -1  
$EndComp
Text GLabel 3350 3150 0    50   Input ~ 0
SW1
Text GLabel 3350 3250 0    50   Input ~ 0
SW2
Text GLabel 3350 3350 0    50   Input ~ 0
SW3
NoConn ~ 3100 2000
NoConn ~ 3100 2100
NoConn ~ 3100 2300
Text GLabel 4200 2000 2    50   Input ~ 0
SW3
Text GLabel 4200 2400 2    50   Input ~ 0
SW2
Text GLabel 4200 2500 2    50   Input ~ 0
SW1
$EndSCHEMATC

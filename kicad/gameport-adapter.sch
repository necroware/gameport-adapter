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
$Comp
L MCU_Module:Arduino_Nano_Every A1
U 1 1 5FB6A14E
P 3500 2100
F 0 "A1" H 3200 3100 50  0000 C CNN
F 1 "Arduino Nano" V 3500 2150 50  0000 C CNN
F 2 "Module:Arduino_Nano" H 3500 2100 50  0001 C CIN
F 3 "https://content.arduino.cc/assets/NANOEveryV3.0_sch.pdf" H 3500 2100 50  0001 C CNN
	1    3500 2100
	1    0    0    -1  
$EndComp
Text GLabel 3000 1600 0    50   Input ~ 0
Button_1
Text GLabel 3000 1700 0    50   Input ~ 0
Button_3
Text GLabel 3000 2000 0    50   Input ~ 0
Button_2
Text GLabel 3000 1900 0    50   Input ~ 0
Button_4
Text GLabel 3500 3100 3    50   Input ~ 0
GND
Text GLabel 4150 900  0    50   Input ~ 0
VCC
Text GLabel 4300 1900 2    50   Input ~ 0
GND
Text GLabel 3000 1800 0    50   Input ~ 0
MIDI_TXD
Text GLabel 3000 2100 0    50   Input ~ 0
MIDI_RXD
Text GLabel 4000 2100 2    50   Input ~ 0
AxisX_1
Text GLabel 4000 2200 2    50   Input ~ 0
AxisX_2
Text GLabel 4000 2300 2    50   Input ~ 0
AxisY_2
Text GLabel 4000 2400 2    50   Input ~ 0
AxisY_1
Wire Wire Line
	3700 1100 3700 1000
Wire Wire Line
	3600 1100 3600 800 
$Comp
L Device:CP C1
U 1 1 5FB812A2
P 4150 1900
F 0 "C1" V 4405 1900 50  0000 C CNN
F 1 "10u" V 4314 1900 50  0000 C CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.00mm" H 4188 1750 50  0001 C CNN
F 3 "~" H 4150 1900 50  0001 C CNN
	1    4150 1900
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R1
U 1 1 5FB853B7
P 4300 900
F 0 "R1" V 4200 850 50  0000 C CNN
F 1 "100" V 4300 900 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4230 900 50  0001 C CNN
F 3 "~" H 4300 900 50  0001 C CNN
	1    4300 900 
	0    1    1    0   
$EndComp
Wire Wire Line
	3600 800  4450 800 
Wire Wire Line
	3700 1000 4450 1000
$Comp
L Connector_Generic:Conn_01x03 JP1
U 1 1 5FB850AD
P 4650 900
F 0 "JP1" H 4800 850 50  0000 C CNN
F 1 "Conn_01x03" H 4950 950 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 4650 900 50  0001 C CNN
F 3 "~" H 4650 900 50  0001 C CNN
	1    4650 900 
	1    0    0    1   
$EndComp
$EndSCHEMATC

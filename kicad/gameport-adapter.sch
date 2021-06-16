EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "GamePort Adapter"
Date "2021-06-03"
Rev "0.3"
Comp "Necroware"
Comment1 "by Scorp"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 5150 4500 0    50   Input ~ 0
Button_2
Text GLabel 5150 4400 0    50   Input ~ 0
Button_4
Text GLabel 6250 3800 2    50   Input ~ 0
VCC
Text GLabel 5150 4200 0    50   Input ~ 0
MIDI_TXD
Text GLabel 5150 4600 0    50   Input ~ 0
MIDI_RXD
Text GLabel 5150 4100 0    50   Input ~ 0
AxisY_2
Text GLabel 5150 4300 0    50   Input ~ 0
AxisY_1
NoConn ~ 6250 3500
NoConn ~ 5150 3500
NoConn ~ 5150 3600
NoConn ~ 5150 4000
$Comp
L Connector:DB15_Female_MountingHoles CONN1
U 1 1 5FB67AC2
P 4100 4050
F 0 "CONN1" H 4254 4096 50  0000 L CNN
F 1 "GamePort" H 4254 4005 50  0000 L CNN
F 2 "Connector_Dsub:DSUB-15_Female_Horizontal_P2.77x2.84mm_EdgePinOffset7.70mm_Housed_MountingHolesOffset9.12mm" H 4100 4050 50  0001 C CNN
F 3 " ~" H 4100 4050 50  0001 C CNN
F 4 "C77835" H 4100 4050 50  0001 C CNN "LCSC"
	1    4100 4050
	1    0    0    -1  
$EndComp
Text GLabel 3800 4250 0    50   Input ~ 0
AxisY_2
Text GLabel 3800 4350 0    50   Input ~ 0
AxisY_1
Text GLabel 3800 3850 0    50   Input ~ 0
AxisX_2
Text GLabel 3800 3750 0    50   Input ~ 0
AxisX_1
Text GLabel 3800 4450 0    50   Input ~ 0
Button_4
Text GLabel 3800 3650 0    50   Input ~ 0
Button_3
Text GLabel 3800 4550 0    50   Input ~ 0
Button_2
Text GLabel 3800 3550 0    50   Input ~ 0
Button_1
Text GLabel 3800 4050 0    50   Input ~ 0
MIDI_TXD
Text GLabel 3800 4650 0    50   Input ~ 0
MIDI_RXD
Text GLabel 3800 3450 0    50   Input ~ 0
VCC
Text GLabel 3800 4750 0    50   Input ~ 0
VCC
Text GLabel 3800 3350 0    50   Input ~ 0
VCC
Text GLabel 3800 4150 0    50   Input ~ 0
GND
Text GLabel 3800 3950 0    50   Input ~ 0
GND
Text GLabel 6250 3600 2    50   Input ~ 0
GND
Text GLabel 5150 3800 0    50   Input ~ 0
GND
Text GLabel 5150 3700 0    50   Input ~ 0
GND
Text GLabel 5400 5250 0    50   Input ~ 0
SW3
Text GLabel 5400 5150 0    50   Input ~ 0
SW2
Text GLabel 5400 5050 0    50   Input ~ 0
SW1
Text GLabel 4100 4950 3    50   Input ~ 0
GND
$Comp
L Device:R R1
U 1 1 604BCFE5
P 7900 3400
F 0 "R1" V 8000 3450 50  0000 R CNN
F 1 "100k" V 7900 3500 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 7830 3400 50  0001 C CNN
F 3 "~" H 7900 3400 50  0001 C CNN
F 4 "C172965" V 7900 3400 50  0001 C CNN "LCSC"
	1    7900 3400
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R2
U 1 1 604BE4D6
P 7900 3600
F 0 "R2" V 7800 3650 50  0000 R CNN
F 1 "100k" V 7900 3700 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 7830 3600 50  0001 C CNN
F 3 "~" H 7900 3600 50  0001 C CNN
F 4 "C172965" V 7900 3600 50  0001 C CNN "LCSC"
	1    7900 3600
	0    1    1    0   
$EndComp
Text GLabel 8050 3600 2    50   Input ~ 0
GND
Text GLabel 8050 3400 2    50   Input ~ 0
GND
Text GLabel 7750 3400 0    50   Input ~ 0
AxisX_2
Text GLabel 7750 3600 0    50   Input ~ 0
AxisX_1
$Comp
L Device:R R3
U 1 1 604C20F3
P 7900 3800
F 0 "R3" V 8000 3850 50  0000 R CNN
F 1 "100k" V 7900 3900 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 7830 3800 50  0001 C CNN
F 3 "~" H 7900 3800 50  0001 C CNN
F 4 "C172965" V 7900 3800 50  0001 C CNN "LCSC"
	1    7900 3800
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R4
U 1 1 604C282A
P 7900 4000
F 0 "R4" V 7800 4050 50  0000 R CNN
F 1 "100k" V 7900 4100 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 7830 4000 50  0001 C CNN
F 3 "~" H 7900 4000 50  0001 C CNN
F 4 "C172965" V 7900 4000 50  0001 C CNN "LCSC"
	1    7900 4000
	0    1    1    0   
$EndComp
Text GLabel 8050 3800 2    50   Input ~ 0
GND
Text GLabel 8050 4000 2    50   Input ~ 0
GND
Text GLabel 7750 3800 0    50   Input ~ 0
AxisY_1
Text GLabel 7750 4000 0    50   Input ~ 0
AxisY_2
Text GLabel 5400 5350 0    50   Input ~ 0
SW4
Text GLabel 6250 3900 2    50   Input ~ 0
SW4
Text GLabel 6250 4000 2    50   Input ~ 0
SW3
Text GLabel 6250 4600 2    50   Input ~ 0
Button_1
Text GLabel 6250 4500 2    50   Input ~ 0
Button_3
Text GLabel 6250 4400 2    50   Input ~ 0
SW1
Text GLabel 6250 4300 2    50   Input ~ 0
SW2
Text GLabel 6250 4200 2    50   Input ~ 0
AxisX_1
Text GLabel 6250 4100 2    50   Input ~ 0
AxisX_2
NoConn ~ 5150 3900
Text GLabel 6000 5350 2    50   Input ~ 0
GND
Text GLabel 6000 5050 2    50   Input ~ 0
GND
Text GLabel 6000 5150 2    50   Input ~ 0
GND
Text GLabel 6000 5250 2    50   Input ~ 0
GND
$Comp
L Switch:SW_DIP_x04 SW1
U 1 1 5FF7E601
P 5700 5250
F 0 "SW1" H 5700 5717 50  0000 C CNN
F 1 "SW_DIP_x04" H 5700 5626 50  0000 C CNN
F 2 "Button_Switch_THT:SW_DIP_SPSTx04_Slide_9.78x12.34mm_W7.62mm_P2.54mm" H 5700 5250 50  0001 C CNN
F 3 "~" H 5700 5250 50  0001 C CNN
F 4 "C15781" H 5700 5250 50  0001 C CNN "LCSC"
	1    5700 5250
	-1   0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW2
U 1 1 60B7F645
P 6700 3700
F 0 "SW2" H 6700 3985 50  0000 C CNN
F 1 "Reset" H 6700 3894 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm" H 6700 3900 50  0001 C CNN
F 3 "~" H 6700 3900 50  0001 C CNN
	1    6700 3700
	1    0    0    -1  
$EndComp
$Comp
L Necroware:Arduino_Pro_Micro U1
U 1 1 5FF74419
P 5150 3300
F 0 "U1" H 5700 3365 50  0000 C CNN
F 1 "Arduino_Pro_Micro" H 5700 3274 50  0000 C CNN
F 2 "Necroware:Arduino_Micro_Wide" H 5150 3300 50  0001 C CNN
F 3 "" H 5150 3300 50  0001 C CNN
F 4 "C72120" H 5150 3300 50  0001 C CNN "LCSC"
	1    5150 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 3700 6500 3700
Text GLabel 6900 3700 2    50   Input ~ 0
GND
$EndSCHEMATC

EESchema Schematic File Version 4
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "GamePort Adapter"
Date "2021-03-12"
Rev "0.2"
Comp "Necroware"
Comment1 "by Scorp"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 4950 3450 0    50   Input ~ 0
Button_2
Text GLabel 4950 3350 0    50   Input ~ 0
Button_4
Text GLabel 6050 2750 2    50   Input ~ 0
VCC
Text GLabel 4950 3150 0    50   Input ~ 0
MIDI_TXD
Text GLabel 4950 3550 0    50   Input ~ 0
MIDI_RXD
Text GLabel 4950 3050 0    50   Input ~ 0
AxisY_2
Text GLabel 4950 3250 0    50   Input ~ 0
AxisY_1
NoConn ~ 6050 2450
NoConn ~ 4950 2450
NoConn ~ 4950 2550
$Comp
L Necroware:Arduino_Pro_Micro U1
U 1 1 5FF74419
P 4950 2250
F 0 "U1" H 5500 2315 50  0000 C CNN
F 1 "Arduino_Pro_Micro" H 5500 2224 50  0000 C CNN
F 2 "Package_DIP:DIP-24_W15.24mm_Socket" H 4950 2250 50  0001 C CNN
F 3 "" H 4950 2250 50  0001 C CNN
F 4 "C72120" H 4950 2250 50  0001 C CNN "LCSC"
	1    4950 2250
	1    0    0    -1  
$EndComp
$Comp
L Connector:DB15_Female_MountingHoles CONN1
U 1 1 5FB67AC2
P 3450 3050
F 0 "CONN1" H 3604 3096 50  0000 L CNN
F 1 "GamePort" H 3604 3005 50  0000 L CNN
F 2 "Connector_Dsub:DSUB-15_Female_Horizontal_P2.77x2.84mm_EdgePinOffset7.70mm_Housed_MountingHolesOffset9.12mm" H 3450 3050 50  0001 C CNN
F 3 " ~" H 3450 3050 50  0001 C CNN
F 4 "C77835" H 3450 3050 50  0001 C CNN "LCSC"
	1    3450 3050
	1    0    0    -1  
$EndComp
Text GLabel 3150 3250 0    50   Input ~ 0
AxisY_2
Text GLabel 3150 3350 0    50   Input ~ 0
AxisY_1
Text GLabel 3150 2850 0    50   Input ~ 0
AxisX_2
Text GLabel 3150 2750 0    50   Input ~ 0
AxisX_1
Text GLabel 3150 3450 0    50   Input ~ 0
Button_4
Text GLabel 3150 2650 0    50   Input ~ 0
Button_3
Text GLabel 3150 3550 0    50   Input ~ 0
Button_2
Text GLabel 3150 2550 0    50   Input ~ 0
Button_1
Text GLabel 3150 3050 0    50   Input ~ 0
MIDI_TXD
Text GLabel 3150 3650 0    50   Input ~ 0
MIDI_RXD
Text GLabel 1900 3200 0    50   Input ~ 0
SW_Vcc
Text GLabel 1850 2350 0    50   Input ~ 0
VCC
Text GLabel 3150 3150 0    50   Input ~ 0
GND
Text GLabel 3150 2950 0    50   Input ~ 0
GND
Text GLabel 6050 2550 2    50   Input ~ 0
GND
Text GLabel 4950 2750 0    50   Input ~ 0
GND
Text GLabel 5200 4200 0    50   Input ~ 0
SW3
Text GLabel 5200 4100 0    50   Input ~ 0
SW2
Text GLabel 5200 4000 0    50   Input ~ 0
SW1
Text GLabel 3450 3950 3    50   Input ~ 0
GND
$Comp
L Device:R R1
U 1 1 604BCFE5
P 8200 2550
F 0 "R1" V 8300 2600 50  0000 R CNN
F 1 "100k" V 8200 2650 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 8130 2550 50  0001 C CNN
F 3 "~" H 8200 2550 50  0001 C CNN
F 4 "C172965" V 8200 2550 50  0001 C CNN "LCSC"
	1    8200 2550
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R2
U 1 1 604BE4D6
P 8200 2750
F 0 "R2" V 8100 2800 50  0000 R CNN
F 1 "100k" V 8200 2850 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 8130 2750 50  0001 C CNN
F 3 "~" H 8200 2750 50  0001 C CNN
F 4 "C172965" V 8200 2750 50  0001 C CNN "LCSC"
	1    8200 2750
	0    1    1    0   
$EndComp
Text GLabel 8350 2750 2    50   Input ~ 0
GND
Text GLabel 8350 2550 2    50   Input ~ 0
GND
Text GLabel 8050 2550 0    50   Input ~ 0
AxisX_2
Text GLabel 8050 2750 0    50   Input ~ 0
AxisX_1
$Comp
L Device:R R3
U 1 1 604C20F3
P 8200 2950
F 0 "R3" V 8300 3000 50  0000 R CNN
F 1 "100k" V 8200 3050 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 8130 2950 50  0001 C CNN
F 3 "~" H 8200 2950 50  0001 C CNN
F 4 "C172965" V 8200 2950 50  0001 C CNN "LCSC"
	1    8200 2950
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R4
U 1 1 604C282A
P 8200 3150
F 0 "R4" V 8100 3200 50  0000 R CNN
F 1 "100k" V 8200 3250 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 8130 3150 50  0001 C CNN
F 3 "~" H 8200 3150 50  0001 C CNN
F 4 "C172965" V 8200 3150 50  0001 C CNN "LCSC"
	1    8200 3150
	0    1    1    0   
$EndComp
Text GLabel 8350 2950 2    50   Input ~ 0
GND
Text GLabel 8350 3150 2    50   Input ~ 0
GND
Text GLabel 8050 2950 0    50   Input ~ 0
AxisY_1
Text GLabel 8050 3150 0    50   Input ~ 0
AxisY_2
Text GLabel 5200 4300 0    50   Input ~ 0
SW4
Text GLabel 6050 2850 2    50   Input ~ 0
SW4
Text GLabel 6050 2950 2    50   Input ~ 0
SW3
Text GLabel 6050 3550 2    50   Input ~ 0
Button_1
Text GLabel 6050 3450 2    50   Input ~ 0
Button_3
Text GLabel 6050 3350 2    50   Input ~ 0
SW1
Text GLabel 6050 3250 2    50   Input ~ 0
SW2
Text GLabel 6050 3150 2    50   Input ~ 0
AxisX_1
Text GLabel 6050 3050 2    50   Input ~ 0
AxisX_2
NoConn ~ 4950 2850
Text GLabel 5800 4300 2    50   Input ~ 0
GND
Text GLabel 5800 4000 2    50   Input ~ 0
GND
Text GLabel 5800 4100 2    50   Input ~ 0
GND
Text GLabel 5800 4200 2    50   Input ~ 0
GND
$Comp
L Switch:SW_DIP_x04 SW1
U 1 1 5FF7E601
P 5500 4200
F 0 "SW1" H 5500 4667 50  0000 C CNN
F 1 "SW_DIP_x04" H 5500 4576 50  0000 C CNN
F 2 "Button_Switch_THT:SW_DIP_SPSTx04_Slide_9.78x12.34mm_W7.62mm_P2.54mm" H 5500 4200 50  0001 C CNN
F 3 "~" H 5500 4200 50  0001 C CNN
F 4 "C15781" H 5500 4200 50  0001 C CNN "LCSC"
	1    5500 4200
	-1   0    0    -1  
$EndComp
$Comp
L Transistor_BJT:BC560 Q1
U 1 1 61EFBFC8
P 2350 2450
F 0 "Q1" V 2678 2450 50  0000 C CNN
F 1 "BC560" V 2587 2450 50  0000 C CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 2550 2375 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC557.pdf" H 2350 2450 50  0001 L CNN
	1    2350 2450
	0    1    -1   0   
$EndComp
Wire Wire Line
	1850 2350 2000 2350
Wire Wire Line
	2000 2350 2000 2400
Wire Wire Line
	2000 2350 2150 2350
Connection ~ 2000 2350
Wire Wire Line
	2550 2350 2650 2350
Wire Wire Line
	3150 2450 2650 2450
Wire Wire Line
	2650 2450 2650 2350
Connection ~ 2650 2350
Wire Wire Line
	2650 2350 3150 2350
Wire Wire Line
	3150 3750 2650 3750
Wire Wire Line
	2650 3750 2650 2450
Connection ~ 2650 2450
Wire Wire Line
	2000 2700 2000 2750
Wire Wire Line
	2000 2750 2350 2750
Wire Wire Line
	2350 2750 2350 2650
Connection ~ 2000 2750
Wire Wire Line
	2000 2750 2000 2800
Wire Wire Line
	2000 3100 2000 3200
Wire Wire Line
	2000 3200 1900 3200
Text GLabel 4950 2950 0    50   Input ~ 0
SW_Vcc
$Comp
L Device:R R5
U 1 1 61F10CE5
P 2000 2550
F 0 "R5" H 1900 2600 50  0000 R CNN
F 1 "10K" V 2000 2650 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 1930 2550 50  0001 C CNN
F 3 "~" H 2000 2550 50  0001 C CNN
F 4 "C172965" V 2000 2550 50  0001 C CNN "LCSC"
	1    2000 2550
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 61F116C0
P 2000 2950
F 0 "R6" H 1900 3000 50  0000 R CNN
F 1 "1K" V 2000 3000 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 1930 2950 50  0001 C CNN
F 3 "~" H 2000 2950 50  0001 C CNN
F 4 "C172965" V 2000 2950 50  0001 C CNN "LCSC"
	1    2000 2950
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW2
U 1 1 61F14188
P 6700 2650
F 0 "SW2" H 6700 2935 50  0000 C CNN
F 1 "SW_Push" H 6700 2844 50  0000 C CNN
F 2 "Button_Switch_THT:SW_Tactile_SPST_Angled_PTS645Vx58-2LFS" H 6700 2850 50  0001 C CNN
F 3 "~" H 6700 2850 50  0001 C CNN
	1    6700 2650
	-1   0    0    -1  
$EndComp
Text GLabel 7150 2650 2    50   Input ~ 0
GND
Wire Wire Line
	6050 2650 6500 2650
Wire Wire Line
	6900 2650 7150 2650
Text Label 2750 2350 0    50   ~ 0
VCC_JOY
NoConn ~ 4950 2650
$EndSCHEMATC

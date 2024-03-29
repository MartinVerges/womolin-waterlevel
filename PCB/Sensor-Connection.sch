EESchema Schematic File Version 4
LIBS:Caravansensor-cache
EELAYER 29 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 2 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Notes 1300 1150 0    50   ~ 0
CBE Voltage Probes
Text Notes 1400 4550 0    50   ~ 0
CBE Wire Probes
Text Notes 4550 1150 0    50   ~ 0
Schaudt Wire Probes
Text Notes 6950 1200 0    50   ~ 0
Votronic Voltage Probes
Text Notes 4500 4150 0    50   ~ 0
Nordelettronica Wire Probe
$Comp
L Connector_Generic:Conn_01x04 JP7-JP1
U 1 1 624D94BE
P 4800 5100
F 0 "JP7-JP1" H 4880 5092 50  0000 L CNN
F 1 "Single Row 2.54 Space" H 4880 5001 50  0000 L CNN
F 2 "" H 4800 5100 50  0001 C CNN
F 3 "~" H 4800 5100 50  0001 C CNN
	1    4800 5100
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x06 JP1
U 1 1 624DA1EA
P 4800 6000
F 0 "JP1" H 4880 5992 50  0000 L CNN
F 1 "Single Row 2.54 Space" H 4880 5901 50  0000 L CNN
F 2 "" H 4800 6000 50  0001 C CNN
F 3 "~" H 4800 6000 50  0001 C CNN
	1    4800 6000
	1    0    0    -1  
$EndComp
Text Notes 4250 4700 0    50   ~ 0
NE356T
Text GLabel 4450 5000 0    50   Output ~ 0
Ground
Text GLabel 4450 5100 0    50   Input ~ 0
1-3
Text GLabel 4450 5900 0    50   Input ~ 0
1-3
Text GLabel 4450 6000 0    50   Input ~ 0
2-3
Text GLabel 4450 6100 0    50   Input ~ 0
3-3
Text GLabel 4450 5200 0    50   Input ~ 0
2-3
Text GLabel 4450 5300 0    50   Input ~ 0
3-3
Text GLabel 4450 5800 0    50   Output ~ 0
Ground
Wire Wire Line
	4600 5000 4450 5000
Wire Wire Line
	4600 5100 4450 5100
Wire Wire Line
	4600 5200 4450 5200
Wire Wire Line
	4600 5300 4450 5300
Wire Wire Line
	4600 5800 4450 5800
Wire Wire Line
	4600 5900 4450 5900
Wire Wire Line
	4600 6000 4450 6000
Wire Wire Line
	4600 6100 4450 6100
Text Notes 4250 4850 0    50   ~ 0
Waste Water R1 and R2
Text Notes 4250 5650 0    50   ~ 0
Fresh Water S1
Wire Notes Line
	4050 4250 6150 4250
Wire Notes Line
	6150 4250 6150 6850
Wire Notes Line
	6150 6850 4050 6850
Wire Notes Line
	4050 6850 4050 4250
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J4
U 1 1 624DF47E
P 2500 1950
F 0 "J4" H 2550 1625 50  0000 C CNN
F 1 "AMP Mini Fit" H 2550 1716 50  0000 C CNN
F 2 "" H 2500 1950 50  0001 C CNN
F 3 "~" H 2500 1950 50  0001 C CNN
	1    2500 1950
	-1   0    0    1   
$EndComp
Text Notes 1550 1400 0    50   ~ 0
DS470\n
Text GLabel 2800 1950 2    50   Output ~ 0
Ground_Brown
Text GLabel 2800 1850 2    50   Output ~ 0
5V_DC_White
Text GLabel 2100 1950 0    50   Input ~ 0
Signal_0-5V-Green
Wire Wire Line
	2700 1850 2800 1850
Wire Wire Line
	2700 1950 2800 1950
Wire Wire Line
	2100 1950 2200 1950
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J5
U 1 1 624E2B1C
P 2500 2650
F 0 "J5" H 2550 2325 50  0000 C CNN
F 1 "AMP Mini Fit" H 2550 2416 50  0000 C CNN
F 2 "" H 2500 2650 50  0001 C CNN
F 3 "~" H 2500 2650 50  0001 C CNN
	1    2500 2650
	-1   0    0    1   
$EndComp
Text GLabel 2800 2650 2    50   Output ~ 0
Ground_Brown
Text GLabel 2800 2550 2    50   Output ~ 0
5V_DC_White
Text GLabel 2100 2650 0    50   Input ~ 0
Signal_0-5V-Green
Wire Wire Line
	2700 2550 2800 2550
Wire Wire Line
	2700 2650 2800 2650
Wire Wire Line
	2100 2650 2200 2650
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J6
U 1 1 624E38E3
P 2500 3350
F 0 "J6" H 2550 3025 50  0000 C CNN
F 1 "AMP Mini Fit" H 2550 3116 50  0000 C CNN
F 2 "" H 2500 3350 50  0001 C CNN
F 3 "~" H 2500 3350 50  0001 C CNN
	1    2500 3350
	-1   0    0    1   
$EndComp
Text GLabel 2800 3350 2    50   Output ~ 0
Ground_Brown
Text GLabel 2800 3250 2    50   Output ~ 0
5V_DC_White
Text GLabel 2100 3350 0    50   Input ~ 0
Signal_0-5V-Green
Wire Wire Line
	2700 3250 2800 3250
Wire Wire Line
	2700 3350 2800 3350
Wire Wire Line
	2100 3350 2200 3350
Text Notes 2250 1550 0    50   ~ 0
Fresh Water
Text Notes 2250 2250 0    50   ~ 0
Waste Water
Text Notes 2300 2950 0    50   ~ 0
Aux Water
Text Notes 1300 4800 0    50   ~ 0
DS300
$Comp
L Connector_Generic:Conn_01x04 J2
U 1 1 624ED413
P 2450 5250
F 0 "J2" H 2530 5242 50  0000 L CNN
F 1 "Single Row 2.54 Space" H 2530 5151 50  0000 L CNN
F 2 "" H 2450 5250 50  0001 C CNN
F 3 "~" H 2450 5250 50  0001 C CNN
	1    2450 5250
	1    0    0    -1  
$EndComp
Text GLabel 2100 5150 0    50   Output ~ 0
Ground
Text GLabel 2100 5250 0    50   Input ~ 0
1-3
Text GLabel 2100 5350 0    50   Input ~ 0
2-3
Text GLabel 2100 5450 0    50   Input ~ 0
3-3
Wire Wire Line
	2250 5150 2100 5150
Wire Wire Line
	2250 5250 2100 5250
Wire Wire Line
	2250 5350 2100 5350
Wire Wire Line
	2250 5450 2100 5450
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 624EDA78
P 2450 6200
F 0 "J3" H 2530 6192 50  0000 L CNN
F 1 "Single Row 2.54 Space" H 2530 6101 50  0000 L CNN
F 2 "" H 2450 6200 50  0001 C CNN
F 3 "~" H 2450 6200 50  0001 C CNN
	1    2450 6200
	1    0    0    -1  
$EndComp
Text GLabel 2100 6200 0    50   Output ~ 0
Ground
Text GLabel 2100 6300 0    50   Input ~ 0
Full
Wire Wire Line
	2100 6200 2250 6200
Wire Wire Line
	2100 6300 2250 6300
Text Notes 2100 4950 0    50   ~ 0
Fresh Water
Text Notes 2150 6000 0    50   ~ 0
Waste Water
Wire Notes Line
	1200 1250 3550 1250
Wire Notes Line
	3550 1250 3550 3550
Wire Notes Line
	3550 3550 1200 3550
Wire Notes Line
	1200 3550 1200 1250
Wire Notes Line
	1200 4650 3550 4650
Wire Notes Line
	3550 4650 3550 6850
Wire Notes Line
	3550 6850 1200 6850
Wire Notes Line
	1200 6850 1200 4650
Text Notes 7300 4450 0    50   ~ 0
Arscilicii S-TTK
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J7
U 1 1 62508E53
P 7650 5300
F 0 "J7" H 7700 4975 50  0000 C CNN
F 1 "AMP Mini Fit" H 7700 5066 50  0000 C CNN
F 2 "" H 7650 5300 50  0001 C CNN
F 3 "~" H 7650 5300 50  0001 C CNN
	1    7650 5300
	-1   0    0    1   
$EndComp
Text GLabel 7950 5300 2    50   Output ~ 0
Level_1
Text GLabel 7950 5200 2    50   Output ~ 0
Level_3
Text GLabel 7250 5300 0    50   Input ~ 0
Level_2
Wire Wire Line
	7850 5200 7950 5200
Wire Wire Line
	7850 5300 7950 5300
Wire Wire Line
	7250 5300 7350 5300
Text Notes 7400 4900 0    50   ~ 0
Fresh Water
Text GLabel 7250 5200 0    50   Input ~ 0
Level_4
Wire Wire Line
	7350 5200 7250 5200
$Comp
L Connector_Generic:Conn_01x02 R1-1
U 1 1 6250D68B
P 7600 5950
F 0 "R1-1" H 7680 5942 50  0000 L CNN
F 1 "AMP Mini Fit" H 7680 5851 50  0000 L CNN
F 2 "" H 7600 5950 50  0001 C CNN
F 3 "~" H 7600 5950 50  0001 C CNN
	1    7600 5950
	1    0    0    -1  
$EndComp
Text GLabel 7250 5950 0    50   Output ~ 0
Level_1
Text GLabel 7250 6050 0    50   Input ~ 0
Level_2
Wire Wire Line
	7250 5950 7400 5950
Wire Wire Line
	7250 6050 7400 6050
Text Notes 7300 5750 0    50   ~ 0
Waste Water
Text Notes 1950 1400 0    50   ~ 0
Connection at Power Block
Text Notes 1650 4800 0    50   ~ 0
Connection at Power Block
Text Notes 4650 4700 0    50   ~ 0
Connection at Power Block
Text Notes 6600 1700 0    50   ~ 0
Weiß = Plus Versorgungsspannung Tanksonde
Text Notes 6600 1850 0    50   ~ 0
Braun = Ground Tanksonde
Text Notes 6650 2000 0    50   ~ 0
Grün = Tanksignal 0-2.2V
Text Notes 6650 1500 0    50   ~ 0
Votronic Tankanzeige S, Infopanel pro\nVBS-2, VPC-System
Wire Notes Line
	6550 1250 6550 2100
Wire Notes Line
	6550 2100 8450 2100
Wire Notes Line
	8450 2100 8450 1250
Wire Notes Line
	8450 1250 6550 1250
Wire Notes Line
	6700 4650 8500 4650
Wire Notes Line
	8500 4650 8500 6350
Wire Notes Line
	8500 6350 6700 6350
Wire Notes Line
	6700 6350 6700 4650
$Comp
L Connector_Generic:Conn_01x05 ST2
U 1 1 6251B16B
P 5100 2900
F 0 "ST2" H 5180 2942 50  0000 L CNN
F 1 "Molex_MicroFit" H 5180 2851 50  0000 L CNN
F 2 "" H 5100 2900 50  0001 C CNN
F 3 "~" H 5100 2900 50  0001 C CNN
	1    5100 2900
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x06 ST1
U 1 1 6251BBA1
P 5100 1950
F 0 "ST1" H 5180 1942 50  0000 L CNN
F 1 "Molex_MicroFit" H 5180 1851 50  0000 L CNN
F 2 "" H 5100 1950 50  0001 C CNN
F 3 "~" H 5100 1950 50  0001 C CNN
	1    5100 1950
	1    0    0    -1  
$EndComp
Text GLabel 4800 1750 0    50   Input ~ 0
100
Text GLabel 4800 1850 0    50   Input ~ 0
75
Text GLabel 4800 1950 0    50   Input ~ 0
50
Text GLabel 4800 2050 0    50   Input ~ 0
25
Text GLabel 4800 2150 0    50   Input ~ 0
Basis_DC_Plus
Text GLabel 4800 2700 0    50   Input ~ 0
100
Text GLabel 4800 2800 0    50   Input ~ 0
75
Text GLabel 4800 2900 0    50   Input ~ 0
50
Text GLabel 4800 3000 0    50   Input ~ 0
25
Text GLabel 4800 3100 0    50   Input ~ 0
Basis_DC_Plus
Wire Wire Line
	4800 1750 4900 1750
Wire Wire Line
	4800 1850 4900 1850
Wire Wire Line
	4800 1950 4900 1950
Wire Wire Line
	4800 2050 4900 2050
Wire Wire Line
	4800 2150 4900 2150
Wire Wire Line
	4800 2700 4900 2700
Wire Wire Line
	4800 2800 4900 2800
Wire Wire Line
	4800 2900 4900 2900
Wire Wire Line
	4800 3000 4900 3000
Wire Wire Line
	4800 3100 4900 3100
Text Notes 4250 1400 0    50   ~ 0
LT100
Text Notes 4550 1400 0    50   ~ 0
Connection at Panel
Wire Notes Line
	4050 1250 6050 1250
Wire Notes Line
	6050 1250 6050 3600
Wire Notes Line
	6050 3600 4050 3600
Wire Notes Line
	4050 3600 4050 1250
Text Notes 4750 1550 0    50   ~ 0
Fresh Water
Text Notes 4800 2500 0    50   ~ 0
Waste Water
$EndSCHEMATC

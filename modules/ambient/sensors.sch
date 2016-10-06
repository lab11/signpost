EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:lab11_adc
LIBS:lab11_battery_chargers
LIBS:lab11_buffers
LIBS:lab11_comparators
LIBS:lab11_connectors
LIBS:lab11_diodes
LIBS:lab11_ldo
LIBS:lab11_level_shifters
LIBS:lab11_mcu
LIBS:lab11_memory
LIBS:lab11_microphones
LIBS:lab11_nucleum
LIBS:lab11_opamps
LIBS:lab11_receptacles
LIBS:lab11_rlc
LIBS:lab11_switches
LIBS:lab11_transistors
LIBS:lab11_voltage_references
LIBS:lab11_sensors
LIBS:lab11_crystals
LIBS:ambient-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ISL29035 U5
U 1 1 57E99A0E
P 3850 2050
F 0 "U5" H 3700 2300 60  0000 C CNN
F 1 "ISL29035" H 3850 1800 60  0000 C CNN
F 2 "lab11-ic:L6_1.5x1.6mm" H 3900 2450 60  0001 C CNN
F 3 "" H 3650 2350 60  0001 C CNN
	1    3850 2050
	1    0    0    -1  
$EndComp
$Comp
L LPS331AP U6
U 1 1 57E99A91
P 3900 4600
F 0 "U6" H 3650 5050 60  0000 C CNN
F 1 "LPS331AP" V 3550 4200 60  0000 C CNN
F 2 "lab11-ic:HCLGA-16L" H 3700 5300 60  0001 C CNN
F 3 "" H 3600 5150 60  0001 C CNN
	1    3900 4600
	1    0    0    -1  
$EndComp
$Comp
L Si7021 U7
U 1 1 57E99AE2
P 6650 2000
F 0 "U7" H 6450 2200 60  0000 C CNN
F 1 "Si7021" H 6650 1850 60  0000 C CNN
F 2 "lab11-ic:DFN_3x3mm_1mm" H 6650 2500 60  0001 C CNN
F 3 "" H 6450 2250 60  0001 C CNN
F 4 "Digikey" H 6700 2600 60  0001 C CNN "Vendor 1"
F 5 "336-3140-5-ND" H 6650 2400 60  0001 C CNN "Vendor 1 Part Number"
	1    6650 2000
	1    0    0    -1  
$EndComp
$Comp
L TSL2561 U8
U 1 1 57E99B39
P 6800 4750
F 0 "U8" H 6500 4950 60  0000 C CNN
F 1 "TSL2561" H 6750 4450 60  0000 C CNN
F 2 "lab11-ic:DFN2" H 6650 5050 60  0001 C CNN
F 3 "" H 6400 4950 60  0001 C CNN
	1    6800 4750
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR019
U 1 1 57E99DBC
P 6100 2050
F 0 "#PWR019" H 6100 1800 50  0001 C CNN
F 1 "GND" H 6100 1900 50  0000 C CNN
F 2 "" H 6100 2050 50  0000 C CNN
F 3 "" H 6100 2050 50  0000 C CNN
	1    6100 2050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR020
U 1 1 57E99EB4
P 6150 4950
F 0 "#PWR020" H 6150 4700 50  0001 C CNN
F 1 "GND" H 6150 4800 50  0000 C CNN
F 2 "" H 6150 4950 50  0000 C CNN
F 3 "" H 6150 4950 50  0000 C CNN
	1    6150 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 4950 6150 4950
$Comp
L +3.3V #PWR021
U 1 1 57E99F26
P 6250 3950
F 0 "#PWR021" H 6250 3800 50  0001 C CNN
F 1 "+3.3V" H 6250 4090 50  0000 C CNN
F 2 "" H 6250 3950 50  0000 C CNN
F 3 "" H 6250 3950 50  0000 C CNN
	1    6250 3950
	1    0    0    -1  
$EndComp
$Comp
L C_0.1u_0402_10V_10%_JB C14
U 1 1 57E99FBE
P 6400 4100
F 0 "C14" H 6425 4200 50  0000 L CNN
F 1 "C_0.1u_0402_10V_10%_JB" H 5950 4850 50  0001 L CNN
F 2 "Capacitors_SMD:C_0402" H 6450 4600 50  0001 C CNN
F 3 "" H 6425 4200 50  0000 C CNN
F 4 "0.1uF" H 6525 4000 60  0000 C CNN "Capacitance"
F 5 "Digikey" H 6425 4675 60  0001 C CNN "Vendor 1"
F 6 "445-10894-1-ND" H 6425 4750 60  0001 C CNN "Vendor 1 Part Number"
	1    6400 4100
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR022
U 1 1 57E9A0E4
P 6400 4300
F 0 "#PWR022" H 6400 4050 50  0001 C CNN
F 1 "GND" H 6400 4150 50  0000 C CNN
F 2 "" H 6400 4300 50  0000 C CNN
F 3 "" H 6400 4300 50  0000 C CNN
	1    6400 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 4650 6250 3950
Wire Wire Line
	6250 3950 6400 3950
Wire Wire Line
	6400 4250 6400 4300
Wire Wire Line
	6250 4800 6150 4800
Wire Wire Line
	6150 4800 6150 4950
Text GLabel 7500 4650 2    60   Input ~ 0
SDA
Text GLabel 7500 4950 2    60   Input ~ 0
SCL
Wire Wire Line
	7300 4650 7500 4650
Wire Wire Line
	7300 4950 7500 4950
Text HLabel 7500 4800 2    60   Input ~ 0
TSL_INT
Wire Wire Line
	7300 4800 7500 4800
Text GLabel 6100 1900 0    60   Input ~ 0
SDA
Text GLabel 7150 1900 2    60   Input ~ 0
SCL
Wire Wire Line
	7050 1900 7150 1900
Wire Wire Line
	6100 1900 6200 1900
Wire Wire Line
	6200 2050 6100 2050
$Comp
L +3.3V #PWR023
U 1 1 57E9AC3D
P 7600 1950
F 0 "#PWR023" H 7600 1800 50  0001 C CNN
F 1 "+3.3V" H 7600 2090 50  0000 C CNN
F 2 "" H 7600 1950 50  0000 C CNN
F 3 "" H 7600 1950 50  0000 C CNN
	1    7600 1950
	1    0    0    -1  
$EndComp
$Comp
L C_0.1u_0402_10V_10%_JB C15
U 1 1 57E9AC92
P 7600 2200
F 0 "C15" H 7625 2300 50  0000 L CNN
F 1 "C_0.1u_0402_10V_10%_JB" H 7150 2950 50  0001 L CNN
F 2 "Capacitors_SMD:C_0402" H 7650 2700 50  0001 C CNN
F 3 "" H 7625 2300 50  0000 C CNN
F 4 "0.1uF" H 7725 2100 60  0000 C CNN "Capacitance"
F 5 "Digikey" H 7625 2775 60  0001 C CNN "Vendor 1"
F 6 "445-10894-1-ND" H 7625 2850 60  0001 C CNN "Vendor 1 Part Number"
	1    7600 2200
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR024
U 1 1 57E9ACD8
P 7600 2400
F 0 "#PWR024" H 7600 2150 50  0001 C CNN
F 1 "GND" H 7600 2250 50  0000 C CNN
F 2 "" H 7600 2400 50  0000 C CNN
F 3 "" H 7600 2400 50  0000 C CNN
	1    7600 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	7050 2050 7600 2050
Wire Wire Line
	7600 2050 7600 1950
Wire Wire Line
	7600 2350 7600 2400
$Comp
L GND #PWR025
U 1 1 57E9B1A6
P 3250 2200
F 0 "#PWR025" H 3250 1950 50  0001 C CNN
F 1 "GND" H 3250 2050 50  0000 C CNN
F 2 "" H 3250 2200 50  0000 C CNN
F 3 "" H 3250 2200 50  0000 C CNN
	1    3250 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 2050 3250 2050
Wire Wire Line
	3250 1850 3250 2200
$Comp
L C_0.1u_0402_10V_10%_JB C11
U 1 1 57E9B20D
P 3250 1700
F 0 "C11" H 3150 1800 50  0000 L CNN
F 1 "C_0.1u_0402_10V_10%_JB" H 2800 2450 50  0001 L CNN
F 2 "Capacitors_SMD:C_0402" H 3300 2200 50  0001 C CNN
F 3 "" H 3275 1800 50  0000 C CNN
F 4 "0.1uF" H 3100 1600 60  0000 C CNN "Capacitance"
F 5 "Digikey" H 3275 2275 60  0001 C CNN "Vendor 1"
F 6 "445-10894-1-ND" H 3275 2350 60  0001 C CNN "Vendor 1 Part Number"
	1    3250 1700
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR026
U 1 1 57E9B285
P 3250 1500
F 0 "#PWR026" H 3250 1350 50  0001 C CNN
F 1 "+3.3V" H 3250 1640 50  0000 C CNN
F 2 "" H 3250 1500 50  0000 C CNN
F 3 "" H 3250 1500 50  0000 C CNN
	1    3250 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	3450 1900 3450 1550
Wire Wire Line
	3450 1550 3250 1550
Wire Wire Line
	3250 1550 3250 1500
Connection ~ 3250 2050
Text GLabel 4400 1900 2    60   Input ~ 0
SDA
Text GLabel 4400 2050 2    60   Input ~ 0
SCL
Text HLabel 4400 2200 2    60   Input ~ 0
ISL29035_INT
Wire Wire Line
	4300 2200 4400 2200
Wire Wire Line
	4300 2050 4400 2050
Wire Wire Line
	4300 1900 4400 1900
$Comp
L GND #PWR027
U 1 1 57E9C883
P 3950 5350
F 0 "#PWR027" H 3950 5100 50  0001 C CNN
F 1 "GND" H 3950 5200 50  0000 C CNN
F 2 "" H 3950 5350 50  0000 C CNN
F 3 "" H 3950 5350 50  0000 C CNN
	1    3950 5350
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 5250 3750 5350
Wire Wire Line
	3400 5350 4050 5350
Wire Wire Line
	3950 5350 3950 5250
Wire Wire Line
	4050 5350 4050 5250
Connection ~ 3950 5350
Wire Wire Line
	4150 5350 4150 5250
Wire Wire Line
	3850 5350 4150 5350
Wire Wire Line
	3850 5250 3850 5350
$Comp
L +3.3V #PWR028
U 1 1 57E9CCBD
P 3950 3450
F 0 "#PWR028" H 3950 3300 50  0001 C CNN
F 1 "+3.3V" H 3950 3590 50  0000 C CNN
F 2 "" H 3950 3450 50  0000 C CNN
F 3 "" H 3950 3450 50  0000 C CNN
	1    3950 3450
	1    0    0    -1  
$EndComp
$Comp
L C_0.1u_0402_10V_10%_JB C12
U 1 1 57E9CD80
P 4050 3600
F 0 "C12" H 4075 3700 50  0000 L CNN
F 1 "C_0.1u_0402_10V_10%_JB" H 3600 4350 50  0001 L CNN
F 2 "Capacitors_SMD:C_0402" H 4100 4100 50  0001 C CNN
F 3 "" H 4075 3700 50  0000 C CNN
F 4 "0.1uF" H 4175 3500 60  0000 C CNN "Capacitance"
F 5 "Digikey" H 4075 4175 60  0001 C CNN "Vendor 1"
F 6 "445-10894-1-ND" H 4075 4250 60  0001 C CNN "Vendor 1 Part Number"
	1    4050 3600
	1    0    0    -1  
$EndComp
$Comp
L C_10u_0603_10V_10%_JB C13
U 1 1 57E9CE6A
P 4400 3600
F 0 "C13" H 4425 3700 50  0000 L CNN
F 1 "C_10u_0603_10V_10%_JB" H 3950 4200 50  0001 L CNN
F 2 "Capacitors_SMD:C_0603" H 4450 3950 50  0001 C CNN
F 3 "" H 4425 3700 50  0000 C CNN
F 4 "10uF" H 4525 3500 60  0000 C CNN "Capacitance"
F 5 "Digikey" H 4425 4025 60  0001 C CNN "Vendor 1"
F 6 "445-11201-1-ND" H 4425 4100 60  0001 C CNN "Vendor 1 Part Number"
	1    4400 3600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR029
U 1 1 57E9CF4F
P 4250 3800
F 0 "#PWR029" H 4250 3550 50  0001 C CNN
F 1 "GND" H 4250 3650 50  0000 C CNN
F 2 "" H 4250 3800 50  0000 C CNN
F 3 "" H 4250 3800 50  0000 C CNN
	1    4250 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 4000 4150 4000
Connection ~ 4050 4000
Wire Wire Line
	3950 4000 3950 3450
Wire Wire Line
	3950 3450 4400 3450
Connection ~ 4050 3450
Wire Wire Line
	4250 3750 4250 3800
Wire Wire Line
	4050 3750 4400 3750
Connection ~ 4250 3750
Text GLabel 3250 4450 0    60   Input ~ 0
SDA
Wire Wire Line
	3400 4450 3250 4450
Text GLabel 3250 4300 0    60   Input ~ 0
SCL
Wire Wire Line
	3250 4300 3400 4300
Text HLabel 4500 4600 2    60   Input ~ 0
LPS_INT1
Text HLabel 4500 4750 2    60   Input ~ 0
LPS_INT2
Wire Wire Line
	4400 4750 4500 4750
Wire Wire Line
	4400 4600 4500 4600
$Comp
L +3.3V #PWR030
U 1 1 57E9E73A
P 2800 4600
F 0 "#PWR030" H 2800 4450 50  0001 C CNN
F 1 "+3.3V" H 2800 4740 50  0000 C CNN
F 2 "" H 2800 4600 50  0000 C CNN
F 3 "" H 2800 4600 50  0000 C CNN
	1    2800 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 4600 2800 4600
Wire Wire Line
	3400 4750 3400 5350
Connection ~ 3750 5350
$EndSCHEMATC

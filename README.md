# DEMO-EK-RA6M4-I1C-SHUNT-PMOD
Demonstration project for running the RL78/I1C Single Phase Shunt-based Metrology PMOD on the EK-RA6M4.

## Requirements
- FSP 3.7

## Connection Settings
- Jumper Wires:
  - P008 &rarr; P114
  - P301 &rarr; P113
- RL78/I1c 1PH Shunt connects to PMOD1 on the EK-RA6M4.
- J11 (Full Speed USB) can be connected to as a serial COM Port:
  - Settings: 115,200 8-N-1.
  - Streams out data from the RL78/I1c PMOD.
  - Requires ANSI compliant serial terminal as the program uses escape sequences for coloured text and cursor manipulation.
  - TeraTerm was used in development.

## Notes
This project has made use of [Gperf](https://www.gnu.org/software/gperf/) to generate the perfect hash function.
In order to manipulate the parser an understanding of this tool is required.

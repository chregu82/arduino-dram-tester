# arduino-dram-tester
Test DRAM with Arduino MEGA 2560 (I used a chinese clone with only 12MHz)
Forked from andremiller/arduino-dram-tester

## Basic DRAM tester, outputs results to Serial, based on:

* https://www.insentricity.com/a.cl/252
* https://github.com/FozzTexx/DRAM-Tester
* http://www.chronworks.com/DRAM

## Pinouts:

* DRAM pin -> Arduino MEGA pin
* DIN -> 53
* DOUT -> 52
* CAS -> 51
* RAS -> 50
* WE -> 49
* A0 to A5 -> 22 to 27
* A6 to A9 -> 36 to 39

(Also connect DRAM supply pins)

The number of address lines needs to be set in ADDR_BITS

## Changes to original code:

* Writes output to serial, displaying which row and col failed
* Times the tests and writes how long it took, just out of curiosity so you can calculate of refresh is happening fast enough
* Does pattern tests first (10101... and 01010...) because this identified failed ram faster for me
* Added a random bit write test

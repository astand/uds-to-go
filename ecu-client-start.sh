#!/bin/sh
./build/ecu-server-test -iface vcan0 -phys 0x701 -resp 0x700 -blksize 7 -stmin 50

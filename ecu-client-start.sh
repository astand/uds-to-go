#!/bin/sh
./build/ecu-server-test -iface vcan0 -phys 0x711 -resp 0x710 -blksize 7 -stmin 50

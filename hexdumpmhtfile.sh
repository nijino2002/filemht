#!/bin/sh
hexdump -e '1/1 "0x%08_ax "' -e '16/1 "%02X " " | "' -e '16/1 "%_p" "\n"' ./mhtfile.mf 

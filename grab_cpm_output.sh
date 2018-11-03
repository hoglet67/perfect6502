#!/bin/bash
egrep "OSWRCH|OSWORD0" | sed "s/. <0d>/# <0d>/" | fgrep -v "<0a>" | grep -v "<00>" | grep -v "BLOCK" | cut -d"O" -f2- | cut -d: -f2- | cut -c2- | sed "s/ <[0-9a-f][0-9a-f]>//" | tr -d "\n"  | tr "#\r" "\n"

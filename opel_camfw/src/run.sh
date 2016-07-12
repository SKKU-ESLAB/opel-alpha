#!/bin/bash
sudo modprobe bcm2835-v4l2
./ipcrm.sh
./deletesem
./test


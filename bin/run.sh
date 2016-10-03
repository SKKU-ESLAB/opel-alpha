#!/bin/bash
sudo modprobe bcm2835-v4l2
${OPEL_DIR}/bin/ipcrm.sh
${OPEL_DIR}/bin/deletesem
#${OPEL_DIR}/bin/cam_manager &


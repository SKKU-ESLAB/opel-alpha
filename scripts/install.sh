#!/bin/bash
#
##########################################################################
#
# Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
# 
# Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#  http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
##########################################################################
# install.sh: OPEL Install Script
##########################################################################

show_usage() {
  echo \
"$0: OPEL install script
usage: $0 [-h] [--help]

You can pass following environment variables to this script.
 - OPEL_OUT_DIR: OPEL build output directory
 - OPEL_BIN_DIR: OPEL binary directory
 - OPEL_CONFIG_DIR: OPEL config directory
 - OPEL_SENSOR_DRIVER_DIR: OPEL sensor driver directory
 - OPEL_APPS_DIR: OPEL application directory"
}

# Get help
getopt --test > /dev/null
if [[ $? -ne 4 ]]; then
  echo "This system has failed to test getopt."
  exit 1
fi

SHORT=h
LONG=help

PARSED=`getopt --options $SHORT --longoptions $LONG --name "$0" -- "$@"`
if [[ $? -ne 0 ]]; then
  exit 2
fi
eval set -- "$PARSED"

ARG_HELP=n
while true; do
  case "$1" in
    -h|--help)
      ARG_HELP=y
      shift
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "Programming error"
      exit 3
      ;;
  esac
done

# Print usage message
if [ $ARG_HELP = "y" ]
then
  show_usage
  exit 0
fi

# Determine environment variables
determine_var() {
  ENV_NAME=$1
  ENV_DESC=$2
  IS_DIRECTORY=$3
  LOOP_CONTINUE=y
  while [ $LOOP_CONTINUE = y ]
  do
    if [ ${!ENV_NAME} ]
    then
      echo -n "Enter $ENV_DESC(${!ENV_NAME}): "
    else
      echo -n "Enter $ENV_DESC: "
    fi
    read $ENV_NAME
    eval ${ENV_NAME}=`readlink --canonicalize ${!ENV_NAME}`

    if [ ${!ENV_NAME} ]
    then
      if [ ! -e ${!ENV_NAME} ]
      then
        echo "${!ENV_NAME} does not exist."
      else
        if [ $IS_DIRECTORY = y ]
        then
          if [ ! -d ${!ENV_NAME} ]
          then
            echo "${!ENV_NAME} is not directory."
          else
            LOOP_CONTINUE=n
          fi
        else
          if [ ! -f ${!ENV_NAME} ]
          then
            echo "${!ENV_NAME} is not regular file."
          else
            LOOP_CONTINUE=n
          fi
        fi
      fi
    fi
  done

  echo "$ENV_NAME= ${!ENV_NAME}"
}
# 1: OPEL_OUT_DIR
determine_var "OPEL_OUT_DIR" "OPEL output directory" y
# 2: OPEL_BIN_DIR
determine_var "OPEL_BIN_DIR" "OPEL binary directory" y
# 3: OPEL_CONFIG_DIR
determine_var "OPEL_CONFIG_DIR" "OPEL config directory" y
# 4: OPEL_SENSOR_DRIVER_DIR
determine_var "OPEL_SENSOR_DRIVER_DIR" "OPEL sensor driver directory" y
# 5: OPEL_APPS_DIR
determine_var "OPEL_APPS_DIR" "OPEL sensor driver directory" y

# Install OPEL


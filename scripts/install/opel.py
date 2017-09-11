#!/usr/bin/python3
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
# opel.py: OPEL SystemServer
##########################################################################

import argparse, signal, os, time, sys


# Check environment variables
try:
    # Environment variables that is used in this script
    gOpelBinDir = os.environ['OPEL_BIN_DIR']
    gOpelDataDir = os.environ['OPEL_DATA_DIR']
    gOpelDeleteSemPath = os.environ['OPEL_DELETESEM_PATH']

    # Environment variables that will be used inside of OPEL frameworks
    gOpelTargetName = os.environ['OPEL_TARGET_NAME']
    gOpelWpaSupplicantPath = os.environ['OPEL_WPA_SUPPLICANT_PATH']
    gOpelWpaCliPath = os.environ['OPEL_WPA_CLI_PATH']
    gOpelConfigDir = os.environ['OPEL_CONFIG_DIR']
    gOpelSensorDriverDir = os.environ['OPEL_SENSOR_DRIVER_DIR']
    gOpelAppsDir = os.environ['OPEL_APPS_DIR']
except KeyError as e:
    print("Not found environment variable. {}".format(str(e)))
    exit()

def signal_handler(signum, frame):
    return

def run_command(command, baseDir=gOpelBinDir):
    # Run command on this system server
    # Change directory
    os.chdir(baseDir)

    # Execute the daemon
    os.system(command)
    return

def run_on_daemon(command, name, uid=0, gid=0, baseDir=gOpelBinDir):
    # Run a command on child daemon process
    pid = os.fork()
    if pid:
        log("{} process is executed (pid {})".format(name, pid))
    else:
        # Set uid & gid
        os.setuid(uid)
        os.setgid(uid)

        # Change directory
        os.chdir(baseDir)

        # Execute the daemon
        os.execv(command[0], command)
    return

def log(message):
    print("[SystemServer] {}".format(message))
    return

############################## EDIT HERE!!! ##############################
def initialize_ipcrm():
    run_command("ipcrm -M 9447")
    run_command("ipcrm -M 5315")
    run_command("ipcrm -M 4941")
    run_command("ipcrm -S 49411")
    run_command("ipcrm -S 49441")
    run_command("ipcrm -M 4944")
    run_command("ipcrm -S 9948")
    return

# on_did_initialize: (event handler) initialization process is completed
# initialization process includes setting options and signal handlers.
def on_did_initialize(debugappcore):
    log("Initializing OPEL daemons...")

    # Execute prerequisites on system server
    run_command("mkdir -p " + gOpelDataDir)
    run_command("hciconfig hci0 piscan")
    initialize_ipcrm()
    run_command(gOpelDeleteSemPath)

    # Execute daemons
    if debugappcore:
        run_on_daemon(command=["/usr/bin/gdb", "./opel-appcore"],
                name="App-core Framework Daemon")
    else:
        run_on_daemon(command=["./opel-appcore"],
                name="App-core Framework Daemon")
    run_on_daemon(command=["./opel-camera"],
            name="Camera Framework Daemon")
    run_on_daemon(command=["./opel-sensor"],
            name="Sensor Framework Daemon")
    return

# on_did_halt_by_user: (event handler) user killed OPEL manager manually
def on_did_halt_by_user():
    log("User key interrupt is detected.")
    log("Terminating OPEL...")
    run_command("./opel_p2p_setup.sh stop")
    return

# on_did_dead_daemon: (event handler) daemon process is dead
def on_did_dead_deamon(pid):
    log("An OPEL process (pid {}) is dead.".format(pid))
    return
##########################################################################

def main():
    # Check python version
    if (sys.version_info[0] < 3):
        log("You are trying to use Python version {}.{}.{}"
            .format(sys.version_info[0], sys.version_info[1],
                    sys.version_info[2]))
        log("OPEL requires Python 3.0+.")
        return

    # Check if this is root user 
    thisUid = os.getuid()
    if thisUid != 0:
        log("You(uid={}) are not root user. Please try it again on root."
            .format(thisUid))
        return

    # Parse option arguments
    parser = argparse.ArgumentParser(description="OPEL options.")
    parser.add_argument("--debug-appcore", "-da", dest="debugappcore",
            action="store_true",
            help="Debug Appcore Manager")
    args = parser.parse_args()

    # Register signal handlers
    signal.signal(signal.SIGCHLD, signal_handler)
    signal.signal(signal.SIGINT, signal_handler)

    # Initialize Completed
    log("args: {}".format(args))
    on_did_initialize(args.debugappcore)

    # Wait for signals
    while True:
        siginfo = signal.sigwaitinfo({signal.SIGCHLD, signal.SIGINT})
        if siginfo.si_signo == signal.SIGCHLD:
            # Child daemon process is dead
            on_did_dead_deamon(siginfo.si_pid)
        elif siginfo.si_signo == signal.SIGINT:
            # Halt OPEL triggered by user
            on_did_halt_by_user()
            return

if __name__ == "__main__":
    main()

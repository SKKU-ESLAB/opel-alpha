#!/usr/bin/python3
#
##########################################################################
#
# Copyright (c) 2015-2016 CISS, and contributors. All rights reserved.
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

opelDir = os.path.dirname(os.path.realpath(__file__))

def signal_handler(signum, frame):
    return

def run_command(command, baseDir=opelDir):
    # Run command on this system server
    # Change directory
    os.chdir(baseDir)

    # Execute the daemon
    os.system(command)
    return

def run_on_daemon(command, name, uid=0, gid=0, baseDir=opelDir):
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
def on_did_initialize():
    log("Initializing OPEL daemons...")

    # Execute prerequisites on system server
    os.environ['OPEL_DIR'] = opelDir;
    run_command("hciconfig hci0 piscan")
    initialize_ipcrm()
    run_command("./deletesem")

    # Execute daemons
    run_on_daemon(command=["./sysAppManager"],
            name="App/Sys Manager Daemon",
            baseDir="./appManager")
    run_on_daemon(command=["./cam_fw"],
            name="Camera Framework Daemon")
    run_on_daemon(command=["./sensorManager"],
            name="Sensor Manager Daemon")
    return

# on_did_halt_by_user: (event handler) user killed OPEL manager manually
def on_did_halt_by_user():
    log("User key interrupt is detected.")
    log("Terminating OPEL...")
    run_command("./p2p_setup.sh stop")
    return

# on_did_dead_daemon: (event handler) daemon process is dead
def on_did_dead_deamon(pid):
    log("An OPEL process (pid {}) is dead.".format(siginfo.si_pid))
    return
##########################################################################

def main():
    # Check python version
    if (sys.version_info[0] < 3) or \
       (sys.version_info[0] == 3 and sys.version_info[1] < 5):
        log("You are trying to use Python version {}.{}.{}"
            .format(sys.version_info[0], sys.version_info[1],
                    sys.version_info[2]))
        log("OPEL requires Python 3.5+.")
        return

    # Check if this is root user 
    thisUid = os.getuid()
    if thisUid != 0:
        log("You(uid={}) are not root user. Please try it again on root."
            .format(thisUid))
        return

    # Parse option arguments
    parser = argparse.ArgumentParser(description="OPEL options.")
### Example of parsing option
#    parser.add_argument("--bt-only", dest="store_true",
#            help="Bluetooth only mode")
###
    args = parser.parse_args()

    # Register signal handlers
    signal.signal(signal.SIGCHLD, signal_handler)
    signal.signal(signal.SIGINT, signal_handler)

    # Initialize Completed
    on_did_initialize()

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
#!/bin/sh
# Copyright (c) 2026 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#
# qml-runner stress test. Runs ON the device. Repeatedly launches,
# relaunches and closes a QML application through SAM, watching for:
#   - failed launches (app process never appears)
#   - crashes (process disappears without being asked to)
#   - memory growth of the app process across relaunch cycles
#
# Usage: qml-runner-stress.sh [-a app-id] [-n iterations] [-r relaunches]
#                              [-s settle-seconds]
#
# Exit code: 0 if every iteration launched, survived its relaunches and
# closed on request; 1 otherwise.

APP_ID="com.palm.app.settings"
ITERATIONS=25
RELAUNCHES=4
SETTLE=5
SAM="luna://com.webos.service.applicationmanager"

while getopts "a:n:r:s:h" opt; do
    case $opt in
        a) APP_ID="$OPTARG" ;;
        n) ITERATIONS="$OPTARG" ;;
        r) RELAUNCHES="$OPTARG" ;;
        s) SETTLE="$OPTARG" ;;
        h|*) echo "Usage: $0 [-a app-id] [-n iterations] [-r relaunches] [-s settle-seconds]"; exit 0 ;;
    esac
done

fail=0
crashes=0
rss_first=0
rss_last=0

app_pid() {
    # The runner's command line is "/usr/bin/qml-runner --appid <id> ...".
    # Anchor on the executable so we never match this script itself (its
    # own command line also contains both "qml-runner" and the app id),
    # and filter our own pid out for good measure.
    pgrep -f "^/usr/bin/qml-runner .*$APP_ID" | grep -v "^$$\$" | head -n1
}

app_rss() {
    pid=$(app_pid)
    [ -n "$pid" ] && awk '/VmRSS/ {print $2}' "/proc/$pid/status" 2>/dev/null || echo 0
}

sam_call() {
    luna-send -n 1 "$SAM/$1" "$2" 2>&1
}

wait_for() {
    # wait_for <up|down> <seconds>
    want=$1; secs=$(( $2 * 10 )); i=0
    while [ $i -lt $secs ]; do
        pid=$(app_pid)
        if [ "$want" = up ] && [ -n "$pid" ]; then return 0; fi
        if [ "$want" = down ] && [ -z "$pid" ]; then return 0; fi
        sleep 0.1 2>/dev/null || sleep 1
        i=$((i + 1))
    done
    return 1
}

echo "Stress testing $APP_ID: $ITERATIONS iterations x $RELAUNCHES relaunches"

n=1
while [ "$n" -le "$ITERATIONS" ]; do
    printf '[%03d/%03d] launch' "$n" "$ITERATIONS"
    sam_call launch "{\"id\":\"$APP_ID\"}" >/dev/null

    if ! wait_for up 15; then
        echo " ... FAILED (process never appeared)"
        fail=$((fail + 1))
        n=$((n + 1))
        continue
    fi
    pid=$(app_pid)
    rss=$(app_rss)
    [ "$rss_first" = 0 ] && rss_first=$rss
    printf ' pid=%s rss=%skB' "$pid" "$rss"

    # Give the app time to finish its registerApp subscription with SAM;
    # a relaunch that arrives before registration completes is answered
    # with a kill-and-restart by design, which is not what we are
    # trying to measure here.
    sleep "$SETTLE"

    r=1
    while [ "$r" -le "$RELAUNCHES" ]; do
        sam_call launch "{\"id\":\"$APP_ID\",\"params\":{\"stress\":$r}}" >/dev/null
        sleep 1
        newpid=$(app_pid)
        if [ -z "$newpid" ]; then
            echo " ... CRASH on relaunch $r"
            crashes=$((crashes + 1))
            fail=$((fail + 1))
            break
        fi
        if [ "$newpid" != "$pid" ]; then
            # SAM killed and restarted it - that is the bug the
            # LSRegisterApplicationService/LSCall patches fix.
            echo " ... RESTARTED on relaunch $r (pid $pid -> $newpid)"
            fail=$((fail + 1))
            pid=$newpid
        fi
        r=$((r + 1))
    done

    rss=$(app_rss)
    rss_last=$rss
    printf ' rss-after=%skB close' "$rss"

    sam_call closeByAppId "{\"id\":\"$APP_ID\"}" >/dev/null
    if ! wait_for down 10; then
        echo " ... FAILED (did not exit on close)"
        kill -9 "$(app_pid)" 2>/dev/null
        fail=$((fail + 1))
    else
        echo " ... ok"
    fi
    n=$((n + 1))
done

echo "----"
echo "iterations: $ITERATIONS  failures: $fail  crashes: $crashes"
echo "first-launch RSS: ${rss_first}kB  last-cycle RSS: ${rss_last}kB"
[ "$fail" -eq 0 ] || exit 1
exit 0

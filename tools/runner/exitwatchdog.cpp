// Copyright (c) 2026 Herman van Hazendonk <github.com@herrie.org>
//
// SPDX-License-Identifier: Apache-2.0

#include "exitwatchdog.h"

#include <QDebug>

#include <thread>

#include <unistd.h>

void armExitWatchdog(int exitCode, std::chrono::milliseconds limit)
{
    std::thread([exitCode, limit] {
        std::this_thread::sleep_for(limit);
        qWarning("Still tearing down %lld ms after the app quit, exiting without waiting any longer",
                 static_cast<long long>(limit.count()));
        // Not exit(): the static destructors it runs are teardown too, and
        // the thread that is stuck may hold what they need.
        _exit(exitCode);
    }).detach();
}

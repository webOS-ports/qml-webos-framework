// Copyright (c) 2026 Herman van Hazendonk <github.com@herrie.org>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef EXITWATCHDOG_H
#define EXITWATCHDOG_H

#include <chrono>

/*
 * Ends the process with exitCode if it is still running `limit` after this
 * is called - for the teardown after the app's event loop has returned.
 *
 * That teardown waits for things the runner does not own, and some of them
 * can wait for ever: ~QCoreApplication waits for every job in Qt's global
 * thread pool, and Qt Multimedia probes each media URL there with a
 * GstDiscoverer whose pipeline cannot be stopped while its decoder hangs.
 * A process that never exits has no window left but is still the app as far
 * as SAM is concerned, so launching the app again shows nothing.
 *
 * A teardown that finishes in time is not touched: the process exits from
 * main() as before and the watchdog's thread goes with it.
 */
void armExitWatchdog(int exitCode, std::chrono::milliseconds limit);

#endif // EXITWATCHDOG_H

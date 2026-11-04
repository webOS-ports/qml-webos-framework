// Copyright (c) 2026 Herman van Hazendonk <github.com@herrie.org>
//
// SPDX-License-Identifier: Apache-2.0

#include <QtTest>
#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtCore/QSemaphore>
#include <QtCore/QThreadPool>

#include <chrono>
#include <thread>

#include <cstdlib>
#include <cstring>
#include <string>

#include <signal.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

#include "../../../tools/runner/exitwatchdog.h"

using namespace std::chrono_literals;

/*
 * qml-runner's teardown after the event loop, in a child process: a
 * QCoreApplication goes away while a job in the global thread pool never
 * ends - the shape of a Qt Multimedia media probe whose decoder hangs - and
 * ~QCoreApplication waits for it.
 *
 * The child is this test run again ("--child"), not a bare fork(): QtTest
 * has a thread of its own by then, and a forked copy of a process with
 * threads may not start any (ThreadSanitizer refuses to).
 */
namespace {

enum class Job { None, Stuck };

// This test's own binary, as it was started (argv[0]): not /proc/self/exe,
// which under valgrind is valgrind.
std::string self;

struct Outcome {
    bool started = false;
    bool exited = false;   // on its own, before the parent gave up on it
    int code = -1;
    qint64 ms = 0;
};

// The child: a pool job that never ends if `stuck`, the watchdog armed for
// limitMs unless it is 0, then the application torn down.
int child(bool stuck, long limitMs)
{
    {
        int argc = 1;
        char name[] = "tst_exitwatchdog-child";
        char *argv[] = { name, nullptr };
        QCoreApplication app(argc, argv);
        static QSemaphore never;
        if (stuck)
            QThreadPool::globalInstance()->start([] { never.acquire(); });
        if (limitMs > 0)
            armExitWatchdog(7, std::chrono::milliseconds(limitMs));
    }
    return 0;
}

Outcome run(Job job, bool watchdog, std::chrono::milliseconds limit, qint64 giveUpMs)
{
    Outcome out;
    QElapsedTimer timer;
    timer.start();
    // Run through a loader when this run was (a target's ld.so on the
    // build host): TEST_LOADER=<its path>.
    const char *loaderEnv = std::getenv("TEST_LOADER");
    std::string loader = loaderEnv ? loaderEnv : "";
    std::string path = self, flag = "--child";
    std::string stuck = job == Job::Stuck ? "stuck" : "none";
    std::string ms = std::to_string(watchdog ? limit.count() : 0);
    char *direct[] = { path.data(), flag.data(), stuck.data(), ms.data(), nullptr };
    char *loaded[] = { loader.data(), path.data(), flag.data(), stuck.data(), ms.data(), nullptr };
    pid_t pid = -1;
    if (posix_spawn(&pid, loader.empty() ? path.c_str() : loader.c_str(), nullptr, nullptr,
                    loader.empty() ? direct : loaded, environ) != 0)
        return out;
    out.started = true;
    int status = 0;
    while (timer.elapsed() < giveUpMs) {
        const pid_t done = waitpid(pid, &status, WNOHANG);
        if (done == pid) {
            out.exited = true;
            out.code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            break;
        }
        std::this_thread::sleep_for(10ms);
    }
    out.ms = timer.elapsed();
    if (!out.exited) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
    }
    return out;
}

} // namespace

class ExitWatchdogTest : public QObject
{
    Q_OBJECT

private slots:
    // Without the watchdog the child does not exit at all: what the
    // others rely on, that the teardown really waits for the job.
    void stuckJobHoldsTheTeardown()
    {
        const Outcome out = run(Job::Stuck, false, 0ms, 1500);
        QVERIFY(out.started);
        QVERIFY2(!out.exited, "the child exited although a pool job never ends");
    }

    // With it, the child is gone soon after the limit, with the app's code.
    void stuckTeardownIsEnded()
    {
        const Outcome out = run(Job::Stuck, true, 300ms, 5000);
        QVERIFY(out.started);
        QVERIFY2(out.exited, "the child is still tearing down");
        QCOMPARE(out.code, 7);
        QVERIFY2(out.ms >= 250, qPrintable(QStringLiteral("ended early: %1 ms").arg(out.ms)));
        QVERIFY2(out.ms < 3000, qPrintable(QStringLiteral("ended late: %1 ms").arg(out.ms)));
    }

    // A teardown that finishes exits as before, not after the limit and
    // not with the watchdog's code.
    void quickTeardownIsNotDelayed()
    {
        const Outcome out = run(Job::None, true, 3000ms, 5000);
        QVERIFY(out.started);
        QVERIFY(out.exited);
        QCOMPARE(out.code, 0);
        QVERIFY2(out.ms < 2000, qPrintable(QStringLiteral("took %1 ms").arg(out.ms)));
    }
};

int main(int argc, char **argv)
{
    if (argc == 4 && std::strcmp(argv[1], "--child") == 0)
        return child(std::strcmp(argv[2], "stuck") == 0, std::strtol(argv[3], nullptr, 10));
    self = argv[0];
    ExitWatchdogTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_exitwatchdog.moc"

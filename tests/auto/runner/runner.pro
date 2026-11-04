# Copyright (c) 2026 Herman van Hazendonk <github.com@herrie.org>
#
# SPDX-License-Identifier: Apache-2.0

TEMPLATE = app
TARGET = tst_exitwatchdog

QT += testlib
QT -= gui

CONFIG += console c++17 warn_on testcase
CONFIG -= app_bundle

SOURCES += \
    tst_exitwatchdog.cpp \
    ../../../tools/runner/exitwatchdog.cpp

HEADERS += \
    ../../../tools/runner/exitwatchdog.h

!no_webos_platform {
    load(webos-variables)
    target.path = $$WEBOS_INSTALL_DATADIR/booster/tests
    INSTALLS += target
}

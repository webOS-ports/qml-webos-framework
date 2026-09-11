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

TEMPLATE = app
TARGET = tst_eosregion

QT += testlib qml gui

CONFIG += console c++17 qt warn_on testcase
CONFIG -= app_bundle

INCLUDEPATH += ../../../../src/common

# Compile the region classes in directly; they only depend on QtCore,
# QtGui and QtQml, so the test does not need the webOS platform stack.
SOURCES += \
    tst_eosregion.cpp \
    ../../../../src/common/eosregion.cpp \
    ../../../../src/common/eosregionrect.cpp

HEADERS += \
    ../../../../src/common/eosregion.h \
    ../../../../src/common/eosregionrect.h

!no_webos_platform {
    load(webos-variables)
    target.path = $$WEBOS_INSTALL_DATADIR/booster/tests
    INSTALLS += target
}

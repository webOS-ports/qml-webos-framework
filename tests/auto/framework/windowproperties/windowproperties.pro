# Copyright (c) 2026 Herman van Hazendonk <github.com@herrie.org>
#
# SPDX-License-Identifier: Apache-2.0
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
TARGET = tst_windowproperties

QT += testlib qml quick gui-private quick-private

CONFIG += console c++17 qt warn_on testcase
CONFIG -= app_bundle

INCLUDEPATH += ../../../../src/common ../../../../include

# WebOSQuickWindow is compiled in directly and linked against stand-ins for
# the webos-platform-interface classes it uses (fakewebosplatform.cpp), so
# the test needs no webOS compositor and runs on QT_QPA_PLATFORM=offscreen.
# Only the library's headers are used; moc runs on the ones that declare
# the QObjects involved, and the library is not linked.
WEBOS_PI_INCDIR = $$[QT_INSTALL_HEADERS]
QMAKE_MOC_OPTIONS += -b qpa/qplatformwindow.h

SOURCES += \
    tst_windowproperties.cpp \
    fakewebosplatform.cpp \
    ../../../../src/common/webosquickwindow.cpp \
    ../../../../src/common/eosregion.cpp \
    ../../../../src/common/eosregionrect.cpp

HEADERS += \
    fakewebosplatform.h \
    ../../../../src/common/webosquickwindow.h \
    ../../../../src/common/eosregion.h \
    ../../../../src/common/eosregionrect.h \
    $$WEBOS_PI_INCDIR/webosinputmanager.h \
    $$WEBOS_PI_INCDIR/webosplatform.h \
    $$WEBOS_PI_INCDIR/webosshell.h \
    $$WEBOS_PI_INCDIR/webosshellsurface.h

!no_webos_platform {
    load(webos-variables)
    target.path = $$WEBOS_INSTALL_DATADIR/booster/tests
    INSTALLS += target
}

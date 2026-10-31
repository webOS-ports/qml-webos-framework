// Copyright (c) 2026 Herman van Hazendonk <github.com@herrie.org>
//
// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <QtTest>
#include <QtCore/private/qobject_p.h>

#include "webosquickwindow.h"

// What a WebOSQuickWindow leaves connected when it is destroyed. Run on any
// QPA, e.g. QT_QPA_PLATFORM=offscreen.
class TestWindowDestroy : public QObject
{
    Q_OBJECT

private:
    static bool receivesOwnVisibleChanged(QWindow *window)
    {
        return QObjectPrivate::get(window)->receiverList("visibleChanged(bool)")
                .contains(window);
    }

private slots:
    // ~QWindow hides a window that is still visible, which emits
    // visibleChanged after ~WebOSQuickWindow has run. The window must not
    // be connected to that signal any more: the slot would be called on an
    // object that is no longer a WebOSQuickWindow (UBSan: "downcast of
    // address ... which does not point to an object of type
    // 'WebOSQuickWindow'").
    void destroyWhileVisible_noSelfConnectionLeft()
    {
        auto *window = new WebOSQuickWindow;
        window->show();
        QVERIFY(window->isVisible());
        // The connection the destructor has to remove is there.
        QVERIFY(receivesOwnVisibleChanged(window));

        QObject probe;
        int emitted = 0;
        bool selfConnected = true;
        QObject::connect(window, &QWindow::visibleChanged, &probe,
                         [&emitted, &selfConnected, window](bool visible) {
            ++emitted;
            QCOMPARE(visible, false);
            selfConnected = receivesOwnVisibleChanged(window);
        });

        delete window;
        QCOMPARE(emitted, 1);
        QVERIFY(!selfConnected);
    }
};

QTEST_MAIN(TestWindowDestroy)
#include "tst_windowdestroy.moc"

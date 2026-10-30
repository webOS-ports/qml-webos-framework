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

#include "fakewebosplatform.h"
#include "webosquickwindow.h"

// How WebOSQuickWindow sends its window properties to the shell surface
// (wl_webos_shell_surface.set_property), against the stand-in shell surfaces
// of fakewebosplatform.cpp. Run on any QPA, e.g. QT_QPA_PLATFORM=offscreen.
class TestWindowProperties : public QObject
{
    Q_OBJECT

private:
    // What Qt does on hide: the window loses its shell surface.
    static void hide(WebOSQuickWindow &window)
    {
        window.hide();
        FakeWebOS::dropSurface(&window);
    }

private slots:
    void init()
    {
        FakeWebOS::reset();
    }

    void cleanup()
    {
        FakeWebOS::reset();
    }

    // A value set while the window is shown goes to the surface once, and
    // setting the same value again sends nothing.
    void setWhileVisible_sameValueSentOnce()
    {
        WebOSQuickWindow window;
        window.show();
        QVERIFY(window.isVisible());
        QCOMPARE(FakeWebOS::surfaceCount(), 1);

        window.setWindowProperty("LuneOS_test", "v1");
        window.setWindowProperty("LuneOS_test", "v1");
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_test"), 1);

        window.setWindowProperty("LuneOS_test", "v2");
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_test"), 2);
        QCOMPARE(FakeWebOS::sends(1).last(), FakeWebOS::Send("LuneOS_test", "v2"));
    }

    // An empty value set while the window is shown goes out too, although
    // the surface's QVariant() for a key it never had is empty as well.
    void setWhileVisible_emptyValueSent()
    {
        WebOSQuickWindow window;
        window.show();

        window.setWindowProperty("LuneOS_empty", "");
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_empty"), 1);
        window.setWindowProperty("LuneOS_empty", "");
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_empty"), 1);
    }

    // The first show sends what was set while hidden, once each.
    void firstShow_sendsPendingOnce()
    {
        WebOSQuickWindow window;
        window.setWindowProperty("LuneOS_window", "popupalert");
        window.setWindowProperty("LuneOS_overLockScreen", "true");
        window.show();

        QCOMPARE(FakeWebOS::surfaceCount(), 1);
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_window"), 1);
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_overLockScreen"), 1);
        QCOMPARE(FakeWebOS::sendCount(1, "appId"), 1);
    }

    // The case PR #4 fixed: a window hidden and shown again gets a new shell
    // surface, which must get every property again - those set before the
    // first show and those set while it was shown - once each, latest value.
    void reshow_newSurfaceGetsEveryProperty()
    {
        WebOSQuickWindow window;
        window.setWindowProperty("LuneOS_window", "popupalert");
        window.show();
        window.setWindowProperty("LuneOS_test", "v1");
        hide(window);
        window.setWindowProperty("LuneOS_hidden", "h1");
        window.show();

        QCOMPARE(FakeWebOS::surfaceCount(), 2);
        const QList<FakeWebOS::Send> sent = FakeWebOS::sends(2);
        QVERIFY(sent.contains(FakeWebOS::Send("LuneOS_window", "popupalert")));
        QVERIFY(sent.contains(FakeWebOS::Send("LuneOS_test", "v1")));
        QVERIFY(sent.contains(FakeWebOS::Send("LuneOS_hidden", "h1")));
        QCOMPARE(FakeWebOS::sendCount(2, "LuneOS_window"), 1);
        QCOMPARE(FakeWebOS::sendCount(2, "LuneOS_test"), 1);
        QCOMPARE(FakeWebOS::sendCount(2, "LuneOS_hidden"), 1);
        QCOMPARE(FakeWebOS::sendCount(2, "appId"), 1);
    }

    // An empty value is still a value: a new surface gets it too, although
    // QVariant().toString() is empty as well.
    void reshow_emptyValueSentToNewSurface()
    {
        WebOSQuickWindow window;
        window.setWindowProperty("LuneOS_empty", "");
        window.show();
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_empty"), 1);

        hide(window);
        window.show();
        QCOMPARE(FakeWebOS::surfaceCount(), 2);
        QCOMPARE(FakeWebOS::sendCount(2, "LuneOS_empty"), 1);
        QVERIFY(FakeWebOS::sends(2).contains(FakeWebOS::Send("LuneOS_empty", "")));
    }

    // Should a platform keep the shell surface across hide and show, only
    // what changed while hidden goes out again.
    void reshow_sameSurfaceSendsOnlyChanges()
    {
        WebOSQuickWindow window;
        window.setWindowProperty("LuneOS_window", "popupalert");
        window.show();
        window.setWindowProperty("LuneOS_test", "v1");
        window.hide();
        window.setWindowProperty("LuneOS_test", "v2");
        window.show();

        QCOMPARE(FakeWebOS::surfaceCount(), 1);
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_window"), 1);
        QCOMPARE(FakeWebOS::sendCount(1, "appId"), 1);
        QCOMPARE(FakeWebOS::sendCount(1, "LuneOS_test"), 2);
        QCOMPARE(window.isVisible(), true);
    }
};

QTEST_MAIN(TestWindowProperties)

#include "tst_windowproperties.moc"

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

#include "eosregion.h"
#include "eosregionrect.h"

// Tests for the EosRect/EosRegion input-region types that back the
// WebOSWindow.inputRegion QML property.
class TestEosRegion : public QObject
{
    Q_OBJECT

private slots:
    void rect_defaultIsInvalidAndReturnsEmpty()
    {
        EosRect rect;
        QVERIFY(!rect.rect().isValid());
        QCOMPARE(rect.rect(), QRect());
    }

    void rect_returnsConfiguredGeometry()
    {
        EosRect rect;
        rect.setX(10);
        rect.setY(20);
        rect.setWidth(300);
        rect.setHeight(400);
        rect.componentComplete();
        QCOMPARE(rect.rect(), QRect(10, 20, 300, 400));
    }

    void rect_invalidGeometryReturnsEmptyRect()
    {
        EosRect rect;
        rect.setX(10);
        rect.setY(20);
        rect.setWidth(-5);
        rect.setHeight(400);
        QCOMPARE(rect.rect(), QRect());
    }

    void region_emptyWithoutRects()
    {
        EosRegion region;
        region.componentComplete();
        QVERIFY(region.region().isEmpty());
    }

    void region_combinesRects()
    {
        EosRegion region;

        EosRect *a = new EosRect(&region);
        a->setX(0); a->setY(0); a->setWidth(100); a->setHeight(100);
        EosRect *b = new EosRect(&region);
        b->setX(200); b->setY(0); b->setWidth(50); b->setHeight(50);

        QQmlListProperty<EosRect> list = region.regionRects();
        list.append(&list, a);
        list.append(&list, b);
        region.componentComplete();

        const QRegion r = region.region();
        QVERIFY(r.contains(QPoint(50, 50)));
        QVERIFY(r.contains(QPoint(220, 20)));
        QVERIFY(!r.contains(QPoint(150, 50))); // gap between the rects
        QCOMPARE(r.boundingRect(), QRect(0, 0, 250, 100));
    }

    void region_overlappingRectsMerge()
    {
        EosRegion region;

        EosRect *a = new EosRect(&region);
        a->setX(0); a->setY(0); a->setWidth(100); a->setHeight(100);
        EosRect *b = new EosRect(&region);
        b->setX(50); b->setY(0); b->setWidth(100); b->setHeight(100);

        QQmlListProperty<EosRect> list = region.regionRects();
        list.append(&list, a);
        list.append(&list, b);
        region.componentComplete();

        QCOMPARE(region.region().boundingRect(), QRect(0, 0, 150, 100));
        QCOMPARE(region.region().rectCount(), 1);
    }
};

QTEST_APPLESS_MAIN(TestEosRegion)
#include "tst_eosregion.moc"

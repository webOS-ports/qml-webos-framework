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

#include "securecoding.h"

// Unit tests for the clamping conversion helpers in
// include/securecoding.h. They are used from the render geometry code,
// so a silent change in clamping behaviour would show up as visual
// corruption at best and out-of-range index writes at worst.
class TestSecureCoding : public QObject
{
    Q_OBJECT

private slots:
    void uint2int_passesThroughSmallValues()
    {
        QCOMPARE(uint2int(0u), 0);
        QCOMPARE(uint2int(1234u), 1234);
        QCOMPARE(uint2int(static_cast<uint32_t>(INT_MAX)), INT_MAX);
    }

    void uint2int_clampsAboveIntMax()
    {
        QCOMPARE(uint2int(static_cast<uint32_t>(INT_MAX) + 1u), INT_MAX);
        QCOMPARE(uint2int(UINT32_MAX), INT_MAX);
    }

    void int2uint_passesThroughNonNegative()
    {
        QCOMPARE(int2uint(0), 0u);
        QCOMPARE(int2uint(INT_MAX), static_cast<uint32_t>(INT_MAX));
    }

    void int2uint_clampsNegativeToZero()
    {
        QCOMPARE(int2uint(-1), 0u);
        QCOMPARE(int2uint(INT_MIN), 0u);
    }

    void int2ushort_passesThroughInRange()
    {
        QCOMPARE(int2ushort(0), static_cast<uint16_t>(0));
        QCOMPARE(int2ushort(USHRT_MAX), static_cast<uint16_t>(USHRT_MAX));
    }

    void int2ushort_clampsOutOfRange()
    {
        QCOMPARE(int2ushort(USHRT_MAX + 1), static_cast<uint16_t>(USHRT_MAX));
        QCOMPARE(int2ushort(INT_MAX), static_cast<uint16_t>(USHRT_MAX));
        QCOMPARE(int2ushort(-1), static_cast<uint16_t>(0));
        QCOMPARE(int2ushort(INT_MIN), static_cast<uint16_t>(0));
    }

    void multiplicationInt_normalProducts()
    {
        QCOMPARE(multiplicationInt(0, 12345), 0);
        QCOMPARE(multiplicationInt(6, 7), 42);
        QCOMPARE(multiplicationInt(-6, 7), -42);
        QCOMPARE(multiplicationInt(46340, 46340), 46340 * 46340); // just below INT_MAX
    }

    void multiplicationInt_clampsOverflow()
    {
        QCOMPARE(multiplicationInt(INT_MAX, 2), INT_MAX);
        QCOMPARE(multiplicationInt(65536, 65536), INT_MAX);
        QCOMPARE(multiplicationInt(INT_MAX, INT_MAX), INT_MAX);
        QCOMPARE(multiplicationInt(INT_MIN, 2), INT_MIN);
        QCOMPARE(multiplicationInt(INT_MAX, -2), INT_MIN);
    }

    void checkIntUpper_clampsAboveIntMax()
    {
        QCOMPARE(checkIntUpper(static_cast<int64_t>(INT_MAX)), INT_MAX);
        QCOMPARE(checkIntUpper(static_cast<int64_t>(INT_MAX) + 1), INT_MAX);
        QCOMPARE(checkIntUpper(INT64_MAX), INT_MAX);
        QCOMPARE(checkIntUpper(42), 42);
    }

    void checkIntLower_clampsBelowIntMin()
    {
        QCOMPARE(checkIntLower(static_cast<int64_t>(INT_MIN)), INT_MIN);
        QCOMPARE(checkIntLower(static_cast<int64_t>(INT_MIN) - 1), INT_MIN);
        QCOMPARE(checkIntLower(INT64_MIN), INT_MIN);
        QCOMPARE(checkIntLower(-42), -42);
    }

    void macros_forwardWithCast()
    {
        // The checkIntMax/checkIntMin macros must accept any integer type.
        QCOMPARE(checkIntMax(static_cast<uint64_t>(INT_MAX) + 1u), INT_MAX);
        QCOMPARE(checkIntMin(static_cast<int64_t>(INT_MIN) - 1), INT_MIN);
    }
};

QTEST_APPLESS_MAIN(TestSecureCoding)
#include "tst_securecoding.moc"

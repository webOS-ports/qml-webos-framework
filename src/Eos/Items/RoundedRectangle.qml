// Copyright (c) 2015-2018 LG Electronics, Inc.
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

import QtQuick

// A rectangle with each corner rounded or square as asked. This used to be built
// from four corner pieces cut out with Qt5Compat's OpacityMask; Rectangle has
// taken a radius per corner since Qt 6.7, so it is one Rectangle now.
Rectangle {
    id: root

    radius: 50

    property bool clipTopLeft: true
    property bool clipTopRight: true
    property bool clipBottomLeft: true
    property bool clipBottomRight: true

    // The corners never get rounder than a half of the height
    readonly property real cappedRadius: Math.min(root.radius, root.height / 2)

    topLeftRadius: clipTopLeft ? cappedRadius : 0
    topRightRadius: clipTopRight ? cappedRadius : 0
    bottomLeftRadius: clipBottomLeft ? cappedRadius : 0
    bottomRightRadius: clipBottomRight ? cappedRadius : 0
}

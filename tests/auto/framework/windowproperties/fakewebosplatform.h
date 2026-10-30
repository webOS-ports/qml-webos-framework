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

#ifndef FAKEWEBOSPLATFORM_H
#define FAKEWEBOSPLATFORM_H

#include <QList>
#include <QPair>
#include <QString>

class QWindow;

// Control of the stand-ins for webos-platform-interface that
// fakewebosplatform.cpp links into the test in place of the real library.
// Each shell surface keeps the properties sent on it, as
// WebOSShellSurfacePrivate does, and logs every send.
namespace FakeWebOS {

using Send = QPair<QString, QString>;

// Shell surfaces made so far, numbered from 1 in the order they were made.
int surfaceCount();

// Every property sent on surface `surface`, in order.
QList<Send> sends(int surface);

// How often `key` was sent on surface `surface`.
int sendCount(int surface, const QString &key);

// What Qt 6.12's QWaylandWindow does when the window is hidden: delete its
// shell surface. The next shellSurfaceFor() makes a new, empty one, as
// WebOSShellPrivate::preCreateShellSurface() does.
void dropSurface(QWindow *window);

// Forget every surface and send.
void reset();

} // namespace FakeWebOS

#endif // FAKEWEBOSPLATFORM_H

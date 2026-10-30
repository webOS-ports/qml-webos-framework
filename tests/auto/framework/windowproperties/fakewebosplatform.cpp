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

// Stand-ins for the parts of webos-platform-interface that WebOSQuickWindow
// uses, so the test needs no Wayland compositor. The declarations are the
// library's own headers; only the definitions are replaced. The library
// itself is not linked.

#include "fakewebosplatform.h"

#include <QHash>
#include <QRegion>
#include <QVariantMap>
#include <QWindow>

#include <webosinputmanager.h>
#include <webosplatform.h>
#include <webosshell.h>
#include <webosshellsurface.h>

// The library's private classes are only declared in its public headers;
// these are the test's own.
class WebOSShellSurfacePrivate
{
public:
    int number = 0;
    QVariantMap properties;
};

class WebOSShellPrivate
{
public:
    static WebOSShellSurface *newSurface(int number)
    {
        auto *surface = new WebOSShellSurface(nullptr, nullptr, nullptr);
        surface->d_ptr->number = number;
        return surface;
    }
};

class WebOSInputManagerPrivate
{
};

class WebOSPlatformPrivate
{
public:
    static WebOSShell *newShell() { return new WebOSShell(nullptr, 0); }
};

namespace {

QHash<QWindow *, WebOSShellSurface *> s_surfaces;
QList<QList<FakeWebOS::Send>> s_sends;
WebOSShell *s_shell = nullptr;

} // namespace

// --- WebOSPlatform --------------------------------------------------------

WebOSPlatform *WebOSPlatform::m_instance = nullptr;

WebOSPlatform::WebOSPlatform() {}
WebOSPlatform::~WebOSPlatform() {}

WebOSPlatform *WebOSPlatform::instance()
{
    if (!m_instance)
        m_instance = new WebOSPlatform();
    return m_instance;
}

WebOSShell *WebOSPlatform::shell()
{
    if (!s_shell)
        s_shell = WebOSPlatformPrivate::newShell();
    return s_shell;
}

WebOSInputManager *WebOSPlatform::inputManager()
{
    return nullptr;
}

// --- WebOSInputManager ---------------------------------------------------
// Never made: inputManager() answers nullptr, as on a platform without one.
// Defined for its vtable and typeinfo only.

WebOSInputManager::~WebOSInputManager() {}

// --- WebOSShell -----------------------------------------------------------

WebOSShell::WebOSShell(QWaylandDisplay *, uint32_t) {}
WebOSShell::~WebOSShell() {}

WebOSShellSurface *WebOSShell::shellSurfaceFor(QWindow *window)
{
    // As the real one: no platform window, no shell surface.
    if (!window || !window->handle())
        return nullptr;

    WebOSShellSurface *&surface = s_surfaces[window];
    if (!surface) {
        s_sends.append(QList<FakeWebOS::Send>());
        surface = WebOSShellPrivate::newSurface(static_cast<int>(s_sends.size()));
    }
    return surface;
}

// --- WebOSShellSurface ----------------------------------------------------

WebOSShellSurface::WebOSShellSurface(wl_webos_shell_surface *, struct ::wl_shell_surface *,
                                     QPlatformWindow *)
    : d_ptr(new WebOSShellSurfacePrivate)
{
}

WebOSShellSurface::~WebOSShellSurface()
{
    delete d_ptr;
}

QVariant WebOSShellSurface::property(const QString &name) const
{
    return d_ptr->properties.value(name);
}

// As WebOSShellSurfacePrivate::setProperty(): keep the value, then send it.
void WebOSShellSurface::setProperty(const QString &name, const QVariant &value)
{
    d_ptr->properties.insert(name, value);
    s_sends[d_ptr->number - 1].append({name, value.toString()});
}

QPointF WebOSShellSurface::position()
{
    return QPointF();
}
void WebOSShellSurface::setState(Qt::WindowStates) {}
WebOSShellSurface::LocationHints WebOSShellSurface::locationHint()
{
    return LocationHintUnknown;
}
void WebOSShellSurface::setLocationHint(LocationHints) {}
void WebOSShellSurface::setKeyMask(KeyMasks) {}
void WebOSShellSurface::setInputRegion(const QRegion &) {}
QString WebOSShellSurface::addon()
{
    return QString();
}
void WebOSShellSurface::setAddon(const QString &) {}
void WebOSShellSurface::resetAddon() {}

// --- test control ---------------------------------------------------------

namespace FakeWebOS {

int surfaceCount()
{
    return static_cast<int>(s_sends.size());
}

QList<Send> sends(int surface)
{
    return s_sends.value(surface - 1);
}

int sendCount(int surface, const QString &key)
{
    int count = 0;
    for (const Send &send : s_sends.value(surface - 1))
        if (send.first == key)
            ++count;
    return count;
}

void dropSurface(QWindow *window)
{
    delete s_surfaces.take(window);
}

void reset()
{
    qDeleteAll(s_surfaces);
    s_surfaces.clear();
    s_sends.clear();
}

} // namespace FakeWebOS

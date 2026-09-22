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

# Build hardening for every target in this tree (pulled in through
# .qmake.conf). The flags mirror what the OpenEmbedded security class
# already passes so OE builds see no change; they exist so that plain
# qmake builds (desktop development, CI) get the same protection.
#
# Opt out with: qmake QWF_NO_HARDENING=1
# Sanitizer builds: qmake CONFIG+=sanitizer CONFIG+=sanitize_address
# (qmake's own sanitize feature; combine with QWF_NO_HARDENING=1 if
# _FORTIFY_SOURCE gets in the way of ASan interceptors.)

isEmpty(QWF_NO_HARDENING) {
    HARDEN_FLAGS = -fstack-protector-strong -Wformat -Wformat-security

    QMAKE_CFLAGS += $$HARDEN_FLAGS
    QMAKE_CXXFLAGS += $$HARDEN_FLAGS

    # FORTIFY needs optimization; only add it to optimized builds. The
    # -U first keeps it a clean redefinition when the environment (OE)
    # already passes its own -D_FORTIFY_SOURCE.
    QMAKE_CFLAGS_RELEASE += -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2
    QMAKE_CXXFLAGS_RELEASE += -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2

    # Hardened libstdc++ precondition checks (cheap, ABI-stable).
    DEFINES += _GLIBCXX_ASSERTIONS

    # Full RELRO and a non-executable stack for everything we link.
    QMAKE_LFLAGS += -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack
}

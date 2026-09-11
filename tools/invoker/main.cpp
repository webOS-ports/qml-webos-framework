// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#include <QtCore/QCoreApplication>
#include <QtCore/QScopedPointer>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/QDataStream>

#include <QtNetwork/QLocalSocket>

#include <QtCore/QDebug>

#include "../common/ipccommon.h"

// Small debug tool: asks a running boosterd to launch a QML app from
// its pre-started runner pool, bypassing LS2/SAM. Usage:
//   invoker --main /path/to/main.qml
//   invoker '{"appId":"com.test","main":"file:///path/main.qml"}'
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString mainQml("");
    QString appId("com.webos.invoker");
    QJsonObject params;

    QStringList allArgs = QCoreApplication::arguments();
    for (int i = 0; i < allArgs.size(); i++) {
        const QString arg = allArgs.at(i);
        if (arg.startsWith('{')) {
            QJsonObject obj = QJsonDocument::fromJson(arg.toUtf8()).object();
            if (obj.contains("main"))
                mainQml = obj.value("main").toString();
            if (obj.contains("appId"))
                appId = obj.value("appId").toString();
            if (obj.contains("params"))
                params = obj.value("params").toObject();
        }
        if (arg == "--main" && (i + 1 < allArgs.size())) {
            mainQml = allArgs.at(i + 1);
            ++i;
            continue;
        }
    }

    if (mainQml.isEmpty()) {
        qWarning("No QML file given. Pass --main <file> or a launch JSON object.");
        return -1;
    }

    QScopedPointer<QLocalSocket> socket (new QLocalSocket());

    socket->connectToServer("EosBooster");
    if (!socket->waitForConnected()) {
        qWarning() << "Cannot reach boosterd:" << socket->error();
        return -1;
    }

    // Same wire format the runners use: a QDataStream-framed QByteArray
    // holding a compact JSON message (see tools/booster/ipcserver.cpp).
    QJsonObject message;
    message.insert(QStringLiteral("header"), LAUNCH_REQUEST);
    message.insert(QStringLiteral("appId"), appId);
    message.insert(QStringLiteral("mainQml"), mainQml);
    message.insert(QStringLiteral("params"), params);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out << QJsonDocument(message).toJson(QJsonDocument::Compact);
#else
    out << QJsonDocument(message).toBinaryData();
#endif
    socket->write(block);
    socket->flush();
    // flush() may already have drained the buffer; only wait if bytes
    // are still pending, so we don't warn on a successful send.
    if (socket->bytesToWrite() > 0 && !socket->waitForBytesWritten(3000)) {
        qWarning("Timed out handing the launch request to boosterd.");
        return -1;
    }
    socket->disconnectFromServer();

    return 0;
}

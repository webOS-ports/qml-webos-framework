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

#include "lunaservice.h"

#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

#define LS_SERVICE "com.webos.booster"
#define LS_METHODS_CATEGORY "/"
#define LS_SIGNALS_CATEGORY "/booster"

namespace {

void reply_error(const LunaServiceMessage &msg, const QString &message)
{
    qWarning("%s", qPrintable(message));
    QJsonObject reply;
    reply.insert(QStringLiteral("returnValue"), false);
    reply.insert(QStringLiteral("errorCode"), -1);
    reply.insert(QStringLiteral("errorText"), message);
    msg.respond(QJsonDocument(reply));
}

void reply_msg(const LunaServiceMessage &msg, const QString &appId, qint64 pid)
{
    QJsonObject reply;
    reply.insert(QStringLiteral("returnValue"), true);
    reply.insert(QStringLiteral("appId"), appId);
    reply.insert(QStringLiteral("pid"), pid);
    msg.respond(QJsonDocument(reply));
}

}

LunaService::LunaService(IpcServer *server, LaunchManager *launchManager, QObject *parent) :
    AbstractLunaService (parent),
    m_ipcServer (server),
    m_launchManager (launchManager)
{
    registerService(LS_SERVICE, LS_METHODS_CATEGORY, LS_SIGNALS_CATEGORY);
}

void LunaService::launch(const LunaServiceMessage &msg)
{
    const QJsonDocument &params = msg.payload();
    if (!params.isObject()) {
        reply_error(msg, QStringLiteral("Invalid message payload"));
        return;
    }

    const QJsonObject &obj = params.object();
    const QString &appId = obj.value(QStringLiteral("appId")).toString();
    const QString &mainQml = obj.value(QStringLiteral("main")).toString();

    const QJsonValue &paramsValue = obj.value(QStringLiteral("params"));
    const QJsonDocument &appParams = paramsValue.isString() ?
                QJsonDocument::fromJson(paramsValue.toString().toUtf8()) : // FIXME: SAM double-serialize params
                QJsonDocument(paramsValue.toObject());

    qint64 old_pid = m_ipcServer->runningApps().value(appId);
    if (old_pid) {
        m_ipcServer->relaunch(appId, appParams, [this, appId, old_pid, msg](qint64 pid) {
            if (pid) {
                reply_msg(msg, appId, pid);
                update();
            } else {
                reply_error(msg, QStringLiteral("Relaunching QML App %1 failed.").arg(appId));
                // The callback reports failure with pid 0; the process to
                // put down is the one that failed to handle the relaunch.
                m_launchManager->terminate(old_pid);
            }
        });
        return;
    }

    m_ipcServer->launch(appId, mainQml, appParams, [this, appId, msg](qint64 pid) {
        if (!pid) {
            reply_error(msg, QStringLiteral("Launching qml-runner failed."));
            return;
        }

        reply_msg(msg, appId, pid);
        update();
    });
}

void LunaService::close(const LunaServiceMessage &msg)
{
    const QJsonDocument &params = msg.payload();
    if (!params.isObject()) {
        reply_error(msg, QStringLiteral("Malformed JSON document."));
        return;
    }

    const QJsonObject &obj = params.object();
    const QString &appId = obj.value(QStringLiteral("appId")).toString();
    if (appId.isEmpty()) {
        reply_error(msg, QStringLiteral("The appId is not set."));
        return;
    }

    qint64 pid = m_ipcServer->runningApps().value(appId);
    if (pid == 0) {
        reply_error(msg, QStringLiteral("There is no process for %1").arg(appId));
        return;
    }

    m_launchManager->terminate(pid);
    reply_msg(msg, appId, pid);
}

void LunaService::running(const LunaServiceMessage &msg)
{
    QJsonObject reply = getRunningList();

    const QJsonDocument &params = msg.payload();
    if (params.isObject()) {
        const bool subscribe = params.object().value(QStringLiteral("subscribe")).toBool();
        if (subscribe) {
            subscribeAdd(msg, "/booster/running");
            update(true);
            return;
        }
    }

    msg.respond(QJsonDocument(reply));
}

void LunaService::update(bool firstResponse)
{
    QJsonObject reply = getRunningList();
    if (firstResponse) {
        reply.insert(QStringLiteral("subscribed"), true);
    }
    subscribesReply(QJsonDocument(reply), "/booster/running");
}

QJsonObject LunaService::getRunningList()
{
    QJsonArray running;
    foreach (const QString &appId, m_ipcServer->runningApps().keys()) {
        qint64 pid = m_ipcServer->runningApps().value(appId);
        QJsonObject obj;
        obj.insert(QStringLiteral("appId"), appId);
        obj.insert(QStringLiteral("pid"), pid);
        running.append(obj);
    }

    QJsonObject reply;
    reply.insert(QStringLiteral("returnValue"), true);
    reply.insert(QStringLiteral("running"), running);

    return reply;
}

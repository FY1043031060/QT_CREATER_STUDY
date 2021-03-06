/**************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
**
** Contact: BlackBerry (qt@blackberry.com)
** Contact: KDAB (info@kdab.com)
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "qnxdeviceprocesssignaloperation.h"

using namespace Qnx;
using namespace Qnx::Internal;

QnxDeviceProcessSignalOperation::QnxDeviceProcessSignalOperation(
        const QSsh::SshConnectionParameters sshParameters)
    : RemoteLinux::RemoteLinuxSignalOperation(sshParameters)
{
}

static QString signalProcessByNameQnxCommandLine(const QString &filePath, int sig)
{
    QString executable = filePath;
    return QString::fromLatin1("for PID in $(ps -f -o pid,comm | grep %1 | awk '/%1/ {print $1}'); "
        "do "
            "kill -%2 $PID; "
        "done").arg(executable.replace(QLatin1String("/"), QLatin1String("\\/"))).arg(sig);
}

QString QnxDeviceProcessSignalOperation::killProcessByNameCommandLine(
        const QString &filePath) const
{
    return QString::fromLatin1("%1; %2").arg(signalProcessByNameQnxCommandLine(filePath, 15),
                                           signalProcessByNameQnxCommandLine(filePath, 9));
}

QString QnxDeviceProcessSignalOperation::interruptProcessByNameCommandLine(
        const QString &filePath) const
{
    return signalProcessByNameQnxCommandLine(filePath, 2);
}


BlackBerryDeviceProcessSignalOperation::BlackBerryDeviceProcessSignalOperation(
        const QSsh::SshConnectionParameters sshParameters)
    : RemoteLinux::RemoteLinuxSignalOperation(sshParameters)
{
}

static QString signalProcessByNameBlackBerryCommandLine(const QString &filePath, int sig)
{
    QString executable = filePath;
    return QString::fromLatin1("for PID in $(pidin -F \"%a %A\" | grep \"%1\" | awk '/%1/ {print $1}'); "
        "do "
            "kill -%2 $PID; "
        "done").arg(executable.replace(QLatin1String("/"), QLatin1String("\\/"))).arg(sig);
}

QString BlackBerryDeviceProcessSignalOperation::killProcessByNameCommandLine(const QString &filePath) const
{
    return QString::fromLatin1("%1; %2").arg(signalProcessByNameBlackBerryCommandLine(filePath, 15),
                                                             signalProcessByNameBlackBerryCommandLine(filePath, 9));
}

QString BlackBerryDeviceProcessSignalOperation::interruptProcessByNameCommandLine(const QString &filePath) const
{
    return signalProcessByNameBlackBerryCommandLine(filePath, 2);
}

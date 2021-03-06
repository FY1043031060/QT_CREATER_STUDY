/**************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved
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

#ifndef QNX_INTERNAL_BLACKBERRYDEVICECONFIGURATIONWIZARD_H
#define QNX_INTERNAL_BLACKBERRYDEVICECONFIGURATIONWIZARD_H

#include <utils/wizard.h>

#include <QProcess>

namespace Qnx {
namespace Internal {

class BlackBerryInstallWizardOptionPage;
class BlackBerryInstallWizardNdkPage;
class BlackBerryInstallWizardTargetPage;
class BlackBerryInstallWizardProcessPage;
class BlackBerryInstallWizardFinalPage;

class BlackBerryInstallerDataHandler {
public:
    enum Mode {
        InstallMode,
        UninstallMode,
        ManuallMode
    };

    QString ndkPath;
    QString target;
    QString version;
    int exitCode;
    QProcess::ExitStatus exitStatus;
    Mode mode;
};

class BlackBerryInstallWizard : public Utils::Wizard
{
    Q_OBJECT
public:
    enum PageId {
        OptionPage,
        NdkPageId,
        TargetPageId,
        ProcessPageId,
        FinalPageId
    };

    explicit BlackBerryInstallWizard(BlackBerryInstallerDataHandler::Mode mode = BlackBerryInstallerDataHandler::InstallMode,
                                     const QString& version = QString(),
                                     QWidget *parent = 0);

signals:
    void processFinished();

private slots:
    void handleProcessCancelled();

private:

    BlackBerryInstallWizardOptionPage *m_optionPage;
    BlackBerryInstallWizardNdkPage *m_ndkPage;
    BlackBerryInstallWizardTargetPage *m_targetPage;
    BlackBerryInstallWizardProcessPage *m_processPage;
    BlackBerryInstallWizardFinalPage *m_finalPage;

    BlackBerryInstallerDataHandler m_data;
};

} // namespace Internal
} // namespace Qnx

#endif // QNX_INTERNAL_BLACKBERRYDEVICECONFIGURATIONWIZARD_H

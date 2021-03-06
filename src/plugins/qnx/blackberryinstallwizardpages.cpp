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

#include "blackberryinstallwizardpages.h"
#include "blackberryconfigurationmanager.h"
#include "blackberryconfiguration.h"

#include "ui_blackberryinstallwizardtargetpage.h"
#include "ui_blackberryinstallwizardprocesspage.h"
#include "ui_blackberryinstallwizardndkpage.h"

#include "qnxutils.h"

#include <utils/synchronousprocess.h>
#include <utils/pathchooser.h>

#include <QProcess>

#include <QTreeWidgetItem>

#include <QFileInfo>
#include <QDir>

#include <QMessageBox>
#include <QLayout>
#include <QRadioButton>

namespace Qnx {
namespace Internal {

namespace {
const QLatin1String targetKeyWord("Native SDK");
}

NdkPathChooser::NdkPathChooser(Mode mode, QWidget *parent)
    : Utils::PathChooser(parent)
    , m_mode(mode)
{
    if (m_mode == NdkPathChooser::InstallMode) {
        setExpectedKind(Utils::PathChooser::Directory);
    } else {
        setExpectedKind(Utils::PathChooser::File);
        setPromptDialogFilter(Utils::HostOsInfo::isWindowsHost() ? QLatin1String("*.bat") :
                                                                   QLatin1String("*.sh"));
    }
}

bool NdkPathChooser::validatePath(const QString &path, QString *errorMessage)
{
    bool result = PathChooser::validatePath(path, errorMessage);
    if (!result)
        return false;

    if (m_mode == NdkPathChooser::InstallMode)
        return !(QnxUtils::sdkInstallerPath(path).isEmpty());

    QFileInfo fi(path);
    if (Utils::HostOsInfo::isWindowsHost())
        return fi.suffix() == QLatin1String("bat");

    return fi.suffix() == QLatin1String("sh");
}

//------------------------------------------------------------------

BlackBerryInstallWizardOptionPage::BlackBerryInstallWizardOptionPage(BlackBerryInstallerDataHandler &data,
                                                                     QWidget *parent)
    : QWizardPage(parent)
    , m_layout(new QVBoxLayout(this))
    , m_installButton(new QRadioButton)
    , m_addButton(new QRadioButton)
    , m_envFileChooser(new NdkPathChooser(NdkPathChooser::ManualMode))
    , m_data(data)
{
    setTitle(tr("Options"));
    connect(m_addButton, SIGNAL(toggled(bool)), this, SLOT(handleOptionChanged()));
    connect(m_envFileChooser, SIGNAL(pathChanged(QString)), this, SLOT(handlePathChanged(QString)));
}

void BlackBerryInstallWizardOptionPage::initializePage()
{
    m_installButton->setText(tr("Install New Target"));
    m_addButton->setText(tr("Add Existing Target"));

    if (m_data.mode == BlackBerryInstallerDataHandler::ManuallMode)
        m_addButton->setChecked(true);
    else
        m_installButton->setChecked(true);

    m_envFileChooser->setEnabled(m_addButton->isChecked());

    m_layout->addWidget(m_installButton);
    m_layout->addWidget(m_addButton);
    m_layout->addWidget(m_envFileChooser);
}

bool BlackBerryInstallWizardOptionPage::isComplete() const
{
    return (m_installButton->isChecked()
            || (m_addButton->isChecked() && m_envFileChooser->isValid()));
}

int BlackBerryInstallWizardOptionPage::nextId() const
{
    if (m_addButton->isChecked())
        return BlackBerryInstallWizard::FinalPageId;

    return BlackBerryInstallWizard::NdkPageId;
}

void BlackBerryInstallWizardOptionPage::handleOptionChanged()
{
    if (m_addButton->isChecked())
        m_data.mode = BlackBerryInstallerDataHandler::ManuallMode;
    else
        m_data.mode = BlackBerryInstallerDataHandler::InstallMode;

    m_envFileChooser->setEnabled(m_addButton->isChecked());
    emit completeChanged();
}

void BlackBerryInstallWizardOptionPage::handlePathChanged(const QString &envFilePath)
{
    if (m_envFileChooser->isValid())
        m_data.ndkPath = envFilePath;

    emit completeChanged();
}

//------------------------------------------------------------------

BlackBerryInstallWizardNdkPage::BlackBerryInstallWizardNdkPage(BlackBerryInstallerDataHandler &data, QWidget *parent)
    : QWizardPage(parent)
    , m_ui(new Ui_BlackBerryInstallWizardNdkPage)
    , m_data(data)
    , m_ndkPathChooser(new NdkPathChooser(NdkPathChooser::InstallMode))
    , m_manual(new QListWidgetItem)
    , m_validNdkPath(false)
{
    m_ui->setupUi(this);
    setTitle(tr("Native SDK"));
    m_ui->verticalLayout->addWidget(m_ndkPathChooser);
    connect(m_ui->ndkPathListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(setNdkPath()));
    connect(m_ndkPathChooser, SIGNAL(pathChanged(QString)), this, SLOT(setManualNdkPath()));
}

BlackBerryInstallWizardNdkPage::~BlackBerryInstallWizardNdkPage()
{
    delete m_ui;
}

void BlackBerryInstallWizardNdkPage::initializePage()
{
    m_manual->setText(tr("Specify 10.2 NDK path manually"));
    m_ui->ndkPathListWidget->addItem(m_manual);
    m_manual->setSelected(true);
    QFont font;
    font.setItalic(true);
    m_manual->setFont(font);
    foreach (const NdkInstallInformation &ndk, QnxUtils::installedNdks()) {
        bool found = false;
        for (int i = 0; i < m_ui->ndkPathListWidget->count(); i++) {
            QListWidgetItem* item = m_ui->ndkPathListWidget->item(i);
            if (item->text() == ndk.path) {
                found = true;
                break;
            }
        }

        if (found)
            continue;

        if (!QnxUtils::sdkInstallerPath(ndk.path).isEmpty()) {
            QListWidgetItem *ndkItem = new QListWidgetItem(m_ui->ndkPathListWidget);
            ndkItem->setText(ndk.path);
        }
    }
}

void BlackBerryInstallWizardNdkPage::setNdkPath()
{
    if (m_ui->ndkPathListWidget->selectedItems().isEmpty())
        return;

    m_ndkPathChooser->setEnabled(m_manual->isSelected());
    QString selected = m_ui->ndkPathListWidget->selectedItems().first()->text();
    if (!QnxUtils::sdkInstallerPath(selected).isEmpty()) {
        m_validNdkPath = true;
        m_data.ndkPath = selected;
    } else {
        m_validNdkPath = false;
    }

    emit completeChanged();
}

void BlackBerryInstallWizardNdkPage::setManualNdkPath()
{
    if (m_ndkPathChooser->isValid()) {
        m_validNdkPath = true;
        m_data.ndkPath = m_ndkPathChooser->path();
    } else {
        m_validNdkPath = false;
    }

    emit completeChanged();
}

bool BlackBerryInstallWizardNdkPage::isComplete() const
{
    return m_validNdkPath;
}

//------------------------------------------------------------------

BlackBerryInstallWizardTargetPage::BlackBerryInstallWizardTargetPage(BlackBerryInstallerDataHandler &data,
                                                                     QWidget *parent)
    : QWizardPage(parent)
    , m_data(data)
    , m_ui(new Ui_BlackBerryInstallWizardTargetPage)
    , m_isTargetValid(false)
    , m_targetListProcess(new QProcess(this))
{
    m_ui->setupUi(this);
    setTitle(tr("Target"));

    connect(m_targetListProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(targetsListProcessFinished()));
    connect(m_ui->targetsTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(setTarget()));
}

BlackBerryInstallWizardTargetPage::~BlackBerryInstallWizardTargetPage()
{
    Utils::SynchronousProcess::stopProcess(*m_targetListProcess);
    delete m_ui;
}

void BlackBerryInstallWizardTargetPage::initializePage()
{
    // process may be running if going back and forth
    if (m_targetListProcess->state() == QProcess::Running) {
        disconnect(m_targetListProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                   this, SLOT(targetsListProcessFinished()));
        Utils::SynchronousProcess::stopProcess(*m_targetListProcess);
        connect(m_targetListProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(targetsListProcessFinished()));
    }

    updateAvailableTargetsList();
}

bool BlackBerryInstallWizardTargetPage::isComplete() const
{
    return m_isTargetValid;
}

bool BlackBerryInstallWizardTargetPage::isProcessRunning() const
{
    return (m_targetListProcess->state() == QProcess::Running);
}

void BlackBerryInstallWizardTargetPage::targetsListProcessFinished()
{
    initTargetsTreeWidget();
    QString output = Utils::SynchronousProcess::normalizeNewlines(QString::fromLatin1(m_targetListProcess->readAll()));
    QList<QString> targetList = output.split(QLatin1Char('\n'));
    m_ui->targetsTreeWidget->clear();
    foreach (const QString &target, targetList) {
        if (!target.isEmpty() && target.contains(targetKeyWord)) {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->targetsTreeWidget);
            QStringList res = target.split(QLatin1Char('-'));
            if (res.count() > 1) {
                item->setText(0, res.at(0));
                item->setText(1, res.at(1));
            }
        }
    }

    m_ui->targetsTreeWidget->sortByColumn(0, Qt::DescendingOrder);

}

void BlackBerryInstallWizardTargetPage::setTarget()
{
    if (m_ui->targetsTreeWidget->selectedItems().isEmpty())
        return;

    QString version = m_ui->targetsTreeWidget->selectedItems().first()->text(0);
    QString target = m_ui->targetsTreeWidget->selectedItems().first()->text(1);
    if (target.contains(targetKeyWord)) {
        m_data.target = target;
        m_data.version = version;
        m_isTargetValid = true;
    } else {
        m_isTargetValid = false;
    }

    emit completeChanged();
}

void BlackBerryInstallWizardTargetPage::initTargetsTreeWidget()
{
    m_ui->targetsTreeWidget->clear();
    m_ui->targetsTreeWidget->setHeaderHidden(false);
    m_ui->targetsTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_ui->targetsTreeWidget->setHeaderItem(new QTreeWidgetItem(QStringList() << tr("Version") << tr("Target")));
    m_ui->targetsTreeWidget->setTextElideMode(Qt::ElideNone);
    m_ui->targetsTreeWidget->setColumnCount(2);
}

void BlackBerryInstallWizardTargetPage::updateAvailableTargetsList()
{
    m_ui->targetsTreeWidget->clear();
    m_ui->targetsTreeWidget->setHeaderHidden(true);
    QTreeWidgetItem *item =  new QTreeWidgetItem(m_ui->targetsTreeWidget);
    item->setText(0, tr("Querying available targets. Please wait..."));
    QFont font;
    font.setItalic(true);
    item->setFont(0, font);
    QString qdeProcess = QnxUtils::qdeInstallProcess(m_data.ndkPath, QLatin1String(" --list"));
    QTC_ASSERT(!qdeProcess.isEmpty(), return);
    m_targetListProcess->start(qdeProcess);
}

//------------------------------------------------------------------

BlackBerryInstallWizardProcessPage::BlackBerryInstallWizardProcessPage(BlackBerryInstallerDataHandler &data,
                                                                       QWidget *parent)
    : QWizardPage(parent)
    , m_ui(new Ui_BlackBerryInstallWizardProcessPage)
    , m_data(data)
    , m_targetProcess(new QProcess(this))
{
    m_ui->setupUi(this);
    if (m_data.mode == BlackBerryInstallerDataHandler::UninstallMode)
        setTitle(tr("Uninstalling"));
    else
        setTitle(tr("Installing"));

    connect(m_targetProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(handleProcessFinished(int,QProcess::ExitStatus)));
}

BlackBerryInstallWizardProcessPage::~BlackBerryInstallWizardProcessPage()
{
    Utils::SynchronousProcess::stopProcess(*m_targetProcess);
    delete m_ui;
}

void BlackBerryInstallWizardProcessPage::initializePage()
{
    if (m_data.mode == BlackBerryInstallerDataHandler::UninstallMode) {
        if (m_data.version.isEmpty()) {
            wizard()->next();
            return;
        }

        foreach (const NdkInstallInformation &ndk, QnxUtils::installedNdks()) {
            if (ndk.version == m_data.version) {
                m_data.ndkPath = ndk.path;
                m_data.target = ndk.name;
                break;
            }
        }

        m_ui->label->setText(tr("Uninstalling target:") + QLatin1Char('\n') + m_data.target);
    } else {
        m_ui->label->setText(tr("Installing target:") + QLatin1Char('\n') + m_data.target);
    }
    // m_targetProcess could be running
    if (m_targetProcess->state() == QProcess::Running) {
        disconnect(m_targetProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                   this, SLOT(handleProcessFinished(int,QProcess::ExitStatus)));
        Utils::SynchronousProcess::stopProcess(*m_targetProcess);
        connect(m_targetProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(handleProcessFinished(int,QProcess::ExitStatus)));
    }

    processTarget();
}

bool BlackBerryInstallWizardProcessPage::isComplete() const
{
    return false;
}

bool BlackBerryInstallWizardProcessPage::isProcessRunning() const
{
    return (m_targetProcess->state() == QProcess::Running);
}

void BlackBerryInstallWizardProcessPage::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_data.exitCode = exitCode;
    m_data.exitStatus = exitStatus;

    if (wizard()->currentPage() == this)
        wizard()->next();
}

void BlackBerryInstallWizardProcessPage::processTarget()
{
    QString option;
    if (m_data.mode == BlackBerryInstallerDataHandler::UninstallMode)
        option = QLatin1String(" --uninstall");
    else
        option = QLatin1String(" --install");

    QString version = m_data.version;
    QTC_ASSERT(!version.isEmpty(), return);

    // deactivate target if activated before uninstalling
    if (m_data.mode == BlackBerryInstallerDataHandler::UninstallMode) {
        foreach (BlackBerryConfiguration *config, BlackBerryConfigurationManager::instance().configurations()) {
            if (m_data.target.contains((config->targetName())) && config->isActive()) {
                config->deactivate();
                break;
            }
        }
    }

    // Killing the sdkinstall process won't kill the qde process it launched
    // thus, let's directly launch the resulting qde process
    QString qdeProcess = QnxUtils::qdeInstallProcess(m_data.ndkPath, option, version);
    QTC_ASSERT(!qdeProcess.isEmpty(), return);
    m_targetProcess->start(qdeProcess);

    m_ui->progressBar->setMaximum(0);
    m_ui->progressBar->setMinimum(0);
    m_ui->progressBar->setValue(0);
}

// --------------------------------------------------------------------------------

BlackBerryInstallWizardFinalPage::BlackBerryInstallWizardFinalPage(BlackBerryInstallerDataHandler &data,
                                                                   QWidget *parent)
    : QWizardPage(parent)
    , m_data(data)
{
    setTitle(tr("Summary"));
}

void BlackBerryInstallWizardFinalPage::initializePage()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel(this);
    layout->addWidget(label);

    if (m_data.mode == BlackBerryInstallerDataHandler::ManuallMode) {
        BlackBerryConfigurationManager  &configManager = BlackBerryConfigurationManager::instance();
        BlackBerryConfiguration *config = configManager.configurationFromEnvFile(Utils::FileName::fromString(m_data.ndkPath));

        if (!config) {
            config = new BlackBerryConfiguration(Utils::FileName::fromString(m_data.ndkPath), false);
            if (!configManager.addConfiguration(config)) {
                delete config;
                // TODO: more explicit error message!
                label->setText(tr("An error has occurred while adding target from:\n %1").arg(m_data.ndkPath));
                return;
            }

            label->setText(tr("Target %1 is being added.").arg(m_data.ndkPath));
            emit done();
            return;
        } else {
            label->setText(tr("Target %1 is already added.").arg(m_data.ndkPath));
            return;
        }
    }

    QString message;
    if (m_data.exitCode == 0 && m_data.exitStatus == QProcess::NormalExit) {
        message = m_data.mode == BlackBerryInstallerDataHandler::UninstallMode ?
            tr("Finished uninstalling target:\n %1").arg(m_data.target) :
            tr("Finished installing target:\n %1").arg(m_data.target);
        emit done();
    } else {
        message = m_data.mode == BlackBerryInstallerDataHandler::UninstallMode ?
            tr("An error has occurred while uninstalling target:\n %1").arg(m_data.target) :
            tr("An error has occurred while installing target:\n %1").arg(m_data.target);
    }
    label->setText(message);
}

} // namespace Internal
} // namespace Qnx

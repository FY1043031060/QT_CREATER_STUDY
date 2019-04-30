/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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

#ifndef BASEQTVERSION_H
#define BASEQTVERSION_H

#include "qtsupport_global.h"

#include <utils/fileutils.h>

#include <projectexplorer/abi.h>

#include <QStringList>
#include <QVariantMap>

namespace Utils {
class Environment;
} // namespace Utils

namespace Core {
class FeatureSet;
} // namespace Core

namespace ProjectExplorer {
class IOutputParser;
class Kit;
class ToolChain;
class HeaderPath;
class Task;
} // namespace ProjectExplorer

QT_BEGIN_NAMESPACE
class ProFileEvaluator;
class QSettings;
QT_END_NAMESPACE

namespace QtSupport
{
class QtConfigWidget;

class QTSUPPORT_EXPORT QtVersionNumber
{
public:
    QtVersionNumber(int ma, int mi, int p);
    QtVersionNumber(const QString &versionString);
    QtVersionNumber();

    int majorVersion;
    int minorVersion;
    int patchVersion;
    bool operator <(const QtVersionNumber &b) const;
    bool operator <=(const QtVersionNumber &b) const;
    bool operator >(const QtVersionNumber &b) const;
    bool operator >=(const QtVersionNumber &b) const;
    bool operator !=(const QtVersionNumber &b) const;
    bool operator ==(const QtVersionNumber &b) const;
};

namespace Internal { class QtOptionsPageWidget; }

class QTSUPPORT_EXPORT BaseQtVersion
{
    friend class QtVersionFactory;
    friend class QtVersionManager;
    friend class QtSupport::Internal::QtOptionsPageWidget;
public:
    virtual ~BaseQtVersion();

    virtual void fromMap(const QVariantMap &map);
    virtual BaseQtVersion *clone() const = 0;
    virtual bool equals(BaseQtVersion *other);

    bool isAutodetected() const;
    QString autodetectionSource() const;

    QString displayName() const;
    void setDisplayName(const QString &name);

    // All valid Ids are >= 0
    int uniqueId() const;

    virtual QString type() const = 0;

    virtual QVariantMap toMap() const;
    virtual bool isValid() const;
    virtual QString invalidReason() const;
    virtual QStringList warningReason() const;

    virtual ProjectExplorer::ToolChain *preferredToolChain(const Utils::FileName &ms) const;

    virtual QString description() const = 0;
    virtual QString toHtml(bool verbose) const;

    QList<ProjectExplorer::Abi> qtAbis() const;
    virtual QList<ProjectExplorer::Abi> detectQtAbis() const = 0;

    // Returns the PREFIX, BINPREFIX, DOCPREFIX and similar information
    QHash<QString,QString> versionInfo() const;
    enum PropertyVariant { PropertyVariantGet, PropertyVariantSrc };
    QString qmakeProperty(const QByteArray &name) const;
    virtual void addToEnvironment(const ProjectExplorer::Kit *k, Utils::Environment &env) const;
    virtual Utils::Environment qmakeRunEnvironment() const;

    virtual Utils::FileName sourcePath() const;
    // used by UiCodeModelSupport
    virtual QString uicCommand() const;
    virtual QString designerCommand() const;
    virtual QString linguistCommand() const;
    QString qmlsceneCommand() const;
    QString qmlviewerCommand() const;

    virtual QString qtVersionString() const;
    virtual QtVersionNumber qtVersion() const;

    bool hasExamples() const;
    QString examplesPath() const;

    bool hasDocumentation() const;
    QString documentationPath() const;

    bool hasDemos() const;
    QString demosPath() const;

    virtual QList<ProjectExplorer::HeaderPath> systemHeaderPathes(const ProjectExplorer::Kit *k) const;
    virtual QString frameworkInstallPath() const;

    // former local functions
    Utils::FileName qmakeCommand() const;

    /// @returns the name of the mkspec
    Utils::FileName mkspec() const;
    Utils::FileName mkspecFor(ProjectExplorer::ToolChain *tc) const;
    /// @returns the full path to the default directory
    /// specifally not the directory the symlink/ORIGINAL_QMAKESPEC points to
    Utils::FileName mkspecPath() const;

    bool hasMkspec(const Utils::FileName &spec) const;

    enum QmakeBuildConfig
    {
        NoBuild = 1,
        DebugBuild = 2,
        BuildAll = 8
    };

    Q_DECLARE_FLAGS(QmakeBuildConfigs, QmakeBuildConfig)

    virtual QmakeBuildConfigs defaultBuildConfig() const;
    virtual void recheckDumper();
    virtual bool supportsShadowBuilds() const;

    /// Check a .pro-file/Qt version combination on possible issues
    /// @return a list of tasks, ordered on severity (errors first, then
    ///         warnings and finally info items.
    QList<ProjectExplorer::Task> reportIssues(const QString &proFile, const QString &buildDir) const;

    static bool queryQMakeVariables(const Utils::FileName &binary, const Utils::Environment &env,
                                    QHash<QString, QString> *versionInfo, QString *error = 0);
    static Utils::FileName mkspecDirectoryFromVersionInfo(const QHash<QString, QString> &versionInfo);
    static Utils::FileName mkspecFromVersionInfo(const QHash<QString, QString> &versionInfo);
    static Utils::FileName sourcePath(const QHash<QString, QString> &versionInfo);

    static bool isQmlDebuggingSupported(ProjectExplorer::Kit *k, QString *reason = 0);
    bool isQmlDebuggingSupported(QString *reason = 0) const;
    static void buildDebuggingHelper(ProjectExplorer::Kit *k, int tools);
    void buildDebuggingHelper(ProjectExplorer::ToolChain *tc, int tools);

    virtual bool supportsBinaryDebuggingHelper() const;
    virtual QString gdbDebuggingHelperLibrary() const;
    virtual QString qmlDumpTool(bool debugVersion) const;
    virtual QStringList debuggingHelperLibraryLocations() const;

    virtual bool hasGdbDebuggingHelper() const;
    virtual bool hasQmlDump() const;
    virtual bool hasQmlDumpWithRelocatableFlag() const;
    virtual bool needsQmlDump() const;
    Utils::Environment qmlToolsEnvironment() const;

    virtual QtConfigWidget *createConfigurationWidget() const;

    static QString defaultDisplayName(const QString &versionString,
                                      const Utils::FileName &qmakePath,
                                      bool fromPath = false);

    virtual Core::FeatureSet availableFeatures() const;
    virtual QString platformName() const;
    virtual QString platformDisplayName() const;
    virtual bool supportsPlatform(const QString &platformName) const;

    virtual QList<ProjectExplorer::Task> validateKit(const ProjectExplorer::Kit *k);

    Utils::FileName headerPath() const;
    Utils::FileName docsPath() const;
    Utils::FileName libraryPath() const;
    Utils::FileName pluginPath() const;
    Utils::FileName binPath() const;
    Utils::FileName mkspecsPath() const;

    QString qtNamespace() const;
    QString qtLibInfix() const;
    bool isFrameworkBuild() const;
    // Note: A Qt version can have both a debug and a release built at the same time!
    bool hasDebugBuild() const;
    bool hasReleaseBuild() const;

    QStringList configValues() const;
    QStringList qtConfigValues() const;

protected:
    BaseQtVersion();
    BaseQtVersion(const Utils::FileName &path, bool isAutodetected = false, const QString &autodetectionSource = QString());

    static QString qmakeProperty(const QHash<QString,QString> &versionInfo, const QByteArray &name,
                                 PropertyVariant variant = PropertyVariantGet);

    virtual QList<ProjectExplorer::Task> reportIssuesImpl(const QString &proFile, const QString &buildDir) const;

    // helper function for desktop and simulator to figure out the supported abis based on the libraries
    static QList<Utils::FileName> qtCorePaths(const QHash<QString,QString> &versionInfo,
                                              const QString &versionString);
    static QList<ProjectExplorer::Abi> qtAbisFromLibrary(const QList<Utils::FileName> &coreLibraries);

    void ensureMkSpecParsed() const;
    virtual void parseMkSpec(ProFileEvaluator *) const;
private:
    void setAutoDetectionSource(const QString &autodetectionSource);
    static int getUniqueId();
    void ctor(const Utils::FileName &qmakePath);
    void updateSourcePath() const;
    void updateVersionInfo() const;
    enum Binaries { QmlViewer, QmlScene, Designer, Linguist, Uic };
    QString findQtBinary(Binaries binary) const;
    void updateMkspec() const;
    void setId(int id); // used by the qtversionmanager for legacy restore
                        // and by the qtoptionspage to replace Qt versions

    int m_id;

    bool m_isAutodetected;
    mutable bool m_hasDebuggingHelper; // controlled by m_versionInfoUpToDate
    mutable bool m_hasQmlDump;         // controlled by m_versionInfoUpToDate
    mutable bool m_mkspecUpToDate;
    mutable bool m_mkspecReadUpToDate;
    mutable bool m_defaultConfigIsDebug;
    mutable bool m_defaultConfigIsDebugAndRelease;
    mutable bool m_frameworkBuild;
    mutable bool m_versionInfoUpToDate;
    mutable bool m_installed;
    mutable bool m_hasExamples;
    mutable bool m_hasDemos;
    mutable bool m_hasDocumentation;
    mutable bool m_qmakeIsExecutable;
    mutable bool m_hasQtAbis;

    mutable QStringList m_configValues;
    mutable QStringList m_qtConfigValues;

    QString m_displayName;
    QString m_autodetectionSource;
    mutable Utils::FileName m_sourcePath;

    mutable Utils::FileName m_mkspec;
    mutable Utils::FileName m_mkspecFullPath;

    mutable QHash<QString, QString> m_mkspecValues;

    mutable QHash<QString,QString> m_versionInfo;

    mutable Utils::FileName m_qmakeCommand;
    mutable QString m_qtVersionString;
    mutable QString m_uicCommand;
    mutable QString m_designerCommand;
    mutable QString m_linguistCommand;
    mutable QString m_qmlsceneCommand;
    mutable QString m_qmlviewerCommand;

    mutable QList<ProjectExplorer::Abi> m_qtAbis;
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QtSupport::BaseQtVersion::QmakeBuildConfigs)
#endif // BASEQTVERSION_H

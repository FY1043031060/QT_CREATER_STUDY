/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Build Suite.
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

#ifndef QBS_LANGUAGE_H
#define QBS_LANGUAGE_H

#include "filetags.h"
#include "forward_decls.h"
#include "jsimports.h"
#include "propertymapinternal.h"
#include <buildgraph/forward_decls.h>
#include <tools/codelocation.h>
#include <tools/fileinfo.h>
#include <tools/persistentobject.h>
#include <tools/settings.h>
#include <tools/weakpointer.h>

#include <QByteArray>
#include <QDataStream>
#include <QHash>
#include <QProcessEnvironment>
#include <QRegExp>
#include <QScriptProgram>
#include <QScriptValue>
#include <QScopedPointer>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QScriptEngine;
QT_END_NAMESPACE

namespace qbs {
namespace Internal {
class BuildGraphLoader;

class FileTagger : public PersistentObject
{
public:
    static FileTaggerPtr create() { return FileTaggerPtr(new FileTagger); }
    static FileTaggerPtr create(const QRegExp &artifactExpression, const FileTags &fileTags) {
        return FileTaggerPtr(new FileTagger(artifactExpression, fileTags));
    }

    const QRegExp &artifactExpression() const { return m_artifactExpression; }
    const FileTags &fileTags() const { return m_fileTags; }

private:
    FileTagger(const QRegExp &artifactExpression, const FileTags &fileTags)
        : m_artifactExpression(artifactExpression), m_fileTags(fileTags)
    { }

    FileTagger()
        : m_artifactExpression(QString(), Qt::CaseSensitive, QRegExp::Wildcard)
    {}

    void load(PersistentPool &);
    void store(PersistentPool &) const;

    QRegExp m_artifactExpression;
    FileTags m_fileTags;
};

class RuleArtifact : public PersistentObject
{
public:
    static RuleArtifactPtr create() { return RuleArtifactPtr(new RuleArtifact); }

    QString fileName;
    FileTags fileTags;
    bool alwaysUpdated;

    class Binding
    {
    public:
        QStringList name;
        QString code;
        CodeLocation location;
    };

    QVector<Binding> bindings;

private:
    RuleArtifact()
        : alwaysUpdated(true)
    {}

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};
uint qHash(const RuleArtifact::Binding &b);
bool operator==(const RuleArtifact::Binding &b1, const RuleArtifact::Binding &b2);
inline bool operator!=(const RuleArtifact::Binding &b1, const RuleArtifact::Binding &b2) {
    return !(b1 == b2);
}
bool operator==(const RuleArtifact &a1, const RuleArtifact &a2);
inline bool operator!=(const RuleArtifact &a1, const RuleArtifact &a2) { return !(a1 == a2); }

class SourceArtifact : public PersistentObject
{
public:
    static SourceArtifactPtr create() { return SourceArtifactPtr(new SourceArtifact); }

    QString absoluteFilePath;
    FileTags fileTags;
    bool overrideFileTags;
    PropertyMapPtr properties;

private:
    SourceArtifact() : overrideFileTags(true) {}

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};
bool operator==(const SourceArtifact &sa1, const SourceArtifact &sa2);
inline bool operator!=(const SourceArtifact &sa1, const SourceArtifact &sa2) {
    return !(sa1 == sa2);
}

bool sourceArtifactListsAreEqual(const QList<SourceArtifactPtr> &l1,
                                 const QList<SourceArtifactPtr> &l2);

class SourceWildCards : public PersistentObject
{
public:
    typedef QSharedPointer<SourceWildCards> Ptr;
    typedef QSharedPointer<const SourceWildCards> ConstPtr;

    static Ptr create() { return Ptr(new SourceWildCards); }

    QSet<QString> expandPatterns(const GroupConstPtr &group, const QString &baseDir) const;

    // TODO: Use back pointer to Group instead?
    QString prefix;

    QStringList patterns;
    QStringList excludePatterns;
    QList<SourceArtifactPtr> files;

private:
    SourceWildCards() {}

    QSet<QString> expandPatterns(const GroupConstPtr &group, const QStringList &patterns,
                                 const QString &baseDir) const;
    void expandPatterns(QSet<QString> &result, const GroupConstPtr &group,
                        const QStringList &parts, const QString &baseDir) const;

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};

class ResolvedGroup : public PersistentObject
{
public:
    static GroupPtr create() { return GroupPtr(new ResolvedGroup); }

    CodeLocation location;

    QString name;
    bool enabled;
    QList<SourceArtifactPtr> files;
    SourceWildCards::Ptr wildcards;
    PropertyMapPtr properties;

    QList<SourceArtifactPtr> allFiles() const;

private:
    ResolvedGroup()
        : enabled(true)
    {}

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};

class ResolvedFileContext : public PersistentObject
{
public:
    static ResolvedFileContextPtr create()
    {
        return ResolvedFileContextPtr(new ResolvedFileContext);
    }

    QString filePath;
    QStringList jsExtensions;
    JsImports jsImports;

private:
    ResolvedFileContext() {}

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};

bool operator==(const ResolvedFileContext &a, const ResolvedFileContext &b);
inline bool operator!=(const ResolvedFileContext &a, const ResolvedFileContext &b)
{ return !(a == b); }

class ScriptFunction : public PersistentObject
{
public:
    static ScriptFunctionPtr create() { return ScriptFunctionPtr(new ScriptFunction); }

    QString sourceCode;
    QStringList argumentNames;
    CodeLocation location;
    ResolvedFileContextConstPtr fileContext;
    mutable QScriptValue scriptFunction;    // cache

private:
    ScriptFunction() {}

    void load(PersistentPool &);
    void store(PersistentPool &) const;
};

bool operator==(const ScriptFunction &a, const ScriptFunction &b);
inline bool operator!=(const ScriptFunction &a, const ScriptFunction &b) { return !(a == b); }

class ResolvedModule : public PersistentObject
{
public:
    static ResolvedModulePtr create() { return ResolvedModulePtr(new ResolvedModule); }

    QString name;
    QStringList moduleDependencies;
    ScriptFunctionConstPtr setupBuildEnvironmentScript;
    ScriptFunctionConstPtr setupRunEnvironmentScript;

private:
    ResolvedModule() {}

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};
bool operator==(const ResolvedModule &m1, const ResolvedModule &m2);
inline bool operator!=(const ResolvedModule &m1, const ResolvedModule &m2) { return !(m1 == m2); }

/**
  * Per default each rule is a "non-multiplex rule".
  *
  * A "multiplex rule" creates one transformer that takes all
  * input artifacts with the matching input file tag and creates
  * one or more artifacts. (e.g. linker rule)
  *
  * A "non-multiplex rule" creates one transformer per matching input file.
  */
class Rule : public PersistentObject
{
public:
    static RulePtr create() { return RulePtr(new Rule); }

    ResolvedModuleConstPtr module;
    ScriptFunctionConstPtr script;
    FileTags inputs;
    FileTags auxiliaryInputs;
    FileTags usings;
    FileTags explicitlyDependsOn;
    bool multiplex;
    QList<RuleArtifactConstPtr> artifacts;

    // members that we don't need to save
    int ruleGraphId;

    QString toString() const;
    FileTags outputFileTags() const;

private:
    Rule() : multiplex(false), ruleGraphId(-1) {}

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};
bool operator==(const Rule &r1, const Rule &r2);
inline bool operator!=(const Rule &r1, const Rule &r2) { return !(r1 == r2); }
bool ruleListsAreEqual(const QList<RulePtr> &l1, const QList<RulePtr> &l2);

class ResolvedTransformer : public PersistentObject
{
public:
    static ResolvedTransformerPtr create()
    {
        return ResolvedTransformerPtr(new ResolvedTransformer);
    }

    ResolvedModuleConstPtr module;
    QStringList inputs;
    QList<SourceArtifactPtr> outputs;
    ScriptFunctionConstPtr transform;
    FileTags explicitlyDependsOn;

private:
    ResolvedTransformer() {}

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};

bool operator==(const ResolvedTransformer &t1, const ResolvedTransformer &t2);
inline bool operator!=(const ResolvedTransformer &t1, const ResolvedTransformer &t2) {
    return !(t1 == t2);
}
bool transformerListsAreEqual(const QList<ResolvedTransformerConstPtr> &l1,
                              const QList<ResolvedTransformerConstPtr> &l2);

class TopLevelProject;
class ScriptEngine;

class ResolvedProduct : public PersistentObject
{
public:
    static ResolvedProductPtr create() { return ResolvedProductPtr(new ResolvedProduct); }

    ~ResolvedProduct();

    bool enabled;
    FileTags fileTags;
    FileTags additionalFileTags;
    QString name;
    QString targetName;
    QString sourceDirectory;
    QString destinationDirectory;
    CodeLocation location;
    WeakPointer<ResolvedProject> project;
    PropertyMapPtr properties;
    QSet<RulePtr> rules;
    QSet<ResolvedProductPtr> dependencies;
    QList<FileTaggerConstPtr> fileTaggers;
    QList<ResolvedModuleConstPtr> modules;
    QList<ResolvedTransformerConstPtr> transformers;
    QList<GroupPtr> groups;
    QList<ArtifactPropertiesPtr> artifactProperties;
    QScopedPointer<ProductBuildData> buildData;

    mutable QProcessEnvironment buildEnvironment; // must not be saved
    mutable QProcessEnvironment runEnvironment; // must not be saved
    QHash<QString, QString> executablePathCache;

    QList<SourceArtifactPtr> allFiles() const;
    QList<SourceArtifactPtr> allEnabledFiles() const;
    FileTags fileTagsForFileName(const QString &fileName) const;
    void setupBuildEnvironment(ScriptEngine *scriptEngine, const QProcessEnvironment &env) const;
    void setupRunEnvironment(ScriptEngine *scriptEngine, const QProcessEnvironment &env) const;

    const QList<RuleConstPtr> &topSortedRules() const;
    TopLevelProject *topLevelProject() const;

    QStringList generatedFiles(const QString &baseFile, const FileTags &tags) const;

private:
    ResolvedProduct();

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
};

class ResolvedProject : public PersistentObject
{
public:
    static ResolvedProjectPtr create() { return ResolvedProjectPtr(new ResolvedProject); }

    QString name;
    CodeLocation location;
    bool enabled;
    QList<ResolvedProductPtr> products;
    QList<ResolvedProjectPtr> subProjects;
    WeakPointer<ResolvedProject> parentProject;

    void setProjectProperties(const QVariantMap &config) { m_projectProperties = config; }
    const QVariantMap &projectProperties() const { return m_projectProperties; }

    TopLevelProject *topLevelProject();
    QList<ResolvedProjectPtr> allSubProjects() const;
    QList<ResolvedProductPtr> allProducts() const;

protected:
    ResolvedProject();

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;
private:
    QVariantMap m_projectProperties;
    TopLevelProject *m_topLevelProject;
};

class TopLevelProject : public ResolvedProject
{
    friend class BuildGraphLoader;
public:
    ~TopLevelProject();

    static TopLevelProjectPtr create() { return TopLevelProjectPtr(new TopLevelProject); }

    static QString deriveId(const QVariantMap &config);
    static QString deriveBuildDirectory(const QString &buildRoot, const QString &id);

    QString buildDirectory; // Not saved
    QProcessEnvironment environment;
    QVariantMap platformEnvironment;
    QHash<QString, QString> usedEnvironment; // Environment variables requested by the project while resolving.
    QHash<QString, bool> fileExistsResults; // Results of calls to "File.exists()".
    QScopedPointer<ProjectBuildData> buildData;

    QSet<QString> buildSystemFiles;

    void setBuildConfiguration(const QVariantMap &config);
    const QVariantMap &buildConfiguration() const { return m_buildConfiguration; }
    QString id() const { return m_id; }

    QString buildGraphFilePath() const;
    void store(const Logger &logger) const;

private:
    TopLevelProject();

    void load(PersistentPool &pool);
    void store(PersistentPool &pool) const;

    QString m_id;
    QVariantMap m_buildConfiguration;
};

bool artifactPropertyListsAreEqual(const QList<ArtifactPropertiesPtr> &l1,
                                   const QList<ArtifactPropertiesPtr> &l2);

} // namespace Internal
} // namespace qbs

QT_BEGIN_NAMESPACE
Q_DECLARE_TYPEINFO(qbs::Internal::JsImport, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(qbs::Internal::RuleArtifact::Binding, Q_MOVABLE_TYPE);
QT_END_NAMESPACE

#endif // QBS_LANGUAGE_H

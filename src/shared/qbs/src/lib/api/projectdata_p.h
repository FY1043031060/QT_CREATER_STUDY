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
#ifndef QBS_PROJECTDATA_P_H
#define QBS_PROJECTDATA_P_H

#include "projectdata.h"

#include <QSharedData>

namespace qbs {
namespace Internal {

class GroupDataPrivate : public QSharedData
{
public:
    GroupDataPrivate() : isValid(false)
    { }

    QString name;
    CodeLocation location;
    QStringList filePaths;
    QStringList expandedWildcards;
    PropertyMap properties;
    bool isEnabled;
    bool isValid;
};

class TargetArtifactPrivate : public QSharedData
{
public:
    TargetArtifactPrivate() : isValid(false) {}

    QString filePath;
    QStringList fileTags;
    PropertyMap properties;
    bool isValid;
};

class InstallableFilePrivate: public QSharedData
{
public:
    InstallableFilePrivate() : isValid(false) {}

    QString sourceFilePath;
    QString targetDirectory;
    QStringList fileTags;
    bool isValid;
};

class ProductDataPrivate : public QSharedData
{
public:
    ProductDataPrivate() : isValid(false)
    { }

    QString name;
    CodeLocation location;
    QList<GroupData> groups;
    QList<TargetArtifact> targetArtifacts;
    bool isEnabled;
    bool isValid;
};

class ProjectDataPrivate : public QSharedData
{
public:
    ProjectDataPrivate() : isValid(false)
    { }

    QString name;
    CodeLocation location;
    bool enabled;
    bool isValid;
    QList<ProductData> products;
    QList<ProjectData> subProjects;
    QString buildDir;
};

} // namespace Internal
} // namespace qbs

#endif // Include guard.

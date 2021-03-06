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

#ifndef QBS_AUTOMOC_H
#define QBS_AUTOMOC_H

#include "forward_decls.h"

#include <language/forward_decls.h>
#include <logging/logger.h>

#include <QObject>

struct ScannerPlugin;

namespace qbs {
namespace Internal {
class FileTag;
class ScanResultCache;

/**
  * Scans cpp and hpp files for the Q_OBJECT / Q_GADGET macro and
  * applies the corresponding rule then.
  * Also scans the files for moc_XXX.cpp files to find out if we must
  * compile and link a moc_XXX.cpp file or not.
  *
  * This whole thing is an ugly hack, I know.
  */
class AutoMoc : public QObject
{
    Q_OBJECT

public:
    AutoMoc(const Logger &logger, QObject *parent = 0);

    void setScanResultCache(ScanResultCache *scanResultCache);
    void apply(const ResolvedProductPtr &product);

signals:
    void reportCommandDescription(const QString &highlight, const QString &message);

private:
    enum FileType
    {
        UnknownFileType,
        HppFileType,
        CppFileType
    };

private:
    static QString generateMocFileName(Artifact *artifact, FileType fileType);
    static FileType fileType(Artifact *artifact);
    void scan(Artifact *artifact, FileType fileType, bool &hasQObjectMacro,
            QSet<QString> &includedMocCppFiles);
    bool isVictimOfMoc(Artifact *artifact, FileType fileType, FileTag &foundMocFileTag);
    void unmoc(Artifact *artifact, const FileTag &mocFileTag);
    const QList<ScannerPlugin *> &cppScanners() const;
    const QList<ScannerPlugin *> &hppScanners() const;

    mutable QList<ScannerPlugin *> m_cppScanners;
    mutable QList<ScannerPlugin *> m_hppScanners;
    ScanResultCache *m_scanResultCache;
    Logger m_logger;
};

} // namespace Internal
} // namespace qbs

#endif // QBS_AUTOMOC_H

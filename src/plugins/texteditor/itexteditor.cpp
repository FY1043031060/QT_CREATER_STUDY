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

#include "itexteditor.h"

#include <QTextCodec>

using namespace TextEditor;

ITextEditorDocument::ITextEditorDocument(QObject *parent)
    : Core::TextDocument(parent)
{
}

QMap<QString, QString> ITextEditor::openedTextDocumentContents()
{
    QMap<QString, QString> workingCopy;
    foreach (Core::IDocument *document, Core::EditorManager::documentModel()->openedDocuments()) {
        ITextEditorDocument *textEditorDocument = qobject_cast<ITextEditorDocument *>(document);
        if (!textEditorDocument)
            continue;
        QString fileName = textEditorDocument->filePath();
        workingCopy[fileName] = textEditorDocument->contents();
    }
    return workingCopy;
}

QMap<QString, QTextCodec *> TextEditor::ITextEditor::openedTextDocumentEncodings()
{
    QMap<QString, QTextCodec *> workingCopy;
    foreach (Core::IDocument *document, Core::EditorManager::documentModel()->openedDocuments()) {
        ITextEditorDocument *textEditorDocument = qobject_cast<ITextEditorDocument *>(document);
        if (!textEditorDocument)
            continue;
        QString fileName = textEditorDocument->filePath();
        workingCopy[fileName] = const_cast<QTextCodec *>(textEditorDocument->codec());
    }
    return workingCopy;
}

ITextEditorDocument *ITextEditor::textDocument()
{
    return qobject_cast<ITextEditorDocument *>(document());
}

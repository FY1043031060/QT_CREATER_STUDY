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

#ifndef STATESEDITORWIDGET_H
#define STATESEDITORWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QQuickView;
QT_END_NAMESPACE

namespace QmlDesigner {

class Model;
class ModelState;
class StatesEditorModel;
class StatesEditorView;
class NodeInstanceView;

namespace Internal {
class StatesEditorImageProvider;
}

class StatesEditorWidget : public QWidget
{
    Q_OBJECT

public:
    StatesEditorWidget(StatesEditorView *m_statesEditorView, StatesEditorModel *statesEditorModel);
    virtual ~StatesEditorWidget();

    QSize sizeHint() const;

    int currentStateInternalId() const;
    void setCurrentStateInternalId(int internalId);
    void setNodeInstanceView(NodeInstanceView *nodeInstanceView);

    void showAddNewStatesButton(bool showAddNewStatesButton);

private:
    QWeakPointer<QQuickView> m_quickView;
    QWeakPointer<StatesEditorView> m_statesEditorView;
    Internal::StatesEditorImageProvider *m_imageProvider;
};

}

#endif // STATESEDITORWIDGET_H

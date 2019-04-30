/**************************************************************************
**
** Copyright (c) 2014 Denis Mingulov
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

#include "classviewparsertreeitem.h"
#include "classviewsymbollocation.h"
#include "classviewsymbolinformation.h"
#include "classviewconstants.h"
#include "classviewutils.h"

#include <QHash>
#include <QPair>
#include <QIcon>
#include <QStandardItem>
#include <QMutex>

#include <QDebug>

enum { debug = true };

namespace ClassView {
namespace Internal {

///////////////////////////////// ParserTreeItemPrivate //////////////////////////////////

/*!
    \class ParserTreeItemPrivate
    \brief The ParserTreeItemPrivate class defines private class data for
    the ParserTreeItem class.
   \sa ParserTreeItem
 */
class ParserTreeItemPrivate
{
public:
    //! symbol locations
    QSet<SymbolLocation> symbolLocations;

    //! symbol information
    QHash<SymbolInformation, ParserTreeItem::Ptr> symbolInformations;

    //! An icon
    QIcon icon;
};

///////////////////////////////// ParserTreeItem //////////////////////////////////

/*!
    \class ParserTreeItem
    \brief The ParserTreeItem class is an item for the internal Class View tree.

    Not virtual - to speed up its work.
*/

ParserTreeItem::ParserTreeItem() :
    d(new ParserTreeItemPrivate())
{
}

ParserTreeItem::~ParserTreeItem()
{
    delete d;
}

ParserTreeItem &ParserTreeItem::operator=(const ParserTreeItem &other)
{
    d->symbolLocations = other.d->symbolLocations;
    d->icon = other.d->icon;
    d->symbolInformations.clear();
    return *this;
}

/*!
    Copies a parser tree item from the location specified by \a from to this
    item.
*/

void ParserTreeItem::copy(const ParserTreeItem::ConstPtr &from)
{
    if (from.isNull())
        return;

    d->symbolLocations = from->d->symbolLocations;
    d->icon = from->d->icon;
    d->symbolInformations = from->d->symbolInformations;
}

/*!
    \fn void copyTree(const ParserTreeItem::ConstPtr &from)
    Copies a parser tree item with children from the location specified by
    \a from to this item.
*/

void ParserTreeItem::copyTree(const ParserTreeItem::ConstPtr &target)
{
    if (target.isNull())
        return;

    // copy content
    d->symbolLocations = target->d->symbolLocations;
    d->icon = target->d->icon;
    d->symbolInformations.clear();

    // reserve memory
//    int amount = qMin(100 , target->d_ptr->symbolInformations.count() * 2);
//    d_ptr->symbolInformations.reserve(amount);

    // every child
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator cur =
            target->d->symbolInformations.constBegin();
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator end =
            target->d->symbolInformations.constEnd();

    for (; cur != end; cur++) {
        ParserTreeItem::Ptr item(new ParserTreeItem());
        item->copyTree(cur.value());
        appendChild(item, cur.key());
    }
}

/*!
    Adds information about symbol location from a \location.
    \sa SymbolLocation, removeSymbolLocation, symbolLocations
*/

void ParserTreeItem::addSymbolLocation(const SymbolLocation &location)
{
    d->symbolLocations.insert(location);
}

/*!
    Adds information about symbol locations from \a locations.
    \sa SymbolLocation, removeSymbolLocation, symbolLocations
*/

void ParserTreeItem::addSymbolLocation(const QSet<SymbolLocation> &locations)
{
    d->symbolLocations.unite(locations);
}

/*!
    Removes information about \a location.
    \sa SymbolLocation, addSymbolLocation, symbolLocations
*/

void ParserTreeItem::removeSymbolLocation(const SymbolLocation &location)
{
    d->symbolLocations.remove(location);
}

/*!
    Removes information about \a locations.
    \sa SymbolLocation, addSymbolLocation, symbolLocations
*/

void ParserTreeItem::removeSymbolLocations(const QSet<SymbolLocation> &locations)
{
    d->symbolLocations.subtract(locations);
}

/*!
    Gets information about symbol positions.
    \sa SymbolLocation, addSymbolLocation, removeSymbolLocation
*/

QSet<SymbolLocation> ParserTreeItem::symbolLocations() const
{
    return d->symbolLocations;
}

/*!
    Appends the child item \a item to \a inf symbol information.
*/

void ParserTreeItem::appendChild(const ParserTreeItem::Ptr &item, const SymbolInformation &inf)
{
    // removeChild must be used to remove an item
    if (item.isNull())
        return;

    d->symbolInformations[inf] = item;
}

/*!
    Removes the \a inf symbol information.
*/

void ParserTreeItem::removeChild(const SymbolInformation &inf)
{
    d->symbolInformations.remove(inf);
}

/*!
    Returns the child item specified by \a inf symbol information.
*/

ParserTreeItem::Ptr ParserTreeItem::child(const SymbolInformation &inf) const
{
    if (!d->symbolInformations.contains(inf))
        return ParserTreeItem::Ptr();
    return d->symbolInformations[inf];
}

/*!
    Returns the amount of children of the tree item.
*/

int ParserTreeItem::childCount() const
{
    return d->symbolInformations.count();
}

/*!
    \property QIcon::icon
    \brief the icon assigned to the tree item
*/

QIcon ParserTreeItem::icon() const
{
    return d->icon;
}

/*!
    Sets the \a icon for the tree item.
 */
void ParserTreeItem::setIcon(const QIcon &icon)
{
    d->icon = icon;
}

/*!
    Adds an internal state with \a target, which contains the correct current
    state.
*/

void ParserTreeItem::add(const ParserTreeItem::ConstPtr &target)
{
    if (target.isNull())
        return;

    // add locations
    d->symbolLocations = d->symbolLocations.unite(target->d->symbolLocations);

    // add children
    // every target child
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator cur =
            target->d->symbolInformations.constBegin();
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator end =
            target->d->symbolInformations.constEnd();
    while (cur != end) {
        const SymbolInformation &inf = cur.key();
        const ParserTreeItem::Ptr &targetChild = cur.value();
        if (d->symbolInformations.contains(inf)) {
            // this item has the same child node
            const ParserTreeItem::Ptr &child = d->symbolInformations[inf];
            if (!child.isNull()) {
                child->add(targetChild);
            } else {
                ParserTreeItem::Ptr add(new ParserTreeItem());
                add->copyTree(targetChild);
                d->symbolInformations[inf] = add;
            }
        } else {
            ParserTreeItem::Ptr add(new ParserTreeItem());
            add->copyTree(targetChild);
            d->symbolInformations[inf] = add;
        }
        // next item
        ++cur;
    }
}

/*!
    Subtracts an internal state with \a target, which contains the subtrahend.
*/

void ParserTreeItem::subtract(const ParserTreeItem::ConstPtr &target)
{
    if (target.isNull())
        return;

    // every target child
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator cur =
            target->d->symbolInformations.constBegin();
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator end =
            target->d->symbolInformations.constEnd();
    while (cur != end) {
        const SymbolInformation &inf = cur.key();
        if (d->symbolInformations.contains(inf)) {
            // this item has the same child node
            if (!d->symbolInformations[inf].isNull())
                d->symbolInformations[inf]->subtract(cur.value());
            if (d->symbolInformations[inf].isNull()
                || d->symbolInformations[inf]->childCount() == 0)
                d->symbolInformations.remove(inf);
        }
        // next item
        ++cur;
    }
}

/*!
    Appends this item to the QStandardIten item \a item.
    \a recursive does it recursively for the tree items (might be needed for
    lazy data population.
*/

void ParserTreeItem::convertTo(QStandardItem *item, bool recursive) const
{
    if (!item)
        return;

    QMap<SymbolInformation, ParserTreeItem::Ptr> map;

    // convert to map - to sort it
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator curHash =
            d->symbolInformations.constBegin();
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator endHash =
            d->symbolInformations.constEnd();
    while (curHash != endHash) {
        map.insert(curHash.key(), curHash.value());
        ++curHash;
    }

    // add to item
    QMap<SymbolInformation, ParserTreeItem::Ptr>::const_iterator cur = map.constBegin();
    QMap<SymbolInformation, ParserTreeItem::Ptr>::const_iterator end = map.constEnd();
    while (cur != end) {
        const SymbolInformation &inf = cur.key();
        ParserTreeItem::Ptr ptr = cur.value();

        QStandardItem *add = new QStandardItem();
        Utils::setSymbolInformationToItem(inf, add);
        if (!ptr.isNull()) {
            // icon
            add->setIcon(ptr->icon());

            // locations
            add->setData(Utils::locationsToRole(ptr->symbolLocations()),
                         Constants::SymbolLocationsRole);

            if (recursive)
                cur.value()->convertTo(add, false);
        }
        item->appendRow(add);
        ++cur;
    }
}

/*!
    Checks \a item in a QStandardItemModel for lazy data population.
*/

bool ParserTreeItem::canFetchMore(QStandardItem *item) const
{
    if (!item)
        return false;

    // incremental data population - so we have to check children
    // count subchildren of both - current QStandardItem and our internal

    // for the current UI item
    int storedChildren = 0;
    for (int i = 0; i < item->rowCount(); i++) {
        QStandardItem *child = item->child(i);
        if (!child)
            continue;
        storedChildren += child->rowCount();
    }
    // children for the internal state
    int internalChildren = 0;
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator curHash =
            d->symbolInformations.constBegin();
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator endHash =
            d->symbolInformations.constEnd();
    while (curHash != endHash) {
        const ParserTreeItem::Ptr &child = curHash.value();
        if (!child.isNull()) {
            internalChildren += child->childCount();
            // if there is already more items than stored, then can be stopped right now
            if (internalChildren > storedChildren)
                break;
        }
        ++curHash;
    }

    if (storedChildren < internalChildren)
        return true;

    return false;
}

/*!
    Performs lazy data population for \a item in a QStandardItemModel if needed.
*/

void ParserTreeItem::fetchMore(QStandardItem *item) const
{
    if (!item)
        return;

    for (int i = 0; i < item->rowCount(); i++) {
        QStandardItem *child = item->child(i);
        if (!child)
            continue;

        const SymbolInformation &childInf = Utils::symbolInformationFromItem(child);

        if (d->symbolInformations.contains(childInf)) {
            const ParserTreeItem::Ptr &childPtr = d->symbolInformations[childInf];
            if (childPtr.isNull())
                continue;

            // create a standard
            QScopedPointer<QStandardItem> state(new QStandardItem());
            childPtr->convertTo(state.data(), false);

            Utils::fetchItemToTarget(child, state.data());
        }
    }
}

/*!
    Debug dump.
*/

void ParserTreeItem::debugDump(int ident) const
{
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator curHash =
            d->symbolInformations.constBegin();
    QHash<SymbolInformation, ParserTreeItem::Ptr>::const_iterator endHash =
            d->symbolInformations.constEnd();
    while (curHash != endHash) {
        const SymbolInformation &inf = curHash.key();
        qDebug() << QString(2*ident, QLatin1Char(' ')) << inf.iconType() << inf.name() << inf.type()
                << curHash.value().isNull();
        if (!curHash.value().isNull())
            curHash.value()->debugDump(ident + 1);

        ++curHash;
    }
}

} // namespace Internal
} // namespace ClassView


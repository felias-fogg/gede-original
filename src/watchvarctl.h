/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef WATCHVAR_CTL_H
#define WATCHVAR_CTL_H

#include <QString>
#include <QTreeWidget>

#include "core.h"
#include "varctl.h"


class WatchVarCtl : public VarCtl
{
    Q_OBJECT

public:
    WatchVarCtl();
    
    void setWidget(QTreeWidget *varWidget);

    void ICore_onWatchVarChildAdded(VarWatch &watch, QString valueString, QString varType, bool hasChildren, bool inScope);
    void addNewWatch(QString varName);
    void deleteSelected();

public slots:
    void onWatchWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onWatchWidgetCurrentItemChanged ( QTreeWidgetItem * current, int column );
    void onWatchWidgetItemExpanded(QTreeWidgetItem *item );
    void onWatchWidgetItemCollapsed(QTreeWidgetItem *item);


private:
    void fillInVars();

private:
    QTreeWidget *m_varWidget;
    VarCtl::DispInfoMap m_watchVarDispInfo;
};

#endif // WATCHVAR_CTL_H

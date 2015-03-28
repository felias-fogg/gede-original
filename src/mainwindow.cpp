#include "mainwindow.h"
#include "util.h"
#include "log.h"
#include "core.h"
#include <assert.h>
#include "aboutdialog.h"
#include "settingsdialog.h"
#include "tagscanner.h"

#include <QDirIterator>
#include <QMessageBox>
#include <QScrollBar>


MainWindow::MainWindow(QWidget *parent)
      : QMainWindow(parent)
{

    m_ui.setupUi(this);

    m_autoVarCtl.setWidget(m_ui.autoWidget);

    m_ui.codeView->setInterface(this);

    m_fileIcon.addFile(QString::fromUtf8(":/images/res/file.png"), QSize(), QIcon::Normal, QIcon::Off);
    m_folderIcon.addFile(QString::fromUtf8(":/images/res/folder.png"), QSize(), QIcon::Normal, QIcon::Off);

    //
    m_ui.varWidget->setColumnCount(3);
    m_ui.varWidget->setColumnWidth(0, 80);
    QStringList names;
    names += "Name";
    names += "Value";
    names += "Type";
    m_ui.varWidget->setHeaderLabels(names);
    connect(m_ui.varWidget, SIGNAL(itemChanged(QTreeWidgetItem * ,int)), this, SLOT(onWatchWidgetCurrentItemChanged(QTreeWidgetItem * ,int)));
    connect(m_ui.varWidget, SIGNAL(itemDoubleClicked( QTreeWidgetItem * , int  )), this, SLOT(onWatchWidgetItemDoubleClicked(QTreeWidgetItem *, int )));
    connect(m_ui.varWidget, SIGNAL(itemExpanded( QTreeWidgetItem * )), this, SLOT(onWatchWidgetItemExpanded(QTreeWidgetItem * )));
    connect(m_ui.varWidget, SIGNAL(itemCollapsed( QTreeWidgetItem *)), this, SLOT(onWatchWidgetItemCollapsed(QTreeWidgetItem *)));
    


    //
    m_ui.treeWidget_breakpoints->setColumnCount(2);
    m_ui.treeWidget_breakpoints->setColumnWidth(0, 80);
    names.clear();
    names += "Filename";
    names += "Func";
    names += "Line";
    names += "Addr";
    m_ui.treeWidget_breakpoints->setHeaderLabels(names);
    connect(m_ui.treeWidget_breakpoints, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this, SLOT(onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * ,int)));



    //
    m_ui.autoWidget->setColumnCount(2);
    m_ui.autoWidget->setColumnWidth(0, 80);
    names.clear();
    names += "Name";
    names += "Value";
    m_ui.autoWidget->setHeaderLabels(names);
    connect(m_ui.autoWidget, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this,
                            SLOT(onAutoWidgetItemDoubleClicked(QTreeWidgetItem *, int )));
    connect(m_ui.autoWidget, SIGNAL(itemExpanded ( QTreeWidgetItem * )), this,
                            SLOT(onAutoWidgetItemExpanded(QTreeWidgetItem * )));
    connect(m_ui.autoWidget, SIGNAL(itemCollapsed ( QTreeWidgetItem *  )), this,
                            SLOT(onAutoWidgetItemCollapsed(QTreeWidgetItem * )));



    //
    QTreeWidget *treeWidget = m_ui.treeWidget_file;
    names.clear();
    names += "Name";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(1);
    treeWidget->setColumnWidth(0, 200);


    //
    treeWidget = m_ui.treeWidget_threads;
    names.clear();
    names += "Name";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(1);
    treeWidget->setColumnWidth(0, 200);

    connect(m_ui.treeWidget_threads, SIGNAL(itemSelectionChanged()), this,
                SLOT(onThreadWidgetSelectionChanged()));

    //
    treeWidget = m_ui.treeWidget_stack;
    names.clear();
    names += "Name";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(1);
    treeWidget->setColumnWidth(0, 200);

    connect(m_ui.treeWidget_stack, SIGNAL(itemSelectionChanged()), this,
                SLOT(onStackWidgetSelectionChanged()));



    //
    QList<int> slist;
    slist.append(100);
    slist.append(300);
    m_ui.splitter->setSizes(slist);

     
    //
    QList<int> slist2;
    slist2.append(500);
    slist2.append(70);
    m_ui.splitter_2->setSizes(slist2);



    //
    QList<int> slist3;
    slist3.append(300);
    slist3.append(120);
    slist3.append(120);
    m_ui.splitter_3->setSizes(slist3);



    //
    QList<int> slist4;
    slist4.append(300);
    slist4.append(120);
    m_ui.splitter_4->setSizes(slist4);

     

    connect(m_ui.treeWidget_file, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(onFolderViewItemActivated(QTreeWidgetItem*,int)));

    connect(m_ui.actionQuit, SIGNAL(triggered()), SLOT(onQuit()));
    connect(m_ui.actionStop, SIGNAL(triggered()), SLOT(onStop()));
    connect(m_ui.actionNext, SIGNAL(triggered()), SLOT(onNext()));
    connect(m_ui.actionAbout, SIGNAL(triggered()), SLOT(onAbout()));
    connect(m_ui.actionStep_In, SIGNAL(triggered()), SLOT(onStepIn()));
    connect(m_ui.actionStep_Out, SIGNAL(triggered()), SLOT(onStepOut()));
    connect(m_ui.actionRun, SIGNAL(triggered()), SLOT(onRun()));
    connect(m_ui.actionContinue, SIGNAL(triggered()), SLOT(onContinue()));


    connect(m_ui.actionSettings, SIGNAL(triggered()), SLOT(onSettings()));

    connect(m_ui.comboBox_funcList, SIGNAL(activated(int)), SLOT(onFuncListItemActivated(int)));

    m_tagScanner.init();

    fillInVars();



    Core &core = Core::getInstance();
    core.setListener(this);


    //
    QFont font = m_ui.logView->font();
    font.setPointSize(9);
    m_ui.logView->setFont(font);


    installEventFilter(this);

    loadConfig();
    
}

void MainWindow::loadConfig()
{
    m_cfg.load(CONFIG_FILENAME);

    m_ui.codeView->setConfig(&m_cfg);


    m_cfg.save(CONFIG_FILENAME);

}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QWidget *widget = QApplication::focusWidget();

        // 'Delete' key pressed in the var widget 
        if(widget == m_ui.varWidget && keyEvent->key() == Qt::Key_Delete)
        {
            QList<QTreeWidgetItem *> items = m_ui.varWidget->selectedItems();

            for(int i =0;i < items.size();i++)
            {
                QTreeWidgetItem *item = items[i];
            
                // Delete the item
                Core &core = Core::getInstance();
                QTreeWidgetItem *rootItem = m_ui.varWidget->invisibleRootItem();
                QString watchId = item->data(0, Qt::UserRole).toString();
                if(watchId != "")
                {
                    rootItem->removeChild(item);
                    core.gdbRemoveVarWatch(watchId);
                }
            }
        }
        
        //qDebug() << "key " << keyEvent->key() << " from " << obj << "focus " << widget;

    }
    return QObject::eventFilter(obj, event);
}


/**
 * @brief Execution has stopped.
 * @param lineNo   The line which is about to execute (1=first).
 */
void MainWindow::ICore_onStopped(ICore::StopReason reason, QString path, int lineNo)
{
    Q_UNUSED(reason);

    if(reason == ICore::EXITED_NORMALLY)
    {
        QString title = "Program exited";
        QString text = "Program exited normally";
        QMessageBox::information (this, title, text); 
    }
    
    m_currentFile = path;
    m_currentLine = lineNo;
    if(!path.isEmpty())
    {
        open(path);
    }
    else
        m_ui.codeView->disableCurrentLine();

    fillInStack();
}


void MainWindow::ICore_onLocalVarReset()
{
    QTreeWidget *autoWidget = m_ui.autoWidget;

    autoWidget->clear();
}


/**
 * @brief Adds a path of directories to a tree widget.
 * @return returns the root directory of the newly created directories.
 */
QTreeWidgetItem *MainWindow::addTreeWidgetPath(QTreeWidget *treeWidget, QTreeWidgetItem *parent, QString path)
{
    QString firstName;
    QString restPath;
    QTreeWidgetItem *newItem = NULL;


    // Divide the path into a folder and name part.
    firstName = path;
    int divPos = path.indexOf('/');
    if(divPos != -1)
    {
        firstName = path.left(divPos);        
        restPath = path.mid(divPos+1);
    }

    // Handle "../" paths
    if(firstName == ".." && parent != NULL)
    {
        return addTreeWidgetPath(treeWidget, parent->parent(), restPath);
    }
    
    // Empty name and only a path?
    if(firstName.isEmpty())
    {
        if(restPath.isEmpty())
            return NULL;
        else
            return addTreeWidgetPath(treeWidget, parent, restPath);
    }
        
//    debugMsg("inserting: '%s', '%s'\n", stringToCStr(firstName), stringToCStr(restPath));


    // Check if the item already exist?
    QTreeWidgetItem * lookParent;
    if(parent == NULL)
        lookParent = treeWidget->invisibleRootItem();
    else
        lookParent = parent;
    for(int i = 0;i < lookParent->childCount() && newItem == NULL;i++)
    {
        QTreeWidgetItem *item = lookParent->child(i);


        if(item->text(0) == firstName)
            newItem = item;
    }
    

    // Add the item
    if(newItem == NULL)
    {
        newItem = new QTreeWidgetItem;
        newItem->setText(0, firstName); 
        newItem->setIcon(0, m_folderIcon);
    }
    if(parent == NULL)
    {
        treeWidget->insertTopLevelItem(0, newItem);
    }
    else
    {
        parent->addChild(newItem);
    }

    if(restPath.isEmpty())
        return newItem;
    else
        return addTreeWidgetPath(treeWidget, newItem, restPath);
}


/**
 * @brief Fills in the source file treeview.
 */
void MainWindow::insertSourceFiles()
{
    QTreeWidget *treeWidget = m_ui.treeWidget_file;
    Core &core = Core::getInstance();

    // Get source files
    QVector <SourceFile*> sourceFiles = core.getSourceFiles();
    m_sourceFiles.clear();
    for(int i = 0;i < sourceFiles.size();i++)
    {
        SourceFile* source = sourceFiles[i];

        FileInfo info;
        info.name = source->name;
        info.fullName = source->fullName;
        
        m_sourceFiles.push_back(info);
    }

    

    for(int i = 0;i < m_sourceFiles.size();i++)
    {
        FileInfo &info = m_sourceFiles[i];

        m_tagScanner.scan(info.fullName, &info.m_tagList);
        

        QTreeWidgetItem *item = new QTreeWidgetItem;
        QTreeWidgetItem *parentNode  = NULL;

        // Get parent path
        QString folderPath;
        QString filename;
        dividePath(info.name, &filename, &folderPath);
        folderPath = simplifyPath(folderPath);
        
        if(!folderPath.isEmpty())
            parentNode = addTreeWidgetPath(treeWidget, NULL, folderPath);
            
        item->setText(0, filename);
        item->setData(0, Qt::UserRole, info.fullName);
        item->setIcon(0, m_fileIcon);
        
        if(parentNode == NULL)
            treeWidget->insertTopLevelItem(0, item);
        else
        {
            parentNode->addChild(item);
        }
    }

    treeWidget->sortItems(0, Qt::AscendingOrder);

}


QString getTreeWidgetItemPath(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();
    if(parent)
        return getTreeWidgetItemPath(parent) + "/" + item->text(0);
    else
        return item->text(0);
}

        
QTreeWidgetItem *MainWindow::insertTreeWidgetItem(
                    VarCtl::DispInfoMap *map,
                    QString fullPath,
                    QString name,
                    QString value)
{
    QString displayValue = value;

    //
    if(map->contains(fullPath))
    {
        VarCtl::DispInfo &dispInfo = (*map)[fullPath];
        dispInfo.orgValue = value;

        // Update the variable value
        if(dispInfo.orgFormat == VarCtl::DISP_DEC)
        {
            displayValue = VarCtl::valueDisplay(value.toLongLong(), dispInfo.dispFormat);
        }
    }
    else
    {
        VarCtl::DispInfo dispInfo;
        dispInfo.orgValue = value;
        dispInfo.orgFormat = VarCtl::findVarType(value);
        dispInfo.dispFormat = dispInfo.orgFormat;
        dispInfo.isExpanded = false;
        (*map)[fullPath] = dispInfo;
    }

    //
    QStringList names;
    names.clear();
    names += name;
    names += displayValue;
    QTreeWidgetItem *item;
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    return item;
}




void MainWindow::addVariableDataTree(
                QTreeWidget *treeWidget,
                VarCtl::DispInfoMap *map,
                QTreeWidgetItem *item, TreeNode *rootNode)
{
    QList <TreeNode *> m_list;

    QString parentPath = getTreeWidgetItemPath(item);

    for(int i = 0;i < rootNode->getChildCount();i++)
    {
        TreeNode *child = rootNode->getChild(i);

        QString varPath = parentPath + "/" + child->getName();
        QTreeWidgetItem *childItem;

        if(child->getChildCount() == 0)
        {
            childItem = insertTreeWidgetItem(
                    map,
                    varPath,
                    child->getName(),
                    child->getData());

            item->addChild(childItem);

        }
        else
        {
            QStringList names;
            names += child->getName();

            childItem = new QTreeWidgetItem(names);
            item->addChild(childItem);

            addVariableDataTree(treeWidget, map, childItem, child);
        }


        // Expand it?
        if(m_autoVarDispInfo.contains(varPath))
        {
            VarCtl::DispInfo &dispInfo = (*map)[varPath];

            if(dispInfo.isExpanded)
            {
                treeWidget->expandItem(childItem);
            }
        }
        else
        {
            // Add it to the dispinfomap
            VarCtl::DispInfo dispInfo;
            dispInfo.isExpanded = false;
            (*map)[varPath] = dispInfo;
        }
    }
}

    
    
void MainWindow::ICore_onLocalVarChanged(QString name, CoreVarValue varValue)
{
    QString value = varValue.toString();
    QTreeWidget *autoWidget = m_ui.autoWidget;
    QTreeWidgetItem *item;
    QStringList names;


    item = insertTreeWidgetItem(&m_autoVarDispInfo, name, name, value);
    autoWidget->insertTopLevelItem(0, item);

    // Expand it?
    QString varPath = getTreeWidgetItemPath(item);
    if(m_autoVarDispInfo.contains(varPath))
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
        if(dispInfo.isExpanded)
        {
            autoWidget->expandItem(item);
        }
    }

    // Insert children
    Tree *valueTree = varValue.toTree();
    if(valueTree)
    {
        addVariableDataTree(autoWidget, &m_autoVarDispInfo, item, valueTree->getRoot());
    }
    
    delete valueTree;
}



void MainWindow::ICore_onWatchVarChanged(QString watchId, QString name, QString valueString)
{
    ICore_onWatchVarExpanded( watchId,  name,  valueString, "");
    
}


            
void MainWindow::ICore_onWatchVarExpanded(QString watchId_, QString name, QString valueString, QString varType)
{
    QTreeWidget *varWidget = m_ui.varWidget;
    QStringList names;

    Q_UNUSED(name);

    //
    QTreeWidgetItem * rootItem = varWidget->invisibleRootItem();
    QStringList watchIdParts = watchId_.split('.');
    QString thisWatchId;
    for(int partIdx = 0; partIdx < watchIdParts.size();partIdx++)
    {
        // Get the watchid to look for
        if(thisWatchId != "")
            thisWatchId += ".";
        thisWatchId += watchIdParts[partIdx];

        // Look for the item with the specified watchId
        QTreeWidgetItem* foundItem = NULL;
        for(int i = 0;foundItem == NULL && i < rootItem->childCount();i++)
        {
            QTreeWidgetItem* item =  rootItem->child(i);
            QString itemKey = item->data(0, Qt::UserRole).toString();

            if(thisWatchId == itemKey)
            {
                foundItem = item;
            }
        }

        // Did not find one
        QTreeWidgetItem *item;
        if(foundItem == NULL)
        {
            debugMsg("Adding %s=%s", stringToCStr(name), stringToCStr(valueString));

            // Create the item
            item = new QTreeWidgetItem(QStringList(name));
            item->setData(0, Qt::UserRole, thisWatchId);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            rootItem->addChild(item);
            rootItem = item;

            if(varType == "struct {...}")
                rootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

            rootItem->setText(1, valueString);            
            
        
        }
        else
        {
            item = foundItem;
            rootItem = foundItem;
        }

        if(partIdx +1 == watchIdParts.size())
        {
            // Update the text
            if(m_watchVarDispInfo.contains(thisWatchId))
            {
                VarCtl::DispInfo &dispInfo = m_watchVarDispInfo[thisWatchId];
                dispInfo.orgValue = valueString;

                // Update the variable value
                if(dispInfo.orgFormat == VarCtl::DISP_DEC)
                {
                    valueString = VarCtl::valueDisplay(valueString.toLongLong(), dispInfo.dispFormat);
                }
            }
            item->setText(1, valueString);
        }
    }
}


/**
 * @brief User doubleclicked on the border
 * @param lineNo    The line pressed (1=first row).
 */
void MainWindow::ICodeView_onRowDoubleClick(int lineNo)
{
    Core &core = Core::getInstance();

    BreakPoint* bkpt = core.findBreakPoint(m_filename, lineNo);
    if(bkpt)
        core.gdbRemoveBreakpoint(bkpt);
    else
        core.gdbSetBreakpoint(m_filename, lineNo);
}

    

void MainWindow::ICore_onConsoleStream(QString text)
{
    m_ui.logView->appendPlainText(text);
}

void MainWindow::ICore_onMessage(QString message)
{
    m_ui.logView->appendPlainText(message);

}
    

void MainWindow::fillInStack()
{
    Core &core = Core::getInstance();
    
    core.getStackFrames();

}

     
void MainWindow::fillInVars()
{
    QTreeWidget *varWidget = m_ui.varWidget;
    QTreeWidgetItem *item;
    QStringList names;
    
    varWidget->clear();



    names.clear();
    names += "...";
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    varWidget->insertTopLevelItem(0, item);
   

}


void
MainWindow::onThreadWidgetSelectionChanged( )
{
    // Get the new selected thread
    QTreeWidget *threadWidget = m_ui.treeWidget_threads;
    QList <QTreeWidgetItem *> selectedItems = threadWidget->selectedItems();
    if(selectedItems.size() > 0)
    {
        QTreeWidgetItem * currentItem;
        currentItem = selectedItems[0];
        int selectedThreadId = currentItem->data(0, Qt::UserRole).toInt();

        // Select the thread
        Core &core = Core::getInstance();
        core.selectThread(selectedThreadId);
    }
}

void MainWindow::onStackWidgetSelectionChanged()
{
    Core &core = Core::getInstance();
        
    int selectedFrame = -1;
    // Get the new selected frame
    QTreeWidget *threadWidget = m_ui.treeWidget_stack;
    QList <QTreeWidgetItem *> selectedItems = threadWidget->selectedItems();
    if(selectedItems.size() > 0)
    {
        QTreeWidgetItem * currentItem;
        currentItem = selectedItems[0];
        selectedFrame = currentItem->data(0, Qt::UserRole).toInt();

        core.selectFrame(selectedFrame);
    }
}



void 
MainWindow::onWatchWidgetCurrentItemChanged( QTreeWidgetItem * current, int column )
{
    QTreeWidget *varWidget = m_ui.varWidget;
    Core &core = Core::getInstance();
    QString oldKey = current->data(0, Qt::UserRole).toString();
    QString oldName  = oldKey == "" ? "" : core.gdbGetVarWatchName(oldKey);
    QString newName = current->text(0);

    if(column != 0)
        return;

    if(oldKey != "" && oldName == newName)
        return;
    
     debugMsg("oldName:'%s' newName:'%s' ", stringToCStr(oldName), stringToCStr(newName));

    if(newName == "...")
        newName = "";
    if(oldName == "...")
        oldName = "";
        
    // Nothing to do?
    if(oldName == "" && newName == "")
    {
        current->setText(0, "...");
        current->setText(1, "");
        current->setText(2, "");
    }
    // Remove a variable?
    else if(newName.isEmpty())
    {
        QTreeWidgetItem *item = varWidget->invisibleRootItem();
        item->removeChild(current);

        core.gdbRemoveVarWatch(oldKey);

        m_watchVarDispInfo.remove(oldKey);
    }
    // Add a new variable?
    else if(oldName == "")
    {
        //debugMsg("%s", stringToCStr(current->text(0)));
        QString value;
        QString watchId;
        QString varType;
        if(core.gdbAddVarWatch(newName, &varType, &value, &watchId) == 0)
        {
            if(varType == "struct {...}")
                current->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
            current->setData(0, Qt::UserRole, watchId);
            current->setText(1, value);
            current->setText(2, varType);

            VarCtl::DispInfo dispInfo;
            dispInfo.orgValue = value;
            dispInfo.orgFormat = VarCtl::findVarType(value);
            dispInfo.dispFormat = dispInfo.orgFormat;
            m_watchVarDispInfo[watchId] = dispInfo;

            // Create a new dummy item
            QTreeWidgetItem *item;
            QStringList names;
            names += "...";
            item = new QTreeWidgetItem(names);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            varWidget->addTopLevelItem(item);
            
        }
        else
        {
            current->setText(0, "...");
            current->setText(1, "");
            current->setText(2, "");
        }
    
    }
    // Change a existing variable
    else
    {
        //debugMsg("'%s' -> %s", stringToCStr(current->text(0)), stringToCStr(current->text(0)));

        // Remove any children
        while(current->childCount())
        {
            QTreeWidgetItem *childItem =  current->takeChild(0);
            delete childItem;
        }
        

        // Remove old watch
        core.gdbRemoveVarWatch(oldKey);

        m_watchVarDispInfo.remove(oldKey);

        QString value;
        QString watchId;
        QString varType;
        if(core.gdbAddVarWatch(newName, &varType, &value, &watchId) == 0)
        {
            current->setData(0, Qt::UserRole, watchId);
            current->setText(1, value);
            current->setText(2, varType);

            if(varType == "struct {...}")
            {
                current->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
            }
            core.gdbExpandVarWatchChildren(watchId);
            
            // Add display information
            VarCtl::DispInfo dispInfo;
            dispInfo.orgValue = value;
            dispInfo.orgFormat = VarCtl::findVarType(value);
            dispInfo.dispFormat = dispInfo.orgFormat;
            m_watchVarDispInfo[watchId] = dispInfo;
            
        }
        else
        {
            QTreeWidgetItem *rootItem = varWidget->invisibleRootItem();
            rootItem->removeChild(current);
        }
    }

}



void MainWindow::onWatchWidgetItemExpanded(QTreeWidgetItem *item )
{
    Core &core = Core::getInstance();
    //QTreeWidget *varWidget = m_ui.varWidget;

    // Get watchid of the item
    QString watchId = item->data(0, Qt::UserRole).toString();


    // Get the children
    core.gdbExpandVarWatchChildren(watchId);
    

}

void MainWindow::onWatchWidgetItemCollapsed(QTreeWidgetItem *item)
{
    Q_UNUSED(item);
    
}



void MainWindow::onWatchWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QTreeWidget *varWidget = m_ui.varWidget;

    
    if(column == 0)
        varWidget->editItem(item,column);
    else if(column == 1)
    {
        QString varName = item->text(0);
        QString watchId = item->data(0, Qt::UserRole).toString();

        if(m_watchVarDispInfo.contains(watchId))
        {
            VarCtl::DispInfo &dispInfo = m_watchVarDispInfo[watchId];
            if(dispInfo.orgFormat == VarCtl::DISP_DEC)
            {
                long long val = dispInfo.orgValue.toLongLong();

                if(dispInfo.dispFormat == VarCtl::DISP_DEC)
                {
                    dispInfo.dispFormat = VarCtl::DISP_HEX;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_HEX)
                {
                    dispInfo.dispFormat = VarCtl::DISP_BIN;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_BIN)
                {
                    dispInfo.dispFormat = VarCtl::DISP_CHAR;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_CHAR)
                {
                    dispInfo.dispFormat = VarCtl::DISP_DEC;
                }

                QString valueText = VarCtl::valueDisplay(val, dispInfo.dispFormat);

                item->setText(1, valueText);
            }
        }
    }
}


void MainWindow::onAutoWidgetItemCollapsed(QTreeWidgetItem *item)
{
    QString varPath = getTreeWidgetItemPath(item);
    if(m_autoVarDispInfo.contains(varPath))
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
        dispInfo.isExpanded = false;
    }

}

void MainWindow::onAutoWidgetItemExpanded(QTreeWidgetItem *item)
{
    QString varPath = getTreeWidgetItemPath(item);
    if(m_autoVarDispInfo.contains(varPath))
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
        dispInfo.isExpanded = true;

    }
}





void MainWindow::onAutoWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(column == 0)
    {
    }
    else if(column == 1)
    {
        QString varName = getTreeWidgetItemPath(item);
        if(m_autoVarDispInfo.contains(varName))
        {
            VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varName];
            if(dispInfo.orgFormat == VarCtl::DISP_DEC)
            {
                long long val = dispInfo.orgValue.toLongLong();

                if(dispInfo.dispFormat == VarCtl::DISP_DEC)
                {
                    dispInfo.dispFormat = VarCtl::DISP_HEX;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_HEX)
                {
                    dispInfo.dispFormat = VarCtl::DISP_BIN;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_BIN)
                {
                    dispInfo.dispFormat = VarCtl::DISP_CHAR;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_CHAR)
                {
                    dispInfo.dispFormat = VarCtl::DISP_DEC;
                }

                QString valueText = VarCtl::valueDisplay(val, dispInfo.dispFormat);

                item->setText(1, valueText);
            }
        }
    }
}


void MainWindow::onFolderViewItemActivated ( QTreeWidgetItem * item, int column )
{
    Q_UNUSED(column);

    if(item->childCount() == 0)
    {
        QString filename  = item->data(0, Qt::UserRole).toString();

        open(filename);
    }
}


void MainWindow::open(QString filename)
{
    if(filename.isEmpty())
        return;

    QString text;

    // Read file content
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMsg("Failed to open '%s'", stringToCStr(filename));
        return;
    }
    while (!file.atEnd())
    {
         QByteArray line = file.readLine();
         text += line;
     }

    m_ui.scrollArea_codeView->setWidgetResizable(true);

    setWindowTitle(getFilenamePart(filename));

    m_filename = filename;
    m_ui.codeView->setPlainText(text);

    // Fill in the functions
    m_ui.comboBox_funcList->clear();
    QList<Tag> tagList;
    m_tagScanner.scan(filename, &tagList);
    for(int tagIdx = 0;tagIdx < tagList.size();tagIdx++)
    {
        Tag &tag = tagList[tagIdx];
        if(tag.type == Tag::TAG_FUNC)
        {
            m_ui.comboBox_funcList->addItem(tag.getLongName(), QVariant(tag.lineNo));
        }
        
    }


    // Update the current line view
    if(m_currentFile == filename)
    {
        m_ui.codeView->setCurrentLine(m_currentLine);

        // Scroll to the current line
        ensureLineIsVisible(m_currentLine);
    }
    else
        m_ui.codeView->disableCurrentLine();


    ICore_onBreakpointsChanged();
}

 
void MainWindow::onQuit()
{
    QApplication::instance()->quit();
}

void MainWindow::onStop()
{
    Core &core = Core::getInstance();
    core.stop();
}

void MainWindow::onNext()
{
    Core &core = Core::getInstance();
    core.gdbNext();
    
}



/**
 * @brief Called when user presses "Help->About". Shows the about box.
 */
void MainWindow::onAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}


void MainWindow::onRun()
{
    Core &core = Core::getInstance();
    core.gdbRun();

}

void MainWindow::onContinue()
{
    Core &core = Core::getInstance();
    core.gdbContinue();

    m_ui.codeView->disableCurrentLine();

}

void MainWindow::onStepIn()
{
    Core &core = Core::getInstance();
    core.gdbStepIn();
    
}

void MainWindow::onStepOut()
{
    Core &core = Core::getInstance();
    core.gdbStepOut();
    
}


void MainWindow::ICore_onThreadListChanged()
{
    Core &core = Core::getInstance();

    QTreeWidget *threadWidget = m_ui.treeWidget_threads;
    threadWidget->clear();

    QList<ThreadInfo> list = core.getThreadList();
    
    for(int idx = 0;idx < list.size();idx++)
    {
        // Get name
        QString name = list[idx].m_name;
        

        // Add the item
        QStringList names;
        names.push_back(name);
        QTreeWidgetItem *item = new QTreeWidgetItem(names);
        item->setData(0, Qt::UserRole, list[idx].id);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        threadWidget->insertTopLevelItem(0, item);

    }
}


void MainWindow::ICore_onCurrentThreadChanged(int threadId)
{
    QTreeWidget *threadWidget = m_ui.treeWidget_threads;
    QTreeWidgetItem *rootItem = threadWidget->invisibleRootItem();
    for(int i = 0;i < rootItem->childCount();i++)
    {
        QTreeWidgetItem *item = rootItem->child(i);
    
        int id = item->data(0, Qt::UserRole).toInt();
        if(id == threadId)
        {
            item->setSelected(true);    
        }
        else
            item->setSelected(false);
    }
    
}




void MainWindow::ICore_onBreakpointsChanged()
{
    Core &core = Core::getInstance();
    QList<BreakPoint*>  bklist = core.getBreakPoints();
    QVector<int> numList;


    // Update the settings
    m_cfg.m_breakpoints.clear();
    for(int u = 0;u < bklist.size();u++)
    {
        BreakPoint* bkpt = bklist[u];
        SettingsBreakpoint bkptCfg;
        bkptCfg.filename = bkpt->fullname;
        bkptCfg.lineNo = bkpt->lineNo;
        m_cfg.m_breakpoints.push_back(bkptCfg);
    }
    m_cfg.save(CONFIG_FILENAME);
    

    // Update the breakpoint list widget
    m_ui.treeWidget_breakpoints->clear();
    for(int i = 0;i <  bklist.size();i++)
    {
        BreakPoint* bk = bklist[i];

        QStringList nameList;
        QString name;
        nameList.append(getFilenamePart(bk->fullname));
        nameList.append(bk->m_funcName);
        name.sprintf("%d", bk->lineNo);
        nameList.append(name);
        nameList.append(longLongToHexString(bk->m_addr));
        
        

        QTreeWidgetItem *item = new QTreeWidgetItem(nameList);
        item->setData(0, Qt::UserRole, i);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        // Add the item to the widget
        m_ui.treeWidget_breakpoints->insertTopLevelItem(0, item);

    }

    // Update the fileview
    for(int i = 0;i <  bklist.size();i++)
    {
        BreakPoint* bk = bklist[i];

        if(bk->fullname == m_filename)
            numList.push_back(bk->lineNo);
    }
    m_ui.codeView->setBreakpoints(numList);
    m_ui.codeView->update();
}



void MainWindow::ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList)
{
    m_stackFrameList = stackFrameList;
    QTreeWidget *stackWidget = m_ui.treeWidget_stack;
    
    stackWidget->clear();


    
    for(int idx = 0;idx < stackFrameList.size();idx++)
    {
        // Get name
        StackFrameEntry &entry = stackFrameList[stackFrameList.size()-idx-1];
        

        // Create the item
        QStringList names;
        names.push_back(entry.m_functionName);
        
        QTreeWidgetItem *item = new QTreeWidgetItem(names);

        
        item->setData(0, Qt::UserRole, idx);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        // Add the item to the widget
        stackWidget->insertTopLevelItem(0, item);


    }
    
}



/**
 * @brief The current frame has changed.
 * @param frameIdx    The frame  (0 being the newest frame) 
*/
void MainWindow::ICore_onCurrentFrameChanged(int frameIdx)
{
    QTreeWidget *threadWidget = m_ui.treeWidget_stack;
    QTreeWidgetItem *rootItem = threadWidget->invisibleRootItem();

    // Update the sourceview (with the current row).
    if(frameIdx < m_stackFrameList.size())
    {
        StackFrameEntry &entry = m_stackFrameList[m_stackFrameList.size()-frameIdx-1];

        m_currentFile = entry.m_sourcePath;
        m_currentLine = entry.m_line;
        if(!m_currentFile.isEmpty())
        {
            open(m_currentFile);
        }
        else
            m_ui.codeView->disableCurrentLine();
    }

    // Update the selection of the current thread
    for(int i = 0;i < rootItem->childCount();i++)
    {
        QTreeWidgetItem *item = rootItem->child(i);
    
        int id = item->data(0, Qt::UserRole).toInt();
        if(id == frameIdx)
        {
            item->setSelected(true);    
        }
        else
            item->setSelected(false);
    }
    
}

void MainWindow::ICore_onFrameVarReset()
{

}

void MainWindow::ICore_onFrameVarChanged(QString name, QString value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);

}

void MainWindow::onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * item,int column)
{
    Q_UNUSED(column);

    Core &core = Core::getInstance();
    QList<BreakPoint*>  bklist = core.getBreakPoints();
    int idx = item->data(0, Qt::UserRole).toInt();
    BreakPoint* bk = bklist[idx];

    open(bk->fullname);
    
    ensureLineIsVisible(bk->lineNo);
    
}
    

void MainWindow::ensureLineIsVisible(int lineIdx)
{
    m_ui.scrollArea_codeView->ensureVisible(0, m_ui.codeView->getRowHeight()*lineIdx-1);

    // Select the function in the function combobox
    int bestFitIdx = -1;
    int bestFitDist = -1;
    for(int u = 0;u < m_ui.comboBox_funcList->count();u++)
    {
        int funcLineNo = m_ui.comboBox_funcList->itemData(u).toInt();
        int dist = lineIdx-funcLineNo;
        if((bestFitDist > dist || bestFitIdx == -1) && dist >= 0)
        {
            bestFitDist = dist;
            bestFitIdx = u;
        }
    }

    if(m_ui.comboBox_funcList->count() > 0)
    {

        if(bestFitIdx == -1)
        {
            m_ui.comboBox_funcList->hide();
        }
        else
        {
            m_ui.comboBox_funcList->show();
            m_ui.comboBox_funcList->setCurrentIndex(bestFitIdx);
        }

    }
}


/**
 * @brief User right clicked in the codeview.
 * @param lineNo    The row (1=first row).
 *
 */
void MainWindow::ICodeView_onContextMenu(QPoint pos, int lineNo, QStringList text)
{
    QAction *action;
    int totalItemCount = 0;

    m_popupMenu.clear();
    for(int i = 0;i < text.size();i++)
    {
        action = m_popupMenu.addAction("Add '" + text[i] + "' to watch list");
        action->setData(text[i]);
        connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuAddWatch()));

    }

    // Add 'toggle breakpoint'
    QString title;
    title.sprintf("Toggle breakpoint at L%d", lineNo);
    action = m_popupMenu.addAction(title);
    action->setData(lineNo);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuToggleBreakpoint()));

    action = m_popupMenu.addSeparator();
    

    for(int k = 0;k < text.size();k++)
    {
        // Get the tag to look for
        QString wantedTag = text[k];
        if(wantedTag.lastIndexOf('.') != -1)
            wantedTag = wantedTag.mid(wantedTag.lastIndexOf('.')+1);

        
        // Loop through all the source files
        for(int i = 0;i < m_sourceFiles.size();i++)
        {
            FileInfo& fileInfo = m_sourceFiles[i];

            // Loop through all the tags
            for(int j = 0;j < fileInfo.m_tagList.size();j++)
            {
                Tag &tagInfo = fileInfo.m_tagList[j];
                QString tagName = tagInfo.m_name;
                
                // Tag match?
                if(tagName == wantedTag)
                {

                    if(totalItemCount++ < 20)
                    {
                        // Get filename and lineNo
                        QStringList defList;
                        defList.push_back(fileInfo.fullName);
                        QString lineNoStr;
                        lineNoStr.sprintf("%d", tagInfo.lineNo);
                        defList.push_back(lineNoStr);

                        // Add to popupmenu
                        action = m_popupMenu.addAction("Show definition of '" + tagInfo.getLongName() + "'");
                        action->setData(defList);
                        connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuShowDefinition()));
                    }
                }
            }
        }
    }

    
    // Add 'Show current PC location'
    title = "Show current PC location";
    action = m_popupMenu.addAction(title);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuShowCurrentLocation()));

    
    m_popupMenu.popup(pos);
}


void MainWindow::onCodeViewContextMenuToggleBreakpoint()
{
    QAction *action = static_cast<QAction *>(sender ());
    int lineNo = action->data().toInt();
    Core &core = Core::getInstance();

    BreakPoint* bkpt = core.findBreakPoint(m_filename, lineNo);
    if(bkpt)
        core.gdbRemoveBreakpoint(bkpt);
    else
        core.gdbSetBreakpoint(m_filename, lineNo);

}


void MainWindow::onCodeViewContextMenuShowCurrentLocation()
{
    // Open file
    open(m_currentFile);

    ensureLineIsVisible(m_currentLine);    
}


void MainWindow::onCodeViewContextMenuShowDefinition()
{
    // Get the selected function name
    QAction *action = static_cast<QAction *>(sender ());
    QStringList list = action->data().toStringList();

    // Get filepath and lineNo
    if(list.size() != 2)
        return;
    QString foundFilepath = list[0];
    int lineNo = list[1].toInt();

    // Open file
    open(foundFilepath);            

    // Scroll to the function
    int showLineNo = lineNo-1;
    if(showLineNo < 0)
        showLineNo = 0;
    m_ui.scrollArea_codeView->verticalScrollBar()->setValue(m_ui.codeView->getRowHeight()*showLineNo);


    // Select the function in the function combobox
    int idx = m_ui.comboBox_funcList->findData(QVariant(lineNo));
    if(idx >= 0)
        m_ui.comboBox_funcList->setCurrentIndex(idx);

}


void MainWindow::onCodeViewContextMenuAddWatch()
{
    // Get the selected variable name
    QAction *action = static_cast<QAction *>(sender ());
    QString varName = action->data().toString();

    
    // Add the new variable to the watch list
    QTreeWidgetItem* rootItem = m_ui.varWidget->invisibleRootItem();
    QTreeWidgetItem* lastItem = rootItem->child(rootItem->childCount()-1);
    lastItem->setText(0, varName);
    
}

void MainWindow::onSettings()
{
    SettingsDialog dlg(this, &m_cfg);
    if(dlg.exec() == QDialog::Accepted)
    {
        dlg.getConfig(&m_cfg);

        m_ui.codeView->setConfig(&m_cfg);

        m_cfg.save(CONFIG_FILENAME);
    }
   
}

void MainWindow::ICore_onSignalReceived(QString signalName)
{
    if(signalName != "SIGINT")
    {
        //
        QString msgText;
        msgText.sprintf("Program received signal %s.", stringToCStr(signalName));
        QString title = "Signal received";
        QMessageBox::warning(this, title, msgText);
    }
    
    m_ui.codeView->disableCurrentLine();

    fillInStack();

}

void MainWindow::ICore_onTargetOutput(QString message)
{
    if(message.endsWith("\n"))
    {
        message = message.left(message.length()-1);
    }
    if(message.endsWith("\r"))
    {
        message = message.left(message.length()-1);
    }
    m_ui.targetOutputView->appendPlainText(message);    
}



void MainWindow::ICore_onStateChanged(TargetState state)
{
    m_ui.actionNext->setEnabled(state == TARGET_STOPPED ? true : false);
    m_ui.actionStep_In->setEnabled(state == TARGET_STOPPED ? true : false);
    m_ui.actionStep_Out->setEnabled(state == TARGET_STOPPED ? true : false);
    m_ui.actionStop->setEnabled(state == TARGET_STOPPED ? false : true);
    m_ui.actionContinue->setEnabled(state == TARGET_STOPPED ? true : false);
    m_ui.actionRun->setEnabled(state == TARGET_STOPPED ? true : false);
    
    if(state == TARGET_RUNNING)
    {
        m_ui.treeWidget_stack->clear();
        m_ui.autoWidget->clear();
    }
}


void MainWindow::onFuncListItemActivated(int index)
{
    int funcLineNo = m_ui.comboBox_funcList->itemData(index).toInt();
    int lineIdx = funcLineNo-2;
    if(lineIdx < 0)
        lineIdx = 0;
    m_ui.scrollArea_codeView->verticalScrollBar()->setValue(m_ui.codeView->getRowHeight()*lineIdx);
}


    

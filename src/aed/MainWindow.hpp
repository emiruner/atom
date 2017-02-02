#ifndef AED_MAINWINDOW_HPP_INCLUDED
#define AED_MAINWINDOW_HPP_INCLUDED

#include <QtGui>

#include <vm/Memory.hpp>
#include "ObjectModel.hpp"

namespace atom
{

namespace aed
{

class MainWindow : public QMainWindow
{
    Q_OBJECT;

    Memory* mem;

    ObjectModel* objectTreeModel;

    QSplitter* splitter;
    QTreeView* objectTree;

    // Actions
    QAction* newAction;
    QAction* loadAction;
    QAction* quitAction;

    QAction* newSimpleFunctionAction;
    QAction* deleteArrayElementAction;
    QAction* setObjectAction;
    QAction* resizeArrayAction;

    // Menus
    QMenu* fileMenu;
    QMenu* actionsMenu;

public:
    MainWindow();

private:
    QAction* createAction(char const* label, char const* shortcut = 0);
    void createActions();
    void createMenus();

    void deleteFromObjectArray(QModelIndex const& childIndex, RefNode* parentNode);
    void deleteFromByteArray(QModelIndex const& childIndex, RefNode* parentNode);

    Ref putIntoArray(Ref object);

private slots:
    void newObjectStore();
    void loadObjectStore();
    void newSimpleFunction();
    void deleteArrayElement();
    void setObject();
    void resizeArray();
};

} // namespace aed

} // namespace atom

#endif /* AED_MAINWINDOW_HPP_INCLUDED */

#include "MainWindow.hpp"
#include <os/ObjectStore.hpp>

#include <stdexcept>

namespace
{
    inline char* c_str(QString const& qs)
    {
        QByteArray ba = qs.toLatin1();
        return ba.data();
    }

    void dummy(atom::Thread*, int, atom::Ref)
    {
        throw std::runtime_error("must not be called");
    }

} // namespace <unnamed>

namespace atom
{

namespace aed
{

MainWindow::MainWindow()
{
    setWindowTitle("atom Object Editor");

    objectTreeModel = new ObjectModel(this);
    mem = new Memory(10240);

    objectTree = new QTreeView(this);
    objectTree->setModel(objectTreeModel);

    splitter = new QSplitter(Qt::Horizontal);
    setCentralWidget(splitter);
    splitter->addWidget(objectTree);
    splitter->addWidget(new QTextEdit());

    createActions();
    createMenus();

    newObjectStore();
}

QAction* MainWindow::createAction(char const* label, char const* shortcut)
{
    QAction* action = new QAction(tr(label), this);

    action->setShortcut(tr(shortcut));
    action->setStatusTip(tr(label));

    return action;
}

void MainWindow::createActions()
{
    newAction = createAction("New Object Store", "Ctrl+N");
    loadAction = createAction("Load Object Store", "Ctrl+L");
    newSimpleFunctionAction = createAction("New Simple Function");
    deleteArrayElementAction = createAction("Delete Array Element", "Ctrl+D");
    setObjectAction = createAction("Set Object", "Ctrl+U");
    resizeArrayAction = createAction("Resize Array", "Ctrl+R");
    quitAction = createAction("Quit", "Ctrl+X");

    connect(newAction, SIGNAL(triggered()), this, SLOT(newObjectStore()));
    connect(loadAction, SIGNAL(triggered()), this, SLOT(loadObjectStore()));
    connect(newSimpleFunctionAction, SIGNAL(triggered()), this, SLOT(newSimpleFunction()));
    connect(deleteArrayElementAction, SIGNAL(triggered()), this, SLOT(deleteArrayElement()));
    connect(setObjectAction, SIGNAL(triggered()), this, SLOT(setObject()));
    connect(resizeArrayAction, SIGNAL(triggered()), this, SLOT(resizeArray()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(loadAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    actionsMenu = menuBar()->addMenu(tr("Actions"));

    actionsMenu->addAction(setObjectAction);
    actionsMenu->addAction(newSimpleFunctionAction);
    actionsMenu->addAction(deleteArrayElementAction);
    actionsMenu->addAction(resizeArrayAction);
}

Ref MainWindow::putIntoArray(Ref ref)
{
    ObjectArray* oa = mem->createObjectArray(1);
    oa->atPut(0, ref);
    return ptr2ref(oa);
}

void MainWindow::newObjectStore()
{
    objectTreeModel->setRootFromRef(putIntoArray(ptr2ref(mem->createObjectArray(1))));
}

void MainWindow::loadObjectStore()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Object Store"), ".", tr("Object Store Files (*.os)"));

    if(!filename.isEmpty())
    {
        objectTreeModel->setRootFromRef(putIntoArray(ptr2ref(ObjectStoreReader(c_str(filename), mem).readAll())));
    }
}

void MainWindow::newSimpleFunction()
{
    QModelIndexList selected = objectTree->selectionModel()->selectedIndexes();

    if(selected.size() > 0)
    {
        bool ok;
        int tmpCount = QInputDialog::getInt(this, tr("Please enter number of temporary slots this function will use"),
                tr("# of temporary slots"), 0, 0, 100, 1, &ok);

        if(ok)
        {
            Object* oldArr = cast<Object>(static_cast<RefNode*>(objectTreeModel->getRoot())->getObject());
            Object* newArr = mem->createObject<Object>(1 + oldArr->size() + 1);

            for(int i = 0; i < oldArr->size(); ++i)
            {
                newArr->atPut(i, oldArr->at(i));
            }

            newArr->atPut(oldArr->size(), ptr2ref(mem->createObject<SimpleFunction>(sizeof(SimpleFunction) / sizeof(Ref) + tmpCount)));
            objectTreeModel->setRootFromRef(ptr2ref(newArr));
        }
    }
}

void MainWindow::deleteFromObjectArray(QModelIndex const& childIndex, RefNode* parentNode)
{
    Object* oldArr = cast<Object>(parentNode->getObject());

    if(oldArr->size() == 0)
    {
        QMessageBox::critical(this, "Object Editor", "Array is empty. Cannot remove more elements.!", QMessageBox::Ok);
        return;
    }

    Object* newArr = mem->createObject<Object>(1 + oldArr->size() - 1);

    for(int i = 0; i < childIndex.row(); ++i)
    {
        newArr->atPut(i, oldArr->at(i));
    }

    for(int i = childIndex.row() + 1; i < oldArr->size(); ++i)
    {
        newArr->atPut(i - 1, oldArr->at(i));
    }

    objectTreeModel->updateRefNodeContents(parentNode, ptr2ref(newArr));
}

void MainWindow::deleteFromByteArray(QModelIndex const& childIndex, RefNode* parentNode)
{
    ByteArray* oldArr = cast<ByteArray>(parentNode->getObject());

    if(oldArr->size() == 0)
    {
        QMessageBox::critical(this, "Object Editor", "Array is empty. Cannot remove more elements.!", QMessageBox::Ok);
        return;
    }


    Ref newDataRef = mem->createManagedByteArray(oldArr->size() - 1);
    byte* newData = cast<ByteArray>(newDataRef)->data();

    for(int i = 0; i < childIndex.row(); ++i)
    {
        newData[i] = oldArr->data()[i];
    }

    for(int i = childIndex.row() + 1; i < oldArr->size(); ++i)
    {
        newData[i - 1] = oldArr->data()[i];
    }

    objectTreeModel->updateRefNodeContents(parentNode, newDataRef);
}

void MainWindow::deleteArrayElement()
{
    QModelIndexList selected = objectTree->selectionModel()->selectedIndexes();

    if(selected.size() > 0 && selected.at(0).internalPointer() != 0)
    {
        QModelIndex childIndex = selected.at(0);
        ObjectNode* child = static_cast<ObjectNode*>(childIndex.internalPointer());

        RefNode* parentNode = dynamic_cast<RefNode*>(static_cast<ObjectNode*>(child->getParent()));

        if(parentNode == 0 || (!is_object(parentNode->getObject()) && !is_byte_array(parentNode->getObject())))
        {
            QMessageBox::critical(this, "Object Editor", "Elements can be removed from either an ObjectArray or ByteArray!", QMessageBox::Ok);
        }
        else
        {
            if(is_object(parentNode->getObject()))
            {
                deleteFromObjectArray(childIndex, parentNode);
            }
            else if(is_byte_array(parentNode->getObject()))
            {
                deleteFromByteArray(childIndex, parentNode);
            }
            else
            {
                throw std::runtime_error("Must not reach here.");
            }
        }
    }
    else
    {
        QMessageBox::critical(this, "Object Editor", "You must select an array element to execute this action!", QMessageBox::Ok);
    }
}

void MainWindow::setObject()
{
    QModelIndexList selected = objectTree->selectionModel()->selectedIndexes();

    if(selected.size() > 0 && selected.at(0).internalPointer() != 0)
    {
        QModelIndex childIndex = selected.at(0);
        RefNode* child = dynamic_cast<RefNode*>(static_cast<ObjectNode*>(childIndex.internalPointer()));

        RefNode* parentNode = dynamic_cast<RefNode*>(static_cast<ObjectNode*>(child->getParent()));

        if(parentNode == 0 || dynamic_cast<RefNode*>(child) == 0)
        {
            QMessageBox::critical(this, "Object Editor", "Selected node must be a ref node!", QMessageBox::Ok);
        }
        else
        {
            // TODO: Implement updating.
        }
    }
    else
    {
        QMessageBox::critical(this, "Object Editor", "You must select an array element to execute this action!", QMessageBox::Ok);
    }
}

void MainWindow::resizeArray()
{
    QModelIndexList selected = objectTree->selectionModel()->selectedIndexes();

    if(selected.size() > 0 && selected.at(0).internalPointer() != 0)
    {
        QModelIndex childIndex = selected.at(0);
        ObjectArrayNode* childNode = dynamic_cast<ObjectArrayNode*>(static_cast<ObjectNode*>(childIndex.internalPointer()));

        if(childNode == 0)
        {
            QMessageBox::critical(this, "Object Editor", "Selected node must be an object array node!", QMessageBox::Ok);
        }
        else
        {
            Object* child = cast<Object>(childNode->getObject());
            bool ok;
            int newSize = QInputDialog::getInt(this, tr("Please enter number of temporary slots this function will use"),
                    tr("# of temporary slots"), child->size(), 0, 100, 1, &ok);

            Object* newArr = mem->createObject<Object>(1 + newSize);

            for(int i = 0; i < child->size(); ++i)
            {
                newArr->atPut(i, child->at(i));
            }

            objectTreeModel->updateRefNodeContents(childNode, ptr2ref(newArr));
        }
    }
    else
    {
        QMessageBox::critical(this, "Object Editor", "You must select an array element to execute this action!", QMessageBox::Ok);
    }
}

} // namespace aed

} // namespace atom

#include "MainWindow.hpp.moc"

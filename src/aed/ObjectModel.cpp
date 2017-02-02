#include "ObjectModel.hpp"

#include <stdexcept>

namespace atom
{

namespace aed
{

ObjectModel::ObjectModel(QObject* parent)
: QAbstractItemModel(parent)
{
    root_ = 0;
}

void ObjectModel::setRoot(ObjectNode* root)
{
    root_ = root;
    reset();
}

void ObjectModel::setRootFromRef(Ref ref)
{
    setRoot(RefNode::createRefNode(0, ref));
}

QModelIndex ObjectModel::index(int row, int column, const QModelIndex &parentIndex) const
{
    ObjectNode* parent = nodeFromIndex(parentIndex);
    return createIndex(row, column, parent->getChild(row));
}

ObjectNode* ObjectModel::nodeFromIndex(QModelIndex const& index) const
{
    if(index.isValid())
    {
        return static_cast<ObjectNode*>(index.internalPointer());
    }
    else
    {
        return root_;
    }
}

int ObjectModel::rowCount(const QModelIndex &parentIndex) const
{
    ObjectNode* parent = nodeFromIndex(parentIndex);

    if(!parent)
    {
        return 0;
    }
    else
    {
        return parent->getChildCount();
    }
}

int ObjectModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QModelIndex ObjectModel::parent(const QModelIndex &child) const
{
    ObjectNode *node = nodeFromIndex(child);

    if(!node)
    {
        return QModelIndex();
    }

    ObjectNode *parentNode = node->getParent();

    if(!parentNode)
    {
        return QModelIndex();
    }

    ObjectNode *grandparentNode = parentNode->getParent();

    if(grandparentNode == 0)
    {
        return QModelIndex();
    }
    else
    {
        int row = grandparentNode->getIndexOfChild(parentNode);
        return createIndex(row, 0, parentNode);
    }
}

QVariant ObjectModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    ObjectNode *node = nodeFromIndex(index);

    if(!node)
    {
        return QVariant();
    }

    if (index.column() == 0)
    {
        return node->getNodeLabel().c_str();
    }
    else if(index.column() == 1)
    {
        return node->getAddressStr().c_str();
    }
    else if(index.column() == 2)
    {
        return node->getValueStr().c_str();
    }

    return QVariant();
}

QVariant ObjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if(section == 0)
        {
            return tr("Object");
        }
        else if(section == 1)
        {
            return tr("Address");
        }
        else if (section == 2)
        {
            return tr("Value");
        }
    }

    return QVariant();
}

void ObjectModel::updateRefNodeContents(RefNode* refNode, Ref newContent)
{
    refNode->setObject(newContent);
    emit layoutChanged();
}

} // namespace aed

} // namespace atom

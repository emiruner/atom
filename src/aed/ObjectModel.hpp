#ifndef AED_OBJECTMODEL_HPP_INCLUDED
#define AED_OBJECTMODEL_HPP_INCLUDED

#include <QAbstractItemModel>

#include "ObjectNode.hpp"

namespace atom
{

namespace aed
{

class ObjectModel : public QAbstractItemModel
{
public:
    ObjectModel(QObject* parent = 0);
    void setRoot(ObjectNode* root);
    void setRootFromRef(Ref ref);

    inline ObjectNode* getRoot()
    {
        return root_;
    }

    void updateRefNodeContents(RefNode* refNode, Ref newContent);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    ObjectNode* root_;
    ObjectNode* nodeFromIndex(QModelIndex const& index) const;
};

} // namespace aed

} // namespace atom

#endif /* AED_OBJECTMODEL_HPP_INCLUDED */

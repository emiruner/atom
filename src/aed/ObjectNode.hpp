#ifndef AED_OBJECTNODE_HPP_INCLUDED
#define AED_OBJECTNODE_HPP_INCLUDED

#include <vector>
#include <vm/Object.hpp>

namespace atom
{

namespace aed
{

class ObjectNode;

typedef std::vector<ObjectNode*> ObjectNodeVec;

class ObjectNode
{
protected:
    ObjectNode* parent_;
    ObjectNodeVec children_;
    std::string prefix_;

public:
    inline ObjectNode()
    : parent_(0)
    {
    }

    explicit inline ObjectNode(ObjectNode* parent)
    : parent_(parent)
    {
    }

    virtual ~ObjectNode()
    {
        for(ObjectNodeVec::const_iterator it = children_.begin(); it != children_.end(); ++it)
        {
            delete *it;
        }
    }

    ObjectNode* getChild(int index);
    int getChildCount();
    int getIndexOfChild(ObjectNode* child);

    inline ObjectNode* getParent()
    {
        return parent_;
    }

    inline void setParent(ObjectNode* parent)
    {
        parent_ = parent;
    }

    inline void setPrefix(std::string prefix)
    {
        prefix_ = prefix;
    }

    inline std::string getNodeLabel()
    {
        if(prefix_.empty())
        {
            return getNodeName();
        }
        else
        {
            return prefix_ + " :" + getNodeName();
        }
    }

    virtual std::string getNodeName() = 0;
    virtual std::string getAddressStr() = 0;
    virtual std::string getValueStr() = 0;

protected:
    virtual void createChildren() = 0;

    inline void resetChildren()
    {
        children_.clear();
    }

private:
    void cacheChildren();
};

class RefNode : public ObjectNode
{
protected:
    Ref object_;

public:
    inline RefNode()
    : ObjectNode(0), object_(zeroRef())
    {
    }

    inline RefNode(ObjectNode* parent, Ref object)
    : ObjectNode(parent), object_(object)
    {
    }

    inline void setObject(Ref object)
    {
        object_ = object;
        resetChildren();
    }

    inline Ref getObject()
    {
        return object_;
    }

    virtual std::string getNodeName() = 0;
    virtual std::string getAddressStr();
    virtual std::string getValueStr();

    static RefNode* createRefNode(ObjectNode* parent, Ref ref, std::string prefix = "");

protected:
    virtual void createChildren();
};

class IntRefNode : public RefNode
{
public:
    inline IntRefNode(ObjectNode* parent, Ref intRef)
    : RefNode(parent, intRef)
    {
    }

    virtual std::string getNodeName();
    virtual std::string getAddressStr();
    virtual std::string getValueStr();
    virtual void createChildren();
};

class FloatRefNode : public RefNode
{
public:
    inline FloatRefNode(ObjectNode* parent, Ref ref)
    : RefNode(parent, ref)
    {
    }

    virtual std::string getNodeName();
    virtual std::string getValueStr();
    virtual void createChildren();
};

class ByteArrayNode : public RefNode
{
    bool isCString_;
    std::string cStringVal_;

public:
    inline ByteArrayNode(ObjectNode* parent, Ref intRef)
    : RefNode(parent, intRef), isCString_(false)
    {
        ByteArray* ba = cast<ByteArray>(intRef);

        if(ba->data()[ba->size() - 1] == '\0')
        {
            bool allNormalChars = true;

            for(int i = 0; i < (ba->size() - 1); ++i)
            {
                byte ch = ba->data()[i];

                if(ch < 32)
                {
                    allNormalChars = false;
                    break;
                }
            }

            if(allNormalChars)
            {
                isCString_ = true;
                cStringVal_ = "a c-string: \'" + std::string((char const*) ba->data()) + "\'";
            }
        }
    }

    virtual std::string getNodeName();
    virtual std::string getValueStr();
    virtual void createChildren();
};

class SimpleFunctionNode : public RefNode
{
public:
    inline SimpleFunctionNode(ObjectNode* parent, Ref intRef)
    : RefNode(parent, intRef)
    {
    }

    virtual std::string getNodeName();
    virtual void createChildren();
};

class ObjectArrayNode : public RefNode
{
public:
    inline ObjectArrayNode(ObjectNode* parent, Ref oaRef)
    : RefNode(parent, oaRef)
    {
    }

    virtual std::string getNodeName();
};

class NativeFunctionNode : public RefNode
{
public:
    inline NativeFunctionNode(ObjectNode* parent, Ref intRef)
    : RefNode(parent, intRef)
    {
    }

    virtual std::string getNodeName();
    virtual void createChildren();
};

class RegularObjectNode : public RefNode
{
public:
    inline RegularObjectNode(ObjectNode* parent, Ref intRef)
    : RefNode(parent, intRef)
    {
    }

    virtual std::string getNodeName();
    virtual void createChildren();
};

} // namespace aed

} // namespace atom

#endif /* AED_OBJECTNODE_HPP_INCLUDED */

#include "ObjectNode.hpp"

#include <stdexcept>
#include <sstream>

namespace
{

template <typename T>
inline std::string toStr(T const& t)
{
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

} // namespace <unnamed>


namespace atom
{

namespace aed
{

ObjectNode* ObjectNode::getChild(int index)
{
    cacheChildren();

    if(index < 0 || static_cast<unsigned int>(index) >= children_.size())
    {
        return 0;
        // throw std::runtime_error("invalid child index");
    }

    return children_[index];
}

int ObjectNode::getChildCount()
{
    cacheChildren();
    return children_.size();
}

int ObjectNode::getIndexOfChild(ObjectNode* child)
{
    unsigned int count = children_.size();

    for(unsigned int i = 0; i < count; ++i)
    {
        if(child == children_[i])
        {
            return i;
        }
    }

    throw std::runtime_error("must not reach here");
}

void ObjectNode::cacheChildren()
{
    if(children_.empty())
    {
        createChildren();
    }
}

void RefNode::createChildren()
{
    Object* oa = cast<Object>(object_);

    for(unsigned int i = 0; i < oa->size(); ++i)
    {
        children_.push_back(createRefNode(this, oa->at(i)));
    }
}

std::string RefNode::getAddressStr()
{
    return toStr(ptr_val(object_)).c_str();
}

std::string RefNode::getValueStr()
{
    return "";
}

RefNode* RefNode::createRefNode(ObjectNode* parent, Ref ref, std::string prefix)
{
    RefNode* r;

    if(is_int(ref))
    {
        r = new IntRefNode(parent, ref);
    }
    else if(is_byte_array(ref))
    {
        r = new ByteArrayNode(parent, ref);
    }
    else if(is_simple_fn(ref))
    {
        r = new SimpleFunctionNode(parent, ref);
    }
    else if(is_native_fn(ref))
    {
        r = new NativeFunctionNode(parent, ref);
    }
    else if(is_float(ref))
    {
        r = new FloatRefNode(parent, ref);
    }
    else if(is_object(ref))
    {
        r = new RegularObjectNode(parent, ref);
    }
    else if(is_object_array(ref))
    {
        r = new ObjectArrayNode(parent, ref);
    }
    else
    {
        throw std::runtime_error("must not reach here normally");
    }

    r->setPrefix(prefix);

    return r;
}

// ---- IntRefNode methods -------------------------------------------------------------------------------------------
std::string IntRefNode::getAddressStr()
{
    return "";
}

std::string IntRefNode::getValueStr()
{
    return toStr(int_val(object_));
}

void IntRefNode::createChildren()
{
    // Do nothing.
}

std::string IntRefNode::getNodeName()
{
    return "Integer";
}

// ---- FloatRefNode methods -----------------------------------------------------------------------------------------
std::string FloatRefNode::getValueStr()
{
    return toStr(float_val(object_));
}

void FloatRefNode::createChildren()
{
    // Do nothing.
}

std::string FloatRefNode::getNodeName()
{
    return "Float";
}

// ---- ByteArrayNode methods ----------------------------------------------------------------------------------------
void ByteArrayNode::createChildren()
{
    if(!isCString_)
    {
        ByteArray* ba = cast<ByteArray>(object_);

        for(unsigned int i = 0; i < ba->size(); ++i)
        {
            children_.push_back(createRefNode(this, ba->at(i), "[" + toStr(i) + "]"));
        }
    }
}

std::string ByteArrayNode::getValueStr()
{
    if(isCString_)
    {
        return cStringVal_;
    }
    else
    {
        return std::string("size: ") + toStr(cast<ByteArray>(object_)->size());
    }
}

std::string ByteArrayNode::getNodeName()
{
    return "Byte Array";
}

// ---- SimpleFunctionNode methods -----------------------------------------------------------------------------------
void SimpleFunctionNode::createChildren()
{
    SimpleFunction* s = cast<SimpleFunction>(object_);

    children_.push_back(createRefNode(this, s->bytecodes_, "bytecodes"));
    children_.push_back(createRefNode(this, s->tempCount_, "temp count"));

    Object* oa = cast<Object>(object_);

    for(unsigned int i = 2; i < oa->size(); ++i)
    {
        children_.push_back(createRefNode(this, oa->at(i), "[" + toStr(i) + "]"));
    }
}

std::string SimpleFunctionNode::getNodeName()
{
    return "Simple Function";
}

// ---- ObjectArrayNode methods -----------------------------------------------------------------------------------

std::string ObjectArrayNode::getNodeName()
{
    return "Object Array";
}

// ---- NativeFunctionNode methods -----------------------------------------------------------------------------------
void NativeFunctionNode::createChildren()
{
    // No children.
}

std::string NativeFunctionNode::getNodeName()
{
    return "Native Function";
}

// ---- ContinuationNode methods -------------------------------------------------------------------------------------
void RegularObjectNode::createChildren()
{
    Object* o = cast<Object>(object_);

    children_.push_back(createRefNode(this, o->metaObject_, "meta object"));

    for(unsigned int i = 0; i < (o->size() - 1); ++i)
    {
        children_.push_back(createRefNode(this, o->at(i + 1)));
    }
}

std::string RegularObjectNode::getNodeName()
{
    return "Regular Object";
}

} // namespace aed

} // namespace atom

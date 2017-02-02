/**
 * This file is part of Atom project.
 *
 * Copyright (C) 2010 Emir Uner
 */

#ifndef ATOM_ASSEMBLER_PARSER_HPP_INCLUDED
#define ATOM_ASSEMBLER_PARSER_HPP_INCLUDED

#include "Lexer.hpp"

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace atom
{

class ObjectArray;

namespace assembler
{

class VarNode;
class Node;
class TopLevelDef;
class InstructionNode;

typedef std::vector<Node*> NodeVec;
typedef std::vector<InstructionNode*> InstructionNodeVec;
typedef std::vector<TopLevelDef*> TopLevelDefVec;
typedef std::vector<VarNode*> VarNodeVec;
typedef std::vector<int> IntVec;
typedef std::map<std::string, int> StringIntMap;
typedef std::map<int, std::string> IntStringMap;

template <typename T>
inline bool isa(Node* n)
{
    return dynamic_cast<T*>(n) != 0;
}

class Node
{
public:
    inline virtual ~Node()
    {
    }
};

class TopLevelDef
{
public:
    inline TopLevelDef()
    {
    }

    inline TopLevelDef(std::string const& id, Node* def)
    : name_(id), def_(def)
    {
    }

    inline Node* def()
    {
        return def_;
    }

    inline void def(Node* def)
    {
        def_ = def;
    }

    inline std::string const& name() const
    {
        return name_;
    }

    inline void name(std::string const& name)
    {
        name_ = name;
    }

    template <typename T>
    T& extract()
    {
        return dynamic_cast<T&>(*def_);
    }

protected:
    std::string name_;
    Node* def_;
};

class VarNode : public Node
{
public:
    VarNode(int index, std::string const& name, Node* value)
    : index_(index), name_(name), value_(value)
    {
    }

    inline int const& index() const
    {
        return index_;
    }

    inline void index(int const& index)
    {
        index_ = index;
    }

    inline std::string const& name() const
    {
        return name_;
    }

    inline void name(std::string const& name)
    {
        name_ = name;
    }

    inline Node* const& value() const
    {
        return value_;
    }

    inline void value(Node* value)
    {
        value_ = value;
    }

private:
    int index_;
    std::string name_;
    Node* value_;
};

class SymbolicRef : public Node
{
public:
    inline SymbolicRef(std::string const& name)
    : name_(name)
    {
    }

    inline std::string const& name() const
    {
        return name_;
    }

    inline void name(std::string const& name)
    {
        name_ = name;
    }

protected:
    std::string name_;
};

class InstructionNode : public Node
{
};

class SendNode : public InstructionNode
{
public:
    inline SendNode()
    : message_(0), target_(0), result_(0)
    {
    }

    inline Node* const& message() const
    {
        return message_;
    }

    inline void message(Node* message)
    {
        message_ = message;
    }

    inline Node* const& target() const
    {
        return target_;
    }

    inline void target(Node* target)
    {
        target_ = target;
    }

    inline SymbolicRef* const& result() const
    {
        return result_;
    }

    inline void result(SymbolicRef* result)
    {
        result_ = result;
    }

private:
    Node* message_;
    Node* target_;
    SymbolicRef* result_;
};

class InstallExHandlerNode : public InstructionNode
{
public:
    inline Node* const& handler() const
    {
        return handler_;
    }
    
    inline void handler(Node* handler)
    {
        handler_ = handler;
    }
    
private:
    Node* handler_;
};

class RaiseExceptionNode : public InstructionNode
{
public:
    inline Node* const& exception() const
    {
        return exception_;
    }
    
    inline void exception(Node* exception)
    {
        exception_ = exception;
    }
    
private:
    Node* exception_;
};

class ArrayOpNode : public InstructionNode
{
public:
    inline SymbolicRef* const& arr() const
    {
        return arr_;
    }

    inline void arr(SymbolicRef* arr)
    {
        arr_ = arr;
    }

    inline SymbolicRef* const& result() const
    {
        return result_;
    }

    inline void result(SymbolicRef* result)
    {
        result_ = result;
    }

private:
    SymbolicRef* arr_;
    SymbolicRef* result_;

};

class JumpNode : public InstructionNode
{
public:
    inline SymbolicRef* const& label() const
    {
        return label_;
    }

    inline void label(SymbolicRef* label)
    {
        label_ = label;
    }

private:
    SymbolicRef* label_;
};

class SetNode : public InstructionNode
{
public:
    inline SymbolicRef* const& target() const
    {
        return target_;
    }

    inline void target(SymbolicRef* target)
    {
        target_ = target;
    }
    
    inline Node* const& value() const
    {
        return value_;
    }

    inline void value(Node* value)
    {
        value_ = value;
    }
    
private:
    Node* value_;
    SymbolicRef* target_;
};

class AlenNode : public ArrayOpNode
{
};

class IndexedArrayOpNode : public ArrayOpNode
{
public:
    inline Node* const& index() const
    {
        return index_;
    }

    inline void index(Node* index)
    {
        index_ = index;
    }

private:
    Node* index_;
};

class AatNode : public IndexedArrayOpNode
{
};

class AputNode : public IndexedArrayOpNode
{
public:
    inline Node* const& value() const
    {
        return value_;
    }

    inline void value(Node* value)
    {
        value_ = value;
    }

private:
    Node* value_;
};

class ConditionalNode : public InstructionNode
{
public:
    inline ConditionalNode()
    : cond_(0)
    {
    }

    inline SymbolicRef* const& cond() const
    {
        return cond_;
    }

    inline void cond(SymbolicRef* cond)
    {
        cond_ = cond;
    }

    inline bool positive() const
    {
        return positive_;
    }

    inline void positive(bool positive)
    {
        positive_ = positive;
    }

private:
    SymbolicRef* cond_;
    bool positive_;
};

class LabelNode : public InstructionNode
{
public:
    inline LabelNode()
    : name_(0)
    {
    }

    inline SymbolicRef* const& name() const
    {
        return name_;
    }

    inline void name(SymbolicRef* name)
    {
        name_ = name;
    }

private:
    SymbolicRef* name_;
};


class ReturnNode : public InstructionNode
{
public:
    inline ReturnNode()
    : result_(0)
    {
    }

    inline Node* const& result() const
    {
        return result_;
    }

    inline void result(Node* result)
    {
        result_ = result;
    }

private:
    Node* result_;
};

class CroaNode : public InstructionNode
{
public:
    inline SymbolicRef* const& result() const
    {
        return result_;
    }

    inline void result(SymbolicRef* result)
    {
        result_ = result;
    }

    inline NodeVec& elements()
    {
        return elements_;
    }

    inline void elements(NodeVec const& elements)
    {
        elements_ = elements;
    }

private:
    NodeVec elements_;
    SymbolicRef* result_;
};

class CroNode : public InstructionNode
{
public:
    inline SymbolicRef* const& result() const
    {
        return result_;
    }

    inline void result(SymbolicRef* result)
    {
        result_ = result;
    }

    inline NodeVec& elements()
    {
        return elements_;
    }

    inline void elements(NodeVec const& elements)
    {
        elements_ = elements;
    }

private:
    NodeVec elements_;
    SymbolicRef* result_;
};

class HaltNode : public InstructionNode
{
};

class ObjectDefNode : public Node
{
};

class ObjectObjectDefNode : public ObjectDefNode
{
public:
    inline ObjectObjectDefNode(Node* metaObject, NodeVec const& remaining)
    : metaObject_(metaObject), remaining_(remaining)
    {
    }

    inline Node* metaObject()
    {
        return metaObject_;
    }

    inline void metaObject(Node* metaObject)
    {
        metaObject_ = metaObject;
    }

    inline NodeVec& remaining()
    {
        return remaining_;
    }

    inline void remaining(NodeVec const& remaining)
    {
        remaining_ = remaining;
    }

private:
    Node* metaObject_;
    NodeVec remaining_;
};

class ArrayDefBase : public ObjectDefNode
{
public:
    inline NodeVec& elements()
    {
        return elements_;
    }

    inline void elements(NodeVec const& elements)
    {
        elements_ = elements;
    }

private:
    NodeVec elements_;
};

class ObjectArrayDefNode : public ArrayDefBase
{
};

class BytearrayDefNode : public ArrayDefBase
{
};

class SimpleFnDefNode : public ObjectDefNode
{
public:
    inline SimpleFnDefNode()
    : lastFreeTempIndex_(-1), temps_(0)
    {
    }

    inline VarNodeVec& vars()
    {
        return vars_;
    }

    inline void vars(VarNodeVec const& vars)
    {
        vars_ = vars;
    }

    inline InstructionNodeVec& instructions()
    {
        return instructions_;
    }

    inline int lastFreeTempIndex()
    {
        return lastFreeTempIndex_;
    }
    inline void instructions(InstructionNodeVec const& instructions)
    {
        instructions_ = instructions;
    }

    inline ObjectArray* temps()
    {
        return temps_;
    }

    inline void temps(ObjectArray* t)
    {
        temps_ = t;
    }

    inline void allocateFreeTemp(std::string const& name)
    {
        tempNames_[name] = ++lastFreeTempIndex_;
    }

    inline void setNameForIndex(int index, std::string const& name)
    {
        assignedTemps_.push_back(index);
        tempNames_[name] = index;
    }

    inline int allocateKnownObject(std::string const& name)
    {
        int index = allocateFreeSlotFromTemps();
        tempNames_[name] = index;
        return index;
    }

    inline int allocateFreeSlotFromTemps()
    {
        for(int i = 0; i < 255; ++i)
        {
            if(std::find(assignedTemps_.begin(), assignedTemps_.end(), i) == assignedTemps_.end())
            {
                assignedTemps_.push_back(i);
                return i;
            }
        }

        throw std::runtime_error("cannot find free temp slot");
    }

    inline int indexOfNamedTemp(std::string const& name)
    {
        if(tempNames_.find(name) == tempNames_.end())
        {
            return -1;
        }

        return tempNames_[name];
    }

    inline void fixFreeTemps(int offset)
    {
        for(VarNodeVec::iterator it = vars_.begin(); it != vars_.end(); ++it)
        {
            VarNode* vn = *it;

            if(vn->value() == 0 && vn->index() == -1)
            {
                tempNames_[vn->name()] = tempNames_[vn->name()] + offset;
            }
        }
    }

    inline int emptySlotsBefore(int max)
    {
        int freeCount = 0;

        for(int i = 0; i < max; ++i)
        {
            if(std::find(assignedTemps_.begin(), assignedTemps_.end(), i) == assignedTemps_.end())
            {
                ++freeCount;
            }
        }

        return freeCount;
    }

    inline void dump()
    {
        std::cout << "lastFreeTempIndex: " << lastFreeTempIndex_ << '\n';
        std::cout << "Assigned temps: \n";

        for(IntVec::iterator it = assignedTemps_.begin(); it != assignedTemps_.end(); ++it)
        {
            std::cout << *it << ", ";
        }

        std::cout << "\n----------------\n";

        std::cout << "Temp Names: \n";

        for(StringIntMap::iterator it = tempNames_.begin(); it != tempNames_.end(); ++it)
        {
            std::cout << it->first << " -> " << it->second << '\n';
        }

        std::cout << "----------------\n";
    }

    bool isTempVarRef(Node* n)
    {
        return isa<SymbolicRef>(n) && indexOfNamedTemp(static_cast<SymbolicRef*>(n)->name()) != -1;
    }

private:
    int lastFreeTempIndex_;
    ObjectArray* temps_;
    IntVec assignedTemps_;
    StringIntMap tempNames_;
    VarNodeVec vars_;
    InstructionNodeVec instructions_;
};

class SimplefnRawNode : public ObjectDefNode
{
public:
    inline Node* bytecodes()
    {
        return bytecodes_;
    }

    inline Node* tempcount()
    {
        return tempcount_;
    }

    inline NodeVec& knownObjects()
    {
        return knownObjects_;
    }

    inline void bytecodes(Node* bytecodes)
    {
        bytecodes_ = bytecodes;
    }

    inline void tempcount(Node* tempcount)
    {
        tempcount_ = tempcount;
    }

    inline void knownObjects(NodeVec const& knownObjects)
    {
        knownObjects_ = knownObjects;
    }

private:
    // The following may be either symbolic names referring to object or object definitions
    Node* bytecodes_;
    Node* tempcount_;
    NodeVec knownObjects_;
};

class IntegerLiteralNode : public ObjectDefNode
{
public:
    IntegerLiteralNode(int literal)
    : literal_(literal)
    {
    }

    inline int literal() const
    {
        return literal_;
    }
private:
    int literal_;
};

class StringLiteralNode : public ObjectDefNode
{
public:
    StringLiteralNode(std::string literal)
    : literal_(literal)
    {
    }

    inline std::string literal() const
    {
        return literal_;
    }
private:
    std::string literal_;
};

class FloatLiteralNode : public ObjectDefNode
{
public:
    FloatLiteralNode(double literal)
    : literal_(literal)
    {
    }

    inline double literal() const
    {
        return literal_;
    }
private:
    double literal_;
};

/**
 * Parser token stream supplied by a lexer and produces an intermediate expression tree.
 */
class Parser
{
public:
    Parser(Lexer& in);
    void parse();
    TopLevelDef* parseToplevel();
    ObjectDefNode* parseAnyObjectDef();
    SimplefnRawNode* parseSimplefnRawBody();
    Node* parseSymbolicRefOrObjectDef();
    SymbolicRef* parseSymbolicRef();
    ObjectArrayDefNode* parseObjectArray();
    void parseCommaSeparatedNodes(NodeVec& elements, TokenType::type closingTokenType = TokenType::RCurly);
    SimpleFnDefNode* parseSimpleFnBody();
    ObjectObjectDefNode* parseObjectBody();
    BytearrayDefNode* parseByteArray();

    inline TopLevelDefVec& toplevelNodes()
    {
        return toplevelNodes_;
    }

private:
    Lexer& in_;
    TopLevelDefVec toplevelNodes_;
};

} // namespace assembler

} // namespace atom

#endif

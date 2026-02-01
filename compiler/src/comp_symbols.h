/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    comp_symbols.h
 * authors: nvitya
 * created: 2026-02-01
 * brief:   Compiler Symbol Objects
 */

#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

class OType;
class OScope;

class OSymbol
{
public:
  string      name;
  OType *     ptype;

  OSymbol(const string aname, OType * atype = nullptr)
  :
    name(aname),
    ptype(atype)
  {

  }

  virtual ~OSymbol() = default;
};

class OScope
{
public:
  OScope *    parent_scope;
  string      debugname; // Helpful for debugging (e.g., "Class Body", "Func Body")

  map<string, OSymbol *>  symbols;

public:
  OScope(OScope * aparent, const string & adebugname)
  :
    parent_scope(aparent),
    debugname(adebugname)
  {
  }

  void Define(OSymbol * asymbol)
  {
    // In a real compiler, check for duplicates here
    symbols[asymbol->name] = asymbol;
  }

  OSymbol * Find(const string & name, OScope ** rscope = nullptr)
  {
    auto it = symbols.find(name);
    if (it != symbols.end())
    {
      if (rscope)
      {
        *rscope = this;
      }
      return it->second;
    }

    // If not found here, check the parent scope
    if (parent_scope != nullptr)
    {
      return parent_scope->Find(name, rscope);
    }

    return nullptr;
  }
};

enum ETypeKind
{
  TK_UNSPECIFIED = 0,
  TK_PRIMITIVE,
  TK_POINTER,
  TK_ARRAY,
  TK_FUNCTION,
  TK_COMPOUND   // object, struct
};

class OType : public OSymbol
{
private:
  using        super = OSymbol;

public:
  ETypeKind    kind = TK_UNSPECIFIED;

  OType(const string aname, ETypeKind akind)
  :
    super(aname, nullptr),  // Types usually don't have a "type" themselves, or are meta-types
    kind(akind)
  {
  }

  inline bool IsPrimitive()  { return (kind == TK_PRIMITIVE); }
  inline bool IsCompound()   { return (kind == TK_COMPOUND);  }
};

class OPrimitiveType : public OType
{
private:
  using        super = OType;

public:

  OPrimitiveType(const string name)
  :
    super(name, TK_PRIMITIVE)
  {
  }
};

class OCompoundType : public OType
{
private:
  using        super = OType;

public:
  OScope       member_scope;

  OCompoundType(const string name, OScope * aparent_scope)
  :
    super(name, TK_COMPOUND),
    member_scope(aparent_scope, name)
  {
  }

  inline OScope * Members() { return &member_scope; }
};


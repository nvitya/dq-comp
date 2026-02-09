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

#include "stdint.h"

#include <string>
#include <vector>
#include <map>

#include "comp_config.h"

using namespace std;

class OType;
class OValSym;
class OScope;

// Symbol and Scope

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

  map<string, OType *>    typesyms;
  map<string, OValSym *>  valsyms;

public:
  OScope(OScope * aparent, const string & adebugname)
  :
    parent_scope(aparent),
    debugname(adebugname)
  {
  }

  OType *     DefineType(OType * atype);
  OValSym *   DefineValSym(OValSym * atype);

  OType *     FindType(const string & name, OScope ** rscope = nullptr, bool arecursive = true);
  OValSym *   FindValSym(const string & name, OScope ** rscope = nullptr, bool arecursive = true);
};

// Types

enum ETypeKind
{
  TK_ALIAS = 0,
  TK_INT,
  TK_FLOAT,
  TK_BOOL,
  TK_POINTER,
  TK_ARRAY,
  TK_STRING,    // ODynString, OCString

  TK_ENUM,
  TK_COMPOUND,  // object, struct
  TK_FUNCTION
};

class OType : public OSymbol
{
private:
  using        super = OSymbol;

public:
  ETypeKind    kind;
  bool         incomplete = false;
  uint32_t     bytesize = 0;  // 0 = size is not fixed (string, dyn. array)

  OType(const string aname, ETypeKind akind)
  :
    super(aname, nullptr),  // Types usually don't have a "type" themselves, or are meta-types
    kind(akind)
  {
  }

  inline bool IsCompound()   { return (kind == TK_COMPOUND);  }
};

class OTypeAlias : public OType
{
private:
  using        super = OType;

public:
  OType *      ptype;

  OTypeAlias(const string aname, OType * aptype)
  :
    super(aname, TK_ALIAS),
    ptype(aptype)
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

// Value Symbols

enum EValSymKind
{
  VSK_CONST = 0,
  VSK_VARIABLE,
  VSK_PARAMETER,
  VSK_FUNCTION
};

class OValSym : public OSymbol
{
private:
  using        super = OSymbol;

public:
  EValSymKind  kind;

  OValSym(const string aname, OType * atype, EValSymKind akind = VSK_VARIABLE)
  :
    super(aname, atype),  // Types usually don't have a "type" themselves, or are meta-types
    kind(akind)
  {
  }
};

class OConstValSym : public OValSym
{
private:
  using        super = OValSym;

public:
  uint8_t *    dataptr = nullptr;
  uint32_t     datalen = 0;

  uint8_t      inlinedata[16] = {0};  // for primitive data (Float80 is the biggest)

  OConstValSym(const string aname, OType * atype)
  :
    super(aname, atype, VSK_CONST)
  {
  }

  void SetInlineData(void * asrcdata, uint32_t alen);
};

/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    statements.h
 * authors: nvitya
 * created: 2026-02-07
 * brief:   statements, statement blocks
 */

#pragma once

#include <vector>
#include "comp_symbols.h"

using namespace std;

//=============================================================================
// EXPRESSIONS
//=============================================================================

enum class BinOp { Add, Sub, Mul };

class OExpr
{
  virtual ~OExpr() {};
};

class OIntLit : public OExpr
{
public:
  int64_t   value;
  OIntLit(int64_t v)
  :
    value(v)
  {
  }
};

class OVarRef : public OExpr
{
  string name;
  VarRef(string n) : name(n) {}
};

struct BinExpr : Expr
{
    BinOp op;
    Expr* left;
    Expr* right;
    BinExpr(BinOp o, Expr* l, Expr* r) : op(o), left(l), right(r) {}
};


//=============================================================================
// STATEMENTS
//=============================================================================

class OStmt
{
public:
  OStmt() { }
  virtual ~OStmt()  { }
};

struct OStmtReturn : public OStmt
{
public:
  Expr *     value;
  OStmtReturn(Expr * v)
  :
    value(v)
  {
  }
};

#if 0
struct OStmtVarDecl : public OStmt
{
public:
  string   name;
  string   type;
  Expr *   initValue; // nullptr if no initialization
  OStmtVarDecl(string n, string t, Expr* init = nullptr)
  :
    name(n),
    type(t),
    initValue(init)
  {
  }
};

struct OStmtAssign : public OStmt
{
public:
  string varName;
  Expr* value;
  AssignStmt(string n, Expr* v)
  :
    varName(n),
    value(v)
  {
  }
};
#endif

class OStmtBlock
{
public:
  OScope *         scope;
  vector<OStmt *>  stlist;

  OStmtBlock(OScope * ascope)
  :
    scope(ascope)
  {
  }

  virtual ~OStmtBlock()
  {
    for (OStmt * st : stlist)
    {
      delete st;
    }
    stlist.clear();
  }
};

class OStmtAssign : public OStmt
{
public:
  OStmtAssign()
  {

  }
};

class OStmtVarDecl : public OStmt
{
  OScope *   scope;
  OValSym *  vsvar;

public:
  OStmtVarDecl(const string avarname, OType * atype, OValSym * ainitvalue)
  {

  }
};
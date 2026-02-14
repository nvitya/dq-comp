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

class OExpr
{
public:
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

class OBoolLit : public OExpr
{
public:
  bool  value;
  OBoolLit(bool v)
  :
    value(v)
  {
  }
};

class OVarRef : public OExpr
{
public:
  OValSym *  pvalsym;
  OVarRef(OValSym * avalsym)
  :
    pvalsym(avalsym)
  {
  }
};

enum EBinOp
{
  BINOP_ADD = 0,
  BINOP_SUB,
  BINOP_MUL
};

class OBinExpr : public OExpr
{
public:
  EBinOp       op;
  OExpr *      left;
  OExpr *      right;

  OBinExpr(EBinOp aop, OExpr * aleft, OExpr * aright)
  :
    op(aop),
    left(aleft),
    right(aright)
  {
  }
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
  OExpr *     value;
  OStmtReturn(OExpr * v)
  :
    value(v)
  {
  }
};

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

  OStmt * AddStatement(OStmt * astmt)
  {
    stlist.push_back(astmt);
    return astmt;
  }
};

class OStmtAssign : public OStmt
{
public:
  OValSym *   variable;
  OExpr *     value;
  OStmtAssign(OValSym * avariable, OExpr * avalue)
  :
    variable(avariable),
    value(avalue)
  {
  }
};

class OStmtVarDecl : public OStmt
{
public:
  OValSym *  variable;
  OExpr *    initvalue;

  OStmtVarDecl(OValSym * avariable, OExpr * ainitvalue)
  :
    variable(avariable),
    initvalue(ainitvalue)
  {
  }
};
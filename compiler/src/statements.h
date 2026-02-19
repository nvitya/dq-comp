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
#include "ll_defs.h"

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
  {}
};

class OBoolLit : public OExpr
{
public:
  bool  value;
  OBoolLit(bool v)
  :
    value(v)
  {}
};

class OVarRef : public OExpr
{
public:
  OValSym *  pvalsym;
  OVarRef(OValSym * avalsym)
  :
    pvalsym(avalsym)
  {}
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
  {}
};

enum ECompareOp
{
  COMPOP_EQ,
  COMPOP_NE,
  COMPOP_LT,
  COMPOP_LE,
  COMPOP_GT,
  COMPOP_GE
};

class OCompareExpr : public OExpr
{
public:
  ECompareOp   op;
  OExpr *      left;
  OExpr *      right;
  OCompareExpr(ECompareOp aop, OExpr * aleft, OExpr * aright)
  :
    op(aop),
    left(aleft),
    right(aright)
  {}
};

enum ELogicalOp
{
  LOGIOP_OR,
  LOGIOP_AND,
  LOGIOP_XOR
};
// "not" implemented as an individual expression

class OLogicalExpr : public OExpr
{
public:
  ELogicalOp   op;
  OExpr *      left;
  OExpr *      right;
  OLogicalExpr(ELogicalOp aop, OExpr * aleft, OExpr * aright)
  :
    op(aop),
    left(aleft),
    right(aright)
  {}
};

class ONotExpr : public OExpr
{
public:
  OExpr *  operand;
  ONotExpr(OExpr * expr)
  :
    operand(expr)
  {}
};

class ONegExpr : public OExpr
{
public:
  OExpr *  operand;
  ONegExpr(OExpr * expr)
  :
    operand(expr)
  {}
};

class OValSymFunc;  // forward declaration for otype_func.h

class OCallExpr : public OExpr
{
public:
  OValSymFunc *     vsfunc;
  vector<OExpr *>   args;
  OCallExpr(OValSymFunc * avsfunc)
  :
    vsfunc(avsfunc)
  {}

  void AddArgument(OExpr * aarg)
  {
    args.push_back(aarg);
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
  {}
};

class OStmtBlock
{
public:
  OScope *         scope; // owned
  vector<OStmt *>  stlist;

  OStmtBlock(OScope * aparentscope, const string adebugname)
  {
    scope = new OScope(aparentscope, adebugname);
  }

  virtual ~OStmtBlock()
  {
    for (OStmt * st : stlist)
    {
      delete st;
    }
    stlist.clear();

    delete scope;
  }

  OStmt * AddStatement(OStmt * astmt)
  {
    stlist.push_back(astmt);
    return astmt;
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
  {}
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
  {}
};

class OStmtModifyAssign : public OStmt
{
public:
  OValSym *   variable;
  EBinOp      op;
  OExpr *     value;
  OStmtModifyAssign(OValSym * avariable, EBinOp aop, OExpr * avalue)
  :
    variable(avariable),
    op(aop),
    value(avalue)
  {}
};

class OStmtWhile : public OStmt
{
public:
  OExpr *       condition;
  OStmtBlock *  body;
  OStmtWhile(OExpr * acondition, OScope * ascope)
  :
    condition(acondition)
  {
    body = new OStmtBlock(ascope, "while");
  }

  ~OStmtWhile()
  {
    delete body;
  }
};

class OIfBranch
{
public:
  OExpr *       condition; // nullptr for else branch
  OStmtBlock *  body;
  OIfBranch(OExpr * acondition, OScope * aparentscope)
  :
    condition(acondition)
  {
    body = new OStmtBlock(aparentscope, "ifbranch");
  }

  ~OIfBranch()
  {
    delete body;
  }
};

class OStmtIf : public OStmt
{
public:
  OScope *             parentscope;
  vector<OIfBranch *>  branches; // if, elif..., else
  OStmtIf(OScope * aparentscope)
  :
    parentscope(aparentscope)
  {
  }

  ~OStmtIf()
  {
    for (OIfBranch * b : branches)
    {
      delete b;
    }
  }

  OIfBranch * AddBranch(OExpr * acondition)
  {
    OIfBranch * result = new OIfBranch(acondition, parentscope);
    branches.push_back(result);
    return result;
  }
};

class OBreakStmt : public OStmt
{
};

class OContinueStmt : public OStmt
{
};

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

#include <format>
#include <vector>
#include "symbols.h"
#include "ll_defs.h"

using namespace std;

//=============================================================================
// EXPRESSIONS
//=============================================================================

class OIntLit : public OExpr
{
public:
  int64_t   value;
  OIntLit(int64_t v)
  :
    value(v)
  {}

  LlValue * Generate(OScope * scope) override;
};

class OBoolLit : public OExpr
{
public:
  bool  value;
  OBoolLit(bool v)
  :
    value(v)
  {}

  LlValue * Generate(OScope * scope) override;
};

class OVarRef : public OExpr
{
public:
  OValSym *  pvalsym;
  OVarRef(OValSym * avalsym)
  :
    pvalsym(avalsym)
  {}

  LlValue * Generate(OScope * scope) override;
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

  LlValue * Generate(OScope * scope) override;
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

  LlValue * Generate(OScope * scope) override;
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

  LlValue * Generate(OScope * scope) override;
};

class ONotExpr : public OExpr
{
public:
  OExpr *  operand;
  ONotExpr(OExpr * expr)
  :
    operand(expr)
  {}

  LlValue * Generate(OScope * scope) override;
};

class ONegExpr : public OExpr
{
public:
  OExpr *  operand;
  ONegExpr(OExpr * expr)
  :
    operand(expr)
  {}

  LlValue * Generate(OScope * scope) override;
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

  LlValue * Generate(OScope * scope) override;
};


//=============================================================================
// STATEMENTS
//=============================================================================

class OStmt
{
public:
  OScPosition   scpos;

  OStmt(OScPosition & ascpos)
  :
    scpos(ascpos)
  {
  }

  virtual ~OStmt()  { }

  virtual void Generate(OScope * scope)
  {
    throw logic_error(std::format("Unhandled OStmt::Generate for \"{}\"", typeid(this).name()));
  }
};

struct OStmtReturn : public OStmt
{
private:
  using        super = OStmt;
public:
  OExpr *     value;
  OStmtReturn(OScPosition & ascpos, OExpr * v)
  :
    super(ascpos),
    value(v)
  {}

  void Generate(OScope * scope) override;
};

class OStmtBlock
{
private:
  using        super = OStmt;
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
private:
  using        super = OStmt;
public:
  OValSym *  variable;
  OExpr *    initvalue;

  OStmtVarDecl(OScPosition & ascpos, OValSym * avariable, OExpr * ainitvalue)
  :
    super(ascpos),
    variable(avariable),
    initvalue(ainitvalue)
  {}

  void Generate(OScope * scope) override;
};

class OStmtAssign : public OStmt
{
private:
  using        super = OStmt;
public:
  OValSym *   variable;
  OExpr *     value;
  OStmtAssign(OScPosition & ascpos, OValSym * avariable, OExpr * avalue)
  :
    super(ascpos),
    variable(avariable),
    value(avalue)
  {}

  void Generate(OScope * scope) override;
};

class OStmtModifyAssign : public OStmt
{
private:
  using        super = OStmt;
public:
  OValSym *   variable;
  EBinOp      op;
  OExpr *     value;
  OStmtModifyAssign(OScPosition & ascpos, OValSym * avariable, EBinOp aop, OExpr * avalue)
  :
    super(ascpos),
    variable(avariable),
    op(aop),
    value(avalue)
  {}

  void Generate(OScope * scope) override;
};

class OStmtWhile : public OStmt
{
private:
  using        super = OStmt;
public:
  OExpr *       condition;
  OStmtBlock *  body;
  OStmtWhile(OScPosition & ascpos, OExpr * acondition, OScope * ascope)
  :
    super(ascpos),
    condition(acondition)
  {
    body = new OStmtBlock(ascope, "while");
  }

  ~OStmtWhile()
  {
    delete body;
  }

  void Generate(OScope * scope) override;
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
private:
  using        super = OStmt;
public:
  OScope *             parentscope;
  vector<OIfBranch *>  branches; // if, elif..., else
  OStmtIf(OScPosition & ascpos, OScope * aparentscope)
  :
    super(ascpos),
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

  void Generate(OScope * scope) override;
};

class OBreakStmt : public OStmt
{
public:
  void Generate(OScope * scope) override;
};

class OContinueStmt : public OStmt
{
public:
  void Generate(OScope * scope) override;
};

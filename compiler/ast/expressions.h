/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    expressions.h
 * authors: nvitya
 * created: 2026-02-28
 * brief:   expressions
 */

#pragma once

#include <format>
#include <vector>
#include "symbols.h"
#include "ll_defs.h"

using namespace std;

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

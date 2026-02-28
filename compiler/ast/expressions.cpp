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

#include "expressions.h"
#include "scope_builtins.h"


LlValue * OIntLit::Generate(OScope * scope)
{
  return llvm::ConstantInt::get(g_builtins->type_int->GetLlType(), value);
}

LlValue * OBoolLit::Generate(OScope * scope)
{
  return llvm::ConstantInt::get(g_builtins->type_bool->GetLlType(), (value ? 1 : 0));
}

LlValue * OVarRef::Generate(OScope * scope)
{
  if (!pvalsym->ll_value)
  {
    throw logic_error(std::format("Variable \"{}\" was not prepared in the LLVM", pvalsym->name));
  }

  if (VSK_PARAMETER == pvalsym->kind)
  {
    return pvalsym->ll_value;
  }
  else
  {
    return ll_builder.CreateLoad(pvalsym->ll_value->getType(), pvalsym->ll_value, pvalsym->name);
  }
}

LlValue * OBinExpr::Generate(OScope * scope)
{
  LlValue * ll_left  = left->Generate(scope);
  LlValue * ll_right = right->Generate(scope);

  if      (BINOP_ADD == op)  return ll_builder.CreateAdd(ll_left, ll_right);
  else if (BINOP_SUB == op)  return ll_builder.CreateSub(ll_left, ll_right);
  else if (BINOP_MUL == op)  return ll_builder.CreateMul(ll_left, ll_right);

  throw logic_error(std::format("GenerateExpr(): Unhandled binop = {} ", int(op)));
}

LlValue * OCompareExpr::Generate(OScope * scope)
{
  LlValue * ll_left  = left->Generate(scope);
  LlValue * ll_right = right->Generate(scope);

  // TODO: handle unsigned !!!

  if      (COMPOP_EQ == op)   return ll_builder.CreateICmpEQ(ll_left, ll_right);
  else if (COMPOP_NE == op)   return ll_builder.CreateICmpNE(ll_left, ll_right);
  else if (COMPOP_LT == op)   return ll_builder.CreateICmpSLT(ll_left, ll_right);
  else if (COMPOP_GT == op)   return ll_builder.CreateICmpSGT(ll_left, ll_right);
  else if (COMPOP_LE == op)   return ll_builder.CreateICmpSLE(ll_left, ll_right);
  else if (COMPOP_GE == op)   return ll_builder.CreateICmpSGE(ll_left, ll_right);

  throw logic_error(std::format("GenerateExpr(): Unhandled compare operation= {} ", int(op)));
}

LlValue * OLogicalExpr::Generate(OScope * scope)
{
  LlValue * ll_left  = left->Generate(scope);
  LlValue * ll_right = right->Generate(scope);

  if      (LOGIOP_AND == op)  return ll_builder.CreateAnd(ll_left, ll_right);
  else if (LOGIOP_OR  == op)  return ll_builder.CreateOr(ll_left, ll_right);
  else if (LOGIOP_XOR == op)  return ll_builder.CreateXor(ll_left, ll_right);

  throw logic_error(std::format("GenerateExpr(): Unhandled logical operation= {} ", int(op)));
}

LlValue * ONotExpr::Generate(OScope * scope)
{
  LlValue * ll_val = operand->Generate(scope);
  return ll_builder.CreateXor(ll_val, llvm::ConstantInt::get(g_builtins->type_bool->GetLlType(), 1));
}

LlValue * ONegExpr::Generate(OScope * scope)
{
  LlValue * ll_val = operand->Generate(scope);
  return ll_builder.CreateNeg(ll_val);
}

LlValue * OCallExpr::Generate(OScope * scope)
{
  LlFunction * ll_func = vsfunc->ll_func;
  if (!ll_func)
  {
    throw runtime_error("OCallExpr::Generate(): Unknown function: " + vsfunc->name);
  }

  vector<LlValue *>   ll_args;
  for (OExpr * arg : args)
  {
    ll_args.push_back(arg->Generate(scope));
  }
  return ll_builder.CreateCall(ll_func, ll_args);
}

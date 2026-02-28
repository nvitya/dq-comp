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

#include <format>
#include "scope_builtins.h"
#include "statements.h"

using namespace std;

void OStmtReturn::Generate(OScope * scope)
{
  ll_builder.CreateRet(value->Generate(scope));
}

void OStmtVarDecl::Generate(OScope * scope)
{
  // Local variable declaration
  LlType * ll_type = variable->ptype->GetLlType();
  variable->ll_value = ll_builder.CreateAlloca(ll_type, nullptr, variable->name);
  if (initvalue)
  {
    LlValue * ll_initval = initvalue->Generate(scope);
    ll_builder.CreateStore(ll_initval, variable->ll_value);
  }
}

void OStmtAssign::Generate(OScope * scope)
{
  LlValue * ll_set_value = value->Generate(scope);

  if (!variable->ll_value)
  {
    throw logic_error(std::format("Variable \"{}\" was not prepared in the LLVM", variable->name));
  }

  // TODO: check types at parsing
  if (ll_set_value->getType() != variable->ll_value->getType())
  {
    throw logic_error(std::format("Type mismatch at assignment"));
  }

  ll_builder.CreateStore(ll_set_value, variable->ll_value);
}

void OStmtModifyAssign::Generate(OScope *scope)
{
  LlValue * ll_mod_value = value->Generate(scope);

  if (!variable->ll_value)
  {
    throw logic_error(std::format("Variable \"{}\" was not prepared in the LLVM", variable->name));
  }

  // TODO: check types at parsing
  if (ll_mod_value->getType() != variable->ll_value->getType())
  {
    throw logic_error(std::format("Type mismatch at assignment"));
  }

  // Load current value
  LlValue * ll_curval = ll_builder.CreateLoad(variable->ll_value->getType(), variable->ll_value, variable->name);
  if (ll_mod_value->getType() != variable->ll_value->getType())
  {
    throw logic_error(std::format("Type mismatch at modify assignment"));
  }
  LlValue * ll_newval = nullptr;
  if      (BINOP_ADD == op)  ll_newval = ll_builder.CreateAdd(ll_curval, ll_mod_value);
  else if (BINOP_SUB == op)  ll_newval = ll_builder.CreateSub(ll_curval, ll_mod_value);
  else if (BINOP_MUL == op)  ll_newval = ll_builder.CreateMul(ll_curval, ll_mod_value);

  if (ll_newval)
  {
    ll_builder.CreateStore(ll_newval, variable->ll_value);
  }
  else
  {
    throw logic_error(std::format("Unsupported modify-assign operation: {}", int(op)));
  }
}

void OStmtWhile::Generate(OScope * scope)
{
  LlFunction *    ll_func    = ll_builder.GetInsertBlock()->getParent();
  LlBasicBlock *  ll_cond_bb = LlBasicBlock::Create(ll_ctx, "while.cond", ll_func);
  LlBasicBlock *  ll_body_bb = LlBasicBlock::Create(ll_ctx, "while.body", ll_func);
  LlBasicBlock *  ll_end_bb  = LlBasicBlock::Create(ll_ctx, "while.end", ll_func);

  // Push loop context for break/continue
  ll_loop_stack.push_back({ll_cond_bb, ll_end_bb});

  // Jump to condition check
  ll_builder.CreateBr(ll_cond_bb);

  // Generate condition
  ll_builder.SetInsertPoint(ll_cond_bb);
  LlValue * ll_cond = condition->Generate(scope);
  if (ll_cond->getType() != g_builtins->type_bool->GetLlType())
  {
    throw logic_error("Type mismatch: while condition must be bool");
  }

  ll_builder.CreateCondBr(ll_cond, ll_body_bb, ll_end_bb);

  // Generate body
  ll_builder.SetInsertPoint(ll_body_bb);
  for (OStmt * bstmt : body->stlist)
  {
    bstmt->Generate(body->scope);
    if (ll_builder.GetInsertBlock()->getTerminator()) break;
  }

  // Jump back to condition
  if (!ll_builder.GetInsertBlock()->getTerminator())
  {
    ll_builder.CreateBr(ll_cond_bb);
  }

  ll_loop_stack.pop_back();

  // Continue after loop
  ll_builder.SetInsertPoint(ll_end_bb);
}

void OBreakStmt::Generate(OScope * scope)
{
  if (ll_loop_stack.size() < 1)
  {
    throw logic_error("BreakStmt::Generate(): empty loop_stack!");
  }

  ll_builder.CreateBr(ll_loop_stack.back().end_bb);
}

void OContinueStmt::Generate(OScope * scope)
{
  if (ll_loop_stack.size() < 1)
  {
    throw logic_error("BreakStmt::Generate(): empty loop_stack!");
  }

  ll_builder.CreateBr(ll_loop_stack.back().cond_bb);
}

void OStmtIf::Generate(OScope * scope)
{
  LlFunction *   ll_func   = ll_builder.GetInsertBlock()->getParent();
  LlBasicBlock * bb_merge  = LlBasicBlock::Create(ll_ctx, "if.end", ll_func);

  for (size_t i = 0; i < branches.size(); i++)
  {
    OIfBranch * branch = branches[i];
    if (branch->condition == nullptr)
    {
      // else branch - just emit the body
      for (OStmt * bstmt : branch->body->stlist)
      {
        bstmt->Generate(branch->body->scope);
        if (ll_builder.GetInsertBlock()->getTerminator()) break;
      }
      if (!ll_builder.GetInsertBlock()->getTerminator())
      {
        ll_builder.CreateBr(bb_merge);
      }
    }
    else // if or elif branch
    {
      LlValue * ll_cond = branch->condition->Generate(scope);
      if (ll_cond->getType() != g_builtins->type_bool->GetLlType())
      {
        throw runtime_error("Type mismatch: if condition must be bool");
      }

      LlBasicBlock * bb_then = LlBasicBlock::Create(ll_ctx, "if.then", ll_func);
      LlBasicBlock * bb_else;

      // If there's a next branch, the else goes to it; otherwise to merge
      if (i + 1 < branches.size())
      {
        bb_else = LlBasicBlock::Create(ll_ctx, "if.else", ll_func);
      }
      else
      {
        bb_else = bb_merge;
      }

      ll_builder.CreateCondBr(ll_cond, bb_then, bb_else);

      // Generate then body
      ll_builder.SetInsertPoint(bb_then);
      for (OStmt * bstmt : branch->body->stlist)
      {
        bstmt->Generate(branch->body->scope);
        if (ll_builder.GetInsertBlock()->getTerminator()) break;
      }

      if (!ll_builder.GetInsertBlock()->getTerminator())
      {
        ll_builder.CreateBr(bb_merge);
      }

      // Set insert point to else block for next branch
      if (bb_else != bb_merge)
      {
        ll_builder.SetInsertPoint(bb_else);
      }
    }
  }

  ll_builder.SetInsertPoint(bb_merge);
}

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

/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_codegen.cpp
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

#include <print>
#include <format>

#include "dqc_codegen.h"

using namespace std;

void ODqCompCodegen::GenerateIr()
{
  print("Generating IR...\n");

  // generate declarations

  for (ODecl * decl : g_module->declarations)
  {
    GlobalValue::LinkageTypes  linktype =
      (decl->ispublic ? GlobalValue::LinkageTypes::ExternalLinkage
                      : GlobalValue::LinkageTypes::InternalLinkage);

    if (DK_VALSYM == decl->kind)
    {
      OValSym * vs = decl->pvalsym;
      if (VSK_VARIABLE == vs->kind)
      {
        Type * ll_type = LlType(vs->ptype);
        Constant * init_val = ConstantInt::get(ll_type, 0);

        auto * gv = new GlobalVariable(*ll_mod, ll_type, false, linktype, init_val, vs->name);
        ll_globals[vs->name] = gv;
      }
      else if (VSK_FUNCTION == vs->kind)
      {
        OTypeFunc * ptfunc = dynamic_cast<OTypeFunc *>(vs->ptype);
        if (ptfunc)
        {
          print("Found function declaration \"{}\"\n", ptfunc->name);

          vector<Type *> ll_partypes;
          for (OFuncParam * fpar : ptfunc->params)
          {
            ll_partypes.push_back(LlType(fpar->ptype));
          }
          Type *  ll_rettype  = LlType(ptfunc->rettype);
          auto *  ll_functype = FunctionType::get(ll_rettype, ll_partypes, false);
          auto *  ll_func     = Function::Create(ll_functype, linktype, ptfunc->name, ll_mod);
          ll_functions[ptfunc->name] = ll_func;
        }

      }
    }
    else if (DK_TYPE == decl->kind)
    {
      OType * pt = decl->ptype;
      print("Unhandled type declaration \"{}\"\n", pt->name);
    }
  }

  // generate function bodies

  for (ODecl * decl : g_module->declarations)
  {
    GlobalValue::LinkageTypes  linktype =
      (decl->ispublic ? GlobalValue::LinkageTypes::ExternalLinkage
                      : GlobalValue::LinkageTypes::InternalLinkage);

    if (DK_VALSYM == decl->kind)
    {
      OValSym * vs = decl->pvalsym;
      OValSymFunc * vsfunc = dynamic_cast<OValSymFunc *>(vs);
      if (vsfunc)
      {
        GenerateFunction(vsfunc);
      }
    }
  }
}

void ODqCompCodegen::GenerateFunction(OValSymFunc * vsfunc)
{
  // Get the pre-declared function
  Function * ll_func = ll_functions[vsfunc->name];
  OTypeFunc * tfunc = (OTypeFunc *)vsfunc->ptype;

  // Map params to names
  ll_locals.clear();
  //localTypes.clear();
  int i = 0;
  for (auto & arg : ll_func->args())
  {
    arg.setName(tfunc->params[i]->name);
    ll_locals[tfunc->params[i]->name] = &arg;
    ++i;
  }

  // Create entry block and generate body
  auto * entry = BasicBlock::Create(ll_ctx, "entry", ll_func);
  ll_builder.SetInsertPoint(entry);

  // Create implicit 'result' variable for functions with return type
  OType * rettype = tfunc->rettype;
  Type * ll_rettype = nullptr;
  if (rettype)
  {
    ll_rettype = LlType(tfunc->rettype);

    auto * alloca_result = ll_builder.CreateAlloca(ll_rettype, nullptr, "result");
    ll_locals["result"] = alloca_result;
    //localTypes["result"] = retType;
    // Initialize to 0 or false
    if (TK_INT == rettype->kind)
    {
      ll_builder.CreateStore(ConstantInt::get(ll_rettype, 0), alloca_result);
    }
    else if (TK_BOOL == rettype->kind)
    {
      ll_builder.CreateStore(ConstantInt::get(ll_rettype, 0), alloca_result);
    }
  }

  for (auto * stmt : vsfunc->body->stlist)
  {
    GenerateStatement(stmt);
  }

  // Add implicit return
  if (!ll_builder.GetInsertBlock()->getTerminator())
  {
    if (!ll_rettype)
    {
      ll_builder.CreateRetVoid();
    }
    else
    {
      // Return the value of 'result'
      Value * ll_result = ll_builder.CreateLoad(ll_rettype, ll_locals["result"], "result");
      ll_builder.CreateRet(ll_result);
    }
  }

  verifyFunction(*ll_func);
}

void ODqCompCodegen::GenerateStatement(OStmt * stmt)
{
  OStmtReturn * stret = dynamic_cast<OStmtReturn *>(stmt);
  if (stret)
  {
    ll_builder.CreateRet(GenerateExpr(stret->value));
    return;
  }

  OStmtVarDecl * stvar = dynamic_cast<OStmtVarDecl *>(stmt);
  if (stvar)
  {
    OValSym * vs = stvar->variable;
    // Local variable declaration
    Type * ll_type = LlType(vs->ptype);
    auto * alloca_var = ll_builder.CreateAlloca(ll_type, nullptr, vs->name);
    ll_locals[vs->name] = alloca_var;
    //localTypes[varDecl->name] = varType;
    if (stvar->initvalue)
    {
      Value * ll_initval = GenerateExpr(stvar->initvalue);
      ll_builder.CreateStore(ll_initval, alloca_var);
    }
    return;
  }

  OStmtAssign * stassign = dynamic_cast<OStmtAssign *>(stmt);
  if (stassign)
  {
    OValSym * vs = stassign->variable;
    Value * ll_val = GenerateExpr(stassign->value);
    // Check if it's a local variable
    auto found = ll_locals.find(vs->name);
    if (found != ll_locals.end())
    {
      ll_builder.CreateStore(ll_val, found->second);
      return;
    }

    // Check if it's a global variable
    found = ll_globals.find(vs->name);
    if (found != ll_globals.end())
    {
      ll_builder.CreateStore(ll_val, found->second);
      return;
    }

    throw logic_error(format("Variable \"{}\" was not found in the LLVM", vs->name));
  }

  throw logic_error(format("Unhandled statement type: \"{}\"", typeid(*stmt).name()));
}

Value * ODqCompCodegen::GenerateExpr(OExpr * expr)
{
  OIntLit * expr_intlit = dynamic_cast<OIntLit *>(expr);
  if (expr_intlit)
  {
    return ConstantInt::get(LlType(g_builtins->type_int), expr_intlit->value);
  }

  OBoolLit * expr_boollit = dynamic_cast<OBoolLit *>(expr);
  if (expr_boollit)
  {
    return ConstantInt::get(LlType(g_builtins->type_bool), (expr_boollit->value ? 1 : 0));
  }

  OVarRef * expr_varref = dynamic_cast<OVarRef *>(expr);
  if (expr_varref)
  {
    OValSym * vs = expr_varref->pvalsym;

    // Check if it's a local variable
    auto found = ll_locals.find(vs->name);
    if (found != ll_locals.end())
    {
      Value * ll_val = found->second;
      if (VSK_VARIABLE == vs->kind)
      {
        return ll_builder.CreateLoad(ll_val->getType(), ll_val, vs->name);
      }

      // direct value for function parameters
      return ll_val;
    }

    // Check if it's a global variable
    found = ll_globals.find(vs->name);
    if (found != ll_globals.end())
    {
      Value * ll_val = found->second;
      return ll_builder.CreateLoad(ll_val->getType(), ll_val, vs->name);
    }

    throw logic_error(format("Codegen: unknown variable \"{}\"", vs->name));
  }

  OBinExpr * expr_binop = dynamic_cast<OBinExpr *>(expr);
  if (expr_binop)
  {
    Value * ll_left  = GenerateExpr(expr_binop->left);
    Value * ll_right = GenerateExpr(expr_binop->right);

    if (BINOP_ADD == expr_binop->op)
    {
      return ll_builder.CreateAdd(ll_left, ll_right);
    }

    if (BINOP_SUB == expr_binop->op)
    {
      return ll_builder.CreateSub(ll_left, ll_right);
    }

    if (BINOP_MUL == expr_binop->op)
    {
      return ll_builder.CreateMul(ll_left, ll_right);
    }

    throw logic_error(format("GenerateExpr(): Unhandled binop = {} ", int(expr_binop->op)));
  }


#if 0

  if (auto* cmp = dynamic_cast<CompareExpr*>(e)) {
      Value* l = genExpr(cmp->left);
      Value* r = genExpr(cmp->right);
      switch (cmp->op) {
          case CompareOp::LE: return builder.CreateICmpSLE(l, r);
          case CompareOp::LT: return builder.CreateICmpSLT(l, r);
          case CompareOp::GE: return builder.CreateICmpSGE(l, r);
          case CompareOp::GT: return builder.CreateICmpSGT(l, r);
          case CompareOp::EQ: return builder.CreateICmpEQ(l, r);
          case CompareOp::NE: return builder.CreateICmpNE(l, r);
      }
  }
  if (auto* logic = dynamic_cast<LogicalExpr*>(e)) {
      Value* l = genExpr(logic->left);
      Value* r = genExpr(logic->right);
      switch (logic->op) {
          case LogicalOp::And: return builder.CreateAnd(l, r);
          case LogicalOp::Or: return builder.CreateOr(l, r);
      }
  }
  if (auto* call = dynamic_cast<CallExpr*>(e)) {
      if (functions.find(call->funcName) == functions.end()) {
          throw runtime_error("Unknown function: " + call->funcName);
      }
      Function* func = functions[call->funcName];
      vector<Value*> args;
      for (auto* arg : call->args) {
          args.push_back(genExpr(arg));
      }
      return builder.CreateCall(func, args);
  }

#endif

  throw logic_error(format("GenerateExpr(): Unhandled expression type: \"{}\"", typeid(*expr).name()));

  return nullptr;
}

void ODqCompCodegen::PrintIr()
{
  print("=== LLVM IR ===\n");
  ll_mod->print(outs(), nullptr);
  print("===============\n\n");
}

Type * ODqCompCodegen::LlType(OType * atype)
{
  if (!atype)
  {
    return Type::getVoidTy(ll_ctx);
  }

  OTypeInt * ptint = dynamic_cast<OTypeInt *>(atype);
  if (ptint)
  {
    return Type::getIntNTy(ll_ctx, ptint->bitlength);
  }

  OTypeBool * ptbool = dynamic_cast<OTypeBool *>(atype);
  if (ptbool)
  {
    return Type::getInt1Ty(ll_ctx);
  }

  OTypeFunc * ptfunc = dynamic_cast<OTypeFunc *>(atype);
  if (ptfunc)
  {
    return Type::getIntNTy(ll_ctx, ptint->bitlength);
  }

  throw logic_error(format("Unhandled DQ Type \"{}\" at LlType() conversion", typeid(*atype).name()));
}

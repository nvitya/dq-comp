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

#if 0
  // Map params to names
  locals.clear();
  localTypes.clear();
  int i = 0;
  for (auto& arg : func->args()) {
      arg.setName(f->params[i].name);
      locals[f->params[i].name] = &arg;
      i++;
  }

  // Create entry block and generate body
  auto* entry = BasicBlock::Create(ctx, "entry", func);
  builder.SetInsertPoint(entry);

  // Create implicit 'result' variable for functions with return type
  if (f->returnType != "")
  {
    Type* retType = getType(f->returnType);
    auto* resultAlloca = builder.CreateAlloca(retType, nullptr, "result");
    locals["result"] = resultAlloca;
    localTypes["result"] = retType;
    // Initialize to 0 or false
    if (f->returnType == "int") {
        builder.CreateStore(ConstantInt::get(retType, 0), resultAlloca);
    } else if (f->returnType == "bool") {
        builder.CreateStore(ConstantInt::get(retType, 0), resultAlloca);
    }
  }

  for (auto * stmt : vsfunc->body->stlist)
  {
    genStmt(stmt);
  }

  // Add implicit return
  if (!builder.GetInsertBlock()->getTerminator())
  {
    if (f->returnType == "") {
        builder.CreateRetVoid();
    } else {
        // Return the value of 'result'
        Type* retType = localTypes["result"];
        Value* resultVal = builder.CreateLoad(retType, locals["result"], "result");
        builder.CreateRet(resultVal);
    }
  }

#endif

  verifyFunction(*ll_func);
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

  OTypeFunc * ptfunc = dynamic_cast<OTypeFunc *>(atype);
  if (ptfunc)
  {
    return Type::getIntNTy(ll_ctx, ptint->bitlength);
  }

  throw logic_error("Unhandled DQ Type and LLType conversion!");
}

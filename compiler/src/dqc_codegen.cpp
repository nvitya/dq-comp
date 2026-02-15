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

// these include also provide llvm::format() so the std::format() must be fully specified
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/TargetParser/Host.h>

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

    throw logic_error(std::format("Variable \"{}\" was not found in the LLVM", vs->name));
  }

  throw logic_error(std::format("Unhandled statement type: \"{}\"", typeid(*stmt).name()));
}

Value * ODqCompCodegen::GenerateExpr(OExpr * aexpr)
{
  {
    OIntLit * ex = dynamic_cast<OIntLit *>(aexpr);
    if (ex)
    {
      return ConstantInt::get(LlType(g_builtins->type_int), ex->value);
    }
  }
  {
    OBoolLit * ex = dynamic_cast<OBoolLit *>(aexpr);
    if (ex)
    {
      return ConstantInt::get(LlType(g_builtins->type_bool), (ex->value ? 1 : 0));
    }
  }
  {
    OVarRef * ex = dynamic_cast<OVarRef *>(aexpr);
    if (ex)
    {
      OValSym * vs = ex->pvalsym;

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

      throw logic_error(std::format("Codegen: unknown variable \"{}\"", vs->name));
    }
  }
  {
    OBinExpr * ex = dynamic_cast<OBinExpr *>(aexpr);
    if (ex)
    {
      Value * ll_left  = GenerateExpr(ex->left);
      Value * ll_right = GenerateExpr(ex->right);

      if      (BINOP_ADD == ex->op)  return ll_builder.CreateAdd(ll_left, ll_right);
      else if (BINOP_SUB == ex->op)  return ll_builder.CreateSub(ll_left, ll_right);
      else if (BINOP_MUL == ex->op)  return ll_builder.CreateMul(ll_left, ll_right);

      throw logic_error(std::format("GenerateExpr(): Unhandled binop = {} ", int(ex->op)));
    }
  }
  {
    OCompareExpr * ex = dynamic_cast<OCompareExpr *>(aexpr);
    if (ex)
    {
      Value * ll_left  = GenerateExpr(ex->left);
      Value * ll_right = GenerateExpr(ex->right);

      // TODO: handle unsigned !!!

      if      (COMPOP_EQ == ex->op)   return ll_builder.CreateICmpEQ(ll_left, ll_right);
      else if (COMPOP_NE == ex->op)   return ll_builder.CreateICmpNE(ll_left, ll_right);
      else if (COMPOP_LT == ex->op)   return ll_builder.CreateICmpSLT(ll_left, ll_right);
      else if (COMPOP_GT == ex->op)   return ll_builder.CreateICmpSGT(ll_left, ll_right);
      else if (COMPOP_LE == ex->op)   return ll_builder.CreateICmpSLE(ll_left, ll_right);
      else if (COMPOP_GE == ex->op)   return ll_builder.CreateICmpSGE(ll_left, ll_right);

      throw logic_error(std::format("GenerateExpr(): Unhandled compare operation= {} ", int(ex->op)));
    }
  }
  {
    ONotExpr * ex = dynamic_cast<ONotExpr *>(aexpr);
    if (ex)
    {
      Value * ll_val = GenerateExpr(ex->operand);
      return ll_builder.CreateXor(ll_val, ConstantInt::get(LlType(g_builtins->type_bool), 1));
    }
  }
  {
    OLogicalExpr * ex = dynamic_cast<OLogicalExpr *>(aexpr);
    if (ex)
    {
      Value * ll_left  = GenerateExpr(ex->left);
      Value * ll_right = GenerateExpr(ex->right);

      if      (LOGIOP_AND == ex->op)  return ll_builder.CreateAnd(ll_left, ll_right);
      else if (LOGIOP_OR  == ex->op)  return ll_builder.CreateOr(ll_left, ll_right);
      else if (LOGIOP_XOR == ex->op)  return ll_builder.CreateXor(ll_left, ll_right);

      throw logic_error(std::format("GenerateExpr(): Unhandled logical operation= {} ", int(ex->op)));
    }
  }
  {
    OCallExpr * ex = dynamic_cast<OCallExpr *>(aexpr);
    if (ex)
    {
      auto found = ll_functions.find(ex->vsfunc->name);
      if (found == ll_functions.end())
      {
        throw runtime_error("GenerateExpr(): Unknown function: " + ex->vsfunc->name);
      }

      Function * ll_func = found->second;

      vector<Value *>   ll_args;
      for (OExpr * arg : ex->args)
      {
        ll_args.push_back(GenerateExpr(arg));
      }
      return ll_builder.CreateCall(ll_func, ll_args);
    }
  }

  throw logic_error(std::format("GenerateExpr(): Unhandled expression type: \"{}\"", typeid(*aexpr).name()));
  return nullptr;
}

void ODqCompCodegen::EmitObject(const string afilename)
{
  print("Writing object file \"{}\"...\n", afilename);

  // Only initialize native target (not all targets)
  InitializeNativeTarget();
  InitializeNativeTargetAsmParser();
  InitializeNativeTargetAsmPrinter();

  auto triple = sys::getDefaultTargetTriple();
  ll_mod->setTargetTriple(triple);

  string err;
  auto* target = TargetRegistry::lookupTarget(triple, err);
  if (!target) throw runtime_error(err);

  auto* machine = target->createTargetMachine(
      triple, "generic", "", TargetOptions(), Reloc::PIC_);

  ll_mod->setDataLayout(machine->createDataLayout());

  error_code ec;
  raw_fd_ostream out(afilename, ec, sys::fs::OF_None);
  if (ec) throw runtime_error(ec.message());

  legacy::PassManager pm;
  machine->addPassesToEmitFile(pm, out, nullptr, CodeGenFileType::ObjectFile);
  pm.run(*ll_mod);
  out.flush();
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

  throw logic_error(std::format("Unhandled DQ Type \"{}\" at LlType() conversion", typeid(*atype).name()));
}

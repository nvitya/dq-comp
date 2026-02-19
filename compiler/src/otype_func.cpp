/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    otype_func.cpp
 * authors: nvitya
 * created: 2026-02-07
 * brief:   function type
 */

#include "otype_func.h"
#include "dqc.h"

OFuncParam * OTypeFunc::AddParam(const string aname, OType * atype, EParamMode amode)
{
  OFuncParam * result = new OFuncParam(aname, atype, amode);
  params.push_back(result);
  return result;
}

bool OTypeFunc::ParNameValid(const string aname)
{
  if (g_compiler->ReservedWord(aname))
  {
    return false;
  }

  // search for existing parameters
  for (OFuncParam * fp : params)
  {
    if (fp->name == aname)
    {
      return false;
    }
  }
  return true;
}

LlType * OTypeFunc::CreateLlType()  // do not call GetLlType() until the function arguments fully prepared
{
  // collect parameter list
  vector<LlType *> ll_partypes;
  for (OFuncParam * fpar : params)
  {
    ll_partypes.push_back(fpar->ptype->GetLlType());
  }
  // return type
  LlType *  ll_rettype  = rettype->GetLlType();
  // the final function type:
  return LlFuncType::get(ll_rettype, ll_partypes, false);
}

void OValSymFunc::GenDeclaration(bool apublic)
{
  //print("Found function declaration \"{}\"\n", ptfunc->name);

  GlobalValue::LinkageTypes  linktype =
    (apublic ? GlobalValue::LinkageTypes::ExternalLinkage
              : GlobalValue::LinkageTypes::InternalLinkage);

  LlFuncType *  ll_functype = (LlFuncType *)(ptype->GetLlType());  // calls CreateLlType()

  ll_func     = LlFunction::Create(ll_functype, linktype, name, ll_module);

  //ll_functions[ptfunc->name] = ll_func;
}

void OValSymFunc::GenerateFuncBody()
{
  // Get the pre-declared function
  if (!ll_func)
  {
    throw logic_error("GenerateFuncBody: ll_func declaration is missing");
  }

  OTypeFunc * tfunc = (OTypeFunc *)ptype;

  // Map params to names

  //ll_locals.clear();
  int i = 0;
  for (auto & arg : ll_func->args())
  {
    OFuncParam * fpar = tfunc->params[i];
    OValSym * vsarg = args[i];
    arg.setName(fpar->name);
    ++i;
  }

  // Create entry block and generate body
  auto * entry = LlBasicBlock::Create(ll_ctx, "entry", ll_func);
  ll_builder.SetInsertPoint(entry);

  // Create implicit 'result' variable for functions with return type
  LlType * ll_rettype = nullptr;
  if (vsresult)
  {
    ll_rettype = vsresult->ptype->GetLlType();
    vsresult->ll_value = ll_builder.CreateAlloca(ll_rettype, nullptr, "result");
    // TODO: support other types
    ll_builder.CreateStore(ConstantInt::get(ll_rettype, 0), vsresult->ll_value);
  }

  for (auto * stmt : body->stlist)
  {
    //GenerateStatement(stmt);
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
      LlValue * ll_result = ll_builder.CreateLoad(ll_rettype, vsresult->ll_value, "result");
      ll_builder.CreateRet(ll_result);
    }
  }

  verifyFunction(*ll_func);

}

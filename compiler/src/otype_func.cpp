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
#include "ll_defs.h"

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
  vector<LlType *> ll_partypes;
  for (OFuncParam * fpar : params)
  {
    ll_partypes.push_back(fpar->ptype->GetLlType());
  }
  LlType *  ll_rettype  = rettype->GetLlType();
  return LlFuncType::get(ll_rettype, ll_partypes, false);
}

LlDiType * OTypeFunc::CreateDiType()
{
  OTypeFunc * tfunc = (OTypeFunc *)ptype;

  vector<llvm::Metadata *> di_param_types;

  if (rettype)
  {
    di_param_types.push_back(rettype->GetDiType());
  }
  else
  {
    di_param_types.push_back(nullptr);
  }

#if 0
  // 'this' parameter for methods
  if (hasThis)
      paramTypes.push_back(getDebugType("ptr"));
#endif

  // Regular parameters
  for (OFuncParam * fpar : tfunc->params)
  {
    di_param_types.push_back(fpar->ptype->GetDiType());
  }

  return di_builder->createSubroutineType(di_builder->getOrCreateTypeArray(di_param_types));
}

void OValSymFunc::GenDeclaration(bool apublic)
{
  //print("Found function declaration \"{}\"\n", ptfunc->name);

  llvm::GlobalValue::LinkageTypes  linktype =
    (apublic ? llvm::GlobalValue::LinkageTypes::ExternalLinkage
              : llvm::GlobalValue::LinkageTypes::InternalLinkage);

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
    ll_builder.CreateStore(llvm::ConstantInt::get(ll_rettype, 0), vsresult->ll_value);
  }

  // Create allocas for parameters
  int i = 0;
  for (auto & arg : ll_func->args())
  {
    OFuncParam *  fpar  = tfunc->params[i];
    OValSym *     vsarg = args[i];

    arg.setName(fpar->name);
    vsarg->ll_value = ll_builder.CreateAlloca(fpar->ptype->GetLlType(), nullptr, fpar->name);
    ll_builder.CreateStore(&arg, vsarg->ll_value);
    ++i;
  }


  // DEBUG INFO

  llvm::DISubroutineType *  di_func_type = (llvm::DISubroutineType *)ptype->GetDiType();
  LlDiScope * di_scope = di_unit;
  if (!di_scope_stack.empty())
  {
    di_scope = di_scope_stack.back();
  }

  int line_no = scpos.GetLineNo();

  llvm::DISubprogram * debug_func = di_builder->createFunction(
      di_scope,
      name,
      llvm::StringRef(),
      scpos.scfile->di_file,
      line_no,
      di_func_type,
      line_no,
      llvm::DINode::FlagPrototyped,
      llvm::DISubprogram::SPFlagDefinition
  );
  ll_func->setSubprogram(debug_func);

  // STATEMENTS

  for (OStmt * stmt : body->stlist)
  {
    stmt->Generate(body->scope);
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

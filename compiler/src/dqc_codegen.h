/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_codegen.h
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

#pragma once

#include "stdint.h"
#include <string>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

#include "comp_options.h"
#include "dqc_parser.h"

using namespace std;
using namespace llvm;

class ODqCompCodegen : public ODqCompParser
{
private:
  using                super = ODqCompParser;

protected:
  LLVMContext          ll_ctx;
  Module *             ll_mod;
  IRBuilder<>          ll_builder;

  map<string, Function *>  ll_functions;
  map<string, Value *>     ll_locals;
  map<string, Value *>     ll_globals;

  // Loop context for break/continue
  struct SLoopContext
  {
    BasicBlock * cond_bb;  // continue target
    BasicBlock * end_bb;   // break target
  };
  vector<SLoopContext>  ll_loop_stack;


public:
  ODqCompCodegen()
  :
    ll_mod(new Module("dq", ll_ctx)),
    ll_builder(ll_ctx)
  {
  }

  virtual ~ODqCompCodegen() {}

  void GenerateIr();
  void GenerateFunction(OValSymFunc * vsfunc);
  void GenerateStatement(OStmt * astmt);
  Value * GenerateExpr(OExpr * aexpr);

  bool GenWhileStatement(OStmt * astmt);
  bool GenIfStatement(OStmt * astmt);

  void PrintIr();

  void EmitObject(const string afilename);

  Type * LlType(OType * atype);

};
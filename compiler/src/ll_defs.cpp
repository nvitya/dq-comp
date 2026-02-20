/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    ll_defs.h
 * authors: nvitya
 * created: 2026-02-19
 * brief:   LLVM defines, object
 */

#include "ll_defs.h"
#include "dqc.h"
#include "scf_dq.h"

LlContext      ll_ctx;
LlBuilder      ll_builder(ll_ctx);
LlModule *     ll_module;

vector<SLoopContext>   ll_loop_stack;

LlDiBuilder *          di_builder;
LlDiUnit *             di_unit;
vector<LlDiScope *>    di_scope_stack;

void ll_defs_init()
{
  ll_module = new llvm::Module("dq", ll_ctx);
  di_builder = new LlDiBuilder(*ll_module);

  //llvm::DICompileUnit* compile_unit;
}

void ll_init_debug_info()
{
  OScFile * pfile = g_compiler->scf->curfile;

  di_unit = di_builder->createCompileUnit(
      llvm::dwarf::DW_LANG_C,
      pfile->di_file,
      "DQ Compiler",
      false,
      "",
      0
  );
}

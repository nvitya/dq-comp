/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc.cpp
 * authors: nvitya
 * created: 2026-01-31
 * brief:   DQ Compiler Object global instance
 */

#include <print>

#include "ll_defs.h"
#include "scope_builtins.h"
#include "scope_defines.h"

#include "dqc.h"
#include "dq_module.h"

ODqCompiler *  g_compiler = nullptr;

ODqCompiler::ODqCompiler()
{
}

ODqCompiler::~ODqCompiler()
{
}

void ODqCompiler::Run(int argc, char ** argv)
{
  errorcnt = 0;

  ParseCmdLineArgs(argc, argv);
  if (errorcnt)
  {
    return;
  }

  // initialize the source code feeder:
  if (scf->Init(in_filename) != 0)
  {
    ++errorcnt;
    return;
  }

  ll_init_debug_info();

  ParseModule();
  if (errorcnt)
  {
    print("Compile error.\n");
    return;
  }

  GenerateIr();
  if (errorcnt)
  {
    print("Code generation error.\n");
    return;
  }

  if (g_opt.ir_print)
  {
    PrintIr();
  }

  EmitObject(out_filename);

  if (0 == errorcnt)
  {
    print("OK.\n");
  }

  return;
}

void dqc_init()
{
  ll_defs_init();
  init_scope_builtins();
  init_scope_defines();

  init_dq_module();

  g_compiler = new ODqCompiler();
}

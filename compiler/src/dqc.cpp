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

#include "dqc.h"

ODqCompiler *  g_compiler = nullptr;

ODqCompiler::ODqCompiler()
{
}

ODqCompiler::~ODqCompiler()
{
}

int ODqCompiler::Run(int argc, char ** argv)
{
  error = 0;
  errormsg = "";

  ParseCmdLineArgs(argc, argv);
  if (error != 0)
  {
    return error;
  }

  // initialize the source code feeder:
  if (scf->Init(filename) != 0)
  {
    return SetError(1, "Error opening file");
  }

  ParseModule();

  return error;
}

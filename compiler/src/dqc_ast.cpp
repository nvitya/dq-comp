/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_ast.cpp
 * authors: nvitya
 * created: 2026-01-31
 * brief:   
 */

#include <print>
#include <format>

#include "dqc_ast.h"

ODqCompAst::ODqCompAst()
{
}

ODqCompAst::~ODqCompAst()
{
}

void ODqCompAst::AddVarDecl(OScPosition & scpos, string aid, const string atype, bool ainit, int64_t ainitval)
{
  // just printing now.
  print("{}: ", scpos.Format());
  print("AddVarDecl(): var {} : {}", aid, atype);
  if (ainit)
  {
    print(" = {}", ainitval);
  }
  print("\n");
}
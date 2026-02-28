/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    otype_int.h
 * authors: nvitya
 * created: 2026-02-02
 * brief:
 */

#include "otype_bool.h"

LlConst * OValueBool::CreateLlConst()
{
  return llvm::ConstantInt::get(ptype->GetLlType(), (value ? 1 : 0));
}

bool OValueBool::CalculateConstant(OExpr * expr)
{
  return false;
}

/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    otype_bool.h
 * authors: nvitya
 * created: 2026-02-02
 * brief:
 */

#pragma once

#include "symbols.h"

class OTypeBool : public OType
{
private:
  using        super = OType;

public:
  OTypeBool()
  :
    super("bool", TK_BOOL)
  {
    bytesize = 1;
  }

  OValSymConst * CreateConst(const string aname, bool avalue)
  {
    OValSymConst * result = new OValSymConst(aname, this);
    result->SetInlineData(nullptr, 1);
    result->inlinedata[0] = (avalue ? 1 : 0);
    return result;
  }

  LlType * CreateLlType() override
  {
    return LlType::getInt1Ty(ll_ctx);
  }

  LlDiType * CreateDiType() override
  {
    return di_builder->createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
  }
};
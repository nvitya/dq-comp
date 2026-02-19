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

#pragma once

#include "comp_symbols.h"

class OTypeInt : public OType
{
private:
  using        super = OType;

public:
  uint8_t      bitlength;
  bool         issigned;

  OTypeInt(const string name, uint8_t abitlength, bool asigned)
  :
    bitlength(abitlength),
    issigned(asigned),
    super(name, TK_INT)
  {
    bytesize = ((abitlength + 7) >> 3);
  }

  LlType * CreateLlType() override
  {
    return LlType::getIntNTy(ll_ctx, bitlength);
  }

  OValSymConst * CreateConst(const string aname, const int64_t avalue)
  {
    OValSymConst * result = new OValSymConst(aname, this);
    result->SetInlineData((void *)&avalue, bytesize);
    return result;
  }

  OValSymConst * CreateConstU(const string aname, const uint64_t avalue)
  {
    OValSymConst * result = new OValSymConst(aname, this);
    result->SetInlineData((void *)&avalue, bytesize);
    return result;
  }

};

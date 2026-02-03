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
    super(name, TK_INT),
    bitlength(abitlength),
    issigned(asigned)
  {
    bytesize = ((abitlength + 7) >> 3);
  }

  OConstValSym * CreateConst(const string aname, const int64_t avalue)
  {
    OConstValSym * result = new OConstValSym(aname, this);
    result->SetInlineData((void *)&avalue, bytesize);
    return result;
  }

  OConstValSym * CreateConstU(const string aname, const uint64_t avalue)
  {
    OConstValSym * result = new OConstValSym(aname, this);
    result->SetInlineData((void *)&avalue, bytesize);
    return result;
  }

};

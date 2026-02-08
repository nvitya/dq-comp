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

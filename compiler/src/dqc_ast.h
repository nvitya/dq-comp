/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_ast.h
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

#pragma once

#include "stdint.h"
#include <string>
#include "comp_options.h"
#include "comp_symbols.h"
#include "dq_module.h"

#include "dqc_base.h"

using namespace std;

class ODqCompAst : public ODqCompBase
{
private:
  using            super = ODqCompBase;

public:
  bool             section_public = true;
  OScope *         cur_mod_scope = nullptr;

public:
  ODqCompAst();
  virtual ~ODqCompAst();

  ODecl * AddVarDecl(OScPosition & scpos, string aid, OType * atype);

};
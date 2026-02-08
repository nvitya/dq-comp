/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    scope_module.h
 * authors: nvitya
 * created: 2026-02-07
 * brief:
 */

#pragma once

#include "comp_config.h"
#include "comp_symbols.h"

#include "otype_int.h"
#include "otype_bool.h"

class OScopeModule : public OScope  // module global scope
{
private:
  using         super = OScope;

public:
  

  OScopeModule()
  :
    super(nullptr, "global")
  {
  }

  void Init();
};

extern OScopeModule *  g_module;

void init_scope_module();
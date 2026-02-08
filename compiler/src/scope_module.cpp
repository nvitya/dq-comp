/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    scope_module.cpp
 * authors: nvitya
 * created: 2026-02-07
 * brief:
 */

#include "scope_module.h"

OScopeModule *  g_module;

void OScopeModule::Init()
{
}

void init_scope_module()
{
  g_module = new OScopeModule();
  g_module->Init();
}
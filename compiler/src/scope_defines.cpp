/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    scope_defines.cpp
 * authors: nvitya
 * created: 2026-02-02
 * brief:
 */

#include "scope_defines.h"

void OScopeDefines::Init()
{

}

void init_scope_defines()
{
  g_defines = new OScopeDefines();
  g_defines->Init();
}


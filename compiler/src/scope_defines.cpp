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

#include "comp_config.h"
#include "scope_defines.h"
#include "scope_builtins.h"


OScopeDefines *  g_defines;

void OScopeDefines::Init()
{
  #if defined(HOST_WIN)

    WindowsInit();

  #elif defined(HOST_LINUX)

    LinuxInit();

  #else

    #error "unsupported platform"

  #endif
}

void init_scope_defines()
{
  g_defines = new OScopeDefines();
  g_defines->Init();
}

void OScopeDefines::LinuxInit()
{
  DefineValSym(g_builtins->type_bool->CreateConst("LINUX", true));
}

void OScopeDefines::WindowsInit()
{
  DefineValSym(g_builtins->type_bool->CreateConst("WINDOWS", true));
}

/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dq_module.h
 * authors: nvitya
 * created: 2026-02-07
 * brief:   DQ Module Classes
 */

#pragma once

#include "comp_symbols.h"

enum EDeclKind
{
  DK_VAR,
  DK_CONST,
  DK_TYPE,       // type aliases, enums, function types
  DK_FUNCTION,
  DK_OBJECT
};

class ODecl
{
public:
  EDeclKind     kind;
  string        name;
  bool          ispublic;
};

class OModule
{
public:

};


/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    test_symbols.cpp
 * authors: nvitya
 * created: 2026-02-01
 * brief:   Test code for symbols
 */

#include <cstdlib>
#include <unistd.h>

#include <vector>
#include <string>
#include <print>

#include "comp_symbols.h"

using namespace std;

void test_symbols()
{
  print("TEST_SYMBOLS\n");

  // 1. Create Global Scope (Root container)

  OScope * global_scope = new OScope(nullptr, "global");
  OScope * scope = global_scope;

  OPrimitiveType * type_int = new OPrimitiveType("int");
  global_scope->Define(type_int);

  OCompoundType * type_class = new OCompoundType("OMyClass", global_scope);
  scope = type_class->Members();
  scope->Define(new OSymbol("field1", type_int));
  scope->Define(new OSymbol("field2", type_int));

  global_scope->Define(type_class);

  // Resolution Test
  OScope * found_scope;

  OSymbol * found = scope->Find("field1", &found_scope);
  if (found)
  {
    print("Found member: {} in scope {}\n", found->name, found_scope->debugname);
  }

  // If we look for 'int' inside the class, it bubbles up to Global
  OSymbol * found_type = scope->Find("int", &found_scope);
  if (found_type)
  {
    print("Found type: {} in scope {}\n", found_type->name, found_scope->debugname);
  }

  //OSymbol sym("int");
  //OType   ltype("myobj", TK_PRIMITIVE);

  print("to be continued...\n");
}
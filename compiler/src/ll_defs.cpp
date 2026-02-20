/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    ll_defs.h
 * authors: nvitya
 * created: 2026-02-19
 * brief:   LLVM defines, object
 */

#include "ll_defs.h"

LlContext    ll_ctx;
LlBuilder    ll_builder(ll_ctx);
LlModule *   ll_module;

vector<SLoopContext>  ll_loop_stack;

void ll_defs_init()
{
  ll_module = new llvm::Module("dq", ll_ctx);
}

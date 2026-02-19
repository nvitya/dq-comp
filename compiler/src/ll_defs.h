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
 * brief:   LLVM defines, global objects
 */

#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

// Short Aliases
using LlContext = llvm::LLVMContext;
using LlModule  = llvm::Module;
using LlBuilder = llvm::IRBuilder<>;

using LlValue      = llvm::Value;
using LlType       = llvm::Type;
using LlFunction   = llvm::Function;
using LlBasicBlock = llvm::BasicBlock;

// global variables

extern LlContext            ll_ctx;
extern LlBuilder            ll_builder;
extern LlModule *           ll_module;

void ll_defs_init();
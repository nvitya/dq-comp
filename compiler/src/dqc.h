/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc.h
 * authors: nvitya
 * created: 2026-01-31
 * brief:   DQ Compiler Object global instance
 */

#pragma once

#include "stdint.h"
#include <string>
#include "comp_options.h"

#include "dqc_clargs.h"

using namespace std;

class ODqCompiler : ODqCompClargs
{
private:
  using            super = ODqCompClargs;

public:
  ODqCompiler();
  virtual ~ODqCompiler();

  int Run(int argc, char ** argv);
};

extern ODqCompiler *  g_compiler;

void dqc_init();

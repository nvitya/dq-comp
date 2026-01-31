/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_codegen.h
 * authors: nvitya
 * created: 2026-01-31
 * brief:   
 */

#pragma once

#include "stdint.h"
#include <string>
#include "comp_options.h"
#include "comp_defines.h"

#include "dqc_parser.h"

using namespace std;

class ODqCompCodegen : public ODqCompParser
{
private:
  using                super = ODqCompParser;

public:
  ODqCompCodegen();
  virtual ~ODqCompCodegen();

};
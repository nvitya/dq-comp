/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_parser.h
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

#pragma once

#include "stdint.h"
#include <string>
#include "comp_options.h"
#include "statements.h"
#include "dqc_ast.h"

using namespace std;

class ODqCompParser : public ODqCompAst
{
private:
  using            super = ODqCompAst;

public:
  OScPosition      scpos_statement_start;
  OScPosition *    errorpos = nullptr;  // if nullptr then uses the scpos_statement_start

public:
  ODqCompParser();
  virtual ~ODqCompParser();

  void ParseModule();

  void ParseVarDecl();
  void ParseFunction();
  bool CheckStatementClose();

  void StatementError(const string amsg, OScPosition * scpos = nullptr, bool atryrecover = true);

  void ReadStatementBlock(OStmtBlock * block, const string blockend);

  void Error(const string amsg, OScPosition * ascpos = nullptr);
  void Warning(const string amsg, OScPosition * ascpos = nullptr);
  void Hint(const string amsg, OScPosition * ascpos = nullptr);

};
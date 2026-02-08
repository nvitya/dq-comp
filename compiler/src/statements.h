/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    statements.h
 * authors: nvitya
 * created: 2026-02-07
 * brief:   statements, statement blocks
 */

#pragma once

#include <vector>
#include "comp_symbols.h"

using namespace std;

class OStmt
{
public:

  OStmt()
  {
  }

  virtual ~OStmt()
  {
  }
};

class OStmtBlock
{
public:
  OScope *         scope;
  vector<OStmt *>  stlist;

  OStmtBlock(OScope * ascope)
  :
    scope(ascope)
  {
  }

  virtual ~OStmtBlock()
  {
    for (OStmt * st : stlist)
    {
      delete st;
    }
    stlist.clear();
  }
};

class OStmtAssign : public OStmt
{
public:
  OStmtAssign()
  {

  }
};

class OStmtVarDecl : public OStmt
{
  OScope *   scope;
  OValSym *  vsvar;

public:
  OStmtVarDecl(const string avarname, OType * atype, OValSym * ainitvalue)
  {

  }
};
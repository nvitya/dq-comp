/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    scf_dq.h
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

#pragma once

#include <string>
#include <vector>
#include "stdint.h"
#include "scf_base.h"

using namespace std;

class OScFeederDq : public OScFeederBase
{
private:
  using                super = OScFeederBase;

public:
  string               basedir = ".";

  vector<OScFile *>    scfiles;
  vector<OScPosition>  returnpos;

  OScPosition          scpos_start_directive;

  OScFeederDq();
  virtual ~OScFeederDq();

  int Init(const string afilename) override;
  void Reset() override;

  OScFile * LoadFile(const string afilename);

public: // parsing functions
  inline bool Eof() { return ((curp >= bufend) and (returnpos.size() == 0)); }

  void SkipWhite(); // jumps to the first normal token

public: // directive processing
  void ParseDirective();
  void PreprocError(const string amsg, OScPosition * ascpos = nullptr, bool atryrecover = true);

  void ParseDirectiveInclude();

  bool CheckConditionals(const string aid);  // processes if, ifdef, else, elif, elifdef, endif. Returns true, when one of those found

};

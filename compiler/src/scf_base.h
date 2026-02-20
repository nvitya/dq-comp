/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    scf_base.h
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

#pragma once

#include <string>
#include <vector>
#include "stdint.h"
#include "ll_defs.h"

#include "comp_config.h"

using namespace std;

class OScFile
{
public:
  int       index = 0;
  string    name = "";
  string    fullpath = "";
  string    body = "";
  int32_t   length = -1;
  char *    pstart = nullptr;
  char *    pend   = nullptr;

  int       usagecount = 1;

  LlDiFile *  di_file = nullptr;

  OScFile();
  ~OScFile();

  bool Load(const string aname, const string afullpath);
};

class OScPosition
{
public:
  OScFile *  scfile;
  char *     pos;

  OScPosition()
  {
    scfile = nullptr;
    pos = nullptr;
  }

  OScPosition(OScFile * ascfile, char * apos)
  {
    scfile = ascfile;
    pos = apos;
  }

  void Assign(OScPosition & ascpos)
  {
    scfile = ascpos.scfile;
    pos = ascpos.pos;
  }

  int GetLineNo();

  string Format();
};

class OScFeederBase
{
public:
  OScFile *            curfile = nullptr;

  char *               curp = nullptr;    // current parsing position
  char *               bufend = nullptr;
  char *               prevp;             // previous token start position
  int                  prevlen;           // usually signs token length

  OScPosition          prevpos;

  OScFeederBase();
  virtual ~OScFeederBase();

  virtual int Init(const string afilename);
  virtual void Reset();

public: // parsing functions

  void SaveCurPos(OScPosition & rpos) { rpos.scfile = curfile; rpos.pos = curp; }
  void SetCurPos(OScPosition & rpos);
  void SetCurPos(OScFile * afile, char * apos = nullptr);

  void SkipSpaces(bool askiplineend = true);  // jumps to the first non-space character

  bool IsIntLiteral();  // is an integer literal at the current position?
  bool IsNumChar();     // is a number [0-9] at the current position?

  bool ReadLine();                 // sets prevptr, prevlen
  bool ReadTo(const char * checkchars);  // sets prevptr, prevlen
  bool ReadToChar(char achar);     // sets prevptr, prevlen
  bool CheckSymbol(const char * checkstring, bool aconsume = true);
  bool SearchPattern(const char * checkchars, bool aconsume = true);  // sets prevptr, prevlen

  bool ReadIdentifier(string & rvalue);  // returns "" when invalid
  bool ReadInt64Value(int64_t & rvalue);
  bool ReadHex64Value(uint64_t & rvalue);
  bool ReadQuotedString(string & rvalue);

  bool ReadDecimalNumbers();       // sets prevptr, prevlen
  bool ReadHexNumbers();           // sets prevptr, prevlen
  bool ReadFloatNum();             // sets prevptr, prevlen

  string PrevStr();

};

string ExtractFilePath(const string & full_path);
string ExtractFileName(const string & full_path);

/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    testfile.h
 * authors: Codex
 * created: 2026-03-17
 * brief:   autotest file object
 */

#pragma once

#include <string>

using namespace std;

class OTestFile
{
public:
  string            filename;
  string            text;

public:
  OTestFile(const string & afilename);
  virtual ~OTestFile();
};

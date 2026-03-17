/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    at_runner.h
 * authors: Codex
 * created: 2026-03-17
 * brief:   dq autotest runner main coordinator
 */

#pragma once

class ODqAtRunner
{
public:
  ODqAtRunner();
  virtual ~ODqAtRunner();

  int Run();

protected:
  int RunBatch();
  int RunSingle();
};

extern ODqAtRunner *  g_atr;


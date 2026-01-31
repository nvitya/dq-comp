/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_clargs.cpp
 * authors: nvitya
 * created: 2026-01-31
 * brief:   
 */

#include "dqc_clargs.h"

ODqCompClargs::ODqCompClargs()
{
}

ODqCompClargs::~ODqCompClargs()
{
}

int ODqCompClargs::ParseCmdLineArgs(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("Usage: dq-comp <file.dq>\n");

    return SetError(1, "Invalid arguments");
  }

  filename = string(argv[1]);

  printf("Compiling: \"%s\"...\n", filename.c_str());

  return 0;
}
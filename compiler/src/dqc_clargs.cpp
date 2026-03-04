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

#include <print>
#include <string>
#include "dqc_clargs.h"
#include "comp_options.h"

using namespace std;

ODqCompClargs::ODqCompClargs()
{
}

ODqCompClargs::~ODqCompClargs()
{
}

void ODqCompClargs::ParseCmdLineArgs(int argc, char **argv)
{
  for (int i = 1; i < argc; i++)
  {
    string v(argv[i]);

    if ('-' == v[0])  // some compiler switch
    {
      if      ("-v"  == v)    g_opt.verbose = true;
      else if ("-g"  == v)    g_opt.dbg_info = true;
      else if ("-ir" == v)    g_opt.ir_print = true;
      else if ("-O0" == v)    g_opt.optlevel = 0;
      else if ("-O1" == v)    g_opt.optlevel = 1;
      else if ("-O2" == v)    g_opt.optlevel = 2;
      else if ("-O3" == v)    g_opt.optlevel = 3;
      else
      {
        ++errorcnt;
        print("Unknown command line switch: {}\n", v);
        PrintUsage();
        return;
      }
    }
    else if ("" == in_filename)
    {
      in_filename = v;
    }
    else if ("" == out_filename)
    {
      out_filename = v;
    }
    else
    {

    }
  }

  if ("" == in_filename)
  {
    ++errorcnt;
    printf("Input file name is missing.\n");
    PrintUsage();
    return;
  }

  if ("" == out_filename)
  {
    out_filename = in_filename + ".o";
  }

  printf("Compiling: \"%s\"...\n", in_filename.c_str());
  return;
}

void ODqCompClargs::PrintUsage()
{
  print("Usage:\n");
  print("  dq-comp <file.dq> [output.o] [switches]\n");
  print("Switches:\n");
  print("  -On: optimization level, n=0-3\n");
  print("  -g: generate debug info\n");
  print("  -v: print compiler internal trace messages\n");
  print("  -ir: print LLVM IR code\n");
}

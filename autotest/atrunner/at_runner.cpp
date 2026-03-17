/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    at_runner.cpp
 * authors: Codex
 * created: 2026-03-17
 * brief:
 */

#include <print>
#include <filesystem>
#include <string>
#include <algorithm>
#include <vector>

#include "at_runner.h"
#include "atr_options.h"
#include "atr_version.h"
#include "processrunner.h"

using namespace std;
namespace fs = std::filesystem;

OAtRunner *  g_atr = nullptr;

static string TrimLineEnd(string s)
{
  while (!s.empty() and ((s.back() == '\n') or (s.back() == '\r')))
  {
    s.pop_back();
  }

  return s;
}

static string QueryCompilerVersion()
{
  OProcessRunner procrunner;
  SProcessResult procresult;

  vector<string> args;
  args.push_back(g_atropt->compiler_filename);
  args.push_back("--version");

  if (!procrunner.Run(args, &procresult))
  {
    return "?";
  }

  if (!procresult.stdout_text.empty())
  {
    return TrimLineEnd(procresult.stdout_text);
  }

  if (!procresult.stderr_text.empty())
  {
    return TrimLineEnd(procresult.stderr_text);
  }

  return "?";
}

static void PrintBatchHeader()
{
  print("DQ Autotest v{}\n", ATR_VERSION);
  print("Compiler:  {}\n", g_atropt->compiler_filename);
  print("C. ver.:   v{}\n", QueryCompilerVersion());
  print("Test root: {}\n", g_atropt->test_root);
  print("\n");
}

OAtRunner::OAtRunner()
{
}

OAtRunner::~OAtRunner()
{
  for (OTestFile * tf : testfiles)
  {
    delete tf;
  }
}

void OAtRunner::CollectTestFiles()
{
  for (OTestFile * tf : testfiles)
  {
    delete tf;
  }
  testfiles.clear();

  fs::path rootpath(g_atropt->test_root);
  if (!fs::exists(rootpath))
  {
    return;
  }

  vector<fs::path> foundfiles;

  for (const fs::directory_entry & de : fs::recursive_directory_iterator(rootpath))
  {
    if (!de.is_regular_file())
    {
      continue;
    }

    fs::path p = de.path();
    if (".dq" != p.extension().string())
    {
      continue;
    }

    foundfiles.push_back(fs::relative(p, rootpath));
  }

  sort(foundfiles.begin(), foundfiles.end());

  for (const fs::path & rp : foundfiles)
  {
    OTestFile * tf = new OTestFile(rp.generic_string());
    testfiles.push_back(tf);
  }
}

void OAtRunner::DebugPrintCollectedFiles()
{
  for (OTestFile * tf : testfiles)
  {
    print("{}\n", tf->filename);
  }
}

int OAtRunner::Run()
{
  if (!g_atropt)
  {
    return 1;
  }

  if (ATRMODE_BATCH == g_atropt->run_mode)
  {
    return RunBatch();
  }

  return RunSingle();
}

int OAtRunner::RunBatch()
{
  PrintBatchHeader();
  CollectTestFiles();
  DebugPrintCollectedFiles();
  return 0;
}

int OAtRunner::RunSingle()
{
  return 0;
}

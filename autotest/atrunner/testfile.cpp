/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    testfile.cpp
 * authors: Codex
 * created: 2026-03-17
 * brief:
 */

#include "testfile.h"

#include <chrono>
#include <random>
#include <thread>
#include <print>
#include <format>
#include <fstream>

#include "atr_options.h"

OTestFile::OTestFile(const string & afilename)
{
  filename = afilename;
}

OTestFile::~OTestFile()
{
}

void OTestFile::Process()
{
  if (!g_atropt->batchmode and g_atropt->verbose)
  {
    print("Processing \"{}\"...\n", filename);
  }

  if (not LoadText())
  {
    msg_tf.push_back("File load error.");
    processed = true;
    return;
  }

  if (not ParseText())
  {
    processed = true;
    return;
  }

#if 0

  static thread_local mt19937 rng(random_device{}());
  uniform_int_distribution<int> dist(10, 500);

  int sleeptime = dist(rng);
  if (sleeptime < 220)
  {
    exec_err = true;
    errorcnt_err = (sleeptime % 3);
  }
  else
  {
    exec_run = true;
    errorcnt_run = (sleeptime % 3);
  }

  this_thread::sleep_for(chrono::milliseconds(sleeptime));
#endif

  processed = true;
}

bool OTestFile::LoadText()
{
  ifstream f(filename, ios::binary | ios::ate);
  if (!f)
  {
    return false;
  }

  int length = f.tellg();
  text.resize(length);
  if (length > 0)
  {
    f.seekg(0);
    f.read(text.data(), length);
  }

  return true;
}

bool OTestFile::ParseText()
{
  sp.Init(text.data(), text.size());

  string sid;

  // find all markers
  while (sp.readptr < sp.bufend)
  {
    if (not sp.SearchPattern("//?"))
    {
      break; // no more markers
    }

    sp.CheckSymbol("//?"); // the searchpatten does not consume the pattern itself, so do it now
    sp.SkipSpaces();

    if (!sp.ReadIdentifier(sid))
    {
      AddTfError("Identifier expected after \"//?\"");
    }

    if ("error" == sid)
    {
      ParseMarkerError();
    }
    else if ("check" == sid)
    {
      ParseMarkerCheck();
    }
    else
    {
      AddTfError(format("Unknown marker \"{}\"", sid));
    }
  }

  return (0 == errorcnt_tf);
}

void OTestFile::ParseMarkerError()
{
  // sample: //?error(TypeSpecExpected)
  // note the "//?error" is already consumed

  int errline = sp.GetLineNum();

  sp.SkipSpaces();
  if (not sp.CheckSymbol("("))
  {
    AddTfError(format("\"(\" is missing after \"//?error\""));
    return;
  }
  sp.SkipSpaces();
  string errid;
  if (not sp.ReadIdentifier(errid))
  {
    AddTfError(format("Error id is missing after \"//?error\""));
    return;
  }

  sp.SkipSpaces();
  if (not sp.CheckSymbol(")"))
  {
    AddTfError(format("\")\" is missing after \"//?error\""));
    return;
  }

  err_captures.push_back(new OErrCapture(errline, errid));

}

void OTestFile::ParseMarkerCheck()
{
  // sample: printf("Hello2=5\n");   //?check(Hello2, 5)
  // note "//?check" is already consumed

  sp.SkipSpaces();
  if (not sp.CheckSymbol("("))
  {
    AddTfError(format("\"(\" is missing after \"//?check\""));
    return;
  }
  sp.SkipSpaces();
  string strid;
  if (not sp.ReadIdentifier(strid))
  {
    AddTfError(format("Id is missing after \"//?check\""));
    return;
  }

  string sv = "";
  sp.SkipSpaces();
  if (sp.CheckSymbol(","))
  {
    sp.SkipSpaces();
    if (sp.ReadQuotedString())
    {
      sv = sp.PrevStr();
    }
    else
    {
      if (not sp.ReadToChar(')'))
      {
        AddTfError(format("\")\" is missing after \"//?check\""));
        return;
      }

      sv = sp.PrevStr();
      // remove the trailing spaces
      auto pos = sv.find_last_not_of(" \t\n\r\f\v");
      sv.erase(pos == std::string::npos ? 0 : pos + 1);
    }
  }
  else
  {
    if (not sp.CheckSymbol(")"))
    {
      AddTfError(format("\")\" is missing after \"//?check\""));
      return;
    }
  }

  run_captures.push_back(new ORunCapture(strid, sv));
}

void OTestFile::AddTfError(const string astr)
{
  int linenum = sp.GetLineNum(sp.prevptr);
  msg_tf.push_back(format("line {}: {}", linenum, astr));
  ++errorcnt_tf;
}

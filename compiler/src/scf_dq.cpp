/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    scf_dq.cpp
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

// scf_dq.cpp
// principles and some algorithms taken from strparseobj: github....

#include "string.h"
#include "math.h"

#include <fstream>
#include <print>
#include <format>
#include <filesystem>

#include "scf_dq.h"
#include "scope_defines.h"

//---------------------------------------------------------

OScFeederDq::OScFeederDq()
{
}

OScFeederDq::~OScFeederDq()
{
  // free the conditional chain
  while (curcond)
  {
    OScfCondition * p = curcond->parent;
    delete curcond;
    curcond = p;
  }

  Reset();
}

int OScFeederDq::Init(const string afilename)
{
  super::Init(afilename);

  filesystem::path  fpath(afilename);
  basedir = fpath.parent_path().string();
  if ("" == basedir)  basedir = ".";

  curfile = LoadFile(fpath.filename().string());
  if (!curfile)
  {
    return 1;
  }

  SetCurPos(curfile, curfile->pstart);

  return 0;
}

void OScFeederDq::Reset()
{
  super::Reset();

  for (OScFile * f : scfiles)
  {
    delete f;
  }

  scfiles.clear();
  returnpos.clear();
}

OScFile * OScFeederDq::LoadFile(const string afilename)
{
  // check if already loaded
  for (OScFile * fs : scfiles)
  {
    if (fs->name == afilename)
    {
      return fs;
    }
  }

  string fullname = basedir + "/" + afilename;

  OScFile * f = new OScFile();
  if (!f->Load(afilename, fullname))
  {
    print("Error loading file: \"{}\"!\n", fullname);

    delete f;
    return nullptr;
  }

  print("File \"{}\" loaded: {} bytes\n", fullname, f->length);

  scfiles.push_back(f);
  return f;
}

void OScFeederDq::SkipWhite()
{
  // jumps to the next normal token while:
  //  - processes the #{opt } directives
  //  - processes the #{include } directives
  //  - processes the #{if...} directives, skipping inactive branches

  // this is a pretty complex function

repeat_skip:  // jumped here when returning from an include

  while (curp < bufend)
  {
    SkipSpaces();

    if (curp >= bufend)
    {
      return;
    }

    if (CheckSymbol("//")) // single line comment
    {
      ReadTo("\n\r");
    }
    else if (CheckSymbol("/*"))  // multi-line comment start
    {
      // search for the end
      if (SearchPattern("*/", true))
      {
        // closing marker found
      }
      else
      {
        // the closing was not found in this file, jump to the end
        curp = curfile->pend;
      }
    }
    else if (CheckSymbol("#{"))  // compiler directive
    {
      ParseDirective();
    }
    else // should be a normal token then
    {
      return;
    }
  }

  if (returnpos.size() > 0) // is there any return position recorded ?
  {
    OScPosition rpos = returnpos.back();
    returnpos.pop_back();
    SetCurPos(rpos);

    goto repeat_skip;
  }
}

void OScFeederDq::ParseDirective()
{
  // Examples:
  //   #{include "filename.dq"}
  //   #{opt ... }
  //   #{if ... }

  string sid;

  // the prevpos was already set pointint the beginning of the "#{ " symbol
  scpos_start_directive.Assign(prevpos);  // save the start of the directive. prevpos saved in the CheckSymbol()

  SkipSpaces(); // do not use SkipWhite() here !

  // some keyword must come here
  if (not ReadIdentifier(sid))
  {
    PreprocError("Compiler directive keyword is missing. Syntax: #{keyword arguments}");
    return;
  }

  // process the directive...

  if ("include" == sid)
  {
    ParseDirectiveInclude(); // already contains end
  }
  else if (CheckConditionals(sid))  // ifdef, if, else, etc
  {

  }
  else  // unknown
  {
    PreprocError(format("Unknown compiler directive \"#{{{} ... }}\"", sid));
    return;
  }

}

void OScFeederDq::ParseDirectiveInclude()
{
  // #{include "filename.dq" }
  // note: include already consumed!

  string sfname;

  SkipSpaces();

  if (not ReadQuotedString(sfname))
  {
    PreprocError("Include file name is missing. Syntax: #{include \"...\" }");
    return;
  }

  // find the end
  SkipSpaces();
  if (not CheckSymbol("}"))
  {
    PreprocError("Closing \"}\" is missing");
    return;
  }

  print("{}: ", scpos_start_directive.Format());
  print("Including ({})\n", sfname);

  OScFile * incfile = LoadFile(sfname);
  if (!incfile)
  {
    // try with the current path
    filesystem::path  fpath(curfile->name);
    string parentpath = fpath.parent_path(); //.string();
    if (!parentpath.empty())
    {
      parentpath += filesystem::path::preferred_separator;
    }
    incfile = LoadFile(parentpath + sfname);
    if (!incfile)
    {
      PreprocError(format("Include file loading error: \" மூல \"", sfname));
      return;
    }
  }

  // push return position

  OScPosition  retpos;
  SaveCurPos(retpos);            // save the current position (pointing after the directive)
  returnpos.push_back(retpos);   // add to the return stack

  // switch to the include
  SetCurPos(incfile, incfile->pstart);
}

void OScFeederDq::PreprocError(const string amsg, OScPosition * ascpos, bool atryrecover)
{
  OScPosition log_scpos(curfile, curp);

  if (ascpos and ascpos->scfile) // use the position provided
  {
    log_scpos.Assign(*ascpos);
  }

  print("{}: {}\n", log_scpos.Format(), amsg);

  // try to recover
  if (atryrecover)
  {
    if (ReadToChar('}')) // find the closing
    {
      CheckSymbol("}"); // consume
    }
  }
}

bool OScFeederDq::CheckConditionals(const string aid)
{
  string sid;

  // starters

  if (("ifdef" == aid) || ("ifndef" == aid))
  {
    SkipSpaces();

    curcond = new OScfCondition(curcond, scpos_start_directive, inactive_code);

    if (!ReadIdentifier(sid))
    {
      PreprocError("Define symbol name is missing after ifdef");
      inactive_code = true;
    }
    else
    {
      if (not inactive_code)
      {
        bool isdefined = g_defines->Defined(sid);
        if ((("ifdef" == aid) and isdefined) or (("ifndef" == aid) and not isdefined))
        {
          curcond->branch_taken = true;
        }
        else
        {
          inactive_code = true;
        }
      }
    }

    print("{}: #{{{}}} \"{}\" {}\n", scpos_start_directive.Format(), aid, sid, (inactive_code ? "(inactive)" : ""));
  }
  else if ("if" == aid)
  {
    print("{}: #if found, NOT IMPLEMENTED YET!\n", scpos_start_directive.Format());
  }

  // closer

  else if ("endif" == aid)
  {
    if (!curcond)
    {
      PreprocError("#{{endif}} without previous #{{if...}}!");
    }
    else
    {
      if (curcond->startpos.scfile != curfile)
      {
        PreprocError("#{{endif}} for different include file!");
      }
      inactive_code = curcond->parent_inactive;
      curcond = curcond->parent;
      print("{}: #{{endif}} {}\n", scpos_start_directive.Format(), (inactive_code ? "(inactive)" : ""));
    }
  }

  // continuing

  else if ("else" == aid)
  {
    if (!curcond)
    {
      PreprocError("#{{else}} without #{{if...}}");
    }
    else if (FCOND_ELSE == curcond->state)
    {
      PreprocError(format("#{{else}} directive after previous #{{else}} at {}", curcond->elsepos.Format()));
    }
    else
    {
      inactive_code = curcond->parent_inactive;

      if (curcond->startpos.scfile != curfile)  // same include ?
      {
        PreprocError("#{{else}} for #{{if...}} in different include file!");
        curcond = new OScfCondition(curcond, scpos_start_directive, inactive_code);
        inactive_code = true;
      }

      curcond->state = FCOND_ELSE;
      curcond->elsepos.Assign(scpos_start_directive);

      if (not inactive_code and curcond->branch_taken)
      {
        inactive_code = true;
      }

      print("{}: #{{else}} {}\n", scpos_start_directive.Format(), (inactive_code ? "(inactive)" : ""));
    }
  }
  else if (("elifdef" == aid) || ("elifndef" == aid))
  {
    SkipSpaces();
    if (not ReadIdentifier(sid))
    {
      PreprocError("Define symbol name is missing after ifdef");
      inactive_code = true;
    }

    if (!curcond)
    {
      PreprocError(format("#{{{}}} without #{{if...}}", aid));
      inactive_code = true;
    }
    else if (FCOND_ELSE == curcond->state)
    {
      PreprocError(format("#{{elif...}} directive after previous #{{else}} at {}", curcond->elsepos.Format()));
      inactive_code = true;
    }
    else
    {
      // restore the inactive code state
      inactive_code = (curcond->parent_inactive or curcond->branch_taken);

      if (curcond->startpos.scfile != curfile)  // same include ?
      {
        PreprocError("#{{elifdef}} for #{{if...}} in different include file!");
        curcond = new OScfCondition(curcond, scpos_start_directive, inactive_code);
        inactive_code = true;
      }

      curcond->state = FCOND_ELIF;
      if (not inactive_code)
      {
        bool isdefined = g_defines->Defined(sid);
        if ((("elifdef" == aid) and isdefined) or (("elifndef" == aid) and not isdefined))
        {
          curcond->branch_taken = true;
        }
        else
        {
          inactive_code = true;
        }
      }
    }

    print("{}: #{{{}}} \"{}\" {}\n", scpos_start_directive.Format(), aid, sid, (inactive_code ? "(inactive)" : ""));
  }
  else if ("elif" == aid)
  {
    print("{}: #{{elif}} found, NOT IMPLEMENTED YET!\n", scpos_start_directive.Format());
  }
  else
  {
    return false;
  }

  // find the end
  SkipSpaces();
  if (not CheckSymbol("}"))
  {
    PreprocError("Closing \"}\" is missing");
  }

  return true;
}

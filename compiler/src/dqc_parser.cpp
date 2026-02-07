/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_parser.cpp
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

#include <print>
#include <format>

#include "dqc_parser.h"

ODqCompParser::ODqCompParser()
{
}

ODqCompParser::~ODqCompParser()
{

}

void ODqCompParser::ParseModule()
{
  string sid;

  while (not scf->Eof())
  {
    scf->SkipWhite(); // jumps to the first normal token

    scf->SaveCurPos(scpos_statement_start);  // to display the statement start

    // module root starters
    if (not scf->ReadIdentifier(sid))
    {
      StatementError("Module statement keyword expected", &scpos_statement_start);
      continue;
    }

    // The module root statement must start with a keyword like
    //   use, module, var, type, function, implementation

    if ("var" == sid) // global variable definition
    {
      ParseStatementVar();
    }
    else  // unknown
    {
      StatementError("Unknown module root statement qualifier: \"" + sid + "\"", &scpos_statement_start);
    }
  }

  printf("ParseModule finished.");

  //printf("First normal token:\n\"%s\"\n", scf->curp);
}

void ODqCompParser::ParseStatementVar()
{
  // syntax form: "var identifier : type [ = initial value];"
  // note: "var" is already consumed

  string   sid;
  string   stype;
  int64_t  initvalue = 0;
  bool     initialized = false;

  scf->SkipWhite();
  if (not scf->ReadIdentifier(sid))
  {
    StatementError("Identifier is expected after \"var\". Syntax: \"var identifier : type [ = initial value];\"");
    return;
  }

  scf->SkipWhite();
  if (not scf->CheckSymbol(":"))
  {
    StatementError("Type specifier \":\" is expected after \"var\". Syntax: \"var identifier : type [ = initial value];\"");
    return;
  }

  scf->SkipWhite();
  if (not scf->ReadIdentifier(stype))
  {
    StatementError("Type identifier is expected after \"var\". Syntax: \"var identifier : type [ = initial value];\"");
    return;
  }

  // check the type here for proper source code position (scf->prevpos)
  if (!CheckType(stype, &scf->prevpos)) // statement error already issued
  {
    return;
  }

  scf->SkipWhite();
  if (scf->CheckSymbol("="))  // variable initializer specified
  {
    initialized = true;

    scf->SkipWhite();
    // later this comes here:
    // OExpression * expr = ParseExpression();

    if (not scf->ReadInt64Value(initvalue))
    {
      StatementError("Integer literal expected (so far only this supported)");
      return;
    }
  }

  if (not CheckStatementClose())
  {
    // error message already generated.
    return;
  }

  // everything is fine
  AddVarDecl(scpos_statement_start, sid, stype, initialized, initvalue);
}

bool ODqCompParser::CheckType(const string atype, OScPosition * ascpos)
{
  if ("int" == atype)
  {
    // type ok.
  }
  else
  {
    StatementError(format("Unknown type \"{}\"", atype), ascpos);
    return false;
  }

  return true;
}

void ODqCompParser::StatementError(const string amsg, OScPosition * scpos, bool atryrecover)
{
  OScPosition log_scpos(scf->curfile, scf->curp);

  if (scpos and scpos->scfile) // use the position provided
  {
    log_scpos.Assign(*scpos);
  }

  print("{}: {}\n", log_scpos.Format(), amsg);

  // try to recover
  if (atryrecover)
  {
    if (!scf->SearchPattern(";", true))  // TODO: improve to handle #{} and strings
    {

    }
  }

  scf->SkipWhite();
}

bool ODqCompParser::CheckStatementClose()
{
  scf->SkipWhite();
  if (not scf->CheckSymbol(";"))
  {
    StatementError("\";\" is expected to close the previous statement");
    return false;
  }
  return true;
}

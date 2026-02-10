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

#include "dq_module.h"
#include "dqc_parser.h"
#include "otype_func.h"

ODqCompParser::ODqCompParser()
{
}

ODqCompParser::~ODqCompParser()
{

}

void ODqCompParser::ParseModule()
{
  string sid;

  section_public = true;
  cur_mod_scope = g_module->scope_pub;

  while (not scf->Eof())
  {
    scf->SkipWhite(); // jumps to the first normal token
    if (scf->Eof())
    {
      return; // end of module
    }

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
      ParseVarDecl();
    }
    else if ("function" == sid)
    {
      ParseFunction();
    }
    else  // unknown
    {
      StatementError("Unknown module root statement qualifier: \"" + sid + "\"", &scpos_statement_start);
    }
  }

  printf("ParseModule finished.");

  //printf("First normal token:\n\"%s\"\n", scf->curp);
}

void ODqCompParser::ParseVarDecl()
{
  // syntax form: "var identifier : type [ = initial value];"
  // note: "var" is already consumed

  string     sid;
  string     stype;
  OValSym *  pvalsym;
  OType *    ptype;

  scf->SkipWhite();
  if (not scf->ReadIdentifier(sid))
  {
    StatementError("Identifier is expected after \"var\". Syntax: \"var identifier : type [ = initial value];\"");
    return;
  }

  if (g_module->ValSymDeclared(sid, &pvalsym))
  {
    StatementError(format("Variable \"{}\" is already declared with the type \"{}\"", sid, pvalsym->ptype->name), &scf->prevpos);
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
  ptype = g_module->scope_priv->FindType(stype);
  if (not ptype)
  {
    StatementError(format("Unknown type \"{}\"", stype), &scf->prevpos);
    return;
  }

#if 0
  int64_t    initvalue = 0;
  bool       initialized = false;
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
#endif

  if (not CheckStatementClose())
  {
    // error message already generated.
    return;
  }

  AddDeclVar(scpos_statement_start, sid, ptype);

  // TODO: add the initialization
}

void ODqCompParser::ParseFunction()
{
  // note: "function" is already consumed
  // syntax form: "function identifier[(arglist)] [-> return_type] <statement_block | ;>"
  // statement block must follow, when ';' then it is a forward declaration

  string   sid;
  string   stype;

  scf->SkipWhite();
  if (not scf->ReadIdentifier(sid))
  {
    Error("Identifier is expected after \"function\". Syntax: \"function identifier(arglist) -> return_type\"");
    return;
  }

  OTypeFunc    * tfunc  = new OTypeFunc(sid);
  OValSymFunc  * vsfunc = new OValSymFunc(sid, tfunc, cur_mod_scope);

  scf->SkipWhite();
  if (scf->CheckSymbol("("))  // parameter list start
  {
    string spname;
    string sptype;

    while (not scf->Eof())
    {
      scf->SkipWhite();
      if (scf->CheckSymbol(")"))
      {
        break;
      }

      if (tfunc->params.size() > 0)
      {
        if (not scf->CheckSymbol(","))
        {
          Error("\",\" expected for parameter lists", &scf->prevpos);
        }
        else { scf-> SkipWhite(); }
      }

      if (not scf->ReadIdentifier(spname))
      {
        Error("Parameter name expected", &scf->prevpos);
        if (not scf->ReadTo(",)"))  // try to skip to next parameter
        {
          break;  // serious problem, would lead to endless-loop
        }
        continue;
      }

      if (not tfunc->ParNameValid(spname))
      {
        Error("Invalid function parameter name \""+spname+"\"", &scf->prevpos);
        scf->ReadTo(",)");  // try to skip to next parameter
        continue;
      }

      scf->SkipWhite();
      if (not scf->CheckSymbol(":"))
      {
        Error("Parameter type specification expected: \": type\"", &scf->prevpos);
        scf->ReadTo(",)");  // try to skip to next parameter
        continue;
      }

      scf->SkipWhite();
      if (not scf->ReadIdentifier(sptype))
      {
        Error("Function parameter type name expected", &scf->prevpos);
        scf->ReadTo(",)");  // try to skip to next parameter
        continue;
      }

      OType * ptype = cur_mod_scope->FindType(sptype);
      if (!ptype)
      {
        Error(format("Unknown function parameter type \"{}\"", sptype), &scf->prevpos);
        scf->ReadTo(",)");  // try to skip to next parameter
        continue;
      }

      // OK
      tfunc->AddParam(spname, ptype);

    }  // while function parameters
  }

  scf->SkipWhite();
  if (scf->CheckSymbol("->"))  // return type
  {
    scf->SkipWhite();
    string frtname;
    if (not scf->ReadIdentifier(frtname))
    {
      Error("Function return type identifier expected after \"->\"");
    }
    else
    {
      tfunc->rettype = cur_mod_scope->FindType(frtname);
      if (not tfunc->rettype)
      {
        Error(format("Unknown function return type \"{}\"", frtname));
      }
    }
  }

  AddDeclFunc(scpos_statement_start, vsfunc);

  // go on with the function body

  ReadStatementBlock(vsfunc->body, "endfunc");
}

void ODqCompParser::ReadStatementBlock(OStmtBlock * block, const string blockend)
{
  string block_closer;
  string sid;
  OValSym * pvalsym;

  scf->SkipWhite();
  if (scf->CheckSymbol("{"))
  {
    block_closer = "}";
  }
  else if (scf->CheckSymbol(":"))
  {
    block_closer = blockend;
  }
  else
  {
    StatementError("\":\" is missing for statement block start");
    block_closer = blockend;
  }

  while (not scf->Eof())
  {
    scf->SkipWhite();
    if (scf->CheckSymbol(block_closer.c_str()))
    {
      return;  // block closer was found
    }

    if (scf->Eof())
    {
      StatementError(format("Statement block closer \"{}\" is missing", block_closer));
      return;
    }

    // there should be a normal statement
    if (!scf->ReadIdentifier(sid))
    {
      StatementError("keyword or identifier is missing");
      continue;
    }

    if (ReservedWord(sid))
    {
      if (scf->CheckSymbol("var"))  // local variable declaration
      {
        StatementError("var statement parsing is not implemented");
        //ParseStatementVaxr(block->scope);
        continue;
      }
      else if ("return" == sid)
      {
        StatementError("return statement parsing is not implemented");
        //ParseStatementVaxr(block->scope);
        continue;
      }
      else
      {
        StatementError(format("Statement \"{}\" not implemented yet", sid));
        continue;
      }
    }
    else // assignment
    {
      if (!g_module->ValSymDeclared(sid, &pvalsym))
      {
        StatementError(format("Undefined variable \"{}\"", sid));
        continue;
      }

      scf->SkipWhite();
      if (!scf->CheckSymbol("="))
      {
        StatementError("Assignment operator \"=\" is expected.");
        continue;
      }

    }
  }
}

void ODqCompParser::StatementError(const string amsg, OScPosition * scpos, bool atryrecover)
{
  OScPosition log_scpos(scf->curfile, scf->curp);

  if (scpos and scpos->scfile) // use the position provided
  {
    log_scpos.Assign(*scpos);
  }

  Error(amsg, &log_scpos);
  //print("{}: {}\n", log_scpos.Format(), amsg);

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

void ODqCompParser::Error(const string amsg, OScPosition * ascpos)
{
  OScPosition * epos = ascpos;
  if (!epos) epos = errorpos;
  if (!epos) epos = &scpos_statement_start;

  print("{} ERROR: {}\n", epos->Format(), amsg);
}

void ODqCompParser::Warning(const string amsg, OScPosition * ascpos)
{
  OScPosition * epos = ascpos;
  if (!epos) epos = errorpos;
  if (!epos) epos = &scpos_statement_start;

  print("{} WARNING: {}\n", epos->Format(), amsg);
}

void ODqCompParser::Hint(const string amsg, OScPosition * ascpos)
{
  OScPosition * epos = ascpos;
  if (!epos) epos = errorpos;
  if (!epos) epos = &scpos_statement_start;

  print("{} HINT: {}\n", epos->Format(), amsg);
}

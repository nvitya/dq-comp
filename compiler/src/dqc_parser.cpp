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
#include "statements.h"

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
      curscope = cur_mod_scope;
      curblock = nullptr;
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

  curscope = vsfunc->scope;
  curblock = vsfunc->body;

  // go on with the function body

  ReadStatementBlock("endfunc");
}

void ODqCompParser::ReadStatementBlock(const string blockend)
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

    scf->SaveCurPos(scpos_statement_start);

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
        ParseStmtReturn();
        //StatementError("return statement parsing is not implemented");
        //ParseStatementVaxr(block->scope);
        continue;
      }
      else if ("while" == sid)
      {
        ParseStmtWhile();
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

void ODqCompParser::ParseStmtReturn()
{
  // "return" is already consumed.

  scf->SkipWhite();
  OExpr * expr = ParseExpression();
  scf->SkipWhite();
  if (!scf->CheckSymbol(";"))
  {
    Error("\";\" is missing after the return expression");
  }
  if (expr)
  {
    curblock->AddStatement(new OStmtReturn(expr));
  }
}

OScope * ODqCompParser::PushScope(OScope *ascope)
{
  return nullptr;
}

OScope *ODqCompParser::PopScope()
{
  return nullptr;
}

void ODqCompParser::ParseStmtWhile()
{
  // note: "while" is already consumed
  // syntax form: "while <condition>: <statement_block> endwhile"
  string   sid;
  scf->SkipWhite();

  OExpr * cond = ParseExpression();
  if (!cond)
  {
    StatementError("While condition is missing");
    return;
  }

  OScope * prev_scope = curscope;
  OStmtWhile * st = new OStmtWhile(cond, curscope);
  curscope = st->body->scope;


  curscope = prev_scope;
}

void ODqCompParser::ParseStmtIf()
{
}

OExpr * ODqCompParser::ParseExpression()
{
  return ParseExprAdd();
}

OExpr * ODqCompParser::ParseExprAdd()
{
  OExpr * left  = ParseExprMul();
  OExpr * right = nullptr;

  scf->SkipWhite();
  if (scf->CheckSymbol("+"))
  {
    right = ParseExprMul();
    if (right)
    {
      return new OBinExpr(BINOP_ADD, left, right);
    }
  }
  else if (scf->CheckSymbol("-"))
  {
    right = ParseExprMul();
    if (right)
    {
      return new OBinExpr(BINOP_SUB, left, right);
    }
  }

  return left;
}

OExpr * ODqCompParser::ParseExprMul()
{
  OExpr * left  = ParseExprPrimary();
  OExpr * right = nullptr;

  scf->SkipWhite();
  if (scf->CheckSymbol("*"))
  {

    right = ParseExprPrimary();
    if (right)
    {
      return new OBinExpr(BINOP_MUL, left, right);
    }
  }

  return left;
}

OExpr * ODqCompParser::ParseExprPrimary()
{
  OExpr * result = nullptr;

  scf->SkipWhite();
  if (scf->CheckSymbol("("))
  {
    result = ParseExpression();
    scf->SkipWhite();
    if (!scf->CheckSymbol(")"))
    {
      Error("\")\" expected");
    }
    return result;
  }

  if (scf->CheckSymbol("0x"))  // hex number ?
  {
    uint64_t  hexval;
    if (scf->ReadHex64Value(hexval))
    {
      result = new OIntLit(int64_t(hexval));
    }
    else
    {
      Error("hexadecimal numbers expected after \"0x\"");
    }
    return result;
  }

  if (scf->IsNumChar())  // '0' .. '9' ?
  {
    //TODO: support floating point: 0.123, 2.1e-5, 1.234E6

    // int64 only so far (without sign)
    int64_t  intval;
    if (scf->ReadInt64Value(intval))
    {
      result = new OIntLit(intval);
    }
    else  // impossible case
    {
      Error("Integer literal parsing error.");
    }
    return result;
  }

  // identifier

  string  sid;
  if (!scf->ReadIdentifier(sid))
  {
    if (!scf->Eof())
    {
      Error(format("Unexpected expression char \"{}\"", *scf->curp));
    }
    else
    {
      Error("Expression expected.");
    }
    return result;
  }

  OValSym * vs = curscope->FindValSym(sid);
  if (!vs)
  {
    Error(format("Unknown identifier \"{}\"", sid));
    return result;
  }

  // types
  //  - variable reference
  //  - constant
  //  - compbound variable
  //  - function

  //OType * ptype = vs->ptype;
  ETypeKind tk = vs->ptype->kind;

  if (TK_FUNCTION == tk)
  {
    Error(format("Function call \"{}\" not implemented", sid));
    return result;
  }

  if (TK_COMPOUND == tk)
  {
    Error(format("Object/Struct reference \"{}\" not implemented", sid));
    return result;
  }

  if (TK_INT != tk)
  {
    Error("Only int type is supported so far.");
    return result;
  }

  result = new OVarRef(vs);
  return result;
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

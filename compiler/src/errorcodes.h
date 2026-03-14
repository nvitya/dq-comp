/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    errorcodes.h
 * authors: nvitya
 * created: 2026-03-13
 * brief:   DQ Compiler Error/Warning/Hint codes
 */

#pragma once

// Same structure with different names so the call Error() with warning symbol produces an error

struct TDiagDefErr
{
  const char *  strid;
  const char *  text;
};

#define DEF_DQ_ERR(asym, astrid, atxt) \
  inline constexpr TDiagDefErr asym { astrid, atxt }

struct TDiagDefWarn
{
  const char *  strid;
  const char *  text;
};

#define DEF_DQ_WARN(asym, astrid, atxt) \
  inline constexpr TDiagDefWarn asym { astrid, atxt }

struct TDiagDefHint
{
  const char *  strid;
  const char *  text;
};

#define DEF_DQ_HINT(asym, astrid, atxt) \
  inline constexpr TDiagDefWarn asym { astrid, atxt }

//-----------------------------------------------------------------------------
// ERRORS
//-----------------------------------------------------------------------------

DEF_DQ_ERR(DQERR_ID_EXP_AFTER,                     "EIdExpectedAfter",        "Identifier is expected after \"$1\"");
DEF_DQ_ERR(DQERR_SYM_EXPECTED,                     "ESymExpected",            "\"$1\" is expected");
DEF_DQ_ERR(DQERR_SYM_EXPECTED_AFTER,               "ESymExpectedAfter",       "\"$1\" is expected after $2");
DEF_DQ_ERR(DQERR_MISSING_COMMA,                    "EMissingComma",           "\",\" is missing");

DEF_DQ_ERR(DQERR_MODULE_STATEMENT_EXPECTED,        "EModStatementExpected",   "Module statement keyword expected");
DEF_DQ_ERR(DQERR_MODULE_STATEMENT_UNKNOWN,         "EModStatementUnknown",    "Unknown module statement \"$1\"");

DEF_DQ_ERR(DQERR_ATTR_NAME_EXPECTED,               "EAttrNameMissing",        "Attribute name expected after \"[[\"");
DEF_DQ_ERR(DQERR_ATTR_UNKNOWN,                     "EAttrUnknown",            "Unknown attribute \"$1\"");

DEF_DQ_ERR(DQERR_TYPE_SPECIFIER_EXPECTED,          "ETypeSpecExpected",       "Type specifier \":\" is expected");
DEF_DQ_ERR(DQERR_TYPE_SPECIFIER_EXP_AFTER,         "ETypeSpecExpAfter",       "Type specifier \":\" is expected after \"$1\"");
DEF_DQ_ERR(DQERR_TYPE_UNKNOWN,                     "ETypeUnknown",            "Type \"$1\" is unknown");
DEF_DQ_ERR(DQERR_TYPE_ID_EXP,                      "ETypeIdExpected",         "Type identifier is expected");
DEF_DQ_ERR(DQERR_TYPE_ALREADY_DEFINED_IN,          "ETypeAlreadyDef",         "Type \"$1\" is already defined in scope \"$2\"");

DEF_DQ_ERR(DQERR_VS_ALREADY_DEFINED_IN,            "EVsAlreadyDef",           "Symbol \"$1\" is already defined in scope \"$2\"");
DEF_DQ_ERR(DQERR_VAR_ALREADY_DECLARED_WITH_TYPE,   "EVarAlreadyDeclared",     "Variable \"$1\" is already declared with type \"$2\"");

DEF_DQ_ERR(DQERR_CSTR_SIZE_EXPECTED,               "ECstrSizeExpected",       "cstring size expected, example: cstring[n]");
DEF_DQ_ERR(DQERR_CSTR_SIZE_INVALID,                "ECstrSizeInvalid",        "Invalid cstring size, it must be a positive integer");

DEF_DQ_ERR(DQERR_VARARGS_NOT_ALLOWED,              "EVarargsNotAllowed",      "Variadic \"...\" is only allowed on [[external]] functions");
DEF_DQ_ERR(DQERR_VARARGS_ALONE,                    "EVarargsAlone",           "Variadic functions must have at least one named parameter before '...'");

DEF_DQ_ERR(DQERR_FUNCPAR_NAME_EXP,                 "EFuncParNameExpected",    "Function parameter name expected");
DEF_DQ_ERR(DQERR_FUNCPAR_NAME_INVALID,             "EFuncParNameInvalid",     "Invalid function parameter name \"$1\"");
DEF_DQ_ERR(DQERR_FUNC_RETTYPE_EXPECTED,            "EFuncRettypeExpected",    "Function return type identifier expected after \"->\"");
DEF_DQ_ERR(DQERR_FUNC_NO_BODY_ALLOWED_AFTER,       "EFuncNoBodyAllowed",      "\";\" is expected after $1");
DEF_DQ_ERR(DQERR_FUNC_RESULT_NOT_SET,              "EFuncResultNotSet",       "Function \"$1\" result is not set");



//-----------------------------------------------------------------------------
// WARNINGS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HINTS
//-----------------------------------------------------------------------------

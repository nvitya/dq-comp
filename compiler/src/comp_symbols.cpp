/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    comp_symbols.h
 * authors: nvitya
 * created: 2026-02-01
 * brief:   Compiler Symbol Objects
 */

#include <stdexcept>
#include "string.h"
#include <format>

#include "comp_symbols.h"
#include "dqc.h"

using namespace std;

OType * OScope::DefineType(OType * atype)
{
  auto found = typesyms.find(atype->name);
  if (found != typesyms.end())
  {
    g_compiler->Error(format("Type \"{}\" is already defined in scope \"{}\"", atype->name, this->debugname));
    return found->second;
  }

  typesyms[atype->name] = atype;
  return atype;
}

OValSym * OScope::DefineValSym(OValSym * avalsym)
{
  auto found = valsyms.find(avalsym->name);
  if (found != valsyms.end())
  {
    g_compiler->Error(format("\"{}\" is already defined in scope \"{}\"", avalsym->name, this->debugname));
    return found->second;
  }

  valsyms[avalsym->name] = avalsym;
  return avalsym;
}

OType * OScope::FindType(const string & name, OScope ** rscope, bool arecursive)
{
  auto it = typesyms.find(name);
  if (it != typesyms.end())
  {
    if (rscope)
    {
      *rscope = this;
    }
    return it->second;
  }

  // If not found here, check the parent scope
  if (arecursive and (parent_scope != nullptr))
  {
    return parent_scope->FindType(name, rscope);
  }

  return nullptr;
}

OValSym * OScope::FindValSym(const string & name, OScope ** rscope, bool arecursive)
{
  auto it = valsyms.find(name);
  if (it != valsyms.end())
  {
    if (rscope)
    {
      *rscope = this;
    }
    return it->second;
  }

  // If not found here, check the parent scope
  if (arecursive and (parent_scope != nullptr))
  {
    return parent_scope->FindValSym(name, rscope);
  }

  return nullptr;
}

OValSym * OType::CreateValSym(const string aname)
{
  OValSym * result = new OValSym(aname, this);
  return result;
}

void OValSymConst::SetInlineData(void * asrcdata, uint32_t alen)
{
  if (alen > sizeof(inlinedata))
  {
    throw length_error("OConstValSym: data too big for inline storage");
  }
  datalen = alen;
  dataptr = &inlinedata[0];

  if (asrcdata)
  {
    memcpy(&inlinedata[0], asrcdata, datalen);
  }
}

void OValSym::GenDeclaration(bool apublic)
{
  if (VSK_VARIABLE == kind)
  {
    LlLinkType  linktype =
      (apublic ? LlLinkType::ExternalLinkage
               : LlLinkType::InternalLinkage);

    LlType * ll_type = ptype->GetLlType();
    llvm::Constant * init_val = llvm::ConstantInt::get(ll_type, 0);

    auto * gv = new llvm::GlobalVariable(*ll_module, ll_type, false, linktype, init_val, name);
    //ll_globals[vs->name] = gv;
  }
}

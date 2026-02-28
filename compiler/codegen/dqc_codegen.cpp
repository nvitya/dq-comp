/*
 * Copyright (c) 2026 Viktor Nagy
 * This file is part of the DQ-Compiler project at https://github.com/nvitya/dq-comp
 *
 * This source code is licensed under the MIT License.
 * See the LICENSE file in the project root for the full license text.
 * ---------------------------------------------------------------------------------
 * file:    dqc_codegen.cpp
 * authors: nvitya
 * created: 2026-01-31
 * brief:
 */

// these include also provide llvm::format() so the std::format() must be fully specified
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/TargetParser/Host.h>

#include <print>
#include <format>

#include "dqc_codegen.h"

using namespace std;

void ODqCompCodegen::GenerateIr()
{
  print("Generating IR...\n");

  // generate declarations

  for (ODecl * decl : g_module->declarations)
  {
    if (DK_VALSYM == decl->kind)
    {
      OValSym * vs = decl->pvalsym;
      vs->GenDeclaration(decl->ispublic, decl->initvalue);
    }
    else if (DK_TYPE == decl->kind)
    {
      OType * pt = decl->ptype;
      print("Unhandled type declaration \"{}\"\n", pt->name);
    }
  }

  // generate function bodies

  for (ODecl * decl : g_module->declarations)
  {
    if (DK_VALSYM == decl->kind)
    {
      OValSym * vs = decl->pvalsym;
      OValSymFunc * vsfunc = dynamic_cast<OValSymFunc *>(vs);
      if (vsfunc)
      {
        vsfunc->GenerateFuncBody();
      }
    }
  }

  di_builder->finalize();
}

void ODqCompCodegen::EmitObject(const string afilename)
{
  print("Writing object file \"{}\"...\n", afilename);

  // Only initialize native target (not all targets)
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();

  auto triple = llvm::sys::getDefaultTargetTriple();
  ll_module->setTargetTriple(triple);

  string err;
  auto * target = llvm::TargetRegistry::lookupTarget(triple, err);
  if (!target) throw runtime_error(err);

  auto * machine = target->createTargetMachine(
      triple, "generic", "", llvm::TargetOptions(), llvm::Reloc::PIC_);

  ll_module->setDataLayout(machine->createDataLayout());

  error_code ec;
  llvm::raw_fd_ostream out(afilename, ec, llvm::sys::fs::OF_None);
  if (ec) throw runtime_error(ec.message());

  llvm::legacy::PassManager pm;
  machine->addPassesToEmitFile(pm, out, nullptr, llvm::CodeGenFileType::ObjectFile);
  pm.run(*ll_module);
  out.flush();
}

void ODqCompCodegen::PrintIr()
{
  print("=== LLVM IR ===\n");
  ll_module->print(llvm::outs(), nullptr);
  print("===============\n\n");
}


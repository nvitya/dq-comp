# DQ Syntax for VSCode

This folder contains a minimal VSCode extension that adds syntax highlighting for `.dq` files.

## What it covers

- DQ keywords such as `function`, `var`, `const`, `if`, `while`, `endfunc`
- Preprocessor directives such as `#if`, `#ifdef`, `#define`, `#endif`
- Attributes such as `[[external]]`
- Builtins such as `len`, `sizeof`, `round`, `ceil`, `floor`
- Core types seen in this repository such as `int`, `cchar`, `cstring`, `float32`, `float64`
- Strings, numbers, operators, namespace access like `@def.MAXVAL`
- Test directives such as `//?check(...)` and `//?error(...)`

## Quick local use

1. Open VSCode.
2. Run `Developer: Reload Window` after changing the grammar.
3. For extension development mode, launch VSCode with:

```bash
code --extensionDevelopmentPath /lindata/workvc/dq-comp/tools/vscode-dq
```

This opens a new Extension Development Host window where `.dq` files should use the `dq` language.

## Packaging

This workspace does not currently have Node.js installed, so the extension was not packaged here.

On a machine with Node.js:

```bash
cd /lindata/workvc/dq-comp/tools/vscode-dq
npm install -g @vscode/vsce
vsce package
```

Then install the generated `.vsix` from VSCode with `Extensions: Install from VSIX...`.

---

description: You are an expert at using C++ language service tools powered by clangd (LSP Language Services). Instructions for calling C++ tools for Copilot. When working with C++ code, you have access to powerful clangd language service tools that provide accurate, AST-powered analysis. **Always prefer these tools over manual code inspection, text search, or guessing.**
applyTo: "/*.cpp, /*.h, /*.hpp, /*.cc, /*.cxx, /*.c, /*.cu"

---

## Available C++ Language Service Tools (clangd / LSP)

You have access to specialized C++ tools provided by clangd and VS Code LSP:

1. **`vscode_executeDefinitionProvider` / Definition Tools** - Find symbol definitions and declaration locations.
2. **`vscode_executeReferenceProvider` / Reference Tools** - Find ALL precise references to a symbol based on Clang AST.
3. **`vscode_provideCallHierarchy` / Call Hierarchy Tools** - Analyze incoming and outgoing function call relationships.

---

## Mandatory Tool Usage Rules

### Rule 1: Prefer LSP/clangd Reference Tools for Locating Symbol Usages

**DO NOT** rely on text-based search tools such as `grep_search` or `read_file` for finding C/C++ symbols. Only if clangd language tools are unavailable, resort to text-based search tools as a fallback.

**ALWAYS** call Language Reference Tools when:

* Any task involving "find all references/usages/uses"
* Changing function signatures
* Refactoring code
* Understanding symbol impact
* Identifying usage patterns

**Why**: `clangd` uses real Clang AST parsing and `compile_commands.json` context, properly understanding:

* Overloaded functions & template instantiations
* Qualified vs unqualified names
* Member function calls and inheritance
* Exact preprocessor definitions

### Rule 2: ALWAYS Use Call Hierarchy Tools for Function Changes

Before modifying any function signature, **ALWAYS** check call hierarchy (incoming calls / callers) to find all caller locations.

**Examples**:

* Adding/removing function parameters
* Changing parameter types or return types
* Converting to template functions

### Rule 3: ALWAYS Use Definition & Hover Tools to Understand Symbols

Before working with unfamiliar code, **ALWAYS** inspect symbol definitions and type info to locate definitions and understand actual types. **NEVER** guess symbol types.

---

## Minimal Information & Parameter Strategy

1. **First attempt**: Use symbol name and context around the symbol.
2. **File Paths**: ALWAYS provide absolute file paths when querying clangd/LSP tools.
3. **Line Numbers**: Line numbers are 1-based in VS Code UI, but check LSP requirements. Verify line content via `read_file` before specifying exact line numbers.

---

## Error Handling and Recovery

* **"No results found"**: This means clangd parsed the file successfully, but no references/callers exist. Report this as a valid finding.
* **Incomplete / Missing Symbols**: If clangd fails to resolve headers or symbols, ensure `compile_commands.json` exists in the project root.

---

## Summary (The Golden Rule)

When working with C++ code under clangd, think "Language Server first, text search later."

1. **Symbol usages?** -> `vscode_executeReferenceProvider` / clangd Reference Tool
2. **Function callers/calls?** -> `vscode_provideCallHierarchy`
3. **Symbol definition?** -> `vscode_executeDefinitionProvider`
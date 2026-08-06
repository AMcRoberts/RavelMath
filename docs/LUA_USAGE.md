# Lua usage

Lua configures proof campaigns. It does not contain proof scripts.

## Responsibilities

Lua may:

- enable or disable reflection;
- select goals;
- select installed policies or pivots;
- provide bounded configuration values; and
- request reports or artifacts.

Lua may not:

- define determinant algorithms;
- contain Lean proof text;
- encode completed theorem case trees; or
- name an operation that has no installed executor.

## Main location

```text
lua/lua_src/ravel/proof/
```

The universal-n activation is:

```text
lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua
```

## Execution contract

A declaration is accepted only when every requested campaign operation is installed and executable. Unsupported requests fail before proof rendering.

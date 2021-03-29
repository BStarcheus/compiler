# compiler

Compiler project for EECE 5183 at UC.

I chose to use C++ because I wanted to get more experience with the language.
Before this, I had only used it in two previous UC courses, but not at all in work or personal projects.

## Requirements
- C++11
- make
- clang
- LLVM 11 (llvm-config in system path)

## Usage
```
Usage: compiler file [options]
Options:
  -h | --help        Show help
  -d                 Show debug for all components
  --debug-scanner    Show debug for scanner
  --debug-parser     Show debug for parser
  --debug-symtab     Show symbol table on each scope exit
  --debug-codegen    Output LLVM IR in out.ll file
```

### Build
Compile the compiler:
- `make`

### Standard Use
Run the compiler and generate the executable all in one go:
- `./compiler.sh <file> <options>`

And run the executable `./a.out`

### Other Use
Only run the compiler, which generates the out.s assembly file:
- `./compiler <file> <options>`
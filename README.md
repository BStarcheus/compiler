# compiler

Compiler project for EECE 5183 at UC.

I chose to use C++ because I wanted to get more experience with the language.
Before this, I had only used it in two previous UC courses, but not at all in work or personal projects.

## Requirements
- c++11 std library
- make
- clang / clang++
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


## Tests
The test folder contains both correct and incorrect source code files. 
You can run `make test` to confirm that all the correct files successfully parse and compile.  

The incorrect folder also contains two resync test cases.
They do correctly compile and run, but since they have invalid code I kept it in the incorrect folder.
The resync capabilities are very limited, but do work for those test cases.


## Next Steps
There are several improvements that could be made but probably won't due to time constraints.
- Track whether a variable is initialized. Don't allow references before inialization.
- Handle any remaining memory leaks
- Refactor some repetitive code (which was only kept because it allowed more precise error logging)
- Improve the resynchronization point system
- Improve usefulness of some log messages
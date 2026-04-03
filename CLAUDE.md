# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Documentation-only repository covering all 23 Gang of Four (GoF) design patterns with modern C++ examples. There is no build system, test suite, or runnable project — the content lives in `design_patterns.md` (the main reference) and `README.md`.

## Compiling Examples

Code examples are inline in `design_patterns.md` and meant to be extracted and compiled individually:

```bash
g++ -std=c++11 -Wall -Wextra -o example example.cpp
```

Requires a C++11 compiler (GCC, Clang, or MSVC). Do NOT use C++14/17/20 features.

## Architecture

- `design_patterns.md` — Single-file reference containing all 23 patterns organized into three categories: Creational (5), Structural (7), Behavioral (11). Each pattern has a definition, real-world analogy, C++ implementation, usage example, and bad-usage example.
- `README.md` — Project overview with learning path and pattern selection guide.

## C++ Style

Examples use modern C++ idioms: smart pointers, RAII, move semantics, `const` correctness, `std::mutex` for thread safety where relevant. Follow the same style when adding or modifying examples.

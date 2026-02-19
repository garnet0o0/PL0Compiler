 PL/1 Compiler Project | PL/1 编译器项目
1. Project Overview | 项目简介
This project is a complete compiler for the PL/1 language (an extended version of PL/0), developed as part of the "Compiling Principles" course. It covers the entire compilation pipeline, from lexical analysis to intermediate code generation and virtual machine interpretation.
本项目是一个针对 PL/1 语言（PL/0 的扩展版本）的完整编译器，作为《编译原理》课程实验开发。它涵盖了从词法分析到中间代码生成，再到虚拟机解释执行的完整编译流程。

2. Key Features | 核心特性
Lexical Analysis (Lexer): Supports identifiers, integers, and real numbers (e.g., -3.1415, 5.2e5). Includes support for single-line (//) and multi-line (/* */) comments. 词法分析 (Lexer): 支持标识符、整数及实数（如 -3.1415, 5.2e5）。支持单行（//）和多行（/* */）注释处理。
+2

Syntax Analysis (Parser): Implements Recursive Descent LL(1) parsing. Features structured error reporting with line numbers. 语法分析 (Parser): 采用递归下降 LL(1) 分析法。具备带行号的结构化错误报告功能。
+1

Code Generation: Translates source code into a stack-based Intermediate P-code. 代码生成: 将源代码翻译为基于栈的中间 P-code 指令。
+1

Virtual Machine (Interpreter): A stack-based VM that interprets and executes the generated P-code. 虚拟机 (Interpreter): 一个基于栈的虚拟机，用于解释并执行生成的 P-code。

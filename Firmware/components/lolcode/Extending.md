# Extending LULZCODE

## IMPORTANT

First step is to determine if extending with a statement or expression.

Statement: Basic function call, return value is not useful. Should have a side effect on program execution.

Expression: Similar to a statement, however, it can be nested in other expressions or statements. Use if
you're expecting to make use of the return value.

## parser.h
 * Statement: Add ST_xyz at same position as interpeter function in stmtjumptable
 * Expression: Added ET_xyz at same position as in interpreter function in exprjumptable

## tokenizer.h
 * Add TT_xyz enumeration
 * Add string defining token at same position

## parser.c
 * Statement: Add else if to parseStmtNode() to call parse function
 * Expression Add else if to parseExprNode() to call parse function
 * Add ST_xyz case to deleteStmtNode() to call delete function
 * Create createXYZStmtNode()
 * Create parse and delete functions

## interpreter.h
 * Declare interpretXYZStmtNode() function

## interpreter.c
 * Create interpretXYZStmtNode()
 * Add interpretXYZStmtNode to StmtJumpTable (and increment array size)

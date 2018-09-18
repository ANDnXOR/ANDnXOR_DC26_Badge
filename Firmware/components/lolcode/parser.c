#include "esp_log.h"
#include "parser.h"

#include "system.h"

static const char *TAG = "LULZ::Parser";

#ifdef DEBUG
static unsigned int shiftwidth = 0;
void shiftout(void) { shiftwidth += 4; }
void shiftin(void) { shiftwidth -= 4; }
void debug(const char *info)
{
	int n;
	for (n = 0; n < shiftwidth; n++)
		fprintf(stderr, " ");
	fprintf(stderr, "%s\n", info);
}
#endif

/**
 * Creates the main code block of a program.
 *
 * \param [in] block The first code block to execute.
 *
 * \return A pointer to the main code block with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
MainNode *createMainNode(BlockNode *block)
{
	MainNode *p = util_heap_alloc_ext(sizeof(MainNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->block = block;
	return p;
}

/**
 * Deletes the main code block of a program.
 *
 * \param [in,out] node The main code block to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteMainNode(MainNode *node)
{
	if (!node)
		return;
	deleteBlockNode(node->block);
	free(node);
}

/**
 * Creates a code block.
 *
 * \param [in] stmts The list of statements which comprise the code block.
 *
 * \return A pointer to the code block with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
BlockNode *createBlockNode(StmtNodeList *stmts)
{
	BlockNode *p = util_heap_alloc_ext(sizeof(BlockNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->stmts = stmts;
	return p;
}

/**
 * Deletes a code block.
 *
 * \param [in,out] node The code block to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteBlockNode(BlockNode *node)
{
	if (!node)
		return;
	deleteStmtNodeList(node->stmts);
	free(node);
}

/**
 * Creates an empty code block list.
 *
 * \return A pointer to an empty code block list.
 *
 * \retval NULL Memory allocation failed.
 */
BlockNodeList *createBlockNodeList(void)
{
	BlockNodeList *p = util_heap_alloc_ext(sizeof(BlockNodeList));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->num = 0;
	p->blocks = NULL;
	return p;
}

/**
 * Adds a code block to a list.
 *
 * \param [in,out] list The code block list to add \a node to.
 *
 * \param [in] node The code block to add to \a list.
 *
 * \post \a node will be added to \a list and its size will be updated.
 *
 * \retval 0 Memory allocation failed.
 *
 * \retval 1 \a node was added to \a list.
 */
int addBlockNode(BlockNodeList *list,
								 BlockNode *node)
{
	unsigned int newsize = list->num + 1;
	void *mem = util_heap_realloc_ext(list->blocks, sizeof(BlockNode *) * newsize);
	if (!mem)
	{
		perror("realloc addBlockNode()");
		return 0;
	}
	list->blocks = mem;
	list->blocks[list->num] = node;
	list->num = newsize;
	return 1;
}

/**
 * Deletes a code block list.
 *
 * \param [in,out] list The code block list to delete.
 *
 * \post The memory at \a list and all of its members will be freed.
 */
void deleteBlockNodeList(BlockNodeList *list)
{
	unsigned int n;
	if (!list)
		return;
	for (n = 0; n < list->num; n++)
		deleteBlockNode(list->blocks[n]);
	free(list->blocks);
	free(list);
}

/**
 * Creates a boolean constant.
 *
 * \param [in] data The boolean constant value.
 *
 * \return A pointer to a boolean constant which will be FALSE if \a data is \c
 * 0 and TRUE otherwise.
 *
 * \retval NULL Memory allocation failed.
 */
ConstantNode *createBooleanConstantNode(int data)
{
	ConstantNode *p = util_heap_alloc_ext(sizeof(ConstantNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = CT_BOOLEAN;
	p->data.i = (data != 0);
	return p;
}

/**
 * Creates an integer constant.
 *
 * \param [in] data The integer constant value.
 *
 * \return A pointer to an integer constant whose value is \a data.
 *
 * \retval NULL Memory allocation failed.
 */
ConstantNode *createIntegerConstantNode(long long data)
{
	ConstantNode *p = util_heap_alloc_ext(sizeof(ConstantNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = CT_INTEGER;
	p->data.i = data;
	return p;
}

/**
 * Creates a float constant.
 *
 * \param [in] data The float constant value.
 *
 * \return A pointer to a float constant whose value is \a data.
 *
 * \retval NULL Memory allocation failed.
 */
ConstantNode *createFloatConstantNode(float data)
{
	ConstantNode *p = util_heap_alloc_ext(sizeof(ConstantNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = CT_FLOAT;
	p->data.f = data;
	return p;
}

/**
 * Creates a string constant.
 *
 * \param [in] data The string constant value.
 *
 * \return A pointer to the string constant whose value is \a data.
 *
 * \retval NULL Memory allocation failed.
 */
ConstantNode *createStringConstantNode(char *data)
{
	ConstantNode *p = util_heap_alloc_ext(sizeof(ConstantNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = CT_STRING;
	p->data.s = data;
	return p;
}

/**
 * Deletes a constant.
 *
 * \param [in,out] node The constant to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteConstantNode(ConstantNode *node)
{
	if (!node)
		return;
	if (node->type == CT_STRING)
		free(node->data.s);
	free(node);
}

/**
 * Creates an indentifier.
 *
 * \param [in] type The type of the identifier \a id.
 *
 * \param [in] id The identifier data.
 *
 * \param [in] slot An optional slot to index.
 *
 * \param [in] fname The file containing the identifier.
 *
 * \param [in] line The line the identifier occurred on.
 *
 * \return A pointer to the identifier with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
IdentifierNode *createIdentifierNode(IdentifierType type,
																		 void *id,
																		 IdentifierNode *slot,
																		 const char *fname,
																		 unsigned int line)
{
	IdentifierNode *p = util_heap_alloc_ext(sizeof(IdentifierNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = type;
	p->id = id;
	p->slot = slot;
	if (fname)
	{
		p->fname = util_heap_alloc_ext(sizeof(char) * (strlen(fname) + 1));
		strcpy(p->fname, fname);
	}
	else
	{
		p->fname = NULL;
	}
	p->line = line;
	return p;
}

/**
 * Deletes an identifier.
 *
 * \param [in,out] node The identifier to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteIdentifierNode(IdentifierNode *node)
{
	if (!node)
		return;
	switch (node->type)
	{
	case IT_DIRECT:
	{
		free(node->id);
		break;
	}
	case IT_INDIRECT:
	{
		deleteExprNode(node->id);
		break;
	}
	default:
		lulz_error(PR_UNKNOWN_IDENTIFIER_TYPE, node->fname, node->line);
		break;
	}
	if (node->slot)
		deleteIdentifierNode(node->slot);
	if (node->fname)
		free(node->fname);
	free(node);
}

/**
 * Creates an identifier list.
 *
 * \return A pointer to an identifier list.
 *
 * \retval NULL Memory allocation failed.
 */
IdentifierNodeList *createIdentifierNodeList(void)
{
	IdentifierNodeList *p = util_heap_alloc_ext(sizeof(IdentifierNodeList));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->num = 0;
	p->ids = NULL;
	return p;
}

/**
 * Adds an identifier to a list.
 *
 * \param [in,out] list The list of identifiers to add \a node to.
 *
 * \param [in] node The identifier to add to \a list.
 *
 * \post \a token will be added to the end of \a list and the size of \a list
 * will be updated.
 *
 * \retval 0 Memory allocation failed.
 *
 * \retval 1 \a node was added to \a list.
 */
int addIdentifierNode(IdentifierNodeList *list,
											IdentifierNode *node)
{
	unsigned int newsize = list->num + 1;
	void *mem = util_heap_realloc_ext(list->ids, sizeof(IdentifierNode *) * newsize);
	if (!mem)
	{
		perror("realloc addIdentifierNode()");
		return 0;
	}
	list->ids = mem;
	list->ids[list->num] = node;
	list->num = newsize;
	return 1;
}

/**
 * Deletes an identifier list.
 *
 * \param [in,out] list The list of identifiers to delete.
 *
 * \post The memory at \a list and all of its members will be freed.
 */
void deleteIdentifierNodeList(IdentifierNodeList *list)
{
	unsigned int n;
	if (!list)
		return;
	for (n = 0; n < list->num; n++)
		deleteIdentifierNode(list->ids[n]);
	free(list->ids);
	free(list);
}

/**
 * Creates a type.
 *
 * \param [in] type The type to create.
 *
 * \return A pointer to a new type with the desired properties.
 *
 * \retval NULL Memory allocatin failed.
 */
TypeNode *createTypeNode(ConstantType type)
{
	TypeNode *p = util_heap_alloc_ext(sizeof(TypeNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = type;
	return p;
}

/**
 * Deletes a type.
 *
 * \param [in,out] node The type to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteTypeNode(TypeNode *node)
{
	if (!node)
		return;
	free(node);
}

/**
 * Creates a statement.
 *
 * \param [in] type The type of statement.
 *
 * \param [in] stmt The statement data.
 *
 * \return A pointer to a statement with the desired properties.
 *
 * \retval NULL util_heap_alloc_ext was unable to allocate memory.
 */
StmtNode *createStmtNode(StmtType type,
												 void *stmt)
{
	StmtNode *p = util_heap_alloc_ext(sizeof(StmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = type;
	p->stmt = stmt;
	return p;
}

/**
 * Deletes a statement.
 *
 * \param [in,out] node The statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteStmtNode(StmtNode *node)
{
	if (!node)
		return;
	switch (node->type)
	{
	case ST_CAST:
	{
		CastStmtNode *stmt = (CastStmtNode *)node->stmt;
		deleteCastStmtNode(stmt);
		break;
	}
	case ST_PRINT:
	{
		PrintStmtNode *stmt = (PrintStmtNode *)node->stmt;
		deletePrintStmtNode(stmt);
		break;
	}
	case ST_INPUT:
	{
		InputStmtNode *stmt = (InputStmtNode *)node->stmt;
		deleteInputStmtNode(stmt);
		break;
	}
	case ST_ASSIGNMENT:
	{
		AssignmentStmtNode *stmt = (AssignmentStmtNode *)node->stmt;
		deleteAssignmentStmtNode(stmt);
		break;
	}
	case ST_DECLARATION:
	{
		DeclarationStmtNode *stmt = (DeclarationStmtNode *)node->stmt;
		deleteDeclarationStmtNode(stmt);
		break;
	}
	case ST_IFTHENELSE:
	{
		IfThenElseStmtNode *stmt = (IfThenElseStmtNode *)node->stmt;
		deleteIfThenElseStmtNode(stmt);
		break;
	}
	case ST_SWITCH:
	{
		SwitchStmtNode *stmt = (SwitchStmtNode *)node->stmt;
		deleteSwitchStmtNode(stmt);
		break;
	}
	case ST_BREAK:
		break; /* This statement type does not have any content */
	case ST_RETURN:
	{
		ReturnStmtNode *stmt = (ReturnStmtNode *)node->stmt;
		deleteReturnStmtNode(stmt);
		break;
	}
	case ST_LOOP:
	{
		LoopStmtNode *stmt = (LoopStmtNode *)node->stmt;
		deleteLoopStmtNode(stmt);
		break;
	}
	case ST_DEALLOCATION:
	{
		DeallocationStmtNode *stmt = (DeallocationStmtNode *)node->stmt;
		deleteDeallocationStmtNode(stmt);
		break;
	}
	case ST_FUNCDEF:
	{
		FuncDefStmtNode *stmt = (FuncDefStmtNode *)node->stmt;
		deleteFuncDefStmtNode(stmt);
		break;
	}
	case ST_EXPR:
	{
		ExprNode *expr = (ExprNode *)node->stmt;
		deleteExprNode(expr);
		break;
	}
	case ST_ALTARRAYDEF:
	{
		AltArrayDefStmtNode *stmt = (AltArrayDefStmtNode *)node->stmt;
		deleteAltArrayDefStmtNode(stmt);
		break;
	}

		/******************************************************************************
		 * LULZCODE Additions
		 *****************************************************************************/
	case ST_BITMAP:
	case ST_BROADCAST:
	case ST_BUTTON_CLEAR:
	case ST_CHIP8:
	case ST_CIRCLE:
	case ST_CIRCLE_FILLED:
	case ST_COLOR_SET:
	case ST_CURSOR:
	case ST_CURSOR_AREA:
	case ST_DELAY:
	case ST_FILL:
	case ST_FONT_SET:
	case ST_GLOBAL_SET:
	case ST_I2C_WRITE:
	case ST_LED_EYE:
	case ST_LED_PUSH:
	case ST_LED_SET:
	case ST_LED_SET_ALL:
	case ST_LINE:
	case ST_PIXEL:
	case ST_PLAY_BLING:
	case ST_PRINTSCREEN:
	case ST_PUSHBUFFER:
	case ST_RECT:
	case ST_RECT_FILLED:
	case ST_RUN:
	case ST_STATE_SET:
	case ST_SYSTEM_CALL:;
		lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;
		lulz_delete_stmt_node(stmt);
		break;

	default:
		lulz_error(PR_UNKNOWN_STATEMENT_TYPE);
		break;
	}
	free(node);
}

/**
 * Creates an empty statement list.
 *
 * \return A pointer to an empty statement list.
 *
 * \retval NULL Memory allocation failed.
 */
StmtNodeList *createStmtNodeList(void)
{
	StmtNodeList *p = util_heap_alloc_ext(sizeof(StmtNodeList));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->num = 0;
	p->stmts = NULL;
	return p;
}

/**
 * Adds a statement to a list.
 *
 * \param [in,out] list The statement list to add \a node to.
 *
 * \param [in] node The statement to add to \a list.
 *
 * \post \a node will be added to \a list and its size will be updated.
 *
 * \retval 0 Memory allocation failed.
 *
 * \retval 1 \a node was added to \a list.
 */
int addStmtNode(StmtNodeList *list,
								StmtNode *node)
{
	unsigned int newsize = list->num + 1;
	void *mem = util_heap_realloc_ext(list->stmts, sizeof(StmtNode *) * newsize);
	if (!mem)
	{
		perror("realloc addStmtNode()");
		return 0;
	}
	list->stmts = mem;
	list->stmts[list->num] = node;
	list->num = newsize;
	return 1;
}

/**
 * Deletes a statement list.
 *
 * \param [in,out] list The statement list to delete.
 *
 * \post The memory at \a list and all of its members will be freed.
 */
void deleteStmtNodeList(StmtNodeList *list)
{
	unsigned int n;
	if (!list)
		return;
	for (n = 0; n < list->num; n++)
		deleteStmtNode(list->stmts[n]);
	free(list->stmts);
	free(list);
}

/**
 * Creates a cast statement.
 *
 * \param [in] target The variable to cast to \a newtype.
 *
 * \param [in] newtype The type to cast \a target to.
 *
 * \return A pointer to a cast statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
CastStmtNode *createCastStmtNode(IdentifierNode *target,
																 TypeNode *newtype)
{
	CastStmtNode *p = util_heap_alloc_ext(sizeof(CastStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->target = target;
	p->newtype = newtype;
	return p;
}

/**
 * Deletes a cast statement.
 *
 * \param [in,out] node The cast statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteCastStmtNode(CastStmtNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->target);
	deleteTypeNode(node->newtype);
	free(node);
}

/**
 * Creates a print statement.
 *
 * \param [in] args The expressions to print.
 *
 * \param [in] file Where to print (\c stdout or \c stderr).
 *
 * \param [in] nonl Whether an ending newline should be surpressed.
 *
 * \return A pointer to the print statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
PrintStmtNode *createPrintStmtNode(ExprNodeList *args,
																	 FILE *file,
																	 int nonl)
{
	PrintStmtNode *p = util_heap_alloc_ext(sizeof(PrintStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->args = args;
	p->file = file;
	p->nonl = nonl;
	return p;
}

/**
 * Deletes a print statement.
 *
 * \param [in,out] node The print statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deletePrintStmtNode(PrintStmtNode *node)
{
	if (!node)
		return;
	deleteExprNodeList(node->args);
	free(node);
}

/**
 * Creates an input statement.
 *
 * \param [in] target The variable to store input in.
 *
 * \return A pointer to an input statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
InputStmtNode *createInputStmtNode(IdentifierNode *target)
{
	InputStmtNode *p = util_heap_alloc_ext(sizeof(InputStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->target = target;
	return p;
}

/**
 * Deletes an input statement.
 *
 * \param [in,out] node The input statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteInputStmtNode(InputStmtNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->target);
	free(node);
}

/**
 * Creates an assignment statement.
 *
 * \param [in] target The variable to store \a expr in.
 *
 * \param [in] expr The expression to store in \a target.
 *
 * \return A pointer to an assignment statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
AssignmentStmtNode *createAssignmentStmtNode(IdentifierNode *target,
																						 ExprNode *expr)
{
	AssignmentStmtNode *p = util_heap_alloc_ext(sizeof(AssignmentStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->target = target;
	p->expr = expr;
	return p;
}

/**
 * Deletes an assignment statement.
 *
 * \param [in,out] node The assignment statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteAssignmentStmtNode(AssignmentStmtNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->target);
	deleteExprNode(node->expr);
	free(node);
}

/**
 * Creates a declaration statement.
 *
 * \param [in] scope The scope to create the variable in.
 *
 * \param [in] target The variable to create.
 *
 * \param [in] expr An optional expression to initialize \a target to.
 *
 * \param [in] type An optional type to initialize \a target to.
 *
 * \param [in] parent The optional parent to inherit from.
 *
 * \return A pointer to a declaration statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
DeclarationStmtNode *createDeclarationStmtNode(IdentifierNode *scope,
																							 IdentifierNode *target,
																							 ExprNode *expr,
																							 TypeNode *type,
																							 IdentifierNode *parent)
{
	DeclarationStmtNode *p = util_heap_alloc_ext(sizeof(DeclarationStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->scope = scope;
	p->target = target;
	p->expr = expr;
	p->type = type;
	p->parent = parent;
	return p;
}

/**
 * Deletes a declaration statement.
 *
 * \param [in,out] node The declaration statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteDeclarationStmtNode(DeclarationStmtNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->scope);
	deleteIdentifierNode(node->target);
	deleteExprNode(node->expr);
	deleteTypeNode(node->type);
	deleteIdentifierNode(node->parent);
	free(node);
}

/**
 * Creates an if/then/else statement.
 *
 * \param [in] yes The code block to execute if the \ref impvar "implicit
 * variable" is \c true.
 *
 * \param [in] no The code block to execute if the \ref impvar "implicit
 * variable" is \c false and all \a guards are \c false.
 *
 * \param [in] guards The expressions to test if the \ref impvar "implicit
 * variable" is \c false.
 *
 * \param [in] blocks The code blocks to execute if the respective guard is \c
 * true.
 *
 * \return A pointer to the if/then/else statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
IfThenElseStmtNode *createIfThenElseStmtNode(BlockNode *yes,
																						 BlockNode *no,
																						 ExprNodeList *guards,
																						 BlockNodeList *blocks)
{
	IfThenElseStmtNode *p = util_heap_alloc_ext(sizeof(IfThenElseStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->yes = yes;
	p->no = no;
	p->guards = guards;
	p->blocks = blocks;
	return p;
}

/**
 * Deletes an if/then/else statement.
 *
 * \param [in,out] node The if/then/else statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteIfThenElseStmtNode(IfThenElseStmtNode *node)
{
	if (!node)
		return;
	deleteBlockNode(node->yes);
	deleteBlockNode(node->no);
	deleteExprNodeList(node->guards);
	deleteBlockNodeList(node->blocks);
	free(node);
}

/**
 * Creates a switch statement.
 *
 * \param [in] guards The expressions to compare the the \ref impvar "implicit
 * variable".
 *
 * \param [in] blocks The code blocks to execute if a respective guard matches
 * the \ref impvar "implicit variable".
 *
 * \param [in] def The default code block to execute if none of the \a guards
 * match the \ref impvar "implicit variable".
 *
 * \return A pointer to a switch statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
SwitchStmtNode *createSwitchStmtNode(ExprNodeList *guards,
																		 BlockNodeList *blocks,
																		 BlockNode *def)
{
	SwitchStmtNode *p = util_heap_alloc_ext(sizeof(SwitchStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->guards = guards;
	p->blocks = blocks;
	p->def = def;
	return p;
}

/**
 * Deletes a switch statement.
 *
 * \param [in,out] node The switch statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteSwitchStmtNode(SwitchStmtNode *node)
{
	if (!node)
		return;
	deleteExprNodeList(node->guards);
	deleteBlockNodeList(node->blocks);
	deleteBlockNode(node->def);
	free(node);
}

/**
 * Creates a return statement.
 *
 * \param [in] value The return value.
 *
 * \return A pointer to a return statement of the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
ReturnStmtNode *createReturnStmtNode(ExprNode *value)
{
	ReturnStmtNode *p = util_heap_alloc_ext(sizeof(ReturnStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->value = value;
	return p;
}

/**
 * Deletes a return statement.
 *
 * \param [in,out] node The return statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteReturnStmtNode(ReturnStmtNode *node)
{
	if (!node)
		return;
	deleteExprNode(node->value);
	free(node);
}

/**
 * Creates a loop statement.
 *
 * \param [in] name The name of the loop.
 *
 * \param [in] var The induction variable.
 *
 * \param [in] guard The expression to determine if the loop continues.
 *
 * \param [in] update The expression to update \a var using.
 *
 * \param [in] body The code block to execute with each loop iteration.
 *
 * \return A pointer to a loop statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
LoopStmtNode *createLoopStmtNode(IdentifierNode *name,
																 IdentifierNode *var,
																 ExprNode *guard,
																 ExprNode *update,
																 BlockNode *body)
{
	LoopStmtNode *p = util_heap_alloc_ext(sizeof(LoopStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->name = name;
	p->var = var;
	p->guard = guard;
	p->update = update;
	p->body = body;
	return p;
}

/**
 * Deletes a loop statement.
 *
 * \param [in,out] node The loop statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteLoopStmtNode(LoopStmtNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->name);
	deleteIdentifierNode(node->var);
	deleteExprNode(node->guard);
	deleteExprNode(node->update);
	deleteBlockNode(node->body);
	free(node);
}

/**
 * Creates a deallocation statement.
 *
 * \param [in] target The variable to deallocate.
 *
 * \return A pointer to a deallocation statement with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
DeallocationStmtNode *createDeallocationStmtNode(IdentifierNode *target)
{
	DeallocationStmtNode *p = util_heap_alloc_ext(sizeof(DeallocationStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->target = target;
	return p;
}

/**
 * Deletes a deallocation statement.
 *
 * \param [in,out] node The deallocation statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteDeallocationStmtNode(DeallocationStmtNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->target);
	free(node);
}

/**
 * Creates a function definition statement.
 *
 * \param [in] scope The scope to define the function in.
 *
 * \param [in] name The name of the function.
 *
 * \param [in] args The function arguments names.
 *
 * \param [in] body The function's code block.
 *
 * \return A pointer to a function definition statement with the desired
 * properties.
 *
 * \retval NULL Memory allocation failed.
 */
FuncDefStmtNode *createFuncDefStmtNode(IdentifierNode *scope,
																			 IdentifierNode *name,
																			 IdentifierNodeList *args,
																			 BlockNode *body)
{
	FuncDefStmtNode *p = util_heap_alloc_ext(sizeof(FuncDefStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->scope = scope;
	p->name = name;
	p->args = args;
	p->body = body;
	return p;
}

/**
 * Deletes a function definition statement.
 *
 * \param [in,out] node The function definition statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteFuncDefStmtNode(FuncDefStmtNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->scope);
	deleteIdentifierNode(node->name);
	deleteIdentifierNodeList(node->args);
	deleteBlockNode(node->body);
	free(node);
}

/**
 * Creates an alternate array definition statement.
 *
 * \param [in] name The name of the array to define.
 *
 * \param [in] body The body of the array to define.
 *
 * \param [in] parent The optional parent to inherit from.
 *
 * \return A pointer to an array definition statement with the desired
 * properties.
 *
 * \retval NULL Memory allocation failed.
 */
AltArrayDefStmtNode *createAltArrayDefStmtNode(IdentifierNode *name,
																							 BlockNode *body,
																							 IdentifierNode *parent)
{
	AltArrayDefStmtNode *p = util_heap_alloc_ext(sizeof(AltArrayDefStmtNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->name = name;
	p->body = body;
	p->parent = parent;
	return p;
}

/**
 * Deletes an alternate array definition statement.
 *
 * \param [in,out] node The alternate array definition statement to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteAltArrayDefStmtNode(AltArrayDefStmtNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->name);
	deleteBlockNode(node->body);
	free(node);
}

/**
 * Creates an expression.
 *
 * \param [in] type The type of expression.
 *
 * \param [in] expr The expression data.
 *
 * \return A pointer to an expression with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
ExprNode *createExprNode(ExprType type,
												 void *expr)
{
	ExprNode *p = util_heap_alloc_ext(sizeof(ExprNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = type;
	p->expr = expr;
	return p;
}

/**
 * Deletes an expression.
 *
 * \param [in,out] node The expression to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteExprNode(ExprNode *node)
{
	if (!node)
		return;
	switch (node->type)
	{
	case ET_CAST:
		deleteCastExprNode((CastExprNode *)node->expr);
		break;
	case ET_CONSTANT:
		deleteConstantNode((ConstantNode *)node->expr);
		break;
	case ET_IDENTIFIER:
		deleteIdentifierNode((IdentifierNode *)node->expr);
		break;
	case ET_FUNCCALL:
		deleteFuncCallExprNode((FuncCallExprNode *)node->expr);
		break;
	case ET_OP:
		deleteOpExprNode((OpExprNode *)node->expr);
		break;
	case ET_IMPVAR:
		break; /* This expression type does not have any content */
	case ET_ACCEL_OUT:
	case ET_ACCEL_SIDE:
	case ET_ACCEL_UP:
	case ET_ACCEL_TILT:
	case ET_ARRAY_COUNT:
	case ET_BROADCAST_GET:
	case ET_BROADCAST_TX_GET:
	case ET_BUTTON_STATE:
	case ET_BUTTON_WAIT:
	case ET_BUTTON_UP:
	case ET_BUTTON_DOWN:
	case ET_BUTTON_LEFT:
	case ET_BUTTON_RIGHT:
	case ET_BUTTON_A:
	case ET_BUTTON_B:
	case ET_BUTTON_C:
	case ET_FILE_LIST:
	case ET_GLOBAL_GET:
	case ET_I2C_READ:
	case ET_TEXT_HEIGHT:
	case ET_TEXT_WIDTH:
	case ET_LCD_HEIGHT:
	case ET_LCD_WIDTH:
	case ET_MILLIS:
	case ET_PEERS:
	case ET_RANDOM:
	case ET_SERIAL:
	case ET_STATE_GET:
	case ET_HSV_TO_565:
	case ET_HSV_TO_RGB:
	case ET_INPUT:
	case ET_RGB_TO_565:
	case ET_FIRMWARE_VERSION:
	case ET_STRING_GET_AT_INDEX:
	case ET_STRING_SET_AT_INDEX:
	case ET_STRING_LENGTH:
	case ET_STRING_SUBSTRING:
	case ET_UNLOCK_GET:
	case ET_UNLOCK_VALIDATE:
		lulz_delete_expr_node(node->expr);
		break;
	default:
		ESP_LOGD(TAG, "Unable to delete unknown expression, type=%d", node->type);
		lulz_error(PR_UNKNOWN_EXPRESSION_TYPE);
		break;
	}
	free(node);
}

/**
 * Creates an empty expression list.
 *
 * \return A pointer to an empty expression list.
 *
 * \retval NULL Memory allocation failed.
 */
ExprNodeList *createExprNodeList(void)
{
	ExprNodeList *p = util_heap_alloc_ext(sizeof(ExprNodeList));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->num = 0;
	p->exprs = NULL;
	return p;
}

/**
 * Adds an expression to a list.
 *
 * \param [in,out] list The expression list to add \a node to.
 *
 * \param [in] node The expression to add to \a list.
 *
 * \post \a node will be added to \a list and its size will be updated.
 *
 * \retval 0 Memory allocation failed.
 *
 * \retval 1 \a node was added to \a list.
 */
int addExprNode(ExprNodeList *list,
								ExprNode *node)
{
	unsigned int newsize = list->num + 1;
	void *mem = util_heap_realloc_ext(list->exprs, sizeof(ExprNode *) * newsize);
	if (!mem)
	{
		perror("realloc addExprNode()");
		return 0;
	}
	list->exprs = mem;
	list->exprs[list->num] = node;
	list->num = newsize;
	return 1;
}

/**
 * Deletes an expression list.
 *
 * \param [in,out] list The expression list to delete.
 *
 * \post The memory at \a list and all of its members will be freed.
 */
void deleteExprNodeList(ExprNodeList *list)
{
	unsigned int n;
	if (!list)
		return;
	for (n = 0; n < list->num; n++)
		deleteExprNode(list->exprs[n]);
	free(list->exprs);
	free(list);
}

/**
 * Creates a cast expression.
 *
 * \param [in] target The expression to cast.
 *
 * \param [in] newtype The type to cast \a target to.
 *
 * \return A pointer to a cast expression with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
CastExprNode *createCastExprNode(ExprNode *target,
																 TypeNode *newtype)
{
	CastExprNode *p = util_heap_alloc_ext(sizeof(CastExprNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->target = target;
	p->newtype = newtype;
	return p;
}

/**
 * Deletes a cast expression.
 *
 * \param [in,out] node The cast expression to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteCastExprNode(CastExprNode *node)
{
	if (!node)
		return;
	deleteExprNode(node->target);
	deleteTypeNode(node->newtype);
	free(node);
}

/**
 * Creates a function call expression.
 *
 * \param [in] scope The scope to lookup the function in.
 *
 * \param [in] name The name of the function.
 *
 * \param [in] args The arguments to supply the function.
 *
 * \return A pointer to a function call expression with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
FuncCallExprNode *createFuncCallExprNode(IdentifierNode *scope,
																				 IdentifierNode *name,
																				 ExprNodeList *args)
{
	FuncCallExprNode *p = util_heap_alloc_ext(sizeof(FuncCallExprNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->scope = scope;
	p->name = name;
	p->args = args;
	return p;
}

/**
 * Deletes a function call expression.
 *
 * \param [in,out] node The function call expression to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteFuncCallExprNode(FuncCallExprNode *node)
{
	if (!node)
		return;
	deleteIdentifierNode(node->scope);
	deleteIdentifierNode(node->name);
	deleteExprNodeList(node->args);
	free(node);
}

/**
 * Creates an operation expression.
 *
 * \param [in] type The type of operation to perform.
 *
 * \param [in] args The operands.
 *
 * \return A pointer to an operation expression with the desired properties.
 *
 * \retval NULL Memroy allocation failed.
 */
OpExprNode *createOpExprNode(OpType type,
														 ExprNodeList *args)
{
	OpExprNode *p = util_heap_alloc_ext(sizeof(OpExprNode));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = type;
	p->args = args;
	return p;
}

/**
 * Deletes an operation expression.
 *
 * \param [in,out] node The operation expression to delete.
 *
 * \post The memory at \a node and all of its members will be freed.
 */
void deleteOpExprNode(OpExprNode *node)
{
	if (!node)
		return;
	deleteExprNodeList(node->args);
	free(node);
}

/**
 * Checks if a type of token is at a position in a token list, and if so,
 * advances the position.
 *
 * \param [in,out] tokenp The position in a token list to check.
 *
 * \param [in] token The type of token to check for.
 *
 * \post If the type of \a tokenp is not \a token, \a tokenp will remain
 * unchanged, else, it will point to the token after the one matched.
 *
 * \retval 0 The type of \a tokenp is \a token.
 *
 * \retval 1 The type of \a tokenp is not \a token.
 */
int acceptToken(Token ***tokenp,
								TokenType token)
{
	Token **tokens = *tokenp;
	if (!(*tokens))
		return 0;
	if ((*tokens)->type != token)
		return 0;
	tokens++;
	*tokenp = tokens;
	return 1;
}

/**
 * Checks if a type of token is at a position in a token list.
 *
 * \param [in] tokenp The position in a token list to check.
 *
 * \param [in] token The type of token to check for.
 *
 * \post \a tokenp will remain unchanged.
 *
 * \retval 0 The type of \a tokenp is \a token.
 *
 * \retval 1 The type of \a tokenp is not \a token.
 */
int peekToken(Token ***tokenp,
							TokenType token)
{
	Token **tokens = *tokenp;
	if ((*tokens)->type != token)
		return 0;
	return 1;
}

/**
 * Checks if a type of token is after a position in a token list.
 *
 * \param [in] tokenp The position in a token list to check after.
 *
 * \param [in] token The type of token to check for.
 *
 * \post \a tokenp will remain unchanged.
 *
 * \retval 0 The type of the token after \a tokenp is \a token.
 *
 * \retval 1 The type of the token after \a tokenp is not \a token.
 */
int nextToken(Token ***tokenp,
							TokenType token)
{
	Token **tokens = *tokenp;
	if ((*(tokens + 1))->type != token)
		return 0;
	return 1;
}

/**
 * A simple wrapper around the global error printing function tailored to
 * general parser errors.
 *
 * \param [in] type The type of error.
 *
 * \param [in] tokens The tokens being parsed when the error occurred.
 */
void parser_error(ErrorType type,
									Token **tokens)
{
	if (!(*tokens))
	{
		lulz_error(PR_UNHANDLED_STRING);
	}
	else
	{
		lulz_error(type, (*tokens)->fname, (*tokens)->line, (*tokens)->image);
	}
}

/**
 * A simple wrapper around the global error printing function tailored to
 * expected token parser errors.
 *
 * \param [in] token The expected token's image.
 *
 * \param [in] tokens The tokens being parsed when the error occurred.
 */
void parser_error_expected_token(TokenType token,
																 Token **tokens)
{
	lulz_error(PR_EXPECTED_TOKEN,
						 (*tokens)->fname,
						 (*tokens)->line,
						 lulzcode_keywords[token],
						 (*tokens)->image);
}

/**
 * A simple wrapper around the global error printing function tailored to
 * expected two token parser errors.
 *
 * \param [in] token1 The first expected token's image.
 *
 * \param [in] token2 The second expected token's image.
 *
 * \param [in] tokens The tokens being parsed when the error occurred.
 */
void parser_error_expected_either_token(TokenType token1,
																				TokenType token2,
																				Token **tokens)
{
	lulz_error(PR_EXPECTED_TOKEN,
						 (*tokens)->fname,
						 (*tokens)->line,
						 lulzcode_keywords[token1],
						 lulzcode_keywords[token2],
						 (*tokens)->image);
}

/**
 * Parses tokens into a constant.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a constant.
 *
 * \retval NULL Unable to parse.
 */
ConstantNode *parseConstantNode(Token ***tokenp)
{
	ConstantNode *ret = NULL;
	char *data = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	shiftout();
#endif

	/* Boolean */
	if (peekToken(&tokens, TT_BOOLEAN))
	{
#ifdef DEBUG
		debug("CT_BOOLEAN");
#endif
		/* Create the ConstantNode structure */
		ret = createBooleanConstantNode((*tokens)->data.i);
		if (!ret)
			goto parseConstantNodeAbort;

		/* This should succeed; it was checked for above */
		status = acceptToken(&tokens, TT_BOOLEAN);
		if (!status)
		{
			parser_error(PR_EXPECTED_BOOLEAN, tokens);
			goto parseConstantNodeAbort;
		}
	}
	/* Integer */
	else if (peekToken(&tokens, TT_INTEGER))
	{
#ifdef DEBUG
		debug("CT_INTEGER");
#endif
		/* Create the ConstantNode structure */
		ret = createIntegerConstantNode((*tokens)->data.i);
		if (!ret)
			goto parseConstantNodeAbort;

		/* This should succeed; it was checked for above */
		status = acceptToken(&tokens, TT_INTEGER);
		if (!status)
		{
			parser_error(PR_EXPECTED_INTEGER, tokens);
			goto parseConstantNodeAbort;
		}
	}
	/* Float */
	else if (peekToken(&tokens, TT_FLOAT))
	{
#ifdef DEBUG
		debug("CT_FLOAT");
#endif
		/* Create the ConstantNode structure */
		ret = createFloatConstantNode((*tokens)->data.f);
		if (!ret)
			goto parseConstantNodeAbort;

		/* This should succeed; it was checked for above */
		status = acceptToken(&tokens, TT_FLOAT);
		if (!status)
		{
			parser_error(PR_EXPECTED_FLOAT, tokens);
			goto parseConstantNodeAbort;
		}
	}
	/* String */
	else if (peekToken(&tokens, TT_STRING))
	{
		size_t len = strlen((*tokens)->image);
		data = util_heap_alloc_ext(sizeof(char) * (len - 1));
		if (!data)
		{
			perror("util_heap_alloc_ext");
			goto parseConstantNodeAbort;
		}
		strncpy(data, (*tokens)->image + 1, len - 2);
		data[len - 2] = '\0';
#ifdef DEBUG
		debug("CT_STRING");
#endif
		/* Create the ConstantNode structure */
		ret = createStringConstantNode(data);
		if (!ret)
			goto parseConstantNodeAbort;
		data = NULL;

		/* This should succeed; it was checked for above */
		status = acceptToken(&tokens, TT_STRING);
		if (!status)
		{
			parser_error(PR_EXPECTED_STRING, tokens);
			goto parseConstantNodeAbort;
		}
	}
	else
	{
		parser_error(PR_EXPECTED_CONSTANT, tokens);
	}

#ifdef DEBUG
	shiftin();
#endif

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseConstantNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (data)
		free(data);
	if (ret)
		deleteConstantNode(ret);

	return NULL;
}

/**
 * Parses tokens into a type.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a type.
 *
 * \retval NULL Unable to parse.
 */
TypeNode *parseTypeNode(Token ***tokenp)
{
	TypeNode *ret = NULL;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	shiftout();
#endif

	/* Nil */
	if (acceptToken(&tokens, TT_NOOB))
	{
#ifdef DEBUG
		debug("CT_NIL");
#endif
		ret = createTypeNode(CT_NIL);
		if (!ret)
			goto parseTypeNodeAbort;
	}
	/* Boolean */
	else if (acceptToken(&tokens, TT_TROOF))
	{
#ifdef DEBUG
		debug("CT_BOOLEAN");
#endif
		ret = createTypeNode(CT_BOOLEAN);
		if (!ret)
			goto parseTypeNodeAbort;
	}
	/* Integer */
	else if (acceptToken(&tokens, TT_NUMBR))
	{
#ifdef DEBUG
		debug("CT_INTEGER");
#endif
		ret = createTypeNode(CT_INTEGER);
		if (!ret)
			goto parseTypeNodeAbort;
	}
	/* Float */
	else if (acceptToken(&tokens, TT_NUMBAR))
	{
#ifdef DEBUG
		debug("CT_FLOAT");
#endif
		ret = createTypeNode(CT_FLOAT);
		if (!ret)
			goto parseTypeNodeAbort;
	}
	/* String */
	else if (acceptToken(&tokens, TT_YARN))
	{
#ifdef DEBUG
		debug("CT_STRING");
#endif
		ret = createTypeNode(CT_STRING);
		if (!ret)
			goto parseTypeNodeAbort;
	}
	/* Associative array */
	else if (acceptToken(&tokens, TT_BUKKIT))
	{
#ifdef DEBUG
		debug("CT_ARRAY");
#endif
		ret = createTypeNode(CT_ARRAY);
		if (!ret)
			goto parseTypeNodeAbort;
	}
	else if (acceptToken(&tokens, TT_LETR))
	{
		//LETR is just a single character string (AKA YARN)
		ret = createTypeNode(CT_CHAR);
		if (!ret)
			goto parseTypeNodeAbort;
	}
	//If we get this far it was an error
	else
	{
		parser_error(PR_EXPECTED_TYPE, tokens);
	}

#ifdef DEBUG
	shiftin();
#endif

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseTypeNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteTypeNode(ret);

	return NULL;
}

/**
 * Parses tokens into an identifier.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to an identifier.
 *
 * \retval NULL Unable to parse.
 */
IdentifierNode *parseIdentifierNode(Token ***tokenp)
{
	IdentifierType type = IT_DIRECT;
	void *data = NULL;
	IdentifierNode *slot = NULL;
	const char *fname = NULL;
	unsigned int line;

	/* For direct identifier */
	char *temp = NULL;

	/* For indirect identifier */
	ExprNode *expr = NULL;

	IdentifierNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	shiftout();
#endif

	fname = (*tokens)->fname;
	line = (*tokens)->line;

	/* Direct identifier */
	if (peekToken(&tokens, TT_IDENTIFIER))
	{
		type = IT_DIRECT;
#ifdef DEBUG
		debug("IT_DIRECT");
#endif
		/* Copy the token image */
		temp = util_heap_alloc_ext(sizeof(char) * (strlen((*tokens)->image) + 1));
		if (!temp)
			goto parseIdentifierNodeAbort;
		strcpy(temp, (*tokens)->image);
		data = temp;

		/* This should succeed; it was checked for above */
		status = acceptToken(&tokens, TT_IDENTIFIER);
		if (!status)
		{
			parser_error(PR_EXPECTED_IDENTIFIER, tokens);
			goto parseIdentifierNodeAbort;
		}
	}
	else if (acceptToken(&tokens, TT_SRS))
	{
		type = IT_INDIRECT;
#ifdef DEBUG
		debug("IT_INDIRECT");
#endif
		/* Parse the expression representing the identifier */
		expr = parseExprNode(&tokens);
		if (!expr)
			goto parseIdentifierNodeAbort;
		data = expr;
	}
	else
	{
		parser_error(PR_EXPECTED_IDENTIFIER, tokens);
	}

	/* Check if there is a slot access */
	if (acceptToken(&tokens, TT_APOSTROPHEZ))
	{
		slot = parseIdentifierNode(&tokens);
		if (!slot)
			goto parseIdentifierNodeAbort;
	}

#ifdef DEBUG
	shiftin();
#endif

	/* Create the new IdentifierNode structure */
	ret = createIdentifierNode(type, data, slot, fname, line);
	if (!ret)
		goto parseIdentifierNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseIdentifierNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteIdentifierNode(ret);
	else
	{
		/* For indirect identifier */
		if (expr)
			deleteExprNode(expr);

		/* For direct identifier */
		if (temp)
			free(temp);

		if (slot)
			deleteIdentifierNode(slot);
	}

	return NULL;
}

/**
 * Parses tokens into a cast expression.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a cast expression.
 *
 * \retval NULL Unable to parse.
 */
ExprNode *parseCastExprNode(Token ***tokenp)
{
	ExprNode *target = NULL;
	TypeNode *newtype = NULL;
	CastExprNode *expr = NULL;
	ExprNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ET_CAST");
#endif

	/* Parse the cast token */
	status = acceptToken(&tokens, TT_MAEK);
	if (!status)
	{
		parser_error_expected_token(TT_MAEK, tokens);
		goto parseCastExprNodeAbort;
	}

	/* Parse the expression to cast */
	target = parseExprNode(&tokens);
	if (!target)
		goto parseCastExprNodeAbort;

	/* Optionally parse the cast-to token */
	status = acceptToken(&tokens, TT_A);

	/* Parse the type to cast to */
	newtype = parseTypeNode(&tokens);
	if (!newtype)
		goto parseCastExprNodeAbort;

	/* Create the new CastExprNode structure */
	expr = createCastExprNode(target, newtype);
	if (!expr)
		goto parseCastExprNodeAbort;

	/* Create the new ExprNode structure */
	ret = createExprNode(ET_CAST, expr);
	if (!ret)
		goto parseCastExprNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseCastExprNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteExprNode(ret);
	else if (expr)
		deleteCastExprNode(expr);
	else
	{
		if (newtype)
			deleteTypeNode(newtype);
		if (target)
			deleteExprNode(target);
	}

	return NULL;
}

/**
 * Parses tokens into a constant expression.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a constant expression.
 *
 * \retval NULL Unable to parse.
 */
ExprNode *parseConstantExprNode(Token ***tokenp)
{
	ConstantNode *node = NULL;
	ExprNode *ret = NULL;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ET_CONSTANT");
#endif

	/* Parse the constant */
	node = parseConstantNode(&tokens);
	if (!node)
		goto parseConstantExprNodeAbort;

	/* Create the new ExprNode structure */
	ret = createExprNode(ET_CONSTANT, node);
	if (!ret)
		goto parseConstantExprNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseConstantExprNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteExprNode(ret);
	else
	{
		if (node)
			deleteConstantNode(node);
	}

	return NULL;
}

/**
 * Parses tokens into an identifier expression.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to an identifier expression.
 *
 * \retval NULL Unable to parse.
 */
ExprNode *parseIdentifierExprNode(Token ***tokenp)
{
	IdentifierNode *node = NULL;
	ExprNode *ret = NULL;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ET_IDENTIFIER");
#endif

	/* Parse the identifier node */
	node = parseIdentifierNode(&tokens);
	if (!node)
		goto parseIdentifierExprNodeAbort;

	/* Create the new ExprNode structure */
	ret = createExprNode(ET_IDENTIFIER, node);
	if (!ret)
		goto parseIdentifierExprNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseIdentifierExprNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteExprNode(ret);
	else
	{
		if (node)
			deleteIdentifierNode(node);
	}

	return NULL;
}

/**
 * Parses tokens into a function call expression.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a Tokenp will point to the next unparsed token.
 *
 * \return A pointer to a function call expression.
 *
 * \retval NULL Unable to parse.
 */
ExprNode *parseFuncCallExprNode(Token ***tokenp)
{
	IdentifierNode *scope = NULL;
	IdentifierNode *name = NULL;
	ExprNodeList *args = NULL;
	ExprNode *arg = NULL;
	FuncCallExprNode *expr = NULL;
	ExprNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ET_FUNCCALL");
#endif

	/* Parse the scope name */
	scope = parseIdentifierNode(&tokens);
	if (!scope)
		goto parseFuncCallExprNodeAbort;

	/* Parse the function call token */
	status = acceptToken(&tokens, TT_IZ);
	if (!status)
	{
		parser_error_expected_token(TT_IZ, tokens);
		goto parseFuncCallExprNodeAbort;
	}

	/* Parse the function name */
	name = parseIdentifierNode(&tokens);
	if (!name)
		goto parseFuncCallExprNodeAbort;

	/* Create an argument list */
	args = createExprNodeList();
	if (!args)
		goto parseFuncCallExprNodeAbort;

	/* Parse the first argument token */
	if (acceptToken(&tokens, TT_YR))
	{
		/* Parse the first argument */
		arg = parseExprNode(&tokens);
		if (!arg)
			goto parseFuncCallExprNodeAbort;

		/* Add the first argument to the argument list */
		status = addExprNode(args, arg);
		if (!status)
			goto parseFuncCallExprNodeAbort;
		arg = NULL;

		/* Parse additional arguments */
		while (acceptToken(&tokens, TT_ANYR))
		{
			/* Parse the argument */
			arg = parseExprNode(&tokens);
			if (!arg)
				goto parseFuncCallExprNodeAbort;

			/* Add the argument to the argument list */
			status = addExprNode(args, arg);
			if (!status)
				goto parseFuncCallExprNodeAbort;
			arg = NULL;
		}
	}

	/* Parse the end-of-argument token */
	status = acceptToken(&tokens, TT_MKAY);
	if (!status)
	{
		parser_error_expected_token(TT_MKAY, tokens);
		goto parseFuncCallExprNodeAbort;
	}

	/* Create the new FuncCallExprNode structure */
	expr = createFuncCallExprNode(scope, name, args);
	if (!expr)
		goto parseFuncCallExprNodeAbort;

	/* Create the new ExprNode structure */
	ret = createExprNode(ET_FUNCCALL, expr);
	if (!ret)
		goto parseFuncCallExprNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseFuncCallExprNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteExprNode(ret);
	else if (expr)
		deleteFuncCallExprNode(expr);
	else
	{
		if (args)
			deleteExprNodeList(args);
		if (arg)
			deleteExprNode(arg);
		if (name)
			deleteIdentifierNode(name);
		if (scope)
			deleteIdentifierNode(scope);
	}

	return NULL;
}

/**
 * Parses tokens into an operation expression.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a operation expression.
 *
 * \retval NULL Unable to parse.
 */
ExprNode *parseOpExprNode(Token ***tokenp)
{
	enum ArityType
	{
		AT_UNARY,
		AT_BINARY,
		AT_NARY
	};
	enum ArityType arity;
	ExprNodeList *args = NULL;
	ExprNode *arg = NULL;
	OpExprNode *expr = NULL;
	OpType type;
	ExprNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

	/* Unary operations */
	if (acceptToken(&tokens, TT_NOT))
	{
		type = OP_NOT;
		arity = AT_UNARY;
#ifdef DEBUG
		debug("ET_OP (OP_NOT)");
#endif
	}
	/* Binary operations */
	else if (acceptToken(&tokens, TT_SUMOF))
	{
		type = OP_ADD;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_ADD)");
#endif
	}
	else if (acceptToken(&tokens, TT_DIFFOF))
	{
		type = OP_SUB;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_SUB)");
#endif
	}
	else if (acceptToken(&tokens, TT_PRODUKTOF))
	{
		type = OP_MULT;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_MULT)");
#endif
	}
	else if (acceptToken(&tokens, TT_QUOSHUNTOF))
	{
		type = OP_DIV;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_DIV)");
#endif
	}
	else if (acceptToken(&tokens, TT_MODOF))
	{
		type = OP_MOD;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_MOD)");
#endif
	}
	else if (acceptToken(&tokens, TT_BIGGROF))
	{
		type = OP_MAX;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_MAX)");
#endif
	}
	else if (acceptToken(&tokens, TT_SMALLROF))
	{
		type = OP_MIN;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_MIN)");
#endif
	}
	else if (acceptToken(&tokens, TT_BOTHOF))
	{
		type = OP_AND;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_AND)");
#endif
	}
	else if (acceptToken(&tokens, TT_EITHEROF))
	{
		type = OP_OR;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_OR)");
#endif
	}
	else if (acceptToken(&tokens, TT_WONOF))
	{
		type = OP_XOR;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_XOR)");
#endif
	}
	else if (acceptToken(&tokens, TT_BOTHSAEM))
	{
		type = OP_EQ;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_EQ)");
#endif
	}
	else if (acceptToken(&tokens, TT_DIFFRINT))
	{
		type = OP_NEQ;
		arity = AT_BINARY;
#ifdef DEBUG
		debug("ET_OP (OP_NEQ)");
#endif
	}
	/* N-ary operators */
	else if (acceptToken(&tokens, TT_ALLOF))
	{
		type = OP_AND;
		arity = AT_NARY;
#ifdef DEBUG
		debug("ET_OP (OP_AND)");
#endif
	}
	else if (acceptToken(&tokens, TT_ANYOF))
	{
		type = OP_OR;
		arity = AT_NARY;
#ifdef DEBUG
		debug("ET_OP (OP_OR)");
#endif
	}
	else if (acceptToken(&tokens, TT_SMOOSH))
	{
		type = OP_CAT;
		arity = AT_NARY;
#ifdef DEBUG
		debug("ET_OP (OP_CAT)");
#endif
	}
	else
	{
		parser_error(PR_INVALID_OPERATOR, tokens);
		return NULL;
	}

	/* Create the argument list */
	args = createExprNodeList();
	if (!args)
		goto parseOpExprNodeAbort;

	/* Parse the operation arguments */
	if (arity == AT_UNARY)
	{
		/* Parse the argument */
		arg = parseExprNode(&tokens);
		if (!arg)
			goto parseOpExprNodeAbort;

		/* Add the argument to the argument list */
		status = addExprNode(args, arg);
		if (!status)
			goto parseOpExprNodeAbort;
		arg = NULL;
	}
	else if (arity == AT_BINARY)
	{
		/* Parse the first argument */
		arg = parseExprNode(&tokens);
		if (!arg)
			goto parseOpExprNodeAbort;

		/* Add the first argument to the argument list */
		status = addExprNode(args, arg);
		if (!status)
			goto parseOpExprNodeAbort;
		arg = NULL;

		/* Optionally parse the argument-separator token */
		status = acceptToken(&tokens, TT_AN);

		/* Parse the second argument */
		arg = parseExprNode(&tokens);
		if (!arg)
			goto parseOpExprNodeAbort;

		/* Add the second argument to the argument list */
		status = addExprNode(args, arg);
		if (!status)
			goto parseOpExprNodeAbort;
		arg = NULL;
	}
	else if (arity == AT_NARY)
	{
		/* Parse as many arguments as possible */
		while (1)
		{
			/* Parse an argument */
			arg = parseExprNode(&tokens);
			if (!arg)
				goto parseOpExprNodeAbort;

			/* Add the argument to the argument list */
			status = addExprNode(args, arg);
			if (!status)
				goto parseOpExprNodeAbort;
			arg = NULL;

			/* Stop if the end-of-arguments token is present */
			if (acceptToken(&tokens, TT_MKAY))
				break;

			/* Optionally parse the argument-separator token */
			status = acceptToken(&tokens, TT_AN);
		}
	}

	/* Create the new OpExprNode structure */
	expr = createOpExprNode(type, args);
	if (!expr)
		goto parseOpExprNodeAbort;

	/* Create the new ExprNode structure */
	ret = createExprNode(ET_OP, expr);
	if (!ret)
		goto parseOpExprNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseOpExprNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteExprNode(ret);
	else if (expr)
		deleteOpExprNode(expr);
	else
	{
		if (arg)
			deleteExprNode(arg);
		if (args)
			deleteExprNodeList(args);
	}

	return NULL;
}

/**
 * Parses tokens into an expression.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to an expression.
 *
 * \retval NULL Unable to parse.
 */
ExprNode *parseExprNode(Token ***tokenp)
{
	Token **tokens = *tokenp;
	ExprNode *ret = NULL;

#ifdef DEBUG
	shiftout();
#endif

	/* Parse context-sensitive expressions */
	if (peekToken(&tokens, TT_IDENTIFIER) || peekToken(&tokens, TT_SRS))
	{
		IdentifierNode *id = NULL;

		/* Remove the identifier from the token stream */
		id = parseIdentifierNode(&tokens);
		if (!id)
			return NULL;

		/* We do not need to hold onto it */
		deleteIdentifierNode(id);

		/* Function call (must come before identifier) */
		if (peekToken(&tokens, TT_IZ))
		{
			ret = parseFuncCallExprNode(tokenp);
		}
		/* Identifier */
		else
		{
			ret = parseIdentifierExprNode(tokenp);
		}
	}
	/* Cast */
	else if (peekToken(&tokens, TT_MAEK))
	{
		ret = parseCastExprNode(tokenp);
	}
	/* Constant value */
	else if (peekToken(&tokens, TT_BOOLEAN) || peekToken(&tokens, TT_INTEGER) || peekToken(&tokens, TT_FLOAT) || peekToken(&tokens, TT_STRING))
		ret = parseConstantExprNode(tokenp);
	/* Operation */
	else if (peekToken(&tokens, TT_SUMOF) || peekToken(&tokens, TT_DIFFOF) || peekToken(&tokens, TT_PRODUKTOF) || peekToken(&tokens, TT_QUOSHUNTOF) || peekToken(&tokens, TT_MODOF) || peekToken(&tokens, TT_BIGGROF) || peekToken(&tokens, TT_SMALLROF) || peekToken(&tokens, TT_BOTHOF) || peekToken(&tokens, TT_EITHEROF) || peekToken(&tokens, TT_WONOF) || peekToken(&tokens, TT_BOTHSAEM) || peekToken(&tokens, TT_DIFFRINT) || peekToken(&tokens, TT_ANYOF) || peekToken(&tokens, TT_ALLOF) || peekToken(&tokens, TT_SMOOSH) || peekToken(&tokens, TT_NOT))
	{
		ret = parseOpExprNode(tokenp);
	}
	/* Implicit variable */
	else if (acceptToken(&tokens, TT_IT))
	{
#ifdef DEBUG
		debug("ET_IMPVAR");
#endif

		/* Create the new ExprNode structure */
		ret = createExprNode(ET_IMPVAR, NULL);
		if (!ret)
			return NULL;

		/* Since we're successful, update the token stream */
		*tokenp = tokens;
	}

	/** LULZCODE ADDITIONS **/
	else if (peekToken(&tokens, TT_BADGEZ))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_BADGEZ);
	}
	else if (peekToken(&tokens, TT_CRAZY_GO_NUTS))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_CRAZY_GO_NUTS);
	}
	else if (peekToken(&tokens, TT_DAB))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_DAB);
	}
	else if (peekToken(&tokens, TT_HOW_MANY_IN))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_HOW_MANY_IN);
	}
	else if (peekToken(&tokens, TT_TEH))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_TEH);
	}
	else if (peekToken(&tokens, TT_OHIMEMBER))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_OHIMEMBER);
	}
	else if (peekToken(&tokens, TT_CUT))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_CUT);
	}
	else if (peekToken(&tokens, TT_YO))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_YO);
	}
	else if (peekToken(&tokens, TT_KOUNT))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_KOUNT);
	}
	else if (peekToken(&tokens, TT_GNAW))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_GNAW);
	}
	else if (peekToken(&tokens, TT_WHOAMI))
	{
		ret = lulz_parse_value_expr_node(tokenp, TT_WHOAMI);
	}
	else if (peekToken(&tokens, TT_L33T))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_L33T);
	}
	else if (peekToken(&tokens, TT_CHEEZBURGER))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_CHEEZBURGER);
	}
	else if (peekToken(&tokens, TT_MEOWMIX))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_MEOWMIX);
	}
	else if (peekToken(&tokens, TT_TIX))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_TIX);
	}
	else if (peekToken(&tokens, TT_WHO_DIS))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_WHO_DIS);
	}
	else if (peekToken(&tokens, TT_SAY_WUT))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_SAY_WUT);
	}

	/** LULZCODE Accelerometer **/
	else if (peekToken(&tokens, TT_SIDEWAYZ))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_SIDEWAYZ);
	}
	else if (peekToken(&tokens, TT_OUTWAYZ))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_OUTWAYZ);
	}
	else if (peekToken(&tokens, TT_UPWAYZ))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_UPWAYZ);
	}
	else if (peekToken(&tokens, TT_TILTZ))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_TILTZ);
	}

	/** LULZCODE Buttons **/
	else if (peekToken(&tokens, TT_KITTEH))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_KITTEH);
	}
	else if (peekToken(&tokens, TT_POUNCE))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_POUNCE);
	}
	else if (peekToken(&tokens, TT_WHATSUP))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_WHATSUP);
	}
	else if (peekToken(&tokens, TT_WHATSDOWN))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_WHATSDOWN);
	}
	else if (peekToken(&tokens, TT_ISLEFF))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_ISLEFF);
	}
	else if (peekToken(&tokens, TT_ISRIGHT))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_ISRIGHT);
	}
	else if (peekToken(&tokens, TT_WATTA))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_WATTA);
	}
	else if (peekToken(&tokens, TT_WATTB))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_WATTB);
	}
	else if (peekToken(&tokens, TT_ISGO))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_ISGO);
	}

	/** LULZCODE GFX **/
	else if (peekToken(&tokens, TT_HOW_BIG))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_HOW_BIG);
	}
	else if (peekToken(&tokens, TT_HOW_SPREAD))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_HOW_SPREAD);
	}
	else if (peekToken(&tokens, TT_HOW_WIDE))
	{
		ret = lulz_parse_value_expr_node(tokenp, TT_HOW_WIDE);
	}
	else if (peekToken(&tokens, TT_HOW_TALL))
	{
		ret = lulz_parse_value_expr_node(tokenp, TT_HOW_TALL);
	}
	else if (peekToken(&tokens, TT_HSSSVEE2))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_HSSSVEE2);
	}
	else if (peekToken(&tokens, TT_HSSSVEE2BLINKY))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_HSSSVEE2BLINKY);
	}
	else if (peekToken(&tokens, TT_ROYGEEBIF2))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_ROYGEEBIF2);
	}

	/** LULZCODE I2C **/
	else if (peekToken(&tokens, TT_NOM))
	{
		ret = lulz_parse_generic_expr_node(tokenp, TT_NOM);
	}

	/** LULZCODE constants **/
	else if (peekToken(&tokens, TT_VERSHUN))
	{
		ret = lulz_parse_value_expr_node(tokenp, TT_VERSHUN);
	}

	/** Error handling if we got this far **/
	else
	{
		parser_error(PR_EXPECTED_EXPRESSION, tokens);
	}

#ifdef DEBUG
	shiftin();
#endif

	return ret;
}

/**
 * Parses tokens into a cast statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a cast statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseCastStmtNode(Token ***tokenp)
{
	IdentifierNode *target = NULL;
	TypeNode *newtype = NULL;
	CastStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_CAST");
#endif

	/* Parse the target of the cast statement */
	target = parseIdentifierNode(&tokens);
	if (!target)
		goto parseCastStmtNodeAbort;

	/* Remove the cast keywords from the token stream */
	status = acceptToken(&tokens, TT_ISNOWA);
	if (!status)
	{
		parser_error_expected_token(TT_ISNOWA, tokens);
		goto parseCastStmtNodeAbort;
	}

	/* Parse the type to cast to */
	newtype = parseTypeNode(&tokens);
	if (!newtype)
		goto parseCastStmtNodeAbort;

	/* Make sure the statement ends with a newline */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseCastStmtNodeAbort;
	}

	/* Create the new CastStmtNode structure */
	stmt = createCastStmtNode(target, newtype);
	if (!stmt)
		goto parseCastStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_CAST, stmt);
	if (!ret)
		goto parseCastStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseCastStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteCastStmtNode(stmt);
	else
	{
		if (newtype)
			deleteTypeNode(newtype);
		if (target)
			deleteIdentifierNode(target);
	}

	return NULL;
}

/**
 * Parses tokens into a print statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a print statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parsePrintStmtNode(Token ***tokenp)
{
	ExprNode *arg = NULL;
	ExprNodeList *args = NULL;
	FILE *file = stdout;
	int nonl = 0;
	PrintStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_PRINT");
#endif

	/**
	 * Remove the print keyword from the token stream
	 * A "quad-bool" is used to indicate whether we succeeded or failed to
	 * accept either a \c TT_VISIBLE or \c TT_INVISIBLE token.
	 */
	status = acceptToken(&tokens, TT_VISIBLE);
	if (!status)
		status = acceptToken(&tokens, TT_INVISIBLE) ? 2 : -1;
	if (status < 1)
	{
		parser_error_expected_token(TT_VISIBLE - status, tokens);
		goto parsePrintStmtNodeAbort;
	}

	/* Use standard error if we accepted a \c TT_INVISIBLE above */
	if (status == 2)
		file = stderr;

	/* Parse the arguments to the print statement */
	args = createExprNodeList();
	if (!args)
		goto parsePrintStmtNodeAbort;
	do
	{
		/* Parse an argument; it should be an expression */
		arg = parseExprNode(&tokens);
		if (!arg)
			goto parsePrintStmtNodeAbort;

		/* Add it to the list of arguments */
		status = addExprNode(args, arg);
		if (!status)
			goto parsePrintStmtNodeAbort;
		arg = NULL;

		/* Arguments can optionally be separated by an AN keyword */
		status = acceptToken(&tokens, TT_AN);
	} while (!peekToken(&tokens, TT_NEWLINE) && !peekToken(&tokens, TT_BANG));

	/* Check for the no-newline token */
	if (acceptToken(&tokens, TT_BANG))
		nonl = 1;

	/* Make sure the statement ends with a newline */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parsePrintStmtNodeAbort;
	}

	/* Create the new PrintStmtNode structure */
	stmt = createPrintStmtNode(args, file, nonl);
	if (!stmt)
		goto parsePrintStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_PRINT, stmt);
	if (!ret)
		goto parsePrintStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parsePrintStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deletePrintStmtNode(stmt);
	else
	{
		if (arg)
			deleteExprNode(arg);
		if (args)
			deleteExprNodeList(args);
	}

	return NULL;
}

/**
 * Parses tokens into an input statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to an input statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseInputStmtNode(Token ***tokenp)
{
	IdentifierNode *target = NULL;
	InputStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_INPUT");
#endif

	/* Remove the input keyword from the token stream */
	status = acceptToken(&tokens, TT_GIMMEH);
	if (!status)
	{
		parser_error_expected_token(TT_GIMMEH, tokens);
		goto parseInputStmtNodeAbort;
	}

	/* Parse the identifier to store the input into */
	target = parseIdentifierNode(&tokens);
	if (!target)
		goto parseInputStmtNodeAbort;

	/* Make sure the statement ends with a newline */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseInputStmtNodeAbort;
	}

	/* Create the new InputStmtNode structure */
	stmt = createInputStmtNode(target);
	if (!stmt)
		goto parseInputStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_INPUT, stmt);
	if (!ret)
		goto parseInputStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseInputStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteInputStmtNode(stmt);
	else
	{
		if (target)
			deleteIdentifierNode(target);
	}

	return NULL;
}

/**
 * Parses tokens into an assignment statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to an assignment statement.
 *
 * \retval NULL unable to parse.
 */
StmtNode *parseAssignmentStmtNode(Token ***tokenp)
{
	IdentifierNode *target = NULL;
	ExprNode *expr = NULL;
	AssignmentStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_ASSIGNMENT");
#endif

	/* Parse the target of the assignment */
	target = parseIdentifierNode(&tokens);
	if (!target)
		goto parseAssignmentStmtNodeAbort;

	/* Remove the assignment keyword from the token stream */
	if (!acceptToken(&tokens, TT_R))
	{
		parser_error_expected_token(TT_R, tokens);
		goto parseAssignmentStmtNodeAbort;
	}

	/* Parse the expression to assign */
	expr = parseExprNode(&tokens);
	if (!expr)
		goto parseAssignmentStmtNodeAbort;

	/* Make sure the statement ends with a newline */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_STATEMENT, tokens);
		goto parseAssignmentStmtNodeAbort;
	}

	/* Create the new AssignmentStmtNode structure */
	stmt = createAssignmentStmtNode(target, expr);
	if (!stmt)
		goto parseAssignmentStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_ASSIGNMENT, stmt);
	if (!ret)
		goto parseAssignmentStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseAssignmentStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteAssignmentStmtNode(stmt);
	else
	{
		if (expr)
			deleteExprNode(expr);
		if (target)
			deleteIdentifierNode(target);
	}

	return NULL;
}

/**
 * Parses tokens into a declaration statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a declaration statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseDeclarationStmtNode(Token ***tokenp)
{
	IdentifierNode *scope = NULL;
	IdentifierNode *target = NULL;
	ExprNode *expr = NULL;
	TypeNode *type = NULL;
	IdentifierNode *parent = NULL;
	DeclarationStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_DECLARATION");
#endif

	/* Parse the scope to declare the variable in */
	scope = parseIdentifierNode(&tokens);
	if (!scope)
		goto parseDeclarationStmtNodeAbort;

	/* Remove the declaration keywords from the token stream */
	status = acceptToken(&tokens, TT_HASA);
	if (!status)
		status = acceptToken(&tokens, TT_HASAN) || -1;
	if (status < 1)
	{
		parser_error_expected_token(TT_HASA - status, tokens);
		goto parseDeclarationStmtNodeAbort;
	}

	/* Parse the identifier to declare */
	target = parseIdentifierNode(&tokens);
	if (!target)
		goto parseDeclarationStmtNodeAbort;

	/* Check for an optional initialization */
	if (acceptToken(&tokens, TT_ITZ))
	{
		/* Parse the expression to initialize to */
		expr = parseExprNode(&tokens);
		if (!expr)
			goto parseDeclarationStmtNodeAbort;
	}
	/* Check for an optional type initialization */
	else if (acceptToken(&tokens, TT_ITZA))
	{
		/* Parse the type to initialize to */
		type = parseTypeNode(&tokens);
		if (!type)
			goto parseDeclarationStmtNodeAbort;
	}
	/* Check for an optional array inheritance */
	else if (acceptToken(&tokens, TT_ITZLIEKA))
	{
		/* Parse the parent to inherit from */
		parent = parseIdentifierNode(&tokens);
		if (!parent)
			goto parseDeclarationStmtNodeAbort;
	}

	/* Make sure the statement ends with a newline */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		ESP_LOGE(TAG, "%s", __func__);
		parser_error(PR_EXPECTED_END_OF_STATEMENT, tokens);
		goto parseDeclarationStmtNodeAbort;
	}

	/* Create the new DeclarationStmtNode structure */
	stmt = createDeclarationStmtNode(scope, target, expr, type, parent);
	if (!stmt)
		goto parseDeclarationStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_DECLARATION, stmt);
	if (!ret)
		goto parseDeclarationStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseDeclarationStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteDeclarationStmtNode(stmt);
	else
	{
		if (type)
			deleteTypeNode(type);
		if (expr)
			deleteExprNode(expr);
		if (target)
			deleteIdentifierNode(target);
		if (scope)
			deleteIdentifierNode(scope);
	}

	return NULL;
}

/**
 * Parses tokens into an if/then/else statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to an if/then/else statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseIfThenElseStmtNode(Token ***tokenp)
{
	BlockNode *yes = NULL;
	ExprNodeList *guards = NULL;
	BlockNodeList *blocks = NULL;
	ExprNode *guard = NULL;
	BlockNode *block = NULL;
	BlockNode *no = NULL;
	IfThenElseStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_CONDITIONAL");
#endif

	/* Remove the if keyword from the token stream */
	status = acceptToken(&tokens, TT_ORLY);
	if (!status)
	{
		parser_error_expected_token(TT_ORLY, tokens);
		goto parseIfThenElseStmtNodeAbort;
	}

	/* The if keyword must appear on its own line */
	if (!acceptToken(&tokens, TT_NEWLINE))
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseIfThenElseStmtNodeAbort;
	}

	/* Parse the true branch keyword */
	status = acceptToken(&tokens, TT_YARLY);
	if (!status)
	{
		parser_error_expected_token(TT_YARLY, tokens);
		goto parseIfThenElseStmtNodeAbort;
	}

	/* The true branch keyword must appear on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseIfThenElseStmtNodeAbort;
	}

	/* Parse the true part of the branch */
	yes = parseBlockNode(&tokens);
	if (!yes)
		goto parseIfThenElseStmtNodeAbort;

	/* Set up lists of guards and blocks */
	guards = createExprNodeList();
	if (!guards)
		goto parseIfThenElseStmtNodeAbort;
	blocks = createBlockNodeList();
	if (!blocks)
		goto parseIfThenElseStmtNodeAbort;

	/* Parse the else-if keyword */
	while (acceptToken(&tokens, TT_MEBBE))
	{
		/* Parse an else-if guard */
		guard = parseExprNode(&tokens);
		if (!guard)
			goto parseIfThenElseStmtNodeAbort;

		/* Add the else-if guard to the guard list */
		status = addExprNode(guards, guard);
		if (!status)
			goto parseIfThenElseStmtNodeAbort;
		guard = NULL;

		/* The else-if keyword and guard must be on their own line */
		status = acceptToken(&tokens, TT_NEWLINE);
		if (!status)
		{
			parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
			goto parseIfThenElseStmtNodeAbort;
		}

		/* Parse the else-if block */
		block = parseBlockNode(&tokens);
		if (!block)
			goto parseIfThenElseStmtNodeAbort;

		/* Add the else-if block to the block list */
		status = addBlockNode(blocks, block);
		if (!status)
			goto parseIfThenElseStmtNodeAbort;
		block = NULL;
	}

	/* Parse the else keyword */
	if (acceptToken(&tokens, TT_NOWAI))
	{
		/* The else keyword must appear on its own line */
		status = acceptToken(&tokens, TT_NEWLINE);
		if (!status)
		{
			parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
			goto parseIfThenElseStmtNodeAbort;
		}

		/* Parse the else block */
		no = parseBlockNode(&tokens);
		if (!no)
			goto parseIfThenElseStmtNodeAbort;
	}

	/* Parse the end-if-block keyword */
	status = acceptToken(&tokens, TT_OIC);
	if (!status)
	{
		parser_error_expected_token(TT_OIC, tokens);
		goto parseIfThenElseStmtNodeAbort;
	}

	/* The end-if-block keyword must appear on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseIfThenElseStmtNodeAbort;
	}

	/* Create the new IfThenElseStmtNode structure */
	stmt = createIfThenElseStmtNode(yes, no, guards, blocks);
	if (!stmt)
		goto parseIfThenElseStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_IFTHENELSE, stmt);
	if (!ret)
		goto parseIfThenElseStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseIfThenElseStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteIfThenElseStmtNode(stmt);
	else
	{
		if (no)
			deleteBlockNode(no);
		if (blocks)
			deleteBlockNodeList(blocks);
		if (guards)
			deleteExprNodeList(guards);
		if (block)
			deleteBlockNode(block);
		if (guard)
			deleteExprNode(guard);
		if (yes)
			deleteBlockNode(yes);
	}

	return NULL;
}

/**
 * Parses tokens into a switch statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a switch statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseSwitchStmtNode(Token ***tokenp)
{
	ExprNodeList *guards = NULL;
	BlockNodeList *blocks = NULL;
	ConstantNode *c = NULL;
	ExprNode *guard = NULL;
	BlockNode *block = NULL;
	BlockNode *def = NULL;
	SwitchStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_SWITCH");
#endif

	/* Remove the switch keyword from the token stream */
	status = acceptToken(&tokens, TT_WTF);
	if (!status)
	{
		parser_error_expected_token(TT_WTF, tokens);
		goto parseSwitchStmtNodeAbort;
	}

	/* The switch keyword must appear on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseSwitchStmtNodeAbort;
	}

	/* Set up lists of guards and blocks */
	guards = createExprNodeList();
	if (!guards)
		goto parseSwitchStmtNodeAbort;
	blocks = createBlockNodeList();
	if (!blocks)
		goto parseSwitchStmtNodeAbort;

	/* Parse at least one case */
	do
	{
		unsigned int n = 0;

		/* Parse the case keyword */
		status = acceptToken(&tokens, TT_OMG);
		if (!status)
		{
			parser_error_expected_token(TT_OMG, tokens);
			goto parseSwitchStmtNodeAbort;
		}

		/* Parse a constant for the case */
		/** \note The 1.2 specification only allows constant
		 *       values for OMG guards thus this function
		 *       explicitly checks for them. */
		c = parseConstantNode(&tokens);
		if (!c)
			goto parseSwitchStmtNodeAbort;

		/* String interpolation is not allowed */
		if (c->type == CT_STRING && strstr(c->data.s, ":{"))
		{
			parser_error(PR_CANNOT_USE_STR_AS_LITERAL, tokens);
			goto parseSwitchStmtNodeAbort;
		}

		/* Make sure the constant is unique to this switch statement */
		for (n = 0; n < guards->num; n++)
		{
			ConstantNode *test = guards->exprs[n]->expr;
			if (c->type != test->type)
				continue;
			/* Check for equivalent types and values */
			if (((c->type == CT_BOOLEAN || c->type == CT_INTEGER) && c->data.i == test->data.i) || (c->type == CT_FLOAT && fabs(c->data.f - test->data.f) < FLT_EPSILON) || (c->type == CT_STRING && !strcmp(c->data.s, test->data.s)))
			{
				parser_error(PR_LITERAL_MUST_BE_UNIQUE, tokens);
				goto parseSwitchStmtNodeAbort;
			}
		}

		/* Package the constant in an expression */
		guard = createExprNode(ET_CONSTANT, c);
		if (!guard)
			goto parseSwitchStmtNodeAbort;

		/* Add the guard to the list of guards */
		status = addExprNode(guards, guard);
		if (!status)
			goto parseSwitchStmtNodeAbort;
		guard = NULL;

		/* Make sure the guard appears on its own line */
		status = acceptToken(&tokens, TT_NEWLINE);
		if (!status)
		{
			parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
			goto parseSwitchStmtNodeAbort;
		}

		/* Parse the block associated with the guard */
		block = parseBlockNode(&tokens);
		if (!block)
			goto parseSwitchStmtNodeAbort;

		/* Add the block to the list of blocks */
		status = addBlockNode(blocks, block);
		if (!status)
			goto parseSwitchStmtNodeAbort;
		block = NULL;
	} while (peekToken(&tokens, TT_OMG));

	/* Check for the default case */
	if (acceptToken(&tokens, TT_OMGWTF))
	{
		/* Make sure the default case keyword appears on its own line */
		status = acceptToken(&tokens, TT_NEWLINE);
		if (!status)
		{
			parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
			goto parseSwitchStmtNodeAbort;
		}

		/* Parse the default case block */
		def = parseBlockNode(&tokens);
		if (!def)
			goto parseSwitchStmtNodeAbort;
	}

	/* Parse the end-switch keyword */
	status = acceptToken(&tokens, TT_OIC);
	if (!status)
	{
		parser_error_expected_token(TT_OIC, tokens);
		goto parseSwitchStmtNodeAbort;
	}

	/* Make sure the end-switch keyword appears on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseSwitchStmtNodeAbort;
	}

	/* Create the new SwitchStmtNode structure */
	stmt = createSwitchStmtNode(guards, blocks, def);
	if (!stmt)
		goto parseSwitchStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_SWITCH, stmt);
	if (!ret)
		goto parseSwitchStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseSwitchStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteSwitchStmtNode(stmt);
	else
	{
		if (def)
			deleteBlockNode(def);
		if (block)
			deleteBlockNode(block);
		if (guard)
			deleteExprNode(guard);
		if (c)
			deleteConstantNode(c);
		if (blocks)
			deleteBlockNodeList(blocks);
		if (guards)
			deleteExprNodeList(guards);
	}

	return NULL;
}

/**
 * Parses tokens into a break statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a break statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseBreakStmtNode(Token ***tokenp)
{
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_BREAK");
#endif

	/* Remove the break keyword from the token stream */
	status = acceptToken(&tokens, TT_GTFO);
	if (!status)
	{
		parser_error_expected_token(TT_GTFO, tokens);
		goto parseBreakStmtNodeAbort;
	}

	/* The break keyword must appear on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseBreakStmtNodeAbort;
	}

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_BREAK, NULL);
	if (!ret)
		goto parseBreakStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseBreakStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	return NULL;
}

/**
 * Parses tokens into a return statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a return statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseReturnStmtNode(Token ***tokenp)
{
	ExprNode *value = NULL;
	ReturnStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_RETURN");
#endif

	/* Remove the return keyword from the token stream */
	status = acceptToken(&tokens, TT_FOUNDYR);
	if (!status)
	{
		parser_error_expected_token(TT_FOUNDYR, tokens);
		goto parseReturnStmtNodeAbort;
	}

	/* Parse the return value */
	value = parseExprNode(&tokens);
	if (!value)
		goto parseReturnStmtNodeAbort;

	/* The return statement must reside on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseReturnStmtNodeAbort;
	}

	/* Create the new ReturnStmtNode structure */
	stmt = createReturnStmtNode(value);
	if (!stmt)
		goto parseReturnStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_RETURN, stmt);
	if (!ret)
		goto parseReturnStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseReturnStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteReturnStmtNode(stmt);
	else
	{
		if (value)
			deleteExprNode(value);
	}
	return NULL;
}

/**
 * Parses tokens into a loop statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a loop statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseLoopStmtNode(Token ***tokenp)
{
	IdentifierNode *name1 = NULL;
	IdentifierNode *var = NULL;
	ExprNode *update = NULL;
	ExprNode *guard = NULL;
	BlockNode *body = NULL;
	FuncDefStmtNode *def = NULL;
	IdentifierNode *name2 = NULL;
	LoopStmtNode *stmt = NULL;
	ExprNodeList *args = NULL;
	char *id = NULL;

	/* For increment and decrement loops */
	IdentifierNode *varcopy = NULL;
	ExprNode *arg1 = NULL;
	ConstantNode *one = NULL;
	ExprNode *arg2 = NULL;
	OpExprNode *op = NULL;

	/* For function loops */
	IdentifierNode *scope = NULL;
	IdentifierNode *name = NULL;
	FuncCallExprNode *node = NULL;
	ExprNode *arg = NULL;

	/* For loop predicates */
	ExprNode *predarg = NULL;
	ExprNodeList *predargs = NULL;
	OpExprNode *predop = NULL;

	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_LOOP");
#endif

	/* Remove the loop-start keyword from the token stream */
	status = acceptToken(&tokens, TT_IMINYR);
	if (!status)
	{
		parser_error_expected_token(TT_IMINYR, tokens);
		goto parseLoopStmtNodeAbort;
	}

	/* Parse the loop name */
	name1 = parseIdentifierNode(&tokens);
	if (!name1)
		goto parseLoopStmtNodeAbort;
	else if (name1->type != IT_DIRECT)
	{
		parser_error(PR_EXPECTED_LOOP_NAME, tokens);
		goto parseLoopStmtNodeAbort;
	}

	/* Check if an increment or decrement loop */
	if (peekToken(&tokens, TT_UPPIN) || peekToken(&tokens, TT_NERFIN))
	{
		OpType type;

		/* Parse the increment token or decrement token */
		if (acceptToken(&tokens, TT_UPPIN))
		{
			type = OP_ADD;
#ifdef DEBUG
			shiftout();
			debug("ET_OP (OP_ADD)");
#endif
		}
		else if (acceptToken(&tokens, TT_NERFIN))
		{
			type = OP_SUB;
#ifdef DEBUG
			shiftout();
			debug("ET_OP (OP_SUB)");
#endif
		}
		else
		{
			parser_error_expected_either_token(TT_UPPIN, TT_NERFIN, tokens);
			goto parseLoopStmtNodeAbort;
		}

		/* Parse the required syntax */
		if (!acceptToken(&tokens, TT_YR))
		{
			parser_error_expected_token(TT_YR, tokens);
			goto parseLoopStmtNodeAbort;
		}

		/* Parse the variable to operate on */
		var = parseIdentifierNode(&tokens);
		if (!var)
			goto parseLoopStmtNodeAbort;
#ifdef DEBUG
		shiftout();
		debug("ET_CONSTANT (CT_INTEGER)");
		shiftin();
		shiftin();
#endif
		/* Make a copy of the variable for use as a function argument */
		id = util_heap_alloc_ext(sizeof(char) * (strlen(var->id) + 1));
		if (!id)
			goto parseLoopStmtNodeAbort;
		strcpy(id, var->id);
		varcopy = createIdentifierNode(IT_DIRECT, id, NULL, var->fname, var->line);
		if (!varcopy)
			goto parseLoopStmtNodeAbort;
		id = NULL;

		/* Package the variable into an identifier expression */
		arg1 = createExprNode(ET_IDENTIFIER, varcopy);
		if (!arg1)
			goto parseLoopStmtNodeAbort;

		/* Create a constant integer "1" */
		one = createIntegerConstantNode(1);
		if (!one)
			goto parseLoopStmtNodeAbort;

		/* Package the constant into an expression */
		arg2 = createExprNode(ET_CONSTANT, one);
		if (!arg2)
			goto parseLoopStmtNodeAbort;

		/* Create a list of arguments */
		args = createExprNodeList();
		if (!args)
			goto parseLoopStmtNodeAbort;

		/* Add the packaged arguments to the argument list */
		status = addExprNode(args, arg1);
		if (!status)
			goto parseLoopStmtNodeAbort;
		arg1 = NULL;
		status = addExprNode(args, arg2);
		if (!status)
			goto parseLoopStmtNodeAbort;
		arg2 = NULL;

		/* Package the arguments and operation type in an expression */
		op = createOpExprNode(type, args);
		if (!op)
			goto parseLoopStmtNodeAbort;

		/* Create the update expression */
		update = createExprNode(ET_OP, op);
		if (!update)
			goto parseLoopStmtNodeAbort;
	}
	/* Check if a function loop */
	else if (nextToken(&tokens, TT_IZ))
	{
		IdentifierNode *temp = NULL;
#ifdef DEBUG
		debug("ET_FUNCCALL");
#endif
		/* Parse the function scope */
		scope = parseIdentifierNode(&tokens);
		if (!scope)
			goto parseLoopStmtNodeAbort;

		/* Parse the function indicator */
		status = acceptToken(&tokens, TT_IZ);
		if (!status)
		{
			parser_error_expected_token(TT_IZ, tokens);
			goto parseLoopStmtNodeAbort;
		}

		/* Parse the function name */
		name = parseIdentifierNode(&tokens);
		if (!name)
			goto parseLoopStmtNodeAbort;

		/* Create a list of arguments */
		args = createExprNodeList();
		if (!args)
			goto parseLoopStmtNodeAbort;

		/* Check for unary function */
		status = acceptToken(&tokens, TT_YR);
		if (!status)
		{
			parser_error(PR_EXPECTED_UNARY_FUNCTION, tokens);
			goto parseLoopStmtNodeAbort;
		}

		/* Parse the unary function's single argument */
		arg = parseExprNode(&tokens);
		if (!arg)
			goto parseLoopStmtNodeAbort;

		/* Make sure the argument is an identifier */
		if (arg->type != ET_IDENTIFIER)
		{
			parser_error(PR_EXPECTED_IDENTIFIER, tokens);
			goto parseLoopStmtNodeAbort;
		}

		/* Add the argument to the argument list */
		status = addExprNode(args, arg);
		if (!status)
			goto parseLoopStmtNodeAbort;
		/* (Save a pointer to its expression for use, below) */
		temp = (IdentifierNode *)(arg->expr);
		arg = NULL;

		/* Copy the identifier to make it the loop variable */
		id = util_heap_alloc_ext(sizeof(char) * (strlen(temp->id) + 1));
		if (!id)
			goto parseLoopStmtNodeAbort;
		strcpy(id, temp->id);
		var = createIdentifierNode(IT_DIRECT, id, NULL, temp->fname, temp->line);
		if (!var)
			goto parseLoopStmtNodeAbort;
		id = NULL;

		/* Check for unary function */
		status = acceptToken(&tokens, TT_MKAY);
		if (!status)
		{
			parser_error_expected_token(TT_MKAY, tokens);
			goto parseLoopStmtNodeAbort;
		}

		/* Create a function call expression */
		node = createFuncCallExprNode(scope, name, args);
		if (!node)
			goto parseLoopStmtNodeAbort;

		/* Package the function call in an expression */
		update = createExprNode(ET_FUNCCALL, node);
		if (!update)
			goto parseLoopStmtNodeAbort;
	}

	/* If there is an update, parse any predicates */
	if (update)
	{
		/* Check for a while token */
		if (acceptToken(&tokens, TT_WILE))
		{
			/* Parse the while predicate */
			guard = parseExprNode(&tokens);
			if (!guard)
				goto parseLoopStmtNodeAbort;
		}
		/* Check for an until token */
		else if (acceptToken(&tokens, TT_TIL))
		{
#ifdef DEBUG
			shiftout();
			debug("ET_OP (OP_NOT)");
#endif
			/* Parse the until predicate */
			predarg = parseExprNode(&tokens);
			if (!predarg)
				goto parseLoopStmtNodeAbort;

			/* Create a list for predicate arguments */
			predargs = createExprNodeList();
			if (!predargs)
				goto parseLoopStmtNodeAbort;

			/* Add the until predicate to the list */
			status = addExprNode(predargs, predarg);
			if (!status)
				goto parseLoopStmtNodeAbort;
			predarg = NULL;
#ifdef DEBUG
			shiftin();
#endif
			/* Create a NOT operation with the predicate */
			predop = createOpExprNode(OP_NOT, predargs);
			if (!predop)
				goto parseLoopStmtNodeAbort;

			/* Package the NOT operation in an expression */
			guard = createExprNode(ET_OP, predop);
			if (!guard)
				goto parseLoopStmtNodeAbort;
		}
	}

	/* All of the above should be on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseLoopStmtNodeAbort;
	}

	/* Parse the body of the loop */
	body = parseBlockNode(&tokens);
	if (!body)
		goto parseLoopStmtNodeAbort;

	/* Parse the end-loop keywords */
	status = acceptToken(&tokens, TT_IMOUTTAYR);
	if (!status)
	{
		parser_error_expected_token(TT_IMOUTTAYR, tokens);
		goto parseLoopStmtNodeAbort;
	}

	/* Parse the end-of-loop name */
	name2 = parseIdentifierNode(&tokens);
	if (!name2)
		goto parseLoopStmtNodeAbort;
	else if (name2->type != IT_DIRECT)
	{
		parser_error(PR_EXPECTED_LOOP_NAME, tokens);
		goto parseLoopStmtNodeAbort;
	}

	/* Make sure the beginning-of-loop and end-of-loop names match */
	// This causes hard crash when they don't we don't want that :p
	// if (strcmp((char *)(name1->id), (char *)(name2->id)))
	// {
	// 	parser_error(PR_EXPECTED_MATCHING_LOOP_NAME, tokens);
	// 	goto parseLoopStmtNodeAbort;
	// }

	/* We no longer need the end-of-loop name */
	deleteIdentifierNode(name2);

	/* The end-of-loop structure should appear on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto parseLoopStmtNodeAbort;
	}

	/* Create the new LoopStmtNode structure */
	stmt = createLoopStmtNode(name1, var, guard, update, body);
	if (!stmt)
		goto parseLoopStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_LOOP, stmt);
	if (!ret)
		goto parseLoopStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseLoopStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteLoopStmtNode(stmt);
	else
	{
		if (name2)
			deleteIdentifierNode(name2);
		if (def)
			deleteFuncDefStmtNode(def);
		if (body)
			deleteBlockNode(body);
		if (guard)
			deleteExprNode(guard);
		if (update)
			deleteExprNode(update);
		if (var)
			deleteIdentifierNode(var);
		if (name1)
			deleteIdentifierNode(name1);
		if (id)
			free(id);

		/* For increment and decrement loops */
		if (op)
			deleteOpExprNode(op);
		if (args)
			deleteExprNodeList(args);
		if (arg2)
			deleteExprNode(arg2);
		if (one)
			deleteConstantNode(one);
		if (arg1)
			deleteExprNode(arg1);
		if (varcopy)
			deleteIdentifierNode(varcopy);

		/* For function loops */
		if (arg)
			deleteExprNode(arg);
		if (node)
			deleteFuncCallExprNode(node);
		if (name)
			deleteIdentifierNode(name);
		if (scope)
			deleteIdentifierNode(scope);

		/* For loop predicates */
		if (predarg)
			deleteExprNode(predarg);
		if (predargs)
			deleteExprNodeList(predargs);
		if (predop)
			deleteOpExprNode(predop);
	}
	return NULL;
}

/**
 * Parses tokens into a deallocation statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a deallocation statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseDeallocationStmtNode(Token ***tokenp)
{
	IdentifierNode *target = NULL;
	DeallocationStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_DEALLOCATION");
#endif

	/* Parse the variable to deallocate */
	target = parseIdentifierNode(&tokens);
	if (!target)
		goto parseDeallocationStmtNodeAbort;

	/* Parse the deallocation token */
	status = acceptToken(&tokens, TT_RNOOB);
	if (!status)
	{
		parser_error_expected_token(TT_RNOOB, tokens);
		goto parseDeallocationStmtNodeAbort;
	}

	/* The deallocation statement must reside on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_STATEMENT, tokens);
		goto parseDeallocationStmtNodeAbort;
	}

	/* Create the new DeallocationStmtNode structure */
	stmt = createDeallocationStmtNode(target);
	if (!stmt)
		goto parseDeallocationStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_DEALLOCATION, stmt);
	if (!ret)
		goto parseDeallocationStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseDeallocationStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteDeallocationStmtNode(stmt);
	else
	{
		if (target)
			deleteIdentifierNode(target);
	}

	return NULL;
}

/**
 * Parses tokens into a function definition statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a function definition statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseFuncDefStmtNode(Token ***tokenp)
{
	IdentifierNode *scope = NULL;
	IdentifierNode *name = NULL;
	IdentifierNodeList *args = NULL;
	IdentifierNode *arg = NULL;
	BlockNode *body = NULL;
	FuncDefStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_FUNCDEF");
#endif

	/* Parse the function definition token */
	status = acceptToken(&tokens, TT_HOWIZ);
	if (!status)
	{
		parser_error_expected_token(TT_HOWIZ, tokens);
		goto parseFuncDefStmtNodeAbort;
	}

	/* Parse the scope to define the function in */
	scope = parseIdentifierNode(&tokens);
	if (!scope)
		goto parseFuncDefStmtNodeAbort;

	/* Parse the name of the function */
	name = parseIdentifierNode(&tokens);
	if (!name)
		goto parseFuncDefStmtNodeAbort;

	/* Create a list of arguments */
	args = createIdentifierNodeList();
	if (!args)
		goto parseFuncDefStmtNodeAbort;

	/* Parse the first argument indicator */
	if (acceptToken(&tokens, TT_YR))
	{
		/* Parse the first argument name */
		arg = parseIdentifierNode(&tokens);
		if (!arg)
			goto parseFuncDefStmtNodeAbort;

		/* Add the first argument to the arguments list */
		status = addIdentifierNode(args, arg);
		if (!status)
			goto parseFuncDefStmtNodeAbort;
		arg = NULL;

		/* Continue parsing argument indicators */
		while (acceptToken(&tokens, TT_ANYR))
		{
			/* Parse the argument */
			arg = parseIdentifierNode(&tokens);
			if (!arg)
				goto parseFuncDefStmtNodeAbort;

			/* Add the argument to the arguments list */
			status = addIdentifierNode(args, arg);
			if (!status)
				goto parseFuncDefStmtNodeAbort;
			arg = NULL;
		}
	}

	/* The function declaration should appear on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_STATEMENT, tokens);
		goto parseFuncDefStmtNodeAbort;
	}

	/* Parse the body of the function */
	body = parseBlockNode(&tokens);
	if (!body)
		goto parseFuncDefStmtNodeAbort;

	/* Parse the end-function token */
	status = acceptToken(&tokens, TT_IFUSAYSO);
	if (!status)
	{
		parser_error_expected_token(TT_IFUSAYSO, tokens);
		goto parseFuncDefStmtNodeAbort;
	}

	/* The end-function token should appear on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_STATEMENT, tokens);
		goto parseFuncDefStmtNodeAbort;
	}

	/* Create the new FuncDefStmtNode structure */
	stmt = createFuncDefStmtNode(scope, name, args, body);
	if (!stmt)
		goto parseFuncDefStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_FUNCDEF, stmt);
	if (!ret)
		goto parseFuncDefStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseFuncDefStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteFuncDefStmtNode(stmt);
	else
	{
		if (body)
			deleteBlockNode(body);
		if (args)
			deleteIdentifierNodeList(args);
		if (arg)
			deleteIdentifierNode(arg);
		if (name)
			deleteIdentifierNode(name);
		if (scope)
			deleteIdentifierNode(scope);
	}

	return NULL;
}

/**
 * Parses tokens into an alternate array definition statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to an alternate array definition statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseAltArrayDefStmtNode(Token ***tokenp)
{
	IdentifierNode *name = NULL;
	BlockNode *body = NULL;
	IdentifierNode *parent = NULL;
	AltArrayDefStmtNode *stmt = NULL;
	StmtNode *ret = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	debug("ST_ALTARRAYDEF");
#endif

	/* Parse the alternate array definition token */
	status = acceptToken(&tokens, TT_OHAIIM);
	if (!status)
	{
		parser_error_expected_token(TT_OHAIIM, tokens);
		goto parseAltArrayDefStmtNodeAbort;
	}

	/* Parse the array name */
	name = parseIdentifierNode(&tokens);
	if (!name)
		goto parseAltArrayDefStmtNodeAbort;

	/* Check for an optional array inheritance */
	if (acceptToken(&tokens, TT_IMLIEK))
	{
		/* Parse the parent to inherit from */
		parent = parseIdentifierNode(&tokens);
		if (!parent)
			goto parseAltArrayDefStmtNodeAbort;
	}

	/* The alternate array definition token and name should appear on their
	 * own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_STATEMENT, tokens);
		goto parseAltArrayDefStmtNodeAbort;
	}

	/* Parse the array definition body */
	body = parseBlockNode(&tokens);
	if (!body)
		goto parseAltArrayDefStmtNodeAbort;

	/* The end-alternate array definition token should appear on its own
	 * line */
	status = acceptToken(&tokens, TT_KTHX);
	if (!status)
	{
		parser_error_expected_token(TT_KTHX, tokens);
		goto parseAltArrayDefStmtNodeAbort;
	}

	/* The end-function token should appear on its own line */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_STATEMENT, tokens);
		goto parseAltArrayDefStmtNodeAbort;
	}

	/* Create the new AltArrayDefStmtNode structure */
	stmt = createAltArrayDefStmtNode(name, body, parent);
	if (!stmt)
		goto parseAltArrayDefStmtNodeAbort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(ST_ALTARRAYDEF, stmt);
	if (!ret)
		goto parseAltArrayDefStmtNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

parseAltArrayDefStmtNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		deleteAltArrayDefStmtNode(stmt);
	else
	{
		if (name)
			deleteIdentifierNode(name);
		if (body)
			deleteBlockNode(body);
	}

	return NULL;
}

/**
 * Parses tokens into a statement.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a statement.
 *
 * \retval NULL Unable to parse.
 */
StmtNode *parseStmtNode(Token ***tokenp)
{
	StmtNode *ret = NULL;
	ExprNode *expr = NULL;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	shiftout();
#endif

	/* Parse context-sensitive expressions */
	if (peekToken(&tokens, TT_IDENTIFIER) || peekToken(&tokens, TT_SRS))
	{
		IdentifierNode *id = NULL;

		/* Remove the identifier from the token stream */
		id = parseIdentifierNode(&tokens);
		if (!id)
			return NULL;

		/* We do not need to hold onto it */
		deleteIdentifierNode(id);

		/* Casting */
		if (peekToken(&tokens, TT_ISNOWA))
		{
			ret = parseCastStmtNode(tokenp);
		}
		/* Assignment */
		else if (peekToken(&tokens, TT_R))
		{
			ret = parseAssignmentStmtNode(tokenp);
		}
		/* Variable declaration */
		else if (peekToken(&tokens, TT_HASA) || peekToken(&tokens, TT_HASAN))
		{
			ret = parseDeclarationStmtNode(tokenp);
		}
		/* Deallocation */
		else if (peekToken(&tokens, TT_RNOOB))
		{
			ret = parseDeallocationStmtNode(tokenp);
		}
		/* Bare identifier expression */
		else
		{
			/* Reset state and continue parsing */
			tokens = *tokenp;

#ifdef DEBUG
			debug("ST_EXPR");
#endif

			/* Parse the expression */
			expr = parseExprNode(&tokens);
			if (!expr)
				return NULL;

			/* The expression should appear on its own line */
			if (!acceptToken(&tokens, TT_NEWLINE))
			{
				parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
				deleteExprNode(expr);
				return NULL;
			}

			/* Create the new StmtNode structure */
			ret = createStmtNode(ST_EXPR, expr);
			if (!ret)
			{
				deleteExprNode(expr);
				return NULL;
			}

			/* Since we're successful, update the token stream */
			*tokenp = tokens;
		}
	}
	/* Print */
	else if (peekToken(&tokens, TT_VISIBLE))
	{
		ret = parsePrintStmtNode(tokenp);
	}
	/* Warn */
	else if (peekToken(&tokens, TT_INVISIBLE))
	{
		ret = parsePrintStmtNode(tokenp);
	}
	/* Input */
	else if (peekToken(&tokens, TT_GIMMEH))
	{
		ret = parseInputStmtNode(tokenp);
	}
	/* If/then/else */
	else if (peekToken(&tokens, TT_ORLY))
	{
		ret = parseIfThenElseStmtNode(tokenp);
	}
	/* Switch */
	else if (peekToken(&tokens, TT_WTF))
	{
		ret = parseSwitchStmtNode(tokenp);
	}
	/* Break */
	else if (peekToken(&tokens, TT_GTFO))
	{
		ret = parseBreakStmtNode(tokenp);
	}
	/* Return */
	else if (peekToken(&tokens, TT_FOUNDYR))
	{
		ret = parseReturnStmtNode(tokenp);
	}
	/* Loop */
	else if (peekToken(&tokens, TT_IMINYR))
	{
		ret = parseLoopStmtNode(tokenp);
	}
	/* Function definition */
	else if (peekToken(&tokens, TT_HOWIZ))
	{
		ret = parseFuncDefStmtNode(tokenp);
	}
	/* Alternate array definition */
	else if (peekToken(&tokens, TT_OHAIIM))
	{
		ret = parseAltArrayDefStmtNode(tokenp);
	}

	/**************************************************************************
	 * LULZCODE ADDITIONS
	 *************************************************************************/

	else if (peekToken(&tokens, TT_ALL))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_ALL);
	}
	else if (peekToken(&tokens, TT_BAD_KITTEH))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_BAD_KITTEH);
	}
	else if (peekToken(&tokens, TT_BLINK))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_BLINK);
	}
	else if (peekToken(&tokens, TT_BLINKIES))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_BLINKIES);
	}
	else if (peekToken(&tokens, TT_BOX))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_BOX);
	}
	else if (peekToken(&tokens, TT_CATINABOX))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_CATINABOX);
	}
	else if (peekToken(&tokens, TT_CATINAROUND))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_CATINAROUND);
	}
	else if (peekToken(&tokens, TT_CRAYON))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_CRAYON);
	}
	else if (peekToken(&tokens, TT_DOT))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_DOT);
	}
	else if (peekToken(&tokens, TT_HOLLABACK))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_HOLLABACK);
	}
	else if (peekToken(&tokens, TT_INSIDEZ))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_INSIDEZ);
	}
	else if (peekToken(&tokens, TT_KATNIP))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_KATNIP);
	}
	else if (peekToken(&tokens, TT_LAZER))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_LAZER);
	}
	else if (peekToken(&tokens, TT_LOLOL))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_LOLOL);
	}
	else if (peekToken(&tokens, TT_LOLOLOL))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_LOLOLOL);
	}
	else if (peekToken(&tokens, TT_MAH))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_MAH);
	}
	else if (peekToken(&tokens, TT_MEOW))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_MEOW);
	}
	else if (peekToken(&tokens, TT_MEMBER))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_MEMBER);
	}
	else if (peekToken(&tokens, TT_NOMS))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_NOMS);
	}
	else if (peekToken(&tokens, TT_NACHOS))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_NACHOS);
	}
	else if (peekToken(&tokens, TT_PAWS))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_PAWS);
	}
	else if (peekToken(&tokens, TT_RICK_ROLL))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_RICK_ROLL);
	}
	else if (peekToken(&tokens, TT_ROUND))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_ROUND);
	}
	else if (peekToken(&tokens, TT_SHOUT))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_SHOUT);
	}
	else if (peekToken(&tokens, TT_STRAIGHT))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_STRAIGHT);
	}
	else if (peekToken(&tokens, TT_SYSTUM))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_SYSTUM);
	}
	else if (peekToken(&tokens, TT_TROLL))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_TROLL);
	}
	else if (peekToken(&tokens, TT_WINK))
	{
		ret = lulz_parse_generic_stmt_node(tokenp, TT_WINK);
	}

	/**************************************************************************
	 * LULZCODE ADDITIONS
	 *************************************************************************/

	/* Bare expression */
	else if ((expr = parseExprNode(&tokens)))
	{
		int status;

#ifdef DEBUG
		debug("ST_EXPR");
#endif

		/* The expression should appear on its own line */
		status = acceptToken(&tokens, TT_NEWLINE);
		if (!status)
		{
			parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
			deleteExprNode(expr);
			return NULL;
		}

		/* Create the new StmtNode structure */
		ret = createStmtNode(ST_EXPR, expr);
		if (!ret)
		{
			deleteExprNode(expr);
			return NULL;
		}

		/* Since we're successful, update the token stream */
		*tokenp = tokens;
	}
	else
	{
		parser_error(PR_EXPECTED_STATEMENT, tokens);
	}

#ifdef DEBUG
	shiftin();
#endif

	return ret;
}

/**
 * Parses tokens into a code block.
 *
 * \param [in] tokenp The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a code block.
 *
 * \retval NULL Unable to parse.
 */
BlockNode *parseBlockNode(Token ***tokenp)
{
	StmtNodeList *stmts = NULL;
	StmtNode *stmt = NULL;
	BlockNode *block = NULL;
	int status;

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

#ifdef DEBUG
	shiftout();
	debug(">ET_BLOCK");
#endif

	/* Create a list of statements */
	stmts = createStmtNodeList();
	if (!stmts)
		goto parseBlockNodeAbort;
	/* Parse block until certain tokens are found */
	while (!peekToken(&tokens, TT_EOF) && !peekToken(&tokens, TT_KTHXBYE) && !peekToken(&tokens, TT_OIC) && !peekToken(&tokens, TT_YARLY) && !peekToken(&tokens, TT_NOWAI) && !peekToken(&tokens, TT_MEBBE) && !peekToken(&tokens, TT_OMG) && !peekToken(&tokens, TT_OMGWTF) && !peekToken(&tokens, TT_IMOUTTAYR) && !peekToken(&tokens, TT_IFUSAYSO) && !peekToken(&tokens, TT_KTHX))
	{
		/* Parse the next statement */
		stmt = parseStmtNode(&tokens);
		if (!stmt)
			goto parseBlockNodeAbort;

		/* Add the statement to the list */
		status = addStmtNode(stmts, stmt);
		if (!status)
			goto parseBlockNodeAbort;
		stmt = NULL;
	}

#ifdef DEBUG
	debug("<ET_BLOCK");
	shiftin();
#endif

	/* Create the BlockNode structure */
	block = createBlockNode(stmts);
	if (!block)
		goto parseBlockNodeAbort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return block;

parseBlockNodeAbort: /* Exception handling */

	/* Clean up any allocated structures */
	if (block)
		deleteBlockNode(block);
	else
	{
		if (stmt)
			deleteStmtNode(stmt);
		if (stmts)
			deleteStmtNodeList(stmts);
	}
	return NULL;
}

/**
 * Parses tokens into a main code block.
 *
 * \param [in] tokens The position in a token list to start parsing at.
 *
 * \post \a tokenp will point to the next unparsed token.
 *
 * \return A pointer to a main node block.
 *
 * \retval NULL Unable to parse.
 */
MainNode *parseMainNode(Token **tokens)
{
	BlockNode *block = NULL;
	MainNode *_main = NULL;
	int status;

	/* All programs must start with the HAI token */
	status = acceptToken(&tokens, TT_HAI);
	if (!status)
	{
		parser_error_expected_token(TT_HAI, tokens);
		goto parseMainNodeAbort;
	}

	//Parse api level from HAI statement
	//	acceptToken(&tokens, TT_INTEGER);
	//	if (!status) {
	//		ESP_LOGE(TAG, "Expected integer after HAI");
	//		goto parseMainNodeAbort;
	//	}

	ConstantNode *p_api_level_node = parseConstantNode(&tokens);
	ESP_LOGV(TAG, "Expect at least %d, badge provides %d", (int)p_api_level_node->data.i, LULZ_API_LEVEL);
	if (p_api_level_node->data.i > LULZ_API_LEVEL)
	{
		ESP_LOGW(TAG, "LULZCODE API level %d expected but badge provides %d. You will have a bad day.", (int)p_api_level_node->data.i, LULZ_API_LEVEL);
	}
	free(p_api_level_node);

#ifdef DEBUG
	debug("ET_MAINBLOCK");
#endif

	/* Make sure the header line ends with a newline */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_STATEMENT, tokens);
		goto parseMainNodeAbort;
	}

	/* Parse the main block of code */
	block = parseBlockNode(&tokens);
	if (!block)
		goto parseMainNodeAbort;

	//	This breaks pre-processor includes
	//	/* Make sure the program ends with KTHXBYE */
	//	status = acceptToken(&tokens, TT_KTHXBYE);
	//	if (!status) {
	//		parser_error_expected_token(TT_KTHXBYE, tokens);
	//		goto parseMainNodeAbort;
	//	}

	/* Create the MainBlockNode structure */
	_main = createMainNode(block);
	if (!_main)
		goto parseMainNodeAbort;

	return _main;

parseMainNodeAbort: /* In case something goes wrong... */

	ESP_LOGE(TAG, "parseMainNodeAbort!");
	/* Clean up any allocated structures */
	if (_main)
		deleteMainNode(_main);
	else if (block)
		deleteBlockNode(block);

	return NULL;
}

/******************************************************************************
 * LULZCODE Additions
 *****************************************************************************/
lulz_generic_stmt_node *lulz_create_generic_stmt_node(ExprNodeList *args)
{
	lulz_generic_stmt_node *p = util_heap_alloc_ext(sizeof(lulz_generic_stmt_node));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->args = args;
	return p;
}

void lulz_delete_expr_node(lulz_generic_expr_node *node)
{
	if (!node)
		return;
	if (node->expr1)
		deleteExprNode(node->expr1);
	if (node->expr2)
		deleteExprNode(node->expr2);
	if (node->expr3)
		deleteExprNode(node->expr3);
	free(node);
}

void lulz_delete_stmt_node(lulz_generic_stmt_node *node)
{
	if (!node)
		return;
	deleteExprNodeList(node->args);
	free(node);
}

/**
 * Generic parser for color tokens. Simply replace token with color
 * value to save time during interpretation
 */
ExprNode *lulz_parse_value_expr_node(Token ***tokenp, TokenType token_type)
{
	int status;
	uint64_t value = 0;

	status = acceptToken(tokenp, token_type);
	if (!status)
	{
		ESP_LOGE(TAG, "Unable to parse LULZCODE constant.");
		return NULL;
	}

	//Set the constant value. Most are integers. Non-integers should be handled seperately
	if (token_type == TT_HOW_WIDE)
	{
		value = LCD_WIDTH;
	}
	else if (token_type == TT_HOW_TALL)
	{
		value = LCD_HEIGHT;
	}
	else if (token_type == TT_VERSHUN)
	{
		ConstantNode *version_node = createStringConstantNode(copyString(VERSION));
		return createExprNode(ET_CONSTANT, version_node);
	}
	else if (token_type == TT_WHOAMI)
	{
		value = util_serial_get();
	}

	ConstantNode *const_node = createIntegerConstantNode(value);
	return createExprNode(ET_CONSTANT, const_node);
}

ExprNode *lulz_parse_generic_expr_node(Token ***tokenp, TokenType token_type)
{
	ExprNode *one = NULL;
	ExprNode *two = NULL;
	ExprNode *three = NULL;
	lulz_generic_expr_node *expr = NULL;
	ExprNode *ret = NULL;
	ExprType type = 0;
	int status;

	if (token_type == TT_HOW_MANY_IN)
	{
		type = ET_ARRAY_COUNT;
	}
	else if (token_type == TT_WHO_DIS)
	{
		type = ET_BROADCAST_GET;
	}
	else if (token_type == TT_SAY_WUT)
	{
		type = ET_BROADCAST_TX_GET;
	}
	else if (token_type == TT_CHEEZBURGER)
	{
		type = ET_UNLOCK_VALIDATE;
	}
	else if (token_type == TT_KITTEH)
	{
		type = ET_BUTTON_STATE;
	}
	else if (token_type == TT_POUNCE)
	{
		type = ET_BUTTON_WAIT;
	}
	else if (token_type == TT_WHATSUP)
	{
		type = ET_BUTTON_UP;
	}
	else if (token_type == TT_WHATSDOWN)
	{
		type = ET_BUTTON_DOWN;
	}
	else if (token_type == TT_ISLEFF)
	{
		type = ET_BUTTON_LEFT;
	}
	else if (token_type == TT_ISRIGHT)
	{
		type = ET_BUTTON_RIGHT;
	}
	else if (token_type == TT_WATTA)
	{
		type = ET_BUTTON_A;
	}
	else if (token_type == TT_WATTB)
	{
		type = ET_BUTTON_B;
	}
	else if (token_type == TT_ISGO)
	{
		type = ET_BUTTON_C;
	}
	else if (token_type == TT_DAB)
	{
		type = ET_FILE_LIST;
	}
	else if (token_type == TT_NOM)
	{
		type = ET_I2C_READ;
	}
	else if (token_type == TT_HSSSVEE2)
	{
		type = ET_HSV_TO_565;
	}
	else if (token_type == TT_HSSSVEE2BLINKY)
	{
		type = ET_HSV_TO_RGB;
	}
	else if (token_type == TT_ROYGEEBIF2)
	{
		type = ET_RGB_TO_565;
	}
	else if (token_type == TT_TEH)
	{
		type = ET_GLOBAL_GET;
	}
	else if (token_type == TT_OHIMEMBER)
	{
		type = ET_STATE_GET;
	}
	else if (token_type == TT_CUT)
	{
		type = ET_STRING_GET_AT_INDEX;
	}
	else if (token_type == TT_YO)
	{
		type = ET_STRING_SET_AT_INDEX;
	}
	else if (token_type == TT_KOUNT)
	{
		type = ET_STRING_LENGTH;
	}
	else if (token_type == TT_GNAW)
	{
		type = ET_STRING_SUBSTRING;
	}
	else if (token_type == TT_HOW_BIG)
	{
		type = ET_TEXT_HEIGHT;
	}
	else if (token_type == TT_HOW_SPREAD)
	{
		type = ET_TEXT_WIDTH;
	}

	/** Accelerometer **/
	else if (token_type == TT_SIDEWAYZ)
	{
		type = ET_ACCEL_SIDE;
	}
	else if (token_type == TT_OUTWAYZ)
	{
		type = ET_ACCEL_OUT;
	}
	else if (token_type == TT_UPWAYZ)
	{
		type = ET_ACCEL_UP;
	}
	else if (token_type == TT_TILTZ)
	{
		type = ET_ACCEL_TILT;
	}

	/** I2C Read **/
	else if (token_type == TT_NOM)
	{
		type = ET_I2C_READ;
	}

	/** System **/
	else if (token_type == TT_MEOWMIX)
	{
		type = ET_INPUT;
	}
	else if (token_type == TT_BADGEZ)
	{
		type = ET_PEERS;
	}
	else if (token_type == TT_CRAZY_GO_NUTS)
	{
		type = ET_RANDOM;
	}
	else if (token_type == TT_L33T)
	{
		type = ET_UNLOCK_GET;
	}
	else if (token_type == TT_TIX)
	{
		type = ET_MILLIS;
	}

	/* Work from a copy of the token stream in case something goes wrong */
	Token **tokens = *tokenp;

	/* Parse the token, everything after it is passed as arguments */
	status = acceptToken(&tokens, token_type);
	if (!status)
	{
		parser_error_expected_token(token_type, tokens);
		ESP_LOGE(TAG, "Unable to accept token");
		goto lulz_parse_expr_abort;
	}

	//Create an expression that we will populate depending on the token
	expr = util_heap_alloc_ext(sizeof(lulz_generic_expr_node));
	if (!expr)
	{
		ESP_LOGE(TAG, "Could not create generic expression Node");
		goto lulz_parse_expr_abort;
	}

	//Create the new ExprNode structure to return
	ret = createExprNode(type, expr);
	if (!ret)
	{
		ESP_LOGE(TAG, "%s Could not create an expression node", __func__);
		goto lulz_parse_expr_abort;
	}

	//Handle expressions that take 3 arguments
	if (token_type == TT_ROYGEEBIF2 || token_type == TT_GNAW || token_type == TT_HSSSVEE2 || token_type == TT_HSSSVEE2BLINKY || token_type == TT_MEOWMIX || token_type == TT_YO)
	{

		one = parseExprNode(&tokens);
		if (!one)
			goto lulz_parse_expr_abort;
		two = parseExprNode(&tokens);
		if (!two)
			goto lulz_parse_expr_abort;
		three = parseExprNode(&tokens);
		if (!three)
			goto lulz_parse_expr_abort;
	}
	//Handle expressions that take 2 arguments
	else if (token_type == TT_CUT || token_type == TT_NOM)
	{
		one = parseExprNode(&tokens);
		if (!one)
			goto lulz_parse_expr_abort;
		two = parseExprNode(&tokens);
		if (!two)
			goto lulz_parse_expr_abort;
	}
	//Handle expressions that take 1 argument
	else if (token_type == TT_CRAZY_GO_NUTS || token_type == TT_DAB || token_type == TT_HOW_BIG || token_type == TT_HOW_MANY_IN || token_type == TT_HOW_SPREAD || token_type == TT_OHIMEMBER || token_type == TT_KOUNT || token_type == TT_TEH || token_type == TT_CHEEZBURGER)
	{
		one = parseExprNode(&tokens);
		if (!one)
			goto lulz_parse_expr_abort;

		expr->expr1 = one;
	}

	expr->expr1 = one;
	expr->expr2 = two;
	expr->expr3 = three;
	/* Since we're successful, update the token stream */
	*tokenp = tokens;
	return ret;

lulz_parse_expr_abort: /* Exception handling */
	ESP_LOGE(TAG, "Unable to parse generic expression");
	/* Clean up any allocated structures */
	if (ret)
		deleteExprNode(ret);
	else if (expr)
		free(expr);
	else
	{
		if (one)
			free(one);
		if (two)
			free(two);
		if (three)
			free(three);
	}

	return NULL;
}

StmtNode *lulz_parse_generic_stmt_node(Token ***tokenp, TokenType token_type)
{
	ExprNode *arg = NULL;
	ExprNodeList *args = NULL;
	lulz_generic_stmt_node *stmt = NULL;
	StmtNode *ret = NULL;
	StmtType type = 0;
	int status = 0;
	bool args_expected = true;

	//Work from a copy of the token stream in case something goes wrong
	Token **tokens = *tokenp;

	//Search for the token being parsed
	if (token_type == TT_ROUND)
	{
		status = acceptToken(&tokens, TT_ROUND);
		type = ST_CIRCLE;
	}
	else if (token_type == TT_CATINAROUND)
	{
		status = acceptToken(&tokens, TT_CATINAROUND);
		type = ST_CIRCLE_FILLED;
	}
	else if (token_type == TT_ALL)
	{
		status = acceptToken(&tokens, TT_ALL);
		type = ST_FILL;
	}
	else if (token_type == TT_CRAYON)
	{
		status = acceptToken(&tokens, TT_CRAYON);
		type = ST_COLOR_SET;
	}
	else if (token_type == TT_BAD_KITTEH)
	{
		status = acceptToken(&tokens, TT_BAD_KITTEH);
		type = ST_BUTTON_CLEAR;
		args_expected = false;
	}
	else if (token_type == TT_BLINK)
	{
		status = acceptToken(&tokens, TT_BLINK);
		type = ST_LED_SET;
	}
	else if (token_type == TT_BLINKIES)
	{
		status = acceptToken(&tokens, TT_BLINKIES);
		type = ST_LED_SET_ALL;
	}
	else if (token_type == TT_BOX)
	{
		status = acceptToken(&tokens, TT_BOX);
		type = ST_RECT;
	}
	else if (token_type == TT_CATINABOX)
	{
		status = acceptToken(&tokens, TT_CATINABOX);
		type = ST_RECT_FILLED;
	}
	else if (token_type == TT_DOT)
	{
		status = acceptToken(&tokens, TT_DOT);
		type = ST_PIXEL;
	}
	else if (token_type == TT_HOLLABACK)
	{
		status = acceptToken(&tokens, TT_HOLLABACK);
		type = ST_BROADCAST;
	}
	else if (token_type == TT_INSIDEZ)
	{
		status = acceptToken(&tokens, TT_INSIDEZ);
		type = ST_CURSOR_AREA;
	}
	else if (token_type == TT_KATNIP)
	{
		status = acceptToken(&tokens, TT_KATNIP);
		type = ST_BITMAP;
	}
	else if (token_type == TT_LAZER)
	{
		status = acceptToken(&tokens, TT_LAZER);
		type = ST_CURSOR;
	}
	else if (token_type == TT_LOLOL)
	{
		status = acceptToken(&tokens, TT_LOLOL);
		type = ST_LED_PUSH;
		args_expected = false;
	}
	else if (token_type == TT_LOLOLOL)
	{
		status = acceptToken(&tokens, TT_LOLOLOL);
		type = ST_PUSHBUFFER;
		args_expected = false;
	}
	else if (token_type == TT_MAH)
	{
		status = acceptToken(&tokens, TT_MAH);
		type = ST_GLOBAL_SET;
	}
	else if (token_type == TT_MEOW)
	{
		status = acceptToken(&tokens, TT_MEOW);
		type = ST_PRINTSCREEN;
	}
	else if (token_type == TT_MEMBER)
	{
		status = acceptToken(&tokens, TT_MEMBER);
		type = ST_STATE_SET;
	}
	else if (token_type == TT_NACHOS)
	{
		status = acceptToken(&tokens, TT_NACHOS);
		type = ST_CHIP8;
	}
	else if (token_type == TT_NOMS)
	{
		status = acceptToken(&tokens, TT_NOMS);
		type = ST_I2C_WRITE;
	}
	else if (token_type == TT_PAWS)
	{
		status = acceptToken(&tokens, TT_PAWS);
		type = ST_DELAY;
	}
	else if (token_type == TT_RICK_ROLL)
	{
		status = acceptToken(&tokens, TT_RICK_ROLL);
		type = ST_PLAY_BLING;
	}
	else if (token_type == TT_TROLL)
	{
		status = acceptToken(&tokens, TT_TROLL);
		type = ST_RUN;
	}
	else if (token_type == TT_SHOUT)
	{
		status = acceptToken(&tokens, TT_SHOUT);
		type = ST_FONT_SET;
	}
	else if (token_type == TT_STRAIGHT)
	{
		status = acceptToken(&tokens, TT_STRAIGHT);
		type = ST_LINE;
	}
	else if (token_type == TT_SYSTUM)
	{
		status = acceptToken(&tokens, TT_SYSTUM);
		type = ST_SYSTEM_CALL;
	}
	else if (token_type == TT_WINK)
	{
		status = acceptToken(&tokens, TT_WINK);
		type = ST_LED_EYE;
	}
	//If we got this far, token is invalid
	if (!status)
	{
		ESP_LOGE(TAG, "Unexpected LULZCODE Token at %d in %s", (**tokenp)->line, (**tokenp)->fname);
		goto lulz_parse_generic_stmt_node_abort;
	}

	/* Parse the arguments to the statement */
	if (args_expected)
	{
		args = createExprNodeList();
		if (!args)
			goto lulz_parse_generic_stmt_node_abort;

		//Walk through all arguments. Interpret expressions.
		//Build argument list and add that to the statement node
		//Note: validation of the arguments will be deferred to the interpreter
		do
		{
			/* Parse an argument; it should be an expression */
			arg = parseExprNode(&tokens);
			if (!arg)
				goto lulz_parse_generic_stmt_node_abort;

			/* Add it to the list of arguments */
			status = addExprNode(args, arg);
			if (!status)
				goto lulz_parse_generic_stmt_node_abort;
			arg = NULL;

			/* Arguments can optionally be separated by an AN keyword */
			status = acceptToken(&tokens, TT_AN);
		} while (!peekToken(&tokens, TT_NEWLINE));
	}

	/* Make sure the statement ends with a newline */
	status = acceptToken(&tokens, TT_NEWLINE);
	if (!status)
	{
		parser_error(PR_EXPECTED_END_OF_EXPRESSION, tokens);
		goto lulz_parse_generic_stmt_node_abort;
	}

	/* Create the new generic lulzcode structure */
	stmt = lulz_create_generic_stmt_node(args);
	if (!stmt)
		goto lulz_parse_generic_stmt_node_abort;

	/* Create the new StmtNode structure */
	ret = createStmtNode(type, stmt);
	if (!ret)
		goto lulz_parse_generic_stmt_node_abort;

	/* Since we're successful, update the token stream */
	*tokenp = tokens;

	return ret;

/************ Exeception handling ****************/
lulz_parse_generic_stmt_node_abort:

	ESP_LOGE(TAG, "Unable to parse lulz statement node.");
	/* Clean up any allocated structures */
	if (ret)
		deleteStmtNode(ret);
	else if (stmt)
		lulz_delete_stmt_node(stmt);
	else
	{
		if (arg)
			deleteExprNode(arg);
		if (args)
			deleteExprNodeList(args);
	}

	return NULL;
}

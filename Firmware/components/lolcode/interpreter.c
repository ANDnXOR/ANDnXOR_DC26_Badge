#include "system.h"

#include "lolcode.h"
#include "interpreter.h"

static const char *TAG = "LULZ::Interpreter";

typedef struct
{
	uint8_t pattern;
	bool running;
} lulz_callback_data_t;

/**
 * Creates a new string by copying the contents of another string.
 *
 * \param [in] data The string to copy.
 *
 * \return A new string whose contents is a copy of \a data.
 *
 * \retval NULL Memory allocation failed.
 */
char *copyString(char *data)
{
	char *p = util_heap_alloc_ext(sizeof(char) * (strlen(data) + 1));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	strcpy(p, data);
	return p;
}

/**
 * Checks if a string follows the format of a hexadecimal number.
 *
 * \param [in] data The characters to check the format of.
 *
 * \retval 0 The string is not a hexadecimal number.
 *
 * \retval 1 The string is a hexadecimal number.
 */
unsigned int isHexString(const char *data)
{
	size_t n;
	size_t len = strlen(data);

	/* Check for empty string */
	if (len == 0)
		return 0;

	/* Check for non-digit and non-A-through-F characters */
	for (n = 0; n < len; n++)
	{
		if (!isdigit((unsigned char)data[n]) && data[n] != 'A' && data[n] != 'B' && data[n] != 'C' && data[n] != 'D' && data[n] != 'E' && data[n] != 'F' && data[n] != 'a' && data[n] != 'b' && data[n] != 'c' && data[n] != 'd' && data[n] != 'e' && data[n] != 'f')
			return 0;
	}

	return 1;
}

/**
 * Evaluates an identifier to produce its name as a string.
 *
 * \param [in] id The identifier to evaluate.
 *
 * \param [in] scope The scope to evaluate \a id under.
 *
 * \return A new string containing the evaluated name of the identifier.
 *
 * \retval NULL Memory allocation failed.
 */
char *resolveIdentifierName(IdentifierNode *id,
														ScopeObject *scope)
{
	ValueObject *val = NULL;
	ValueObject *str = NULL;
	char *ret = NULL;

	if (!id)
		goto resolveIdentifierNameAbort;

	if (id->type == IT_DIRECT)
	{
		/* Just return a copy of the character array */
		const char *temp = (char *)(id->id);
		ret = util_heap_alloc_ext(sizeof(char) * (strlen(temp) + 1));
		strcpy(ret, temp);
	}
	else if (id->type == IT_INDIRECT)
	{
		ExprNode *expr = (ExprNode *)(id->id);

		/* Interpret the identifier expression */
		val = interpretExprNode(expr, scope);
		if (!val)
			goto resolveIdentifierNameAbort;

		/* Then cast it to a string */
		str = castStringExplicit(val, scope);
		if (!str)
			goto resolveIdentifierNameAbort;
		deleteValueObject(val);

		/* Copy the evaluated string */
		ret = copyString(getString(str));
		if (!ret)
			goto resolveIdentifierNameAbort;
		deleteValueObject(str);
	}
	else
	{
		char *name = resolveIdentifierName(id, scope);
		lulz_error(IN_INVALID_IDENTIFIER_TYPE, id->fname, id->line, name);
		free(name);
	}

	return ret;

resolveIdentifierNameAbort: /* Exception handline */

	/* Clean up any allocated structures */
	if (ret)
		free(ret);
	if (str)
		deleteValueObject(str);
	if (val)
		deleteValueObject(val);

	return NULL;
}

/**
 * Creates a nil-type value.
 *
 * \return A new nil-type value.
 *
 * \retval NULL Memory allocation failed.
 */
ValueObject *createNilValueObject(void)
{
	ValueObject *p = util_heap_alloc_ext(sizeof(ValueObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = VT_NIL;
	p->semaphore = 1;
	return p;
}

/**
 * Creates a boolean-type value.
 *
 * \param [in] data The boolean data to store.
 *
 * \return A boolean-type value equalling 0 if \a data equals 0 and 1 otherwise.
 *
 * \retval NULL Memory allocation failed.
 */
ValueObject *createBooleanValueObject(int data)
{
	ValueObject *p = util_heap_alloc_ext(sizeof(ValueObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = VT_BOOLEAN;
	p->data.i = (data != 0);
	p->semaphore = 1;
	return p;
}

/**
 * Creates a integer-type value.
 *
 * \param [in] data The integer data to store.
 *
 * \return An integer-type value equalling \a data.
 *
 * \retval NULL Memory allocation failed.
 */
ValueObject *createIntegerValueObject(long long data)
{
	ValueObject *p = util_heap_alloc_ext(sizeof(ValueObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = VT_INTEGER;
	p->data.i = data;
	p->semaphore = 1;
	return p;
}

/**
 * Creates a floating-point-type value.
 *
 * \param [in] data The floating-point data to store.
 *
 * \return A floating-point-type value equalling \a data.
 *
 * \retval NULL Memory allocation failed.
 */
ValueObject *createFloatValueObject(float data)
{
	ValueObject *p = util_heap_alloc_ext(sizeof(ValueObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = VT_FLOAT;
	p->data.f = data;
	p->semaphore = 1;
	return p;
}

/**
 * Creates a string-type value.
 *
 * \param [in] data The string data to store.
 *
 * \note \a data is stored as-is; no copy of it is made.
 *
 * \return A string-type value equalling \a data.
 *
 * \retval NULL Memory allocation failed.
 */
ValueObject *createStringValueObject(char *data)
{
	ValueObject *p = util_heap_alloc_ext(sizeof(ValueObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = VT_STRING;
	p->data.s = data;
	p->semaphore = 1;
	return p;
}

/**
 * Creates a function-type value.
 *
 * \param [in] def The function definition to store.
 *
 * \return A function-type value containing \a data.
 *
 * \retval NULL Memory allocation failed.
 */
ValueObject *createFunctionValueObject(FuncDefStmtNode *def)
{
	ValueObject *p = util_heap_alloc_ext(sizeof(ValueObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = VT_FUNC;
	p->data.fn = def;
	p->semaphore = 1;
	return p;
}

/**
 * Creates an array-type value.
 *
 * \param [in] parent The optional parent scope to use.
 *
 * \note \a parent may be NULL, in which case this array is treated as the root.
 *
 * \return An empty array-type value with parent \a parent.
 *
 * \retval NULL Memory allocation failed.
 */
ValueObject *createArrayValueObject(ScopeObject *parent)
{
	ValueObject *p = util_heap_alloc_ext(sizeof(ValueObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = VT_ARRAY;
	p->data.a = createScopeObject(parent);
	if (!p->data.a)
	{
		free(p);
		return NULL;
	}
	p->semaphore = 1;
	return p;
}

/**
 * Copies a value.
 *
 * Instead of actually performing a copy of memory, this function increments a
 * semaphore in \a value and returns \a value again.  The semaphore gets
 * decremented when \a value gets deleted.  This way, an immutable copy of a
 * value may be made without actually copying its blocks of memory; this reduces
 * the overhead associated with copying a value--a fairly common
 * operation--while still preserving its usability.
 *
 * \param [in,out] value The value to copy.
 *
 * \return A value with the same type and contents as \a value.
 *
 * \retval NULL The type of \a value is unrecognized.
 */
ValueObject *copyValueObject(ValueObject *value)
{
	V(value);
	return value;
}

/**
 * Deletes a value.
 *
 * This function decrements a semaphore in \a value and deletes \a value if the
 * semaphore reaches 0 (no copies of this value are need anymore).  The
 * semaphore gets incremented when either the value is created or it gets
 * copied.  This way, an immutable copy of the value may be made without
 * actually copying its memory.
 *
 * \param [in,out] value The value to delete.
 *
 * \post The memory at \a value and any of its members will be freed (although
 * see note for full details).
 */
void deleteValueObject(ValueObject *value)
{
	if (!value)
		return;
	P(value);
	if (!value->semaphore)
	{
		if (value->type == VT_STRING)
			free(value->data.s);
		/* FuncDefStmtNode structures get freed with the parse tree */
		else if (value->type == VT_ARRAY)
			deleteScopeObject(value->data.a);
		free(value);
	}
}

/**
 * Creates a scope.
 *
 * Scopes are used to map identifiers to values.  Scopes are organized
 * hierarchically.
 *
 * \param [in] parent The optional parent scope to use.
 *
 * \return An empty scope with parent \a parent.
 *
 * \retval NULL Memory allocation failed.
 */
ScopeObject *createScopeObject(ScopeObject *parent)
{
	ScopeObject *p = util_heap_alloc_ext(sizeof(ScopeObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->impvar = createNilValueObject();
	if (!p->impvar)
	{
		free(p);
		return NULL;
	}
	p->numvals = 0;
	p->names = NULL;
	p->values = NULL;
	p->parent = parent;
	if (parent)
		p->caller = parent->caller;
	else
		p->caller = NULL;
	return p;
}

/**
 * Creates a scope with a specific caller.
 *
 * \param [in] parent The optional parent scope to use.
 *
 * \param [in] caller The caller scope to use.
 *
 * \return An empty scope with parent \a parent and caller \a caller.
 *
 * \retval NULL Memory allocation failed.
 */
ScopeObject *createScopeObjectCaller(ScopeObject *parent,
																		 ScopeObject *caller)
{
	ScopeObject *p = createScopeObject(parent);
	if (!p)
		return NULL;
	if (caller)
		p->caller = caller;
	return p;
}

/**
 * Deletes a scope.
 *
 * \param [in,out] scope The scope to delete.
 *
 * \post The memory at \a scope and any of its members will be freed.
 */
void deleteScopeObject(ScopeObject *scope)
{
	unsigned int n;
	if (!scope)
		return;
	for (n = 0; n < scope->numvals; n++)
	{
		free(scope->names[n]);
		deleteValueObject(scope->values[n]);
	}
	free(scope->names);
	free(scope->values);
	deleteValueObject(scope->impvar);
	free(scope);
}

/**
 * Creates a new, nil-type value in a scope.
 *
 * \param [in] src The scope to evaluate \a target under.
 *
 * \param [in,out] dest The scope to create the new value in.
 *
 * \param [in] target The name of the value to create.
 *
 * \return The newly-created value.
 *
 * \retval NULL Memory allocation failed.
 */
ValueObject *createScopeValue(ScopeObject *src,
															ScopeObject *dest,
															IdentifierNode *target)
{
	ScopeObject *parent = dest;
	IdentifierNode *child = target;
	int status;
	unsigned int newnumvals;
	void *mem1 = NULL;
	void *mem2 = NULL;
	char *name = NULL;

	/* Traverse the target to the terminal child and parent */
	status = resolveTerminalSlot(src, dest, target, &parent, &child);
	if (!status)
		goto createScopeValueAbort;

	/* Store the new number of values */
	newnumvals = dest->numvals + 1;

	/* Look up the identifier name */
	name = resolveIdentifierName(target, src);
	if (!name)
		goto createScopeValueAbort;

	/* Add value to local scope */
	mem1 = util_heap_realloc_ext(dest->names, sizeof(IdentifierNode *) * newnumvals);
	if (!mem1)
	{
		ESP_LOGD(TAG, "realloc mem1 error in %s", __func__);
		goto createScopeValueAbort;
	}
	mem2 = util_heap_realloc_ext(dest->values, sizeof(ValueObject *) * newnumvals);
	if (!mem2)
	{
		ESP_LOGD(TAG, "realloc mem2 error in %s", __func__);
		goto createScopeValueAbort;
	}

	dest->names = mem1;
	dest->values = mem2;
	dest->names[dest->numvals] = name;
	dest->values[dest->numvals] = createNilValueObject();
	if (!dest->values[dest->numvals])
		goto createScopeValueAbort;
	dest->numvals = newnumvals;

	return dest->values[dest->numvals - 1];

createScopeValueAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (name)
		free(name);
	if (mem1)
		free(mem1);
	if (mem2)
		free(mem2);

	return NULL;
}

/**
 * Updates a value in a scope.
 *
 * \param [in] src The scope to evaluate \a target under.
 *
 * \param [in,out] dest The scope to update the value in.
 *
 * \param [in] target The name of the value to create.
 *
 * \param [in] value The new value to assign.
 *
 * \return The updated value (will be the same as \a val).
 *
 * \retval NULL Either \a target could not be evaluated in \a src or \a target
 * could not be found in \a dest.
 */
ValueObject *updateScopeValue(ScopeObject *src,
															ScopeObject *dest,
															IdentifierNode *target,
															ValueObject *value)
{
	ScopeObject *parent = dest;
	IdentifierNode *child = target;
	int status;
	char *name = NULL;

	/* Traverse the target to the terminal child and parent */
	status = resolveTerminalSlot(src, dest, target, &parent, &child);
	if (!status)
		goto updateScopeValueAbort;

	/* Look up the identifier name */
	name = resolveIdentifierName(child, src);
	if (!name)
		goto updateScopeValueAbort;

	/* Traverse upwards through scopes */
	do
	{
		unsigned int n;
		/* Check for existing value in current scope */
		for (n = 0; n < parent->numvals; n++)
		{
			if (!strcmp(parent->names[n], name))
			{
				free(name);
				/* Wipe out the old value */
				deleteValueObject(parent->values[n]);
				/* Assign the new value */
				if (value)
				{
					parent->values[n] = value;
				}
				else
				{
					parent->values[n] = createNilValueObject();
				}
				return parent->values[n];
			}
		}
	} while ((parent = parent->parent));

	{
		char *name = resolveIdentifierName(target, src);
		lulz_error(IN_UNABLE_TO_STORE_VARIABLE, target->fname, target->line, name);
		free(name);
	}

updateScopeValueAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (name)
		free(name);

	return NULL;
}

/**
 * Gets a stored value in a scope.
 *
 * \param [in] src The scope to evaluate \a target under.
 *
 * \param [in,out] dest The scope to update the value in.
 *
 * \param [in] target The name of the value to get.
 *
 * \return The value in \a dest, named by evaluating \a target under \a src.
 *
 * \retval NULL Either \a target could not be evaluated in \a src or \a target
 * could not be found in \a dest.
 */
ValueObject *getScopeValue(ScopeObject *src,
													 ScopeObject *dest,
													 IdentifierNode *target)
{
	ScopeObject *parent = dest;
	IdentifierNode *child = target;
	char *name = NULL;
	int status;

	/* Traverse the target to the terminal child and parent */
	status = resolveTerminalSlot(src, dest, target, &parent, &child);
	if (!status)
		goto getScopeValueAbort;

	/* Look up the identifier name */
	name = resolveIdentifierName(child, src);
	if (!name)
		goto getScopeValueAbort;

	/* Traverse upwards through scopes */
	do
	{
		unsigned int n;
		/* Check for value in current scope */
		for (n = 0; n < parent->numvals; n++)
		{
			if (!strcmp(parent->names[n], name))
			{
				free(name);
				return parent->values[n];
			}
		}
	} while ((parent = parent->parent));

	{
		char *name = resolveIdentifierName(child, src);
		lulz_error(IN_VARIABLE_DOES_NOT_EXIST, child->fname, child->line, name);
		free(name);
	}

getScopeValueAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (name)
		free(name);

	return NULL;
}

/**
 * Gets a scope without accessing any arrays.
 *
 * \param [in] src The scope to evaluate \a target under.
 *
 * \param [in,out] dest The scope to update the value in.
 *
 * \param [in] target The name of the value containing the scope to get.
 *
 * \return The scope contained in the value in \a dest, named by evaluating \a
 * target under \a src, without accessing any arrays.
 *
 * \retval NULL Either \a target could not be evaluated in \a src or \a target
 * could not be found in \a dest.
 */
/** \todo Add this definition to interpreter.h */
ScopeObject *getScopeObjectLocal(ScopeObject *src,
																 ScopeObject *dest,
																 IdentifierNode *target)
{
	ScopeObject *current = dest;
	char *name = NULL;

	/* Look up the identifier name */
	name = resolveIdentifierName(target, src);
	if (!name)
		goto getScopeObjectLocalAbort;

	/* Check for calling object reference variable */
	if (!strcmp(name, "ME"))
	{
		/* Traverse upwards through callers */
		for (current = dest;
				 current->caller;
				 current = current->caller)
			;
		free(name);
		return current;
	}

	/* Traverse upwards through scopes */
	do
	{
		unsigned int n;
		/* Check for value in current scope */
		for (n = 0; n < current->numvals; n++)
		{
			if (!strcmp(current->names[n], name))
			{
				if (current->values[n]->type != VT_ARRAY)
				{
					lulz_error(IN_VARIABLE_NOT_AN_ARRAY, target->fname, target->line, name);
					goto getScopeObjectLocalAbort;
				}
				free(name);
				return getArray(current->values[n]);
			}
		}
	} while ((current = current->parent));

	{
		char *name = resolveIdentifierName(target, src);
		lulz_error(IN_VARIABLE_DOES_NOT_EXIST, target->fname, target->line, name);
		free(name);
	}

getScopeObjectLocalAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (name)
		free(name);

	return NULL;
}

/**
 * Gets a scope (possibly by casting a function) without accessing any arrays.
 *
 * \param [in] src The scope to evaluate \a target under.
 *
 * \param [in,out] dest The scope to update the value in.
 *
 * \param [in] target The name of the value containing the scope to get.
 *
 * \return The scope contained in the value in \a dest, named by evaluating \a
 * target under \a src, without accessing any arrays.
 *
 * \retval NULL Either \a target could not be evaluated in \a src or \a target
 * could not be found in \a dest.
 */
/** \todo Add this definition to interpreter.h */
ScopeObject *getScopeObjectLocalCaller(ScopeObject *src,
																			 ScopeObject *dest,
																			 IdentifierNode *target)
{
	ScopeObject *current = dest;
	char *name = NULL;

	/* Look up the identifier name */
	name = resolveIdentifierName(target, src);
	if (!name)
		goto getScopeObjectLocalCallerAbort;

	/* Check for calling object reference variable */
	if (!strcmp(name, "ME"))
	{
		/* Traverse upwards through callers */
		for (current = dest;
				 current->caller;
				 current = current->caller)
			;
		free(name);
		return current;
	}

	/* Traverse upwards through scopes */
	do
	{
		unsigned int n;
		/* Check for value in current scope */
		for (n = 0; n < current->numvals; n++)
		{
			if (!strcmp(current->names[n], name))
			{
				if (current->values[n]->type != VT_ARRAY && current->values[n]->type != VT_FUNC)
				{
					lulz_error(IN_VARIABLE_NOT_AN_ARRAY, target->fname, target->line, name);
					goto getScopeObjectLocalCallerAbort;
				}
				free(name);
				if (current->values[n]->type == VT_ARRAY)
				{
					return getArray(current->values[n]);
				}
				else
				{
					return dest;
				}
			}
		}
	} while ((current = current->parent));

	{
		char *name = resolveIdentifierName(target, src);
		lulz_error(IN_VARIABLE_DOES_NOT_EXIST, target->fname, target->line, name);
		free(name);
	}

getScopeObjectLocalCallerAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (name)
		free(name);

	return NULL;
}

/**
 * Gets a value from a scope without accessing its ancestors.
 *
 * \param [in] src The scope to evaluate \a target under.
 *
 * \param [in,out] dest The scope to update the value in.
 *
 * \param [in] target The name of the value to get.
 *
 * \return The value in \a dest, named by evaluating \a target under \a src,
 * without accessing any ancestors of \a dest.
 *
 * \retval NULL Either \a target could not be evaluated in \a src or \a target
 * could not be found in \a dest.
 */
ValueObject *getScopeValueLocal(ScopeObject *src,
																ScopeObject *dest,
																IdentifierNode *target)
{
	unsigned int n;
	char *name = NULL;
	ScopeObject *scope = NULL;

	/* Access any slots */
	while (target->slot)
	{
		/*
		 * Look up the target in the dest scope, using the src scope
		 * for resolving variables in indirect identifiers
		 */
		scope = getScopeObjectLocal(src, dest, target);
		if (!scope)
			return 0;
		dest = scope;

		target = target->slot;
	}

	/* Look up the identifier name */
	name = resolveIdentifierName(target, src);
	if (!name)
		goto getScopeValueLocalAbort;

	/* Check for value in current scope */
	for (n = 0; n < dest->numvals; n++)
	{
		if (!strcmp(dest->names[n], name))
		{
			free(name);
			return dest->values[n];
		}
	}

getScopeValueLocalAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (name)
		free(name);

	return NULL;
}

/**
 * Gets a scope from within another scope.
 *
 * \param [in] src The scope to evaluate \a target under.
 *
 * \param [in,out] dest The scope to update the value in.
 *
 * \param [in] target The name of the scope to get.
 *
 * \return The value in \a dest, named by evaluating \a target under \a src,
 * without accessing any ancestors of \a dest.
 *
 * \retval NULL Either \a target could not be evaluated in \a src or \a target
 * could not be found in \a dest.
 */
ScopeObject *getScopeObject(ScopeObject *src,
														ScopeObject *dest,
														IdentifierNode *target)
{
	ValueObject *val = NULL;
	char *name = NULL;
	int isI;
	int isME;
	ScopeObject *scope;

	/* Look up the identifier name */
	name = resolveIdentifierName(target, src);
	if (!name)
		goto getScopeObjectAbort;

	/* Check for targets with special meanings */
	isI = strcmp(name, "I");
	isME = strcmp(name, "ME");
	free(name);
	name = NULL;

	if (!isI)
	{
		/* The function scope variable */
		return src;
	}
	else if (!isME)
	{
		/* The calling object scope variable */
		scope = getScopeObjectLocal(src, dest, target);
		if (!scope)
			goto getScopeObjectAbort;
		return scope;
	}

	/* Access any slots */
	while (target->slot)
	{
		/*
		 * Look up the target in the dest scope, using the src scope
		 * for resolving variables in indirect identifiers
		 */
		scope = getScopeObjectLocal(src, dest, target);
		if (!scope)
			goto getScopeObjectAbort;
		dest = scope;

		target = target->slot;
	}

	val = getScopeValue(src, dest, target);
	if (!val)
		goto getScopeObjectAbort;
	if (val->type != VT_ARRAY)
	{
		char *name = resolveIdentifierName(target, src);
		lulz_error(IN_VARIABLE_NOT_AN_ARRAY, target->fname, target->line, name);
		free(name);
		goto getScopeObjectAbort;
	}

	return getArray(val);

getScopeObjectAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (name)
		free(name);

	return NULL;
}

/**
 * Deletes a value from a scope.
 *
 * \param [in] src The scope to evaluate \a target under.
 *
 * \param [in,out] dest The scope to update the value in.
 *
 * \param [in] target The name of the value to delete.
 */
void deleteScopeValue(ScopeObject *src,
											ScopeObject *dest,
											IdentifierNode *target)
{
	ScopeObject *current = NULL;
	char *name = NULL;
	void *mem1 = NULL;
	void *mem2 = NULL;
	ScopeObject *scope = NULL;

	/* Access any slots */
	while (target->slot)
	{
		/*
		 * Look up the target in the dest scope, using the src scope
		 * for resolving variables in indirect identifiers
		 */
		scope = getScopeObjectLocal(src, dest, target);
		if (!scope)
			goto deleteScopeValueAbort;
		dest = scope;
		target = target->slot;
	}
	current = dest;

	/* Look up the identifier name */
	name = resolveIdentifierName(target, src);
	if (!name)
		goto deleteScopeValueAbort;

	/* Traverse upwards through scopes */
	do
	{
		unsigned int n;
		/* Check for existing value in current scope */
		for (n = 0; n < current->numvals; n++)
		{
			if (!strcmp(current->names[n], name))
			{
				unsigned int i;
				unsigned int newnumvals = dest->numvals - 1;
				free(name);
				/* Wipe out the name and value */
				free(current->names[n]);
				deleteValueObject(current->values[n]);
				/* Reorder the tables */
				for (i = n; i < current->numvals - 1; i++)
				{
					current->names[i] = current->names[i + 1];
					current->values[i] = current->values[i + 1];
				}
				/* Resize the tables */
				mem1 = util_heap_realloc_ext(dest->names, sizeof(IdentifierNode *) * newnumvals);
				if (!mem1)
				{
					ESP_LOGD(TAG, "realloc mem1 error in %s", __func__);
					goto deleteScopeValueAbort;
				}
				mem2 = util_heap_realloc_ext(dest->values, sizeof(ValueObject *) * newnumvals);
				if (!mem2)
				{
					ESP_LOGD(TAG, "realloc mem2 error in %s", __func__);
					goto deleteScopeValueAbort;
				}
				dest->names = mem1;
				dest->values = mem2;
				dest->numvals = newnumvals;
				return;
			}
		}
	} while ((current = current->parent));

	free(name);

	return;

deleteScopeValueAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (name)
		free(name);
	if (mem1)
		free(mem1);
	if (mem2)
		free(mem2);
	if (scope)
		free(scope);

	return;
}

/**
 * Creates a returned value.
 *
 * \param [in] type The type of returned value.
 *
 * \param [in] value An optional value to return.
 *
 * \return A pointer to a returned value with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
ReturnObject *createReturnObject(ReturnType type,
																 ValueObject *value)
{
	ReturnObject *p = util_heap_alloc_ext(sizeof(ReturnObject));
	if (!p)
	{
		perror("util_heap_alloc_ext");
		return NULL;
	}
	p->type = type;
	p->value = value;
	return p;
}

/**
 * Deletes a returned value.
 *
 * \param [in,out] object The returned value to be deleted.
 *
 * \post The memory at \a object and all of its members will be freed.
 */
void deleteReturnObject(ReturnObject *object)
{
	if (!object)
		return;
	if (object->type == RT_RETURN)
		deleteValueObject(object->value);
	free(object);
}

/**
 * Starting from an initial parent scope and target identifier, traverses down
 * until the target identifier is not a scope.  Stores the value of the terminal
 * child identifier and its parent scope.
 *
 * \param [in] src The scope to resolve \a target in.
 *
 * \param [in] dest The scope to retrieve \a target from.
 *
 * \param [in] target The array slot to traverse.
 *
 * \param [out] parent The parent of the terminal child identifier of \a target.
 *
 * \param [out] child The terminal child identifier of \a target.
 *
 * \return A status code indicating success or failure.
 *
 * \retval 0 Failed to traverse array.
 *
 * \retval 1 Succeeded to traverse array.
 *
 * \post \a parent will point to the parent scope containing the terminal child.
 *
 * \post \a child will point to the terminal child.
 */
int resolveTerminalSlot(ScopeObject *src,
												ScopeObject *dest,
												IdentifierNode *target,
												ScopeObject **parent,
												IdentifierNode **child)
{
	ScopeObject *scope = NULL;

	/* Start with default values */
	*parent = dest;
	*child = target;

	/* Access any slots */
	while (target->slot)
	{
		/*
		 * Look up the target in the dest scope, using the src scope
		 * for resolving variables in indirect identifiers
		 */
		scope = getScopeObjectLocal(src, dest, target);
		if (!scope)
			goto resolveTerminalSlotAbort;
		dest = scope;

		/* Change the target to the old target's slot */
		target = target->slot;
	}

	/* Store the output values */
	*parent = dest;
	*child = target;

	return 1;

resolveTerminalSlotAbort: /* In case something goes wrong... */

	/* Clean up any allocated structures */
	if (scope)
		deleteScopeObject(scope);

	return 0;
}

/**
 * Casts the contents of a value to boolean type in an implicit way.  Casting is
 * not done directly to \a node, instead, it is performed on a copy which is
 * what is returned.
 *
 * \param [in] node The value to cast.
 * 
 * \param [in] scope The scope to use for variable interpolation.
 *
 * \return A pointer to a value with a copy of the contents of \a node, cast to
 * boolean type.
 *
 * \retval NULL An error occurred while casting.
 */
ValueObject *castBooleanImplicit(ValueObject *node,
																 ScopeObject *scope)
{
	if (!node)
		return NULL;
	return castBooleanExplicit(node, scope);
}

/**
 * Casts the contents of a value to integer type in an implicit way.  Casting is
 * not done directly to \a node, instead, it is performed on a copy which is
 * what is returned.
 *
 * \param [in] node The value to cast.
 * 
 * \param [in] scope The scope to use for variable interpolation.
 *
 * \return A pointer to a value with a copy of the contents of \a node, cast to
 * integer type.
 *
 * \retval NULL An error occurred while casting.
 */
ValueObject *castIntegerImplicit(ValueObject *node,
																 ScopeObject *scope)
{
	if (!node)
		return NULL;
	if (node->type == VT_NIL)
	{
		lulz_error(IN_CANNOT_IMPLICITLY_CAST_NIL);
		return NULL;
	}
	else
		return castIntegerExplicit(node, scope);
}

/**
 * Casts the contents of a value to decimal type in an implicit way.  Casting is
 * not done directly to \a node, instead, it is performed on a copy which is
 * what is returned.
 *
 * \param [in] node The value to cast.
 * 
 * \param [in] scope The scope to use for variable interpolation.
 *
 * \return A pointer to a value with a copy of the contents of \a node, cast to
 * decimal type.
 *
 * \retval NULL An error occurred while casting.
 */
ValueObject *castFloatImplicit(ValueObject *node,
															 ScopeObject *scope)
{
	if (!node)
		return NULL;
	if (node->type == VT_NIL)
	{
		lulz_error(IN_CANNOT_IMPLICITLY_CAST_NIL);
		return NULL;
	}
	else
		return castFloatExplicit(node, scope);
}

/**
 * Casts the contents of a value to string type in an implicit way.  Casting is
 * not done directly to \a node, instead, it is performed on a copy which is
 * what is returned.
 *
 * \param [in] node The value to cast.
 * 
 * \param [in] scope The scope to use for variable interpolation.
 *
 * \note \a scope is used to resolve variable interpolation within the string
 * before casting it.  Therefore, a simple way to interpolate the variables
 * within a string is to call this function with it.
 *
 * \return A pointer to a value with a copy of the contents of \a node, cast to
 * string type.
 *
 * \retval NULL An error occurred while casting.
 */
ValueObject *castStringImplicit(ValueObject *node,
																ScopeObject *scope)
{
	if (!node)
		return NULL;
	if (node->type == VT_NIL)
	{
		lulz_error(IN_CANNOT_IMPLICITLY_CAST_NIL);
		return NULL;
	}
	else
		return castStringExplicit(node, scope);
}

/**
 * Casts the contents of a value to boolean type in an explicit way.  Casting is
 * not done directly to \a node, instead, it is performed on a copy which is
 * what is returned.
 *
 * \param [in] node The value to cast.
 * 
 * \param [in] scope The scope to use for variable interpolation.
 *
 * \return A pointer to a value with a copy of the contents of \a node, cast to
 * boolean type.
 *
 * \retval NULL An error occurred while casting.
 */
ValueObject *castBooleanExplicit(ValueObject *node,
																 ScopeObject *scope)
{
	if (!node)
		return NULL;
	switch (node->type)
	{
	case VT_NIL:
		return createBooleanValueObject(0);
	case VT_BOOLEAN:
		return createBooleanValueObject(getInteger(node));
	case VT_INTEGER:
		return createBooleanValueObject(getInteger(node) != 0);
	case VT_FLOAT:
		return createBooleanValueObject(fabs(getFloat(node) - 0.0) > FLT_EPSILON);
	case VT_STRING:
		if (strstr(getString(node), ":{"))
		{
			/* Perform interpolation */
			ValueObject *ret = NULL;
			ValueObject *interp = castStringExplicit(node, scope);
			if (!interp)
				return NULL;
			ret = createBooleanValueObject(getString(interp)[0] != '\0');
			deleteValueObject(interp);
			return ret;
		}
		else
			return createBooleanValueObject(getString(node)[0] != '\0');
	case VT_FUNC:
		lulz_error(IN_CANNOT_CAST_FUNCTION_TO_BOOLEAN);
		return NULL;
	case VT_ARRAY:
		lulz_error(IN_CANNOT_CAST_ARRAY_TO_BOOLEAN);
		return NULL;
	default:
		lulz_error(IN_UNKNOWN_VALUE_DURING_BOOLEAN_CAST);
		return NULL;
	}
}

/**
 * Casts the contents of a value to integer type in an explicit way.  Casting is
 * not done directly to \a node, instead, it is performed on a copy which is
 * what is returned.
 *
 * \param [in] node The value to cast.
 * 
 * \param [in] scope The scope to use for variable interpolation.
 *
 * \return A pointer to a value with a copy of the contents of \a node, cast to
 * integer type.
 *
 * \retval NULL An error occurred while casting.
 */
ValueObject *castIntegerExplicit(ValueObject *node,
																 ScopeObject *scope)
{
	if (!node)
		return NULL;
	switch (node->type)
	{
	case VT_NIL:
		return createIntegerValueObject(0);
	case VT_BOOLEAN:
	case VT_INTEGER:
		return createIntegerValueObject(getInteger(node));
	case VT_FLOAT:
		return createIntegerValueObject((long long)getFloat(node));
	case VT_STRING:
		if (strstr(getString(node), ":{"))
		{
			/* Perform interpolation */
			ValueObject *ret = NULL;
			ValueObject *interp = castStringExplicit(node, scope);
			if (!interp)
				return NULL;
			long long value = strtoll(getString(interp), NULL, 0);
			ret = createIntegerValueObject(value);
			deleteValueObject(interp);
			return ret;
		}
		else
		{
			long long value = strtoll(getString(node), NULL, 0);
			return createIntegerValueObject(value);
		}
	case VT_FUNC:
		lulz_error(IN_CANNOT_CAST_FUNCTION_TO_INTEGER);
		return NULL;
	case VT_ARRAY:
		lulz_error(IN_CANNOT_CAST_ARRAY_TO_INTEGER);
		return NULL;
	default:
		lulz_error(IN_UNKNOWN_VALUE_DURING_INTEGER_CAST);
		return NULL;
	}
}

/**
 * Casts the contents of a value to decimal type in an explicit way.  Casting is
 * not done directly to \a node, instead, it is performed on a copy which is
 * what is returned.
 *
 * \param [in] node The value to cast.
 * 
 * \param [in] scope The scope to use for variable interpolation.
 *
 * \return A pointer to a value with a copy of the contents of \a node, cast to
 * decimal type.
 *
 * \retval NULL An error occurred while casting.
 */
ValueObject *castFloatExplicit(ValueObject *node,
															 ScopeObject *scope)
{
	if (!node)
		return NULL;
	switch (node->type)
	{
	case VT_NIL:
		return createFloatValueObject(0.0);
	case VT_BOOLEAN:
	case VT_INTEGER:
		return createFloatValueObject((float)getInteger(node));
	case VT_FLOAT:
		return createFloatValueObject(getFloat(node));
	case VT_STRING:
		if (strstr(getString(node), ":{"))
		{
			/* Perform interpolation */
			ValueObject *ret = NULL;
			ValueObject *interp = castStringExplicit(node, scope);
			if (!interp)
				return NULL;
			float value = strtof(getString(interp), NULL);
			ret = createFloatValueObject(value);
			deleteValueObject(interp);
			return ret;
		}
		else
		{
			float value = strtof(getString(node), NULL);
			return createFloatValueObject(value);
		}
	case VT_FUNC:
		lulz_error(IN_CANNOT_CAST_FUNCTION_TO_DECIMAL);
		return NULL;
	case VT_ARRAY:
		lulz_error(IN_CANNOT_CAST_ARRAY_TO_DECIMAL);
		return NULL;
	default:
		lulz_error(IN_UNKNOWN_VALUE_DURING_DECIMAL_CAST);
		return NULL;
	}
}

/**
 * Casts the contents of a value to string type in an explicit way.  Casting is
 * not done directly to \a node, instead, it is performed on a copy which is
 * what is returned.
 *
 * \param [in] node The value to cast.
 * 
 * \param [in] scope The scope to use for variable interpolation.
 *
 * \note \a scope is used to resolve variable interpolation within the string
 * before casting it.  Therefore, a simple way to interpolate the variables
 * within a string is to call this function with it.
 *
 * \return A pointer to a value with a copy of the contents of \a node, cast to
 * string type.
 *
 * \retval NULL An error occurred while casting.
 */
ValueObject *castStringExplicit(ValueObject *node,
																ScopeObject *scope)
{
	if (!node)
		return NULL;
	switch (node->type)
	{
	case VT_NIL:
	{
		char *str = copyString("");
		if (!str)
			return NULL;
		return createStringValueObject(str);
	}
	case VT_BOOLEAN:
	{
		/*
		 * \note The spec does not define how TROOFs may be cast
		 * to YARNs.
		 */
		if (node->data.i > 0)
		{
			return createStringValueObject(copyString("1"));
		}
		else
		{
			return createStringValueObject(copyString("0"));
		}
		//		lulz_error(IN_CANNOT_CAST_BOOLEAN_TO_STRING);
		//		return NULL;
	}
	case VT_INTEGER:
	{
		char *data = NULL;
		/*
		 * One character per integer bit plus one more for the
		 * null character
		 */
		size_t size = sizeof(long long) * 8 + 1;
		data = util_heap_alloc_ext(sizeof(char) * size);
		if (!data)
			return NULL;
		sprintf(data, "%lli", getInteger(node));
		return createStringValueObject(data);
	}
	case VT_FLOAT:
	{
		char *data = NULL;
		unsigned int precision = 2;
		/*
		 * One character per float bit plus one more for the
		 * null character
		 */
		size_t size = sizeof(float) * 8 + 1;
		data = util_heap_alloc_ext(sizeof(char) * size);
		if (!data)
			return NULL;
		sprintf(data, "%f", getFloat(node));
		/* Truncate to a certain number of decimal places */
		strchr(data, '.')[precision + 1] = '\0';
		return createStringValueObject(data);
	}
	case VT_STRING:
	{
		char *temp = NULL;
		char *data = NULL;
		char *str = getString(node);
		unsigned int a, b;
		size_t size;
		/* Perform interpolation */
		size = strlen(getString(node)) + 1;
		temp = util_heap_alloc_ext(sizeof(char) * size);
		for (a = 0, b = 0; str[b] != '\0';)
		{
			if (!strncmp(str + b, ":)", 2))
			{
				temp[a] = '\n';
				a++, b += 2;
			}
			else if (!strncmp(str + b, ":>", 2))
			{
				temp[a] = '\t';
				a++, b += 2;
			}
			else if (!strncmp(str + b, ":o", 2))
			{
				temp[a] = '\a';
				a++, b += 2;
			}
			else if (!strncmp(str + b, ":\"", 2))
			{
				temp[a] = '"';
				a++, b += 2;
			}
			else if (!strncmp(str + b, "::", 2))
			{
				temp[a] = ':';
				a++, b += 2;
			}
			//LULZCODE removing unicode support to save 700kb flash
			//			else if (!strncmp(str + b, ":(", 2)) {
			//				const char *start = str + b + 2;
			//				const char *end = strchr(start, ')');
			//				size_t len;
			//				char *image = NULL;
			//				long codepoint;
			//				char out[3];
			//				size_t num;
			//				void *mem = NULL;
			//				if (end < start) {
			//					lulz_error(IN_EXPECTED_CLOSING_PAREN);
			//					free(temp);
			//					return NULL;
			//				}
			//				len = (size_t) (end - start);
			//				image = util_heap_alloc_ext(sizeof(char) * (len + 1));
			//				strncpy(image, start, len);
			//				image[len] = '\0';
			//				if (!isHexString(image)) {
			//					lulz_error(IN_INVALID_HEX_NUMBER);
			//					free(temp);
			//					free(image);
			//					return NULL;
			//				}
			//				codepoint = strtol(image, NULL, 16);
			//				free(image);
			//				if (codepoint < 0) {
			//					lulz_error(IN_CODE_POINT_MUST_BE_POSITIVE);
			//					free(temp);
			//					return NULL;
			//				}
			//				num = convertCodePointToUTF8((unsigned int) codepoint, out);
			//				if (num == 0) {
			//					free(temp);
			//					return NULL;
			//				}
			//				size += num;
			//				mem = util_heap_realloc_ext(temp, size);
			//				if (!mem) {
			//					perror("realloc");
			//					free(temp);
			//					return NULL;
			//				}
			//				temp = mem;
			//				strncpy(temp + a, out, num);
			//				a += num, b += len + 3;
			//			}
			//			else if (!strncmp(str + b, ":[", 2)) {
			//				const char *start = str + b + 2;
			//				const char *end = strchr(start, ']');
			//				size_t len;
			//				char *image = NULL;
			//				long codepoint;
			//				char out[3];
			//				size_t num;
			//				void *mem = NULL;
			//				if (end < start) {
			//					lulz_error(IN_EXPECTED_CLOSING_SQUARE_BRACKET);
			//					free(temp);
			//					return NULL;
			//				}
			//				len = (size_t) (end - start);
			//				image = util_heap_alloc_ext(sizeof(char) * (len + 1));
			//				strncpy(image, start, len);
			//				strncpy(image, start, len);
			//				image[len] = '\0';
			//				codepoint = convertNormativeNameToCodePoint(image);
			//				free(image);
			//				if (codepoint < 0) {
			//					lulz_error(IN_CODE_POINT_MUST_BE_POSITIVE);
			//					free(temp);
			//					return NULL;
			//				}
			//				num = convertCodePointToUTF8((unsigned int) codepoint, out);
			//				size += num;
			//				mem = util_heap_realloc_ext(temp, size);
			//				if (!mem) {
			//					perror("realloc");
			//					free(temp);
			//					return NULL;
			//				}
			//				temp = mem;
			//				strncpy(temp + a, out, num);
			//				a += num, b += len + 3;
			//			}
			else if (!strncmp(str + b, ":{", 2))
			{
				IdentifierNode *target = NULL;
				ValueObject *val = NULL, *use = NULL;
				/* Copy the variable name into image */
				const char *start = str + b + 2;
				const char *end = strchr(start, '}');
				size_t len;
				char *image = NULL;
				void *mem = NULL;
				if (end < start)
				{
					lulz_error(IN_EXPECTED_CLOSING_CURLY_BRACE);
					free(temp);
					return NULL;
				}
				len = (size_t)(end - start);
				image = util_heap_alloc_ext(sizeof(char) * (len + 1));
				strncpy(image, start, len);
				image[len] = '\0';
				if (!strcmp(image, "IT"))
					/* Lookup implicit variable */
					val = scope->impvar;
				else
				{
					/*
					 * Create a new IdentifierNode
					 * structure and look up its
					 * value
					 */
					target = createIdentifierNode(IT_DIRECT, image, NULL, NULL, 0);
					if (!target)
					{
						free(temp);
						return NULL;
					}
					val = getScopeValue(scope, scope, target);
					if (!val)
					{
						lulz_error(IN_VARIABLE_DOES_NOT_EXIST, target->fname, target->line, image);
						deleteIdentifierNode(target);
						free(temp);
						return NULL;
					}
					deleteIdentifierNode(target);
				}
				/* Cast the variable value to a string */
				if (!(use = castStringImplicit(val, scope)))
				{
					free(temp);
					return NULL;
				}
				/* Update the size of the new string */
				size += strlen(getString(use));
				mem = util_heap_realloc_ext(temp, size);
				if (!mem)
				{
					ESP_LOGD(TAG, "realloc mem2 error in %s", __func__);
					free(temp);
				}
				temp = mem;
				/* Copy the variable string into the new string */
				strcpy(temp + a, getString(use));
				a += strlen(getString(use)), b += len + 3;
				deleteValueObject(use);
			}
			else
			{
				temp[a] = str[b];
				a++, b++;
			}
		}
		temp[a] = '\0';
		data = util_heap_alloc_ext(sizeof(char) * (strlen(temp) + 1));
		strcpy(data, temp);
		free(temp);
		return createStringValueObject(data);
	}
	case VT_FUNC:
	{
		lulz_error(IN_CANNOT_CAST_FUNCTION_TO_STRING);
		return NULL;
	}
	case VT_ARRAY:
		lulz_error(IN_CANNOT_CAST_ARRAY_TO_STRING);
		return NULL;
	default:
		lulz_error(IN_UNKNOWN_VALUE_DURING_STRING_CAST);
		return NULL;
	}
}

/**
 * Interprets an implicit variable.
 *
 * \param [in] node Not used (see note).
 *
 * \param [in] scope The scope from which to use the implicit variable.
 *
 * \note \a node is not used by this function but is still included in its
 * prototype to allow this function to be stored in a jump table for fast
 * execution.
 *
 * \return A pointer to the value of \a scope's implicit variable.
 */
ValueObject *interpretImpVarExprNode(ExprNode *node, ScopeObject *scope)
{
	return scope->impvar;
}

/**
 * Interprets a cast.
 *
 * \param [in] node A pointer to the expression to interpret.
 *
 * \param [in] scope A pointer to a scope to evaluate \a node under.
 *
 * \pre \a node contains a expression created by createCastExprNode().
 *
 * \return A pointer to the cast value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretCastExprNode(ExprNode *node,
																	 ScopeObject *scope)
{
	CastExprNode *expr = (CastExprNode *)node->expr;
	ValueObject *val = interpretExprNode(expr->target, scope);
	ValueObject *ret = NULL;
	if (!val)
		return NULL;
	switch (expr->newtype->type)
	{
	case CT_NIL:
		deleteValueObject(val);
		return createNilValueObject();
	case CT_BOOLEAN:
		ret = castBooleanExplicit(val, scope);
		deleteValueObject(val);
		return ret;
	case CT_INTEGER:
		ret = castIntegerExplicit(val, scope);
		deleteValueObject(val);
		return ret;
	case CT_FLOAT:
		ret = castFloatExplicit(val, scope);
		deleteValueObject(val);
		return ret;
	case CT_STRING:
		ret = castStringExplicit(val, scope);
		deleteValueObject(val);
		return ret;
	case CT_CHAR:;
		ValueObject *temp = castIntegerExplicit(val, scope);
		if (getInteger(temp) <= 255 && getInteger(temp) >= 0)
		{
			char *str = util_heap_alloc_ext(2);
			str[0] = (char)getInteger(temp);
			str[1] = 0;
			ret = createStringValueObject(str);
		}

		deleteValueObject(temp);
		deleteValueObject(val);
		return ret;
	default:
		lulz_error(IN_UNKNOWN_CAST_TYPE);
		deleteValueObject(val);
		return NULL;
	}
}

/**
 * Interprets a function call.
 *
 * \param [in] node A pointer to the expression to interpret.
 *
 * \param [in,out] scope A pointer to a scope to evaluate \a node under.
 *
 * \pre \a node contains an expression created by createFuncCallExprNode().
 *
 * \return A pointer to the returned value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretFuncCallExprNode(ExprNode *node, ScopeObject *scope)
{
	FuncCallExprNode *expr = (FuncCallExprNode *)node->expr;
	unsigned int n;
	ScopeObject *outer = NULL;
	ValueObject *def = NULL;
	ReturnObject *retval = NULL;
	ValueObject *ret = NULL;
	ScopeObject *dest = NULL;
	ScopeObject *target = NULL;

	dest = getScopeObject(scope, scope, expr->scope);

	target = getScopeObjectLocalCaller(scope, dest, expr->name);
	if (!target)
		return NULL;

	outer = createScopeObjectCaller(scope, target);
	if (!outer)
		return NULL;

	def = getScopeValue(scope, dest, expr->name);

	if (!def || def->type != VT_FUNC)
	{
		IdentifierNode *id = (IdentifierNode *)(expr->name);
		char *name = resolveIdentifierName(id, scope);
		if (name)
		{
			lulz_error(IN_UNDEFINED_FUNCTION, id->fname, id->line, name);
			free(name);
		}
		deleteScopeObject(outer);
		return NULL;
	}
	/* Check for correct supplied arity */
	if (getFunction(def)->args->num != expr->args->num)
	{
		IdentifierNode *id = (IdentifierNode *)(expr->name);
		char *name = resolveIdentifierName(id, scope);
		if (name)
		{
			lulz_error(IN_INCORRECT_NUMBER_OF_ARGUMENTS, id->fname, id->line, name);
			free(name);
		}
		deleteScopeObject(outer);
		return NULL;
	}
	for (n = 0; n < getFunction(def)->args->num; n++)
	{
		ValueObject *val = NULL;
		if (!createScopeValue(scope, outer, getFunction(def)->args->ids[n]))
		{
			deleteScopeObject(outer);
			return NULL;
		}
		if (!(val = interpretExprNode(expr->args->exprs[n], scope)))
		{
			deleteScopeObject(outer);
			return NULL;
		}
		if (!updateScopeValue(scope, outer, getFunction(def)->args->ids[n], val))
		{
			deleteScopeObject(outer);
			deleteValueObject(val);
			return NULL;
		}
	}
	/**
	 * \note We use interpretStmtNodeList here because we want to have
	 * access to the function's scope as we may need to retrieve the
	 * implicit variable in the case of a default return.
	 */
	if (!(retval = interpretStmtNodeList(getFunction(def)->body->stmts, outer)))
	{
		deleteScopeObject(outer);
		return NULL;
	}
	switch (retval->type)
	{
	case RT_DEFAULT:
		/* Extract return value */
		ret = outer->impvar;
		outer->impvar = NULL;
		break;
	case RT_BREAK:
		ret = createNilValueObject();
		break;
	case RT_RETURN:
		/* Extract return value */
		ret = retval->value;
		retval->value = NULL;
		break;
	default:
		lulz_error(IN_INVALID_RETURN_TYPE);
		break;
	}
	deleteReturnObject(retval);
	deleteScopeObject(outer);
	return ret;
}

/**
 * Interprets an identifier.
 *
 * \param [in] node A pointer to the expression to interpret.
 *
 * \param [in,out] scope A pointer to a scope to evaluate \a node under.
 *
 * \pre \a node contains an identifier created by createIdentifierNode().
 *
 * \return A pointer to the cast value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretIdentifierExprNode(ExprNode *node,
																				 ScopeObject *scope)
{
	ValueObject *val = getScopeValue(scope, scope, node->expr);
	if (!val)
		return NULL;
	return copyValueObject(val);
}

/**
 * Interprets a constant.
 *
 * \param [in] node A pointer to the expression to interpret.
 *
 * \param [in] scope Not used (see note).
 *
 * \note \a node is not used by this function but is still included in its
 * prototype to allow this function to be stored in a jump table for fast
 * execution.
 *
 * \pre \a node contains a constant created by createXConstantNode(), where X is
 * either Boolean, Integer, Float, or String.
 *
 * \return A pointer to the constant value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretConstantExprNode(ExprNode *node, ScopeObject *scope)
{
	ConstantNode *expr = (ConstantNode *)node->expr;
	switch (expr->type)
	{
	case CT_NIL:
		return createNilValueObject();
	case CT_BOOLEAN:
		return createBooleanValueObject(expr->data.i);
	case CT_INTEGER:
		return createIntegerValueObject(expr->data.i);
	case CT_FLOAT:
		return createFloatValueObject(expr->data.f);
	case CT_STRING:
	{
		/*
		 * \note For efficiency, string interpolation should be
		 * performed by caller because it only needs to be
		 * performed when necessary.
		 */
		char *str = copyString(expr->data.s);
		if (!str)
			return NULL;
		return createStringValueObject(str);
	}
	default:
		lulz_error(IN_UNKNOWN_CONSTANT_TYPE);
		return NULL;
	}
}

/**
 * Interprets a logical NOT operation.
 *
 * \param [in] expr A pointer to the expression to interpret.
 *
 * \param [in] scope A pointer to a scope to evaluate \a node under.
 *
 * \note Only the first element of \a args is used.
 *
 * \return A pointer to the value of the logical negation of the first element
 * of \a args.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretNotOpExprNode(OpExprNode *expr,
																		ScopeObject *scope)
{
	ValueObject *val = interpretExprNode(expr->args->exprs[0], scope);
	ValueObject *use = val;
	int retval;
	unsigned short cast = 0;
	if (!val)
		return NULL;
	if (val->type != VT_BOOLEAN && val->type != VT_INTEGER)
	{
		use = castBooleanImplicit(val, scope);
		if (!use)
		{
			deleteValueObject(val);
			return NULL;
		}
		cast = 1;
	}
	retval = getInteger(use);
	if (cast)
		deleteValueObject(use);
	deleteValueObject(val);
	return createBooleanValueObject(!retval);
}

/**
 * Adds an integer to an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the sum of \a a and \a b.
 */
ValueObject *opAddIntegerInteger(ValueObject *a,
																 ValueObject *b)
{
	return createIntegerValueObject(getInteger(a) + getInteger(b));
}

/**
 * Subtracts an integer from an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the difference of \a a and \a b.
 */
ValueObject *opSubIntegerInteger(ValueObject *a,
																 ValueObject *b)
{
	return createIntegerValueObject(getInteger(a) - getInteger(b));
}

/**
 * Multiplies an integer by an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the product of \a a and \a b.
 */
ValueObject *opMultIntegerInteger(ValueObject *a,
																	ValueObject *b)
{
	return createIntegerValueObject(getInteger(a) * getInteger(b));
}

/**
 * Divides an integer by an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the quotient of \a a and \a b.
 *
 * \retval NULL Division by zero.
 */
ValueObject *opDivIntegerInteger(ValueObject *a,
																 ValueObject *b)
{
	if (getInteger(b) == 0)
	{
		lulz_error(IN_DIVISION_BY_ZERO);
		return NULL;
	}
	return createIntegerValueObject(getInteger(a) / getInteger(b));
}

/**
 * Finds the maximum of an integer and an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the maximum of \a a and \a b.
 */
ValueObject *opMaxIntegerInteger(ValueObject *a,
																 ValueObject *b)
{
	return createIntegerValueObject(getInteger(a) > getInteger(b) ? getInteger(a) : getInteger(b));
}

/**
 * Finds the minimum of an integer and an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the minimum of \a a and \a b.
 */
ValueObject *opMinIntegerInteger(ValueObject *a,
																 ValueObject *b)
{
	return createIntegerValueObject(getInteger(a) < getInteger(b) ? getInteger(a) : getInteger(b));
}

/**
 * Calculates the modulus of an integer and an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the modulus of \a a and \a b.
 */
ValueObject *opModIntegerInteger(ValueObject *a,
																 ValueObject *b)
{
	if (getInteger(b) == 0)
	{
		lulz_error(IN_DIVISION_BY_ZERO);
		return NULL;
	}
	return createIntegerValueObject(getInteger(a) % getInteger(b));
}

/**
 * Adds an integer to a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the sum of \a a and \a b.
 */
ValueObject *opAddIntegerFloat(ValueObject *a,
															 ValueObject *b)
{
	return createFloatValueObject((float)(getInteger(a) + getFloat(b)));
}

/**
 * Subtracts an integer from a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the difference of \a a and \a b.
 */
ValueObject *opSubIntegerFloat(ValueObject *a,
															 ValueObject *b)
{
	return createFloatValueObject((float)(getInteger(a) - getFloat(b)));
}

/**
 * Multiplies an integer by a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the product of \a a and \a b.
 */
ValueObject *opMultIntegerFloat(ValueObject *a,
																ValueObject *b)
{
	return createFloatValueObject((float)(getInteger(a) * getFloat(b)));
}

/**
 * Divides an integer by a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the quotient of \a a and \a b.
 *
 * \retval NULL Division by zero.
 */
ValueObject *opDivIntegerFloat(ValueObject *a,
															 ValueObject *b)
{
	if (fabs(getFloat(b) - 0.0) < FLT_EPSILON)
	{
		lulz_error(IN_DIVISION_BY_ZERO);
		return NULL;
	}
	return createFloatValueObject((float)(getInteger(a) / getFloat(b)));
}

/**
 * Finds the maximum of an integer and a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the maximum of \a a and \a b.
 */
ValueObject *opMaxIntegerFloat(ValueObject *a,
															 ValueObject *b)
{
	return createFloatValueObject((float)(getInteger(a)) > getFloat(b) ? (float)(getInteger(a)) : getFloat(b));
}

/**
 * Finds the minimum of an integer and a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the minimum of \a a and \a b.
 */
ValueObject *opMinIntegerFloat(ValueObject *a,
															 ValueObject *b)
{
	return createFloatValueObject((float)(getInteger(a)) < getFloat(b) ? (float)(getInteger(a)) : getFloat(b));
}

/**
 * Calculates the modulus of an integer and a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the modulus of \a a and \a b.
 */
ValueObject *opModIntegerFloat(ValueObject *a,
															 ValueObject *b)
{
	if (fabs(getFloat(b) - 0.0) < FLT_EPSILON)
	{
		lulz_error(IN_DIVISION_BY_ZERO);
		return NULL;
	}
	return createFloatValueObject((float)(fmod((double)(getInteger(a)), getFloat(b))));
}

/**
 * Adds a decimal to an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the sum of \a a and \a b.
 */
ValueObject *opAddFloatInteger(ValueObject *a,
															 ValueObject *b)
{
	return createFloatValueObject(getFloat(a) + getInteger(b));
}

/**
 * Subtracts a decimal from an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the difference of \a a and \a b.
 */
ValueObject *opSubFloatInteger(ValueObject *a,
															 ValueObject *b)
{
	return createFloatValueObject(getFloat(a) - getInteger(b));
}

/**
 * Multiplies a decimal by an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the product of \a a and \a b.
 */
ValueObject *opMultFloatInteger(ValueObject *a,
																ValueObject *b)
{
	return createFloatValueObject(getFloat(a) * getInteger(b));
}

/**
 * Divides a decimal by an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the quotient of \a a and \a b.
 *
 * \retval NULL Division by zero.
 */
ValueObject *opDivFloatInteger(ValueObject *a,
															 ValueObject *b)
{
	if (getInteger(b) == 0)
	{
		lulz_error(IN_DIVISION_BY_ZERO);
		return NULL;
	}
	return createFloatValueObject(getFloat(a) / getInteger(b));
}

/**
 * Finds the maximum of a decimal and an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the maximum of \a a and \a b.
 */
ValueObject *opMaxFloatInteger(ValueObject *a,
															 ValueObject *b)
{
	return createFloatValueObject(getFloat(a) > (float)(getInteger(b)) ? getFloat(a) : (float)(getInteger(b)));
}

/**
 * Finds the minimum of a decimal and an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the minimum of \a a and \a b.
 */
ValueObject *opMinFloatInteger(ValueObject *a,
															 ValueObject *b)
{
	return createFloatValueObject(getFloat(a) < (float)(getInteger(b)) ? getFloat(a) : (float)(getInteger(b)));
}

/**
 * Calculates the modulus of a decimal and an integer.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the modulus of \a a and \a b.
 */
ValueObject *opModFloatInteger(ValueObject *a,
															 ValueObject *b)
{
	if (getInteger(b) == 0)
	{
		lulz_error(IN_DIVISION_BY_ZERO);
		return NULL;
	}
	return createFloatValueObject((float)(fmod(getFloat(a), (double)(getInteger(b)))));
}

/**
 * Adds a decimal to a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the sum of \a a and \a b.
 */
ValueObject *opAddFloatFloat(ValueObject *a,
														 ValueObject *b)
{
	return createFloatValueObject(getFloat(a) + getFloat(b));
}

/**
 * Subtracts a decimal from a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the difference of \a a and \a b.
 */
ValueObject *opSubFloatFloat(ValueObject *a,
														 ValueObject *b)
{
	return createFloatValueObject(getFloat(a) - getFloat(b));
}

/**
 * Multiplies a decimal by a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the product of \a a and \a b.
 */
ValueObject *opMultFloatFloat(ValueObject *a,
															ValueObject *b)
{
	return createFloatValueObject(getFloat(a) * getFloat(b));
}

/**
 * Divides a decimal by a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the quotient of \a a and \a b.
 *
 * \retval NULL Division by zero.
 */
ValueObject *opDivFloatFloat(ValueObject *a,
														 ValueObject *b)
{
	if (fabs(getFloat(b) - 0.0) < FLT_EPSILON)
	{
		lulz_error(IN_DIVISION_BY_ZERO);
		return NULL;
	}
	return createFloatValueObject(getFloat(a) / getFloat(b));
}

/**
 * Finds the maximum of a decimal and a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the maximum of \a a and \a b.
 */
ValueObject *opMaxFloatFloat(ValueObject *a,
														 ValueObject *b)
{
	return createFloatValueObject(getFloat(a) > getFloat(b) ? getFloat(a) : getFloat(b));
}

/**
 * Finds the minimum of a decimal and a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the minimum of \a a and \a b.
 */
ValueObject *opMinFloatFloat(ValueObject *a,
														 ValueObject *b)
{
	return createFloatValueObject(getFloat(a) < getFloat(b) ? getFloat(a) : getFloat(b));
}

/**
 * Calculates the modulus of a decimal and a decimal.
 *
 * \param [in] a The first operand.
 *
 * \param [in] b The second operand.
 *
 * \return A pointer to the value of the modulus of \a a and \a b.
 */
ValueObject *opModFloatFloat(ValueObject *a,
														 ValueObject *b)
{
	if (fabs(getFloat(b) - 0.0) < FLT_EPSILON)
	{
		lulz_error(IN_DIVISION_BY_ZERO);
		return NULL;
	}
	return createFloatValueObject((float)(fmod(getFloat(a), getFloat(b))));
}

/*
 * A jump table for arithmetic operations.  The first index determines the
 * particular arithmetic operation to perform, the second index determines the
 * type of the first argument, and the third index determines the type of the
 * second object.
 */
static ValueObject *(*ArithOpJumpTable[7][2][2])(ValueObject *, ValueObject *) = {
		{{opAddIntegerInteger, opAddIntegerFloat}, {opAddFloatInteger, opAddFloatFloat}},
		{{opSubIntegerInteger, opSubIntegerFloat}, {opSubFloatInteger, opSubFloatFloat}},
		{{opMultIntegerInteger, opMultIntegerFloat}, {opMultFloatInteger, opMultFloatFloat}},
		{{opDivIntegerInteger, opDivIntegerFloat}, {opDivFloatInteger, opDivFloatFloat}},
		{{opModIntegerInteger, opModIntegerFloat}, {opModFloatInteger, opModFloatFloat}},
		{{opMaxIntegerInteger, opMaxIntegerFloat}, {opMaxFloatInteger, opMaxFloatFloat}},
		{{opMinIntegerInteger, opMinIntegerFloat}, {opMinFloatInteger, opMinFloatFloat}}};

/**
 * Interprets an arithmetic operation.
 *
 * \param [in] expr The operation to interpret.
 *
 * \param [in] scope The scope to evaluate \a expr under.
 *
 * \note Only supports binary arithmetic operations.
 *
 * \return A pointer to the value of the arithmetic operation.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretArithOpExprNode(OpExprNode *expr,
																			ScopeObject *scope)
{
	ValueObject *val1 = interpretExprNode(expr->args->exprs[0], scope);
	ValueObject *val2 = interpretExprNode(expr->args->exprs[1], scope);
	ValueObject *use1 = val1;
	ValueObject *use2 = val2;
	unsigned int cast1 = 0;
	unsigned int cast2 = 0;
	ValueObject *ret = NULL;
	if (!val1 || !val2)
	{
		deleteValueObject(val1);
		deleteValueObject(val2);
		return NULL;
	}
	/* Check if a floating point decimal string and cast */
	switch (val1->type)
	{
	case VT_NIL:
	case VT_BOOLEAN:
		use1 = castIntegerImplicit(val1, scope);
		if (!use1)
		{
			deleteValueObject(val1);
			deleteValueObject(val2);
			return NULL;
		}
		cast1 = 1;
		break;
	case VT_INTEGER:
	case VT_FLOAT:
		break;
	case VT_STRING:
	{
		/* Perform interpolation */
		ValueObject *interp = castStringExplicit(val1, scope);
		if (!interp)
		{
			deleteValueObject(val1);
			deleteValueObject(val2);
			return NULL;
		}
		if (strchr(getString(interp), '.'))
			use1 = castFloatImplicit(interp, scope);
		else
			use1 = castIntegerImplicit(interp, scope);
		deleteValueObject(interp);
		if (!use1)
		{
			deleteValueObject(val1);
			deleteValueObject(val2);
			return NULL;
		}
		cast1 = 1;
		break;
	}
	default:
		lulz_error(IN_INVALID_OPERAND_TYPE);
	}
	switch (val2->type)
	{
	case VT_NIL:
	case VT_BOOLEAN:
		use2 = castIntegerImplicit(val2, scope);
		if (!use2)
		{
			deleteValueObject(val1);
			deleteValueObject(val2);
			if (cast1)
				deleteValueObject(use1);
			return NULL;
		}
		cast2 = 1;
		break;
	case VT_INTEGER:
	case VT_FLOAT:
		break;
	case VT_STRING:
	{
		/* Perform interpolation */
		ValueObject *interp = castStringExplicit(val2, scope);
		if (!interp)
		{
			deleteValueObject(val1);
			deleteValueObject(val2);
			if (cast1)
				deleteValueObject(use1);
			return NULL;
		}
		if (strchr(getString(interp), '.'))
			use2 = castFloatImplicit(interp, scope);
		else
			use2 = castIntegerImplicit(interp, scope);
		deleteValueObject(interp);
		if (!use2)
		{
			deleteValueObject(val1);
			deleteValueObject(val2);
			if (cast1)
				deleteValueObject(use1);
			return NULL;
		}
		cast2 = 1;
		break;
	}
	default:
		lulz_error(IN_INVALID_OPERAND_TYPE);
	}
	/* Do math depending on value types */
	ret = ArithOpJumpTable[expr->type][use1->type][use2->type](use1, use2);
	/* Clean up after floating point decimal casts */
	if (cast1)
		deleteValueObject(use1);
	if (cast2)
		deleteValueObject(use2);
	deleteValueObject(val1);
	deleteValueObject(val2);
	return ret;
}

/**
 * Interprets a boolean operation.
 *
 * \param [in] expr The operation to interpret.
 *
 * \param [in] scope The scope to evaluate \a expr under.
 *
 * \return A pointer to the value of the boolean operation.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretBoolOpExprNode(OpExprNode *expr,
																		 ScopeObject *scope)
{
	unsigned int n;
	int acc = 0;
	/*
	 * Proceed to apply the same operation on the accumulator for the
	 * remaining arguments.
	 */
	for (n = 0; n < expr->args->num; n++)
	{
		ValueObject *val = interpretExprNode(expr->args->exprs[n], scope);
		ValueObject *use = val;
		int temp;
		unsigned int cast = 0;
		if (!val)
			return NULL;
		if (val->type != VT_BOOLEAN && val->type != VT_INTEGER)
		{
			use = castBooleanImplicit(val, scope);
			if (!use)
			{
				deleteValueObject(val);
				return NULL;
			}
			cast = 1;
		}
		temp = getInteger(use);
		if (cast)
			deleteValueObject(use);
		deleteValueObject(val);
		if (n == 0)
			acc = temp;
		else
		{
			switch (expr->type)
			{
			case OP_AND:
				acc &= temp;
				break;
			case OP_OR:
				acc |= temp;
				break;
			case OP_XOR:
				acc ^= temp;
				break;
			default:
				lulz_error(IN_INVALID_BOOLEAN_OPERATION_TYPE);
				return NULL;
			}
		}
		/**
		 * \note The specification does not say whether boolean logic
		 * short circuits or not.  Here, we assume it does.
		 */
		if (expr->type == OP_AND && acc == 0)
			break;
		else if (expr->type == OP_OR && acc == 1)
			break;
	}
	return createBooleanValueObject(acc);
}

/**
 * Checks if an integer value is equal to another integer value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is equal to \a b.
 */
ValueObject *opEqIntegerInteger(ValueObject *a,
																ValueObject *b)
{
	return createBooleanValueObject(getInteger(a) == getInteger(b));
}

/**
 * Checks if an integer value is not equal to another integer value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is not equal to \a b.
 */
ValueObject *opNeqIntegerInteger(ValueObject *a,
																 ValueObject *b)
{
	return createBooleanValueObject(getInteger(a) != getInteger(b));
}

/**
 * Checks if an integer value is equal to a decimal value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is equal to \a b.
 */
ValueObject *opEqIntegerFloat(ValueObject *a,
															ValueObject *b)
{
	return createBooleanValueObject(fabs((float)(getInteger(a)) - getFloat(b)) < FLT_EPSILON);
}

/**
 * Checks if an integer value is not equal to a decimal value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is not equal to \a b.
 */
ValueObject *opNeqIntegerFloat(ValueObject *a,
															 ValueObject *b)
{
	return createBooleanValueObject(fabs((float)(getInteger(a)) - getFloat(b)) > FLT_EPSILON);
}

/**
 * Checks if a decimal value is equal to an integer value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is equal to \a b.
 */
ValueObject *opEqFloatInteger(ValueObject *a,
															ValueObject *b)
{
	return createBooleanValueObject(fabs(getFloat(a) - (float)(getInteger(b))) < FLT_EPSILON);
}

/**
 * Checks if a decimal value is not equal to an integer value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is not equal to \a b.
 */
ValueObject *opNeqFloatInteger(ValueObject *a,
															 ValueObject *b)
{
	return createBooleanValueObject(fabs(getFloat(a) - (float)(getInteger(b))) > FLT_EPSILON);
}

/**
 * Checks if a decimal value is equal to another decimal value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is equal to \a b.
 */
ValueObject *opEqFloatFloat(ValueObject *a,
														ValueObject *b)
{
	return createBooleanValueObject(fabs(getFloat(a) - getFloat(b)) < FLT_EPSILON);
}

/**
 * Checks if a decimal value is not equal to another decimal value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is not equal to \a b.
 */
ValueObject *opNeqFloatFloat(ValueObject *a,
														 ValueObject *b)
{
	return createBooleanValueObject(fabs(getFloat(a) - getFloat(b)) > FLT_EPSILON);
}

/**
 * Checks if a boolean value is equal to another boolean value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is equal to \a b.
 */
ValueObject *opEqBooleanBoolean(ValueObject *a,
																ValueObject *b)
{
	return createBooleanValueObject(getInteger(a) == getInteger(b));
}

/**
 * Checks if a boolean value is not equal to another boolean value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is not equal to \a b.
 */
ValueObject *opNeqBooleanBoolean(ValueObject *a,
																 ValueObject *b)
{
	return createBooleanValueObject(getInteger(a) != getInteger(b));
}

/**
 * Checks if a string value is equal to another string value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is equal to \a b.
 */
ValueObject *opEqStringString(ValueObject *a,
															ValueObject *b)
{
	return createBooleanValueObject(strcmp(getString(a), getString(b)) == 0);
}

/**
 * Checks if a string value is not equal to another string value.
 *
 * \param [in] a The first value to check.
 *
 * \param [in] b The second value to check.
 *
 * \return A pointer to a boolean value indicating if \a is not equal to \a b.
 */
ValueObject *opNeqStringString(ValueObject *a,
															 ValueObject *b)
{
	return createBooleanValueObject(strcmp(getString(a), getString(b)) != 0);
}

/**
 * Returns true because two nil values are always equal.
 *
 * \param [in] a Not used.
 *
 * \param [in] b Not used.
 *
 * \return A true boolean value.
 */
ValueObject *opEqNilNil(ValueObject *a, ValueObject *b)
{
	return createBooleanValueObject(1);
}

/**
 * Returns false because two nil values are never not equal.
 *
 * \param [in] a Not used.
 *
 * \param [in] b Not used.
 *
 * \return A false boolean value.
 */
ValueObject *opNeqNilNil(ValueObject *a, ValueObject *b)
{
	return createBooleanValueObject(0);
}

/*
 * A jump table for boolean operations.  The first index determines the
 * particular boolean operation to perform, the second index determines the type
 * of the first argument, and the third index determines the type of the second
 * object.
 */
static ValueObject *(*BoolOpJumpTable[2][5][5])(ValueObject *, ValueObject *) = {
		{/* OP_EQ */
		 /* Integer, Float, Boolean, String, Nil */
		 /* Integer */ {opEqIntegerInteger, opEqIntegerFloat, NULL, NULL, NULL},
		 /* Float   */ {opEqFloatInteger, opEqFloatFloat, NULL, NULL, NULL},
		 /* Boolean */ {NULL, NULL, opEqBooleanBoolean, NULL, NULL},
		 /* String  */ {NULL, NULL, NULL, opEqStringString, NULL},
		 /* Nil     */ {NULL, NULL, NULL, NULL, opEqNilNil}},
		{/* OP_NEQ */
		 /* Integer, Float, Boolean, String, Nil */
		 /* Integer */ {opNeqIntegerInteger, opNeqIntegerFloat, NULL, NULL, NULL},
		 /* Float   */ {opNeqFloatInteger, opNeqFloatFloat, NULL, NULL, NULL},
		 /* Boolean */ {NULL, NULL, opNeqBooleanBoolean, NULL, NULL},
		 /* String  */ {NULL, NULL, NULL, opNeqStringString, NULL},
		 /* Nil     */ {NULL, NULL, NULL, NULL, opNeqNilNil}}};

/**
 * Interprets an equality operation.
 *
 * \param [in] expr The operation to interpret.
 *
 * \param [in] scope The scope to evaluate \a expr under.
 *
 * \return A pointer to the resulting value of the equality operation.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretEqualityOpExprNode(OpExprNode *expr,
																				 ScopeObject *scope)
{
	ValueObject *val1 = interpretExprNode(expr->args->exprs[0], scope);
	ValueObject *val2 = interpretExprNode(expr->args->exprs[1], scope);
	ValueObject *ret = NULL;
	if (!val1 || !val2)
	{
		deleteValueObject(val1);
		deleteValueObject(val2);
		return NULL;
	}
	/*
	 * Since there is no automatic casting, an equality (inequality) test
	 * against a non-number type will always fail (succeed).
	 */
	if ((val1->type != val2->type) && ((val1->type != VT_INTEGER && val1->type != VT_FLOAT) || (val2->type != VT_INTEGER && val2->type != VT_FLOAT)))
	{
		switch (expr->type)
		{
		case OP_EQ:
			ret = createBooleanValueObject(0);
			break;
		case OP_NEQ:
			ret = createBooleanValueObject(1);
			break;
		default:
			lulz_error(IN_INVALID_EQUALITY_OPERATION_TYPE);
			deleteValueObject(val1);
			deleteValueObject(val2);
			return NULL;
		}
	}
	else
		ret = BoolOpJumpTable[expr->type - OP_EQ][val1->type][val2->type](val1, val2);
	deleteValueObject(val1);
	deleteValueObject(val2);
	return ret;
}

/**
 * Interprets a concatenation operation.
 *
 * \param [in] expr The operation to interpret.
 *
 * \param [in] scope The scope to evaluate \a expr under.
 *
 * \return A pointer to the resulting value of the concatenation operation.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretConcatOpExprNode(OpExprNode *expr,
																			 ScopeObject *scope)
{
	unsigned int n;
	/* Start out with the first string to concatenate. */
	ValueObject *val = interpretExprNode(expr->args->exprs[0], scope);
	ValueObject *use = castStringImplicit(val, scope);
	char *acc = NULL;
	void *mem = NULL;
	if (!val || !use)
	{
		deleteValueObject(val);
		deleteValueObject(use);
		return NULL;
	}
	/* Start out an accumulator with the first string. */
	mem = util_heap_realloc_ext(acc, sizeof(char) * (strlen(getString(use)) + 1));
	if (!mem)
	{
		ESP_LOGD(TAG, "realloc mem error in %s", __func__);
		deleteValueObject(val);
		deleteValueObject(use);
		free(acc);
		return NULL;
	}
	acc = mem;
	acc[0] = '\0';
	strcat(acc, getString(use));
	deleteValueObject(val);
	deleteValueObject(use);
	for (n = 1; n < expr->args->num; n++)
	{
		/* Grab the next string to concatenate. */
		val = interpretExprNode(expr->args->exprs[n], scope);
		use = castStringImplicit(val, scope);
		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			free(acc);
			return NULL;
		}
		/* Add the next string to the accumulator. */
		mem = util_heap_realloc_ext(acc, sizeof(char) * (strlen(acc) + strlen(getString(use)) + 1));
		if (!mem)
		{
			ESP_LOGD(TAG, "realloc mem error in %s", __func__);
			deleteValueObject(val);
			deleteValueObject(use);
			free(acc);
			return NULL;
		}
		acc = mem;
		strcat(acc, getString(use));
		deleteValueObject(val);
		deleteValueObject(use);
	}
	return createStringValueObject(acc);
}

/*
 * A jump table for operations.  The index of a function in the table is given
 * by its its index in the enumerated OpType type.
 */
static ValueObject *(*OpExprJumpTable[14])(OpExprNode *, ScopeObject *) = {
		interpretArithOpExprNode,
		interpretArithOpExprNode,
		interpretArithOpExprNode,
		interpretArithOpExprNode,
		interpretArithOpExprNode,
		interpretArithOpExprNode,
		interpretArithOpExprNode,
		interpretBoolOpExprNode,
		interpretBoolOpExprNode,
		interpretBoolOpExprNode,
		interpretNotOpExprNode,
		interpretEqualityOpExprNode,
		interpretEqualityOpExprNode,
		interpretConcatOpExprNode};

/**
 * Interprets an operation.
 *
 * \param [in] node The operation to interpret.
 *
 * \param [in] scope The scope to evaluate \a expr under.
 *
 * \return A pointer to the resulting value of the operation.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretOpExprNode(ExprNode *node, ScopeObject *scope)
{
	OpExprNode *expr = (OpExprNode *)node->expr;
	return OpExprJumpTable[expr->type](expr, scope);
}

/*
 * A jump table for expressions.  The index of a function in the table is given
 * by its its index in the enumerated ExprType type.
 */
static ValueObject *(*ExprJumpTable[44])(ExprNode *, ScopeObject *) = {
		/** Basic LOLCODE **/
		interpretCastExprNode,
		interpretConstantExprNode,
		interpretIdentifierExprNode,
		interpretFuncCallExprNode,
		interpretOpExprNode,
		interpretImpVarExprNode,

		//6
		/** LULZCODE Additions **/
		lulz_interpret_array_count_expr_node,
		lulz_interpret_broadcast_get_expr_node,
		lulz_interpret_broadcast_tx_get_expr_node,
		lulz_interpret_file_list_expr_node,

		//10
		lulz_interpret_global_get_expr_node,
		lulz_interpret_hsv_to_565_expr_node,
		lulz_interpret_hsv_to_rgb_expr_node,
		lulz_interpret_rgb_to_565_expr_node,
		lulz_interpret_input,

		//15
		lulz_interpret_text_width_expr_node,
		lulz_interpret_text_height_expr_node,
		NULL, //LCD_WIDTH
		NULL, //LCD_HEIGHT
		lulz_interpret_millis_expr_node,

		//20
		lulz_interpret_peers_expr_node,
		lulz_interpret_random_expr_node,
		lulz_interpret_serial_get_expr_node,
		lulz_interpret_state_get_expr_node,
		lulz_interpret_string_get_at_index,
		lulz_interpret_string_set_at_index,
		lulz_interpret_string_length,

		//25
		lulz_interpret_string_substring,
		lulz_interpret_unlock_get_expr_node,
		lulz_interpret_unlock_validate_expr_node,

		/** LULZCode Accelerometer **/
		lulz_interpret_accel_out_get_expr_node,
		lulz_interpret_accel_side_get_expr_node,
		lulz_interpret_accel_up_get_expr_node,
		lulz_interpret_accel_tilt_get_expr_node,

		/** LULZCODE Buttons **/
		lulz_interpret_button_state_expr_node,
		lulz_interpret_button_wait_expr_node,
		lulz_interpret_button_check_expr_node,
		lulz_interpret_button_check_expr_node,
		lulz_interpret_button_check_expr_node,
		lulz_interpret_button_check_expr_node,
		lulz_interpret_button_check_expr_node,
		lulz_interpret_button_check_expr_node,
		lulz_interpret_button_check_expr_node,

		/** LULZCODE I2C **/
		lulz_interpret_i2c_read,

		/** LULZCODE constants, these don't need to be interpreted **/
		NULL,
};

/**
 * Interprets an expression.
 *
 * \param [in] node The expression to interpret.
 *
 * \param [in] scope The scope to evaluate \a expr under.
 *
 * \return A pointer to the value of \a expr evaluated under \a scope.
 *
 * \retval NULL An error occurred during interpretation.
 */
ValueObject *interpretExprNode(ExprNode *node, ScopeObject *scope)
{
	return ExprJumpTable[node->type](node, scope);
}

/**
 * Interprets a cast statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createCastStmtNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretCastStmtNode(StmtNode *node,
																		ScopeObject *scope)
{
	CastStmtNode *stmt = (CastStmtNode *)node->stmt;
	ValueObject *val = getScopeValue(scope, scope, stmt->target);
	ValueObject *cast = NULL;
	if (!val)
	{
		IdentifierNode *id = (IdentifierNode *)(stmt->target);
		char *name = resolveIdentifierName(id, scope);
		if (name)
		{
			lulz_error(IN_VARIABLE_DOES_NOT_EXIST, id->fname, id->line, name);
			free(name);
		}
		return NULL;
	}
	switch (stmt->newtype->type)
	{
	case CT_NIL:
		if (!(cast = createNilValueObject()))
			return NULL;
		break;
	case CT_BOOLEAN:
		if (!(cast = castBooleanExplicit(val, scope)))
			return NULL;
		break;
	case CT_INTEGER:
		if (!(cast = castIntegerExplicit(val, scope)))
			return NULL;
		break;
	case CT_FLOAT:
		if (!(cast = castFloatExplicit(val, scope)))
			return NULL;
		break;
	case CT_STRING:
		if (!(cast = castStringExplicit(val, scope)))
			return NULL;
		break;
	case CT_ARRAY:
	{
		IdentifierNode *id = (IdentifierNode *)(stmt->target);
		char *name = resolveIdentifierName(id, scope);
		if (name)
		{
			lulz_error(IN_CANNOT_CAST_VALUE_TO_ARRAY, id->fname, id->line, name);
			free(name);
		}
		return NULL;
		break;
	}
	case CT_CHAR:
		return NULL;
		break;
	}
	if (!updateScopeValue(scope, scope, stmt->target, cast))
	{
		deleteValueObject(cast);
		return NULL;
	}
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets a print statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createPrintStmtNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretPrintStmtNode(StmtNode *node,
																		 ScopeObject *scope)
{
	PrintStmtNode *stmt = (PrintStmtNode *)node->stmt;
	unsigned int n;
	for (n = 0; n < stmt->args->num; n++)
	{
		ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
		ValueObject *use = castStringImplicit(val, scope);
		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}
		fprintf(stmt->file, "%s", getString(use));
		deleteValueObject(val);
		deleteValueObject(use);
	}
	if (!stmt->nonl)
		putc('\n', stmt->file);
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets an input statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createInputStmtNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretInputStmtNode(StmtNode *node,
																		 ScopeObject *scope)
{
	unsigned int size = 16;
	unsigned int cur = 0;
	char *temp = util_heap_alloc_ext(sizeof(char) * size);
	int c;
	void *mem = NULL;
	InputStmtNode *stmt = (InputStmtNode *)node->stmt;
	ValueObject *val = NULL;
	while ((c = getchar()) && !feof(stdin))
	{
		/**
		 * \note The specification is unclear as to the exact semantics
		 * of input.  Here, we read up until the first newline or EOF
		 * but do not store it.
		 */
		if (c == EOF || c == (int)'\r' || c == (int)'\n')
			break;
		temp[cur] = (char)c;
		cur++;
		if (cur > size - 1)
		{
			/* Increasing buffer size. */
			size *= 2;
			mem = util_heap_realloc_ext(temp, sizeof(char) * size);
			if (!mem)
			{
				ESP_LOGE(TAG, "realloc error in %s", __func__);
				free(temp);
				return NULL;
			}
			temp = mem;
		}
	}
	temp[cur] = '\0';
	val = createStringValueObject(temp);
	if (!val)
	{
		free(temp);
		return NULL;
	}
	if (!updateScopeValue(scope, scope, stmt->target, val))
	{
		deleteValueObject(val);
		return NULL;
	}
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets an assignment statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createAssignmentStmtNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretAssignmentStmtNode(StmtNode *node,
																					ScopeObject *scope)
{
	AssignmentStmtNode *stmt = (AssignmentStmtNode *)node->stmt;
	ValueObject *val = interpretExprNode(stmt->expr, scope);
	if (!val)
		return NULL;
	/* interpolate assigned strings */
	if (val->type == VT_STRING)
	{
		ValueObject *use = castStringImplicit(val, scope);
		deleteValueObject(val);
		if (!use)
			return NULL;
		val = use;
	}
	if (!updateScopeValue(scope, scope, stmt->target, val))
	{
		deleteValueObject(val);
		return NULL;
	}
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets a declaration statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createDeclarationStmtNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretDeclarationStmtNode(StmtNode *node,
																					 ScopeObject *scope)
{
	DeclarationStmtNode *stmt = (DeclarationStmtNode *)node->stmt;
	ValueObject *init = NULL;
	ScopeObject *dest = NULL;
	dest = getScopeObject(scope, scope, stmt->scope);
	if (!dest)
		return NULL;
	if (getScopeValueLocal(scope, dest, stmt->target))
	{
		IdentifierNode *id = (IdentifierNode *)(stmt->target);
		char *name = resolveIdentifierName(id, scope);
		if (name)
		{
			lulz_error(IN_REDEFINITION_OF_VARIABLE, id->fname, id->line, name);
			free(name);
		}
		return NULL;
	}
	if (stmt->expr)
		init = interpretExprNode(stmt->expr, scope);
	else if (stmt->type)
	{
		switch (stmt->type->type)
		{
		case CT_NIL:
			init = createNilValueObject();
			break;
		case CT_BOOLEAN:
			init = createBooleanValueObject(0);
			break;
		case CT_INTEGER:
			init = createIntegerValueObject(0);
			break;
		case CT_FLOAT:
			init = createFloatValueObject(0.0);
			break;
		case CT_STRING:
			init = createStringValueObject(copyString(""));
			break;
		case CT_ARRAY:
			init = createArrayValueObject(scope);
			break;
		default:
			lulz_error(IN_INVALID_DECLARATION_TYPE);
			return NULL;
		}
	}
	else if (stmt->parent)
	{
		ScopeObject *parent = getScopeObject(scope, scope, stmt->parent);
		if (!parent)
			return NULL;
		init = createArrayValueObject(parent);
	}
	else
		init = createNilValueObject();
	if (!init)
		return NULL;
	if (!createScopeValue(scope, dest, stmt->target))
	{
		deleteValueObject(init);
		return NULL;
	}
	if (!updateScopeValue(scope, dest, stmt->target, init))
	{
		deleteValueObject(init);
		return NULL;
	}
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets an if/then/else statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createIfThenElseStmtNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretIfThenElseStmtNode(StmtNode *node,
																					ScopeObject *scope)
{
	IfThenElseStmtNode *stmt = (IfThenElseStmtNode *)node->stmt;
	ValueObject *use1 = scope->impvar;
	int use1val;
	unsigned int cast1 = 0;
	BlockNode *path = NULL;
	if (scope->impvar->type != VT_BOOLEAN && scope->impvar->type != VT_INTEGER)
	{
		use1 = castBooleanImplicit(scope->impvar, scope);
		if (!use1)
			return NULL;
		cast1 = 1;
	}
	use1val = getInteger(use1);
	if (cast1)
		deleteValueObject(use1);
	/* Determine which block of code to execute */
	if (use1val)
		path = stmt->yes;
	else
	{
		unsigned int n;
		for (n = 0; n < stmt->guards->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->guards->exprs[n], scope);
			ValueObject *use2 = val;
			int use2val;
			unsigned int cast2 = 0;
			if (!val)
				return NULL;
			if (val->type != VT_BOOLEAN && val->type != VT_INTEGER)
			{
				use2 = castBooleanImplicit(val, scope);
				if (!use2)
				{
					deleteValueObject(val);
					return NULL;
				}
				cast2 = 1;
			}
			use2val = getInteger(use2);
			deleteValueObject(val);
			if (cast2)
				deleteValueObject(use2);
			if (use2val)
			{
				path = stmt->blocks->blocks[n];
				break;
			}
		}
		/* Reached the end without satisfying any guard */
		if (n == stmt->guards->num)
			path = stmt->no;
	}
	/* Interpret a path if one was reached */
	if (path)
	{
		ReturnObject *r = interpretBlockNode(path, scope);
		if (!r)
			return NULL;
		/* Pass this up to the outer block to handle. */
		else if (r->type == RT_BREAK || r->type == RT_RETURN)
			return r;
		else
			deleteReturnObject(r);
	}
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets a switch statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createSwitchStmtNode().
 *
 * \note The specification is unclear as to whether guards are implicitly cast
 * to the type of the implicit variable.  This only matters in the case that
 * mixed guard types are present, and in this code, the action that is performed
 * is the same as the comparison operator, that is, in order for a guard to
 * match, both its type and value must match the implicit variable.
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretSwitchStmtNode(StmtNode *node,
																			ScopeObject *scope)
{
	SwitchStmtNode *stmt = (SwitchStmtNode *)node->stmt;
	unsigned int n;
	/*
	 * Loop over each of the guards, checking if any match the implicit
	 * variable.
	 */
	for (n = 0; n < stmt->guards->num; n++)
	{
		ValueObject *use1 = scope->impvar;
		ValueObject *use2 = interpretExprNode(stmt->guards->exprs[n], scope);
		unsigned int done = 0;
		if (!use2)
			return NULL;
		if (use1->type == use2->type)
		{
			switch (use1->type)
			{
			case VT_NIL:
				break;
			case VT_BOOLEAN:
			case VT_INTEGER:
				if (getInteger(use1) == getInteger(use2))
					done = 1;
				break;
			case VT_FLOAT:
				if (fabs(getFloat(use1) - getFloat(use2)) < FLT_EPSILON)
					done = 1;
				break;
			case VT_STRING:
				/**
				 * \note Strings with interpolation
				 * should have already been checked for.
				 */
				if (!strcmp(getString(use1), getString(use2)))
					done = 1;
				break;
			default:
				lulz_error(IN_INVALID_TYPE);
				deleteValueObject(use2);
				return NULL;
			}
		}
		deleteValueObject(use2);
		if (done)
			break;
	}
	/* If none of the guards match and a default block exists */
	if (n == stmt->blocks->num && stmt->def)
	{
		ReturnObject *r = interpretBlockNode(stmt->def, scope);
		if (!r)
			return NULL;
		else if (r->type == RT_RETURN)
			return r;
		else
			deleteReturnObject(r);
	}
	else
	{
		/*
		 * Keep interpreting blocks starting at n until a break or
		 * return is encountered.
		 */
		for (; n < stmt->blocks->num; n++)
		{
			ReturnObject *r = interpretBlockNode(stmt->blocks->blocks[n], scope);
			if (!r)
				return NULL;
			else if (r->type == RT_BREAK)
			{
				deleteReturnObject(r);
				break;
			}
			else if (r->type == RT_RETURN)
				return r;
			else
				deleteReturnObject(r);
		}
	}
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets a break statement.
 *
 * \param [in] node Not used (see note).
 *
 * \param [in] scope Not used (see note).
 *
 * \pre \a node contains a statement created by createStmtNode() with arguments
 * ST_BREAK and NULL.
 *
 * \note \a node and \a scope are not used by this function but are still
 * included in its prototype to allow this function to be stored in a jump table
 * for fast execution.
 *
 * \return A pointer to a break return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretBreakStmtNode(StmtNode *node, ScopeObject *scope)
{
	return createReturnObject(RT_BREAK, NULL);
}

/**
 * Interprets a return statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createReturnStmtNode().
 *
 * \return A pointer to a return value of \a node interpreted under \a scope.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretReturnStmtNode(StmtNode *node, ScopeObject *scope)
{
	/* Evaluate and return the expression. */
	ReturnStmtNode *stmt = (ReturnStmtNode *)node->stmt;
	ValueObject *value = interpretExprNode(stmt->value, scope);
	if (!value)
		return NULL;
	return createReturnObject(RT_RETURN, value);
}

/**
 * Interprets a loop statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createLoopStmtNode().
 *
 * \return A pointer to a return value of \a node interpreted under \a scope.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretLoopStmtNode(StmtNode *node,
																		ScopeObject *scope)
{
	LoopStmtNode *stmt = (LoopStmtNode *)node->stmt;
	ScopeObject *outer = createScopeObject(scope);
	ValueObject *var = NULL;
	if (!outer)
		return NULL;
	/* Create a temporary loop variable if required */
	if (stmt->var)
	{
		var = createScopeValue(scope, outer, stmt->var);
		if (!var)
		{
			deleteScopeObject(outer);
			return NULL;
		}
		var->type = VT_INTEGER;
		var->data.i = 0;
		var->semaphore = 1;
	}
	while (1)
	{
		if (stmt->guard)
		{
			ValueObject *val = interpretExprNode(stmt->guard, outer);
			ValueObject *use = val;
			unsigned short cast = 0;
			int guardval;
			if (val->type != VT_BOOLEAN && val->type != VT_INTEGER)
			{
				use = castBooleanImplicit(val, scope);
				if (!use)
				{
					deleteScopeObject(outer);
					deleteValueObject(val);
					return NULL;
				}
				cast = 1;
			}
			guardval = getInteger(use);
			if (cast)
				deleteValueObject(use);
			deleteValueObject(val);
			if (guardval == 0)
				break;
		}
		if (stmt->body)
		{
			ReturnObject *result = interpretBlockNode(stmt->body, outer);
			if (!result)
			{
				deleteScopeObject(outer);
				return NULL;
			}
			else if (result->type == RT_BREAK)
			{
				deleteReturnObject(result);
				break;
			}
			else if (result->type == RT_RETURN)
			{
				deleteScopeObject(outer);
				return result;
			}
			else
				deleteReturnObject(result);
		}
		if (stmt->update)
		{
			/*
			 * A little efficiency hack: if we know the operation to
			 * perform, don't bother evaluating the ExprNode
			 * structure, just go ahead and do it to the loop
			 * variable.
			 */
			if (stmt->update->type == ET_OP)
			{
				ValueObject *updated = NULL;
				var = getScopeValue(scope, outer, stmt->var);
				OpExprNode *op = (OpExprNode *)stmt->update->expr;
				if (op->type == OP_ADD)
					updated = createIntegerValueObject(var->data.i + 1);
				else if (op->type == OP_SUB)
					updated = createIntegerValueObject(var->data.i - 1);

				if (!updateScopeValue(scope, outer, stmt->var, updated))
				{
					deleteValueObject(updated);
					deleteScopeObject(outer);
					return NULL;
				}
			}
			else
			{
				ValueObject *update = interpretExprNode(stmt->update, outer);
				if (!update)
				{
					deleteScopeObject(outer);
					return NULL;
				}
				if (!updateScopeValue(scope, outer, stmt->var, update))
				{
					deleteScopeObject(outer);
					deleteValueObject(update);
					return NULL;
				}
			}
		}
	}
	deleteScopeObject(outer);
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets a deallocation statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createDeallocationStmtNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretDeallocationStmtNode(StmtNode *node,
																						ScopeObject *scope)
{
	DeallocationStmtNode *stmt = (DeallocationStmtNode *)node->stmt;
	if (!updateScopeValue(scope, scope, stmt->target, NULL))
		return NULL;
	/* If we want to completely remove the variable, use:
	 deleteScopeValue(scope, stmt->target);
	 */
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets a function definition statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createFuncDefStmtNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretFuncDefStmtNode(StmtNode *node,
																			 ScopeObject *scope)
{
	/* Add the function to the current scope */
	FuncDefStmtNode *stmt = (FuncDefStmtNode *)node->stmt;
	ValueObject *init = NULL;
	ScopeObject *dest = NULL;

	dest = getScopeObject(scope, scope, stmt->scope);
	if (!dest)
		return NULL;
	if (getScopeValueLocal(scope, dest, stmt->name))
	{
		IdentifierNode *id = (IdentifierNode *)(stmt->name);
		char *name = resolveIdentifierName(id, scope);
		if (name)
		{
			lulz_error(IN_FUNCTION_NAME_USED_BY_VARIABLE, id->fname, id->line, name);
			free(name);
		}
		return NULL;
	}
	init = createFunctionValueObject(stmt);
	if (!init)
		return NULL;
	if (!createScopeValue(scope, dest, stmt->name))
	{
		deleteValueObject(init);
		return NULL;
	}
	if (!updateScopeValue(scope, dest, stmt->name, init))
	{
		deleteValueObject(init);
		return NULL;
	}
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets an expression statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createExprNode().
 *
 * \post The implicit variable of \a scope will be set the the value of \a node
 * evaluated under \a scope.
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretExprStmtNode(StmtNode *node,
																		ScopeObject *scope)
{
	/* Set the implicit variable to the result of the expression */
	ExprNode *expr = (ExprNode *)node->stmt;
	deleteValueObject(scope->impvar);
	scope->impvar = interpretExprNode(expr, scope);
	if (!scope->impvar)
		return NULL;
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Interprets an alternate array definition statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by createAltArrayDefNode().
 *
 * \return A pointer to a default return value.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretAltArrayDefStmtNode(StmtNode *node,
																					 ScopeObject *scope)
{
	AltArrayDefStmtNode *stmt = (AltArrayDefStmtNode *)node->stmt;
	ValueObject *init = NULL;
	ScopeObject *dest = scope;
	ReturnObject *ret = NULL;
	if (getScopeValueLocal(scope, dest, stmt->name))
	{
		IdentifierNode *id = (IdentifierNode *)(stmt->name);
		char *name = resolveIdentifierName(id, scope);
		if (name)
		{
			fprintf(stderr, "%s:%u: redefinition of existing variable at: %s\n", id->fname, id->line, name);
			free(name);
		}
		return NULL;
	}
	if (stmt->parent)
	{
		ScopeObject *parent = getScopeObject(scope, scope, stmt->parent);
		if (!parent)
			return NULL;
		init = createArrayValueObject(parent);
	}
	else
	{
		init = createArrayValueObject(scope);
	}
	if (!init)
		return NULL;

	/* Populate the array body */
	ret = interpretStmtNodeList(stmt->body->stmts, getArray(init));
	if (!ret)
	{
		deleteValueObject(init);
		return NULL;
	}
	deleteReturnObject(ret);
	if (!createScopeValue(scope, dest, stmt->name))
	{
		deleteValueObject(init);
		return NULL;
	}
	if (!updateScopeValue(scope, dest, stmt->name, init))
	{
		deleteValueObject(init);
		return NULL;
	}
	return createReturnObject(RT_DEFAULT, NULL);
}

/*
 * A jump table for statements.  The index of a function in the table is given
 * by its its index in the enumerated StmtType type in parser.h.
 */
static ReturnObject *(*StmtJumpTable[42])(StmtNode *, ScopeObject *) = {
		interpretCastStmtNode,
		interpretPrintStmtNode,
		interpretInputStmtNode,
		interpretAssignmentStmtNode,
		interpretDeclarationStmtNode,
		interpretIfThenElseStmtNode,
		interpretSwitchStmtNode,
		interpretBreakStmtNode,
		interpretReturnStmtNode,
		interpretLoopStmtNode,
		interpretDeallocationStmtNode,
		interpretFuncDefStmtNode,
		interpretExprStmtNode,
		interpretAltArrayDefStmtNode,
		//14

		//LULZCODE Additions
		lulz_interpret_bitmap_stmt_node,
		lulz_interpret_broadcast_stmt_node,
		lulz_interpret_button_clear_stmt_node,
		lulz_interpret_chip8_stmt_node,
		lulz_interpret_circle_stmt_node,
		lulz_interpret_circle_filled_stmt_node,
		//20
		lulz_interpret_color_set_stmt_node,
		lulz_interpret_cursor_stmt_node,
		lulz_interpret_cursor_area_stmt_node,
		lulz_interpret_delay_stmt_node,
		lulz_interpret_fill_stmt_node,
		//25
		lulz_interpret_font_set_stmt_node,
		lulz_interpret_global_set_stmt_node,
		lulz_interpret_i2c_write,
		lulz_interpret_led_eye_set,
		lulz_interpret_led_set,
		//30
		lulz_interpret_led_set_all,
		lulz_interpret_led_push,
		lulz_interpret_line_stmt_node,
		lulz_interpret_pixel_stmt_node,
		lulz_interpret_play_bling_stmt_node,
		//35
		lulz_interpret_print_screen_stmt_node,
		lulz_interpret_push_buffer_stmt_node,
		lulz_interpret_rect_stmt_node,
		lulz_interpret_rect_filled_stmt_node,
		lulz_interpret_run_stmt_node,
		//40
		lulz_interpret_state_set_stmt_node,
		lulz_interpret_system_call_stmt_node,
};

/**
 * Interprets a statement.
 *
 * \param [in] node The statement to interpret.
 *
 * \param [in] scope The scope to evaluate \a node under.
 *
 * \pre \a node contains a statement created by parseStmtNode().
 *
 * \return A pointer to a return value set appropriately depending on the
 * statement interpreted.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretStmtNode(StmtNode *node, ScopeObject *scope)
{
	if (node == NULL || scope == NULL)
	{
		return NULL;
	}
	return StmtJumpTable[node->type](node, scope);
}

/**
 * Interprets a list of statements.
 *
 * \param [in] list The statements to interpret.
 *
 * \param [in] scope The scope to evaluate \a list under.
 *
 * \return A pointer to a return value set appropriately depending on the
 * statements interpreted.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretStmtNodeList(StmtNodeList *list, ScopeObject *scope)
{
	ReturnObject *ret = NULL;
	unsigned int n;

	//Ensure list isn't null
	if (list == NULL || scope == NULL)
	{
		return NULL;
	}

	if (list->stmts == NULL)
	{
		return NULL;
	}

	for (n = 0; n < list->num; n++)
	{
		ret = interpretStmtNode(list->stmts[n], scope);
		if (!ret)
			return NULL;
		else if (ret->type == RT_BREAK || ret->type == RT_RETURN)
			return ret;
		else
		{
			deleteReturnObject(ret);
			ret = NULL;
		}
	}
	if (!ret)
		ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Interprets a block of code.
 *
 * \param [in] node The block of code to interpret.
 *
 * \param [in] scope The scope to evaluate \a block under.
 *
 * \pre \a block contains a block of code created by parseBlockNode().
 *
 * \return A pointer to a return value set appropriately depending on the
 * statements interpreted.
 *
 * \retval NULL An error occurred during interpretation.
 */
ReturnObject *interpretBlockNode(BlockNode *node,
																 ScopeObject *scope)
{
	ReturnObject *ret = NULL;
	ScopeObject *inner = createScopeObject(scope);
	if (!inner)
		return NULL;
	ret = interpretStmtNodeList(node->stmts, inner);
	deleteScopeObject(inner);
	return ret;
}

/**
 * Interprets a the main block of code.
 *
 * \param [in] main The main block of code to interpret.
 *
 * \pre \a main contains a block of code created by parseMainNode().
 *
 * \return The final status of the program.
 *
 * \retval 0 \a main was interpreted without any errors.
 *
 * \retval 1 An error occurred while interpreting \a main.
 */
int interpretMainNode(MainNode *main)
{
	ReturnObject *ret = NULL;
	if (!main)
		return 1;
	ret = interpretBlockNode(main->block, NULL);
	if (!ret)
		return 1;
	deleteReturnObject(ret);
	return 0;
}

/******************************************************************************
 * LULZCODE Additions
 *****************************************************************************/

/**
 * Get the Z axis value from accelerometer
 * Format: OUTWAYZ
 */
ValueObject *lulz_interpret_accel_out_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	return createIntegerValueObject(drv_lis2de12_get().z);
}

/**
 * Get the Y axis value from accelerometer
 * Format: SIDEWAYZ
 */
ValueObject *lulz_interpret_accel_side_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	return createIntegerValueObject(drv_lis2de12_get().y);
}

/**
 * Get the X axis value from accelerometer
 * Format: UPWAYZ
 */
ValueObject *lulz_interpret_accel_up_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	return createIntegerValueObject(drv_lis2de12_get().x);
}

/**
 * Get the count of the items in a BUKKIT
 * Format: HOW MANY IN <BUKKIT>
 */
ValueObject *lulz_interpret_array_count_expr_node(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1)
	{

		//Evaluate the expression
		ValueObject *bukkit_node = interpretExprNode(expr->expr1, scope);
		ValueObject *ret = NULL;

		if (bukkit_node == NULL)
		{
			ESP_LOGE(TAG, "Unknown BUKKIT in HOW MANY expression");
			//Cleanup
			deleteValueObject(bukkit_node);
			return createIntegerValueObject(0);
		}

		if (bukkit_node->type == VT_ARRAY)
		{
			ret = createIntegerValueObject(bukkit_node->data.a->numvals);
		}
		else
		{
			ret = createIntegerValueObject(0);
		}

		//Cleanup
		deleteValueObject(bukkit_node);

		return ret;
	}

	ESP_LOGE(TAG, "Invalid or missing expression in HOW MANY IN. Expected HOW MANY IN <BUKKIT>");
	return createIntegerValueObject(0);
}

/**
 * Get the calculated tilt (angle) of X Y accelerometer
 * Format: UPWAYZ
 */
ValueObject *lulz_interpret_accel_tilt_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	uint16_t deg = drv_lis2de12_tilt_get();
	return createIntegerValueObject(deg);
}

/**
 * Draw a bitmap
 * Format: KATNIP <x> <y> <width> <height>
 */
ReturnObject *lulz_interpret_bitmap_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num == 5)
	{
		int16_t x = 0, y = 0, w = 0, h = 0;
		char file_path[256];
		memset(file_path, 0, 256);

		for (uint32_t n = 0; n < stmt->args->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
			ValueObject *use;

			//Cast the arguments properly
			if (n == 0)
			{
				use = castStringExplicit(val, scope);
			}
			else
			{
				use = castIntegerExplicit(val, scope);
			}

			//Ensure we have valid values
			if (!val || !use)
			{
				deleteValueObject(val);
				deleteValueObject(use);
				return NULL;
			}

			//Grab data
			switch (n)
			{
			case 0:
				memcpy(file_path, getString(use), MIN(strlen(getString(use)), 255));
				break;
			case 1:
				x = (int16_t)getInteger(use);
				break;
			case 2:
				y = (int16_t)getInteger(use);
				break;
			case 3:
				w = (uint16_t)getInteger(use);
				break;
			case 4:
				h = (uint16_t)getInteger(use);
				break;
			}

			deleteValueObject(val);
			deleteValueObject(use);
		}

		//Draw the raw file at the given coordinates
		if (file_path != NULL)
		{
			gfx_draw_raw_file(file_path, x, y, w, h, false, 0);
		}
		else
		{
			ESP_LOGE(TAG, "Invalid path given to KATNIP");
		}
	}
	else
	{
		ESP_LOGE(TAG, "Invalid number of arguments to KATNIP");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Set the current broadcast message for chat purposes
 *
 * Format: HOLLABACK <Message>
 */
ReturnObject *lulz_interpret_broadcast_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num == 1)
	{
		char message[BROADCAST_MESSAGE_LENGTH + 1];
		memset(message, 0, BROADCAST_MESSAGE_LENGTH + 1);

		ValueObject *message_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *message_str = castStringExplicit(message_node, scope);

		//Ensure message does not exceed max length
		snprintf(message, BROADCAST_MESSAGE_LENGTH + 1, "%s", getString(message_str));

		broadcast_set(message);

		deleteValueObject(message_node);
		deleteValueObject(message_str);
	}
	else
	{
		ESP_LOGE(TAG, "Invalid number of arguments to HOLLABACK. Expected HOLLABACK <message>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Get the most recent broadcast received
 * Format: WHO DIS
 */
ValueObject *lulz_interpret_broadcast_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	//Use a bukkit like a struct for the last broadcast received
	ValueObject *bukkit = createArrayValueObject(scope);

	broadcast_t broadcast;
	broadcast_last_received_get(&broadcast);

	//Add name to bukkit
	char *name = util_heap_alloc_ext(strlen("NAME") + 1);
	sprintf(name, "NAME");
	IdentifierNode *name_node = createIdentifierNode(IT_DIRECT, name, NULL, NULL, 0);
	ValueObject *name_value = createStringValueObject(copyString(broadcast.name));
	if (!createScopeValue(scope, getArray(bukkit), name_node))
	{
		deleteValueObject(name_value);
		deleteIdentifierNode(name_node);
		ESP_LOGE(TAG, "Unable to add NAME to broadcast BUKKIT");
		return NULL;
	}
	if (!updateScopeValue(scope, getArray(bukkit), name_node, name_value))
	{
		deleteValueObject(name_value);
		deleteIdentifierNode(name_node);
		ESP_LOGE(TAG, "Unable to set NAME value in broadcast BUKKIT");
		return NULL;
	}

	//Add message to bukkit
	char *message = util_heap_alloc_ext(strlen("MESSAGE") + 1);
	sprintf(message, "MESSAGE");
	IdentifierNode *message_node = createIdentifierNode(IT_DIRECT, message, NULL, NULL, 0);
	ValueObject *message_value = createStringValueObject(copyString(broadcast.message));
	if (!createScopeValue(scope, getArray(bukkit), message_node))
	{
		deleteValueObject(name_value);
		deleteIdentifierNode(name_node);
		deleteValueObject(message_value);
		deleteIdentifierNode(message_node);
		ESP_LOGE(TAG, "Unable to add MESSAGE to broadcast BUKKIT");
		return NULL;
	}
	if (!updateScopeValue(scope, getArray(bukkit), message_node, message_value))
	{
		deleteValueObject(name_value);
		deleteIdentifierNode(name_node);
		deleteValueObject(message_value);
		deleteIdentifierNode(message_node);
		ESP_LOGE(TAG, "Unable to set NAME value in broadcast BUKKIT");
		return NULL;
	}

	//Add timestamp to bukkit
	char *timestamp = util_heap_alloc_ext(strlen("TIMESTAMP") + 1);
	sprintf(timestamp, "TIMESTAMP");
	IdentifierNode *timestamp_node = createIdentifierNode(IT_DIRECT, timestamp, NULL, NULL, 0);
	ValueObject *timestamp_value = createIntegerValueObject(broadcast.timestamp);
	if (!createScopeValue(scope, getArray(bukkit), timestamp_node))
	{
		deleteValueObject(name_value);
		deleteIdentifierNode(name_node);
		deleteValueObject(message_value);
		deleteIdentifierNode(message_node);
		deleteValueObject(timestamp_value);
		deleteIdentifierNode(timestamp_node);
		return NULL;
	}
	if (!updateScopeValue(scope, getArray(bukkit), timestamp_node, timestamp_value))
	{
		deleteValueObject(name_value);
		deleteIdentifierNode(name_node);
		deleteValueObject(message_value);
		deleteIdentifierNode(message_node);
		deleteValueObject(timestamp_value);
		deleteIdentifierNode(timestamp_node);
		return NULL;
	}

	return bukkit;
}

/**
 * Get the most recent broadcast received
 * Format: WHO DIS
 */
ValueObject *lulz_interpret_broadcast_tx_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	char *message = (char *)util_heap_alloc_ext(BROADCAST_MESSAGE_LENGTH + 1);
	broadcast_packet_t *p_packet = broadcast_get();
	snprintf(message, BROADCAST_MESSAGE_LENGTH + 1, p_packet->message);
	return createStringValueObject(message);
}

/**
 * Get the current button state
 * Format: KITTEH
 */
ValueObject *lulz_interpret_button_check_expr_node(ExprNode *node, ScopeObject *scope)
{
	bool result = false;

	if (node->type == ET_BUTTON_UP)
	{
		result = btn_up();
	}
	else if (node->type == ET_BUTTON_DOWN)
	{
		result = btn_down();
	}
	else if (node->type == ET_BUTTON_LEFT)
	{
		result = btn_left();
	}
	else if (node->type == ET_BUTTON_RIGHT)
	{
		result = btn_right();
	}
	else if (node->type == ET_BUTTON_A)
	{
		result = btn_a();
	}
	else if (node->type == ET_BUTTON_B)
	{
		result = btn_b();
	}
	else if (node->type == ET_BUTTON_C)
	{
		result = btn_c();
	}

	return createBooleanValueObject(result);
}

/**
 * Get the current button state
 * Format: KITTEH
 */
ReturnObject *lulz_interpret_button_clear_stmt_node(StmtNode *node, ScopeObject *scope)
{
	btn_clear();
	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Get the current button state
 * Format: KITTEH
 */
ValueObject *lulz_interpret_button_state_expr_node(ExprNode *node, ScopeObject *scope)
{
	return createIntegerValueObject(btn_state());
}

/**
 * Execute wait for button command
 * Format: POUNCE
 */
ValueObject *lulz_interpret_button_wait_expr_node(ExprNode *node, ScopeObject *scope)
{
	uint8_t button = btn_wait();
	return createIntegerValueObject(button);
}

/**
 * Execute circle command.
 * Format: ROUND <x> <y> <radius> <color>
 */
ReturnObject *lulz_interpret_circle_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num == 4)
	{
		int16_t x = 0, y = 0, r = 0;
		color_565_t color = 0;
		for (uint32_t n = 0; n < stmt->args->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
			ValueObject *use = castIntegerExplicit(val, scope);

			if (!val || !use)
			{
				deleteValueObject(val);
				deleteValueObject(use);
				return NULL;
			}

			switch (n)
			{
			case 0:
				x = (int16_t)getInteger(use);
				break;
			case 1:
				y = (int16_t)getInteger(use);
				break;
			case 2:
				r = (uint16_t)getInteger(use);
				break;
			case 3:
				color = (color_565_t)getInteger(use);
				break;
			}

			deleteValueObject(val);
			deleteValueObject(use);
		}

		//Move the print to screen cursor
		gfx_draw_circle(x, y, r, color);
	}
	else
	{
		ESP_LOGE(TAG, "Invalid parameters to ROUND. Expected format ROUND <x> <y> <r> <color>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Execute a CHIP8 file
 * Format: NACHOS <file>
 */
ReturnObject *lulz_interpret_chip8_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num == 1)
	{
		char *path;
		ValueObject *path_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *path_str = castStringExplicit(path_node, scope);

		if (!path_node || !path_str)
		{
			deleteValueObject(path_node);
			deleteValueObject(path_str);
			return NULL;
		}

		path = copyString(getString(path_str));
		ESP_LOGD(TAG, "%s NACHOS '%s'", __func__, path);
		chip8_run_file(path);

		deleteValueObject(path_node);
		deleteValueObject(path_str);
	}
	else
	{
		ESP_LOGE(TAG, "Invalid parameters to NACHOS. Expected format NACHOS <file>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Execute circle filled command.
 * Format: CATINAROUND <x> <y> <radius> <color>
 */
ReturnObject *lulz_interpret_circle_filled_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num == 4)
	{
		int16_t x = 0, y = 0, r = 0;
		color_565_t color = 0;
		for (uint32_t n = 0; n < stmt->args->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
			ValueObject *use = castIntegerExplicit(val, scope);

			if (!val || !use)
			{
				deleteValueObject(val);
				deleteValueObject(use);
				return NULL;
			}

			switch (n)
			{
			case 0:
				x = (int16_t)getInteger(use);
				break;
			case 1:
				y = (int16_t)getInteger(use);
				break;
			case 2:
				r = (uint16_t)getInteger(use);
				break;
			case 3:
				color = (color_565_t)getInteger(use);
				break;
			}

			deleteValueObject(val);
			deleteValueObject(use);
		}

		//Move the print to screen cursor
		gfx_fill_circle(x, y, r, color);
	}
	else
	{
		ESP_LOGE(TAG, "Invalid parameters to CATINAROUND. Expected format CATINAROUND <x> <y> <r> <color>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

ReturnObject *lulz_interpret_color_set_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num > 0)
	{
		ValueObject *val = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *use = castIntegerExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}

		//TODO: Ensure color is valid range, perhaps downconvert
		gfx_color_set((color_565_t)use->data.i);

		deleteValueObject(val);
		deleteValueObject(use);
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

ReturnObject *lulz_interpret_cursor_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num >= 2)
	{
		int16_t x = 0, y = 0;
		for (uint32_t n = 0; n < stmt->args->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
			ValueObject *use = castIntegerExplicit(val, scope);

			if (!val || !use)
			{
				deleteValueObject(val);
				deleteValueObject(use);
				return NULL;
			}

			switch (n)
			{
			case 0:
				x = (int16_t)getInteger(use);
				break;
			case 1:
				y = (int16_t)getInteger(use);
				break;
			}

			deleteValueObject(val);
			deleteValueObject(use);
		}

		//Move the print to screen cursor
		gfx_cursor_set((cursor_coord_t){x, y});
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

ReturnObject *lulz_interpret_cursor_area_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num == 4)
	{
		int16_t xs = 0, ys = 0, xe = 0, ye = 0;
		for (uint32_t n = 0; n < stmt->args->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
			ValueObject *use = castIntegerExplicit(val, scope);

			if (!val || !use)
			{
				deleteValueObject(val);
				deleteValueObject(use);
				return NULL;
			}

			switch (n)
			{
			case 0:
				xs = (int16_t)getInteger(use);
				break;
			case 1:
				ys = (int16_t)getInteger(use);
				break;
			case 2:
				xe = (int16_t)getInteger(use);
				break;
			case 3:
				ye = (int16_t)getInteger(use);
				break;
			}

			deleteValueObject(val);
			deleteValueObject(use);
		}

		//Set cursor area
		gfx_cursor_area_set((area_t){xs, ys, xe, ye});
	}
	else
	{
		ESP_LOGE(TAG, "INSIDEZ expects 4 arguments.");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

ReturnObject *lulz_interpret_delay_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	for (uint32_t n = 0; n < stmt->args->num; n++)
	{
		ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
		ValueObject *use = castIntegerExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}
		DELAY(use->data.i);

		deleteValueObject(val);
		deleteValueObject(use);
	}
	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Get a BUKKIT of files in a directory
 * Format: DAB <path>
 */
ValueObject *lulz_interpret_file_list_expr_node(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;
	ValueObject *ret = NULL;
	uint16_t i = 0; //bukkit counter

	if (expr->expr1)
	{
		char *path = NULL;

		//Evaluate the expression
		ValueObject *path_node = interpretExprNode(expr->expr1, scope);
		ValueObject *path_str = castStringExplicit(path_node, scope);

		if (!path_node || !path_str)
		{
			deleteValueObject(path_node);
			deleteValueObject(path_str);
			return NULL;
		}

		path = getString(path_str);

		DIR *directory = opendir(path);

		if (directory)
		{
			ret = createArrayValueObject(scope);

			for (;;)
			{
				struct dirent *entry = readdir(directory);
				if (!entry)
				{
					//Entry will be NULL when we reach the end of the entries
					break;
				}

				//File
				if (entry->d_type == DT_REG)
				{

					void *name = util_heap_alloc_ext(6);
					sprintf(name, "%d", i++);
					IdentifierNode *name_node = createIdentifierNode(IT_DIRECT, name, NULL, NULL, 0);

					void *full_path = util_heap_alloc_ext(512);
					sprintf(full_path, "%s/%s", path, entry->d_name);
					ValueObject *name_value = createStringValueObject(full_path);

					if (!createScopeValue(scope, getArray(ret), name_node))
					{
						deleteValueObject(name_value);
						deleteIdentifierNode(name_node);
						free(name);
						free(full_path);
						return NULL;
					}
					if (!updateScopeValue(scope, getArray(ret), name_node, name_value))
					{
						deleteValueObject(name_value);
						deleteIdentifierNode(name_node);
						free(name);
						free(full_path);
						return NULL;
					}

					free(name);
				}
			}
		}

		closedir(directory);

		//Cleanup
		deleteValueObject(path_node);
		deleteValueObject(path_str);

		//If we got a valid return value earlier, return it otherwise fall out
		//of if block and return nil
		if (ret)
		{
			return ret;
		}
	}

	ESP_LOGE(TAG, "Invalid or missing expression in DAB. Expected DAB <path>");
	return createIntegerValueObject(0);
}

ReturnObject *lulz_interpret_fill_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num > 0)
	{
		ValueObject *val = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *use = castIntegerExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}

		//TODO: Ensure color is valid range, perhaps downconvert
		gfx_fill_screen((uint16_t)use->data.i);

		deleteValueObject(val);
		deleteValueObject(use);
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

ReturnObject *lulz_interpret_font_set_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num > 0)
	{
		ValueObject *val = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *use = castStringExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}

		char *font = getString(use);
		if (strcmp(font, "SMALL") == 0)
		{
			gfx_font_set(font_small);
		}
		else if (strcmp(font, "MEDIUM") == 0)
		{
			gfx_font_set(font_medium);
		}
		else if (strcmp(font, "LARGE") == 0)
		{
			gfx_font_set(font_large);
		}
		else if (strcmp(font, "LUDICROUS") == 0)
		{
			gfx_font_set(font_x_large);
		}
		else if (strcmp(font, "RAP") == 0)
		{
			gfx_font_wrap_set(true);
		}
		else if (strcmp(font, "NO RAP") == 0)
		{
			gfx_font_wrap_set(false);
		}
		else
		{
			ESP_LOGE(TAG, "Invalid font '%s'. Valid fonts are 'SMALL', 'MEDIUM', 'LARGE', 'LUDICROUS', 'RAP', 'NO RAP'", font);
		}

		deleteValueObject(val);
		deleteValueObject(use);
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Expression that retrieves a global value. If not found NOOB is returned.
 *
 * Format: TEH <key>
 */
ValueObject *lulz_interpret_global_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1)
	{
		char *key = NULL;

		//Evaluate the expressio
		ValueObject *val = interpretExprNode(expr->expr1, scope);
		ValueObject *use = castStringExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}

		//Get the key from LULZCODE
		key = getString(use);

		//Retrieve the value from globals
		char *value = lulz_global_get(key);
		ValueObject *ret = NULL;

		//If valid result from globals, create a return value
		if (value)
		{
			//Return a copy of the value, otherwise lulzcode will try to free the
			//global value off the heap later
			ret = createStringValueObject(copyString(value));
		}

		//Cleanup
		deleteValueObject(val);
		deleteValueObject(use);

		//If we got a valid return value earlier, return it otherwise fall out
		//of if block and return nil
		if (ret)
		{
			return ret;
		}
	}

	return createNilValueObject();
}

ReturnObject *lulz_interpret_global_set_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	char *key = NULL;
	char *value = NULL;

	if (stmt->args->num >= 2)
	{
		ValueObject *valkey = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *usekey = castStringImplicit(valkey, scope);
		ValueObject *valvalue = interpretExprNode(stmt->args->exprs[1], scope);
		ValueObject *usevalue = castStringImplicit(valvalue, scope);

		if (!valkey || !usekey || !valvalue || !usevalue)
		{
			deleteValueObject(valkey);
			deleteValueObject(usekey);
			deleteValueObject(valvalue);
			deleteValueObject(usevalue);
			return createReturnObject(RT_DEFAULT, NULL);
		}

		//Retrieve the values
		key = getString(usekey);
		value = getString(usevalue);

		//Actually set the global value
		lulz_global_set(key, value);

		//cleanup
		deleteValueObject(valkey);
		deleteValueObject(usekey);
		deleteValueObject(valvalue);
		deleteValueObject(usevalue);
	}

	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Helper function that converts HSV values to 565 or RGB
 */
static ValueObject *__interpret_hsv_expr_nodes(ExprNode *node, ScopeObject *scope, bool to_565)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	uint8_t h = 0, s = 0, v = 0;
	color_565_t color = 0;

	if (expr->expr1 && expr->expr2 && expr->expr3)
	{

		ValueObject *hval = interpretExprNode(expr->expr1, scope);
		ValueObject *sval = interpretExprNode(expr->expr2, scope);
		ValueObject *vval = interpretExprNode(expr->expr3, scope);
		ValueObject *huse = castIntegerImplicit(hval, scope);
		ValueObject *suse = castIntegerImplicit(sval, scope);
		ValueObject *vuse = castIntegerImplicit(vval, scope);

		if (!hval || !sval || !vval || !huse || !suse || !vuse)
		{
			deleteValueObject(hval);
			deleteValueObject(sval);
			deleteValueObject(vval);
			deleteValueObject(huse);
			deleteValueObject(suse);
			deleteValueObject(vuse);
			return NULL;
		}

		h = getInteger(huse);
		s = getInteger(suse);
		v = getInteger(vuse);

		deleteValueObject(hval);
		deleteValueObject(sval);
		deleteValueObject(vval);
		deleteValueObject(huse);
		deleteValueObject(suse);
		deleteValueObject(vuse);

		//Generate a 565 color, if they gave us bad data, they get a bad color
		float hf = (float)h / 255.0;
		float sf = (float)s / 255.0;
		float vf = (float)v / 255.0;
		color_rgb_t rgb = util_hsv_to_rgb(hf, sf, vf);

		if (to_565)
		{
			color = util_rgb_to_565(rgb);
			return createIntegerValueObject(color);
		}
		else
		{
			return createIntegerValueObject(rgb);
		}
	}

	return createIntegerValueObject(0);
}

/**
 * Interpret HSV to 565 command.
 *
 * Format: HSSSVEE2 <h> <s> <v>
 * H, S, V can be from 0 to 255
 */
ValueObject *lulz_interpret_hsv_to_565_expr_node(ExprNode *node, ScopeObject *scope)
{
	return __interpret_hsv_expr_nodes(node, scope, true);
}

/**
 * Interpret HSV to RGB command.
 *
 * Format: HSSSVEE2BLINKY <h> <s> <v>
 * H, S, V can be from 0 to 255
 */
ValueObject *lulz_interpret_hsv_to_rgb_expr_node(ExprNode *node, ScopeObject *scope)
{
	return __interpret_hsv_expr_nodes(node, scope, false);
}

/**
 * Read a byte from I2C register. Node the address is 7 bits and will be shifted
 *
 * Format:
 * NOM <7-bit address> <register>
 *
 */
ValueObject *lulz_interpret_i2c_read(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	//Handle <index> <rgb>
	if (expr->expr1 && expr->expr2)
	{
		uint8_t address;
		uint8_t reg;

		ValueObject *address_node = interpretExprNode(expr->expr1, scope);
		ValueObject *register_node = interpretExprNode(expr->expr2, scope);
		ValueObject *address_int = castIntegerExplicit(address_node, scope);
		ValueObject *register_int = castIntegerExplicit(register_node, scope);

		address = (uint8_t)getInteger(address_int);
		reg = (uint8_t)getInteger(register_int);

		//Cleanup
		deleteValueObject(address_node);
		deleteValueObject(register_node);
		deleteValueObject(address_int);
		deleteValueObject(register_int);

		uint8_t data = hal_i2c_read_reg_byte(ADDON_I2C_MASTER_NUM, address, reg);
		return createIntegerValueObject(data);
	}
	//Error
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to NOM. Expected format: NOM <address> <register>");
	}

	return createNilValueObject();
}

/**
 * Write a byte to I2C
 *
 * Format:
 *
 * NOMS <7-bit address> <register> <byte>
 *
 * or
 *
 * NOMS <7-bit address> <byte>
 */
ReturnObject *lulz_interpret_i2c_write(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;
	uint8_t address;
	uint8_t reg;
	uint8_t byte;

	//Handle <address> <register> <byte>
	if (stmt->args->num == 3)
	{

		ValueObject *address_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *register_node = interpretExprNode(stmt->args->exprs[1], scope);
		ValueObject *byte_node = interpretExprNode(stmt->args->exprs[2], scope);
		ValueObject *address_int = castIntegerExplicit(address_node, scope);
		ValueObject *register_int = castIntegerExplicit(register_node, scope);
		ValueObject *byte_int = castIntegerExplicit(byte_node, scope);

		address = (uint8_t)getInteger(address_int);
		reg = (uint8_t)getInteger(register_int);
		byte = (uint8_t)getInteger(byte_int);

		//Cleanup
		deleteValueObject(address_node);
		deleteValueObject(register_node);
		deleteValueObject(byte_node);
		deleteValueObject(address_int);
		deleteValueObject(register_int);
		deleteValueObject(byte_int);

		hal_i2c_write_reg_byte(ADDON_I2C_MASTER_NUM, address, reg, byte);
	}

	//Handle <address> <byte>
	else if (stmt->args->num == 2)
	{

		ValueObject *address_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *byte_node = interpretExprNode(stmt->args->exprs[2], scope);
		ValueObject *address_int = castIntegerExplicit(address_node, scope);
		ValueObject *byte_int = castIntegerExplicit(byte_node, scope);

		address = (uint8_t)getInteger(address_int);
		byte = (uint8_t)getInteger(byte_int);

		//Cleanup
		deleteValueObject(address_node);
		deleteValueObject(byte_node);
		deleteValueObject(address_int);
		deleteValueObject(byte_int);

		hal_i2c_write_byte(ADDON_I2C_MASTER_NUM, address, byte);
	}

	//Error
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to NOMS. Expected format: NOMS <address> <byte> or NOMS <addres> <register> <byte>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Read input from user and return a string
 *
 * Format:
 * MEOWMIX <label> <text> <length>
 *
 */
ValueObject *lulz_interpret_input(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	//Handle <text> <length>
	if (expr->expr1 && expr->expr2)
	{
		ValueObject *label_node = interpretExprNode(expr->expr1, scope);
		ValueObject *text_node = interpretExprNode(expr->expr2, scope);
		ValueObject *length_node = interpretExprNode(expr->expr3, scope);
		ValueObject *label_str = castStringExplicit(label_node, scope);
		ValueObject *text_str = castStringExplicit(text_node, scope);
		ValueObject *length_int = castIntegerExplicit(length_node, scope);

		//Get raw data
		char *label = getString(label_str);
		uint8_t length = (uint8_t)getInteger(length_int);
		ESP_LOGD(TAG, "%s Length = %d", __func__, length);
		char *text_raw_str = getString(text_str);

		//Ensure memory is blank
		char *text = (char *)util_heap_alloc_ext(length + 1);
		memset(text, 0, length + 1);

		//Safely build a string to use for input
		memcpy(text, text_raw_str, MIN(strlen(text_raw_str), length));

		ui_input(label, text, length);

		//Cleanup
		deleteValueObject(label_node);
		deleteValueObject(text_node);
		deleteValueObject(length_node);
		deleteValueObject(label_str);
		deleteValueObject(text_str);
		deleteValueObject(length_int);

		return createStringValueObject(text);
	}
	//Error
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to MEOWMIX. Expected format: MEOWMIX <label> <text> <length>");
	}

	return createNilValueObject();
}

/**
 * Set Eye LED to a specific brightness 0 to 255
 *
 * Format:
 * WINK <brightness>
 */
ReturnObject *lulz_interpret_led_eye_set(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	//Handle <brightness>
	if (stmt->args->num == 1)
	{
		uint8_t brightness;

		ValueObject *brightness_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *brightness_int = castIntegerExplicit(brightness_node, scope);

		//Limit brightness value
		brightness = MIN(UINT8_MAX, getInteger(brightness_int));

		//Cleanup
		deleteValueObject(brightness_node);
		deleteValueObject(brightness_int);

		//Do the work
		led_eye_pwm(brightness);
	}

	//Error
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to WINK. Expected format: WINK <brightness>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Push LED buffer to LEDs
 *
 * Format:
 *
 * LOLOL
 */
ReturnObject *lulz_interpret_led_push(StmtNode *node, ScopeObject *scope)
{
	led_show();
	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Set an LED to a color. Only sets the coclor in memory. Needs to be pushed
 *
 * Format:
 * BLINK <index> <red> <green> <blue>
 *
 * or
 *
 * BLINK <index> <RGB>
 *
 * Where index is 0 to 30 (inclusive)
 */
ReturnObject *lulz_interpret_led_set(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	//Handle <index> <rgb>
	if (stmt->args->num == 2)
	{
		uint8_t index;
		color_rgb_t rgb;

		ValueObject *index_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *rgb_node = interpretExprNode(stmt->args->exprs[1], scope);
		ValueObject *index_int = castIntegerExplicit(index_node, scope);
		ValueObject *rgb_int = castIntegerExplicit(rgb_node, scope);

		index = (uint8_t)getInteger(index_int);
		rgb = (color_rgb_t)getInteger(rgb_int);

		//Cleanup
		deleteValueObject(index_node);
		deleteValueObject(rgb_node);
		deleteValueObject(index_int);
		deleteValueObject(rgb_int);

		//Do the work
		led_set_rgb(index, rgb);
	}

	//Handle <index> <r> <g> <b>
	else if (stmt->args->num == 4)
	{
		uint8_t index;
		uint8_t r, g, b;

		//Get index
		ValueObject *index_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *index_int = castIntegerExplicit(index_node, scope);
		index = (uint8_t)getInteger(index_int);
		deleteValueObject(index_node);
		deleteValueObject(index_int);

		//Get red
		ValueObject *r_node = interpretExprNode(stmt->args->exprs[1], scope);
		ValueObject *r_int = castIntegerExplicit(r_node, scope);
		r = (uint8_t)getInteger(r_int);
		deleteValueObject(r_node);
		deleteValueObject(r_int);

		//Get green
		ValueObject *g_node = interpretExprNode(stmt->args->exprs[2], scope);
		ValueObject *g_int = castIntegerExplicit(g_node, scope);
		g = (uint8_t)getInteger(g_int);
		deleteValueObject(g_node);
		deleteValueObject(g_int);

		//Get blue
		ValueObject *b_node = interpretExprNode(stmt->args->exprs[3], scope);
		ValueObject *b_int = castIntegerExplicit(b_node, scope);
		b = (uint8_t)getInteger(b_int);
		deleteValueObject(b_node);
		deleteValueObject(b_int);

		//Do the work
		led_set(index, r, g, b);
	}

	//Error
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to BLINK. Expected format: BLINK <index> <rgb> or BLINK <index> <r> <g> <b>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Set all LEDs to a color. Only sets the color in memory. Needs to be pushed
 *
 * Format:
 * BLINKIES <red> <green> <blue>
 *
 * or
 *
 * BLINKIES <RGB>
 */
ReturnObject *lulz_interpret_led_set_all(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	//Handle <rgb>
	if (stmt->args->num == 1)
	{
		color_rgb_t rgb;

		ValueObject *rgb_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *rgb_int = castIntegerExplicit(rgb_node, scope);

		rgb = (color_rgb_t)getInteger(rgb_int);

		//Cleanup
		deleteValueObject(rgb_node);
		deleteValueObject(rgb_int);

		//Do the work
		led_set_all_rgb(rgb);
	}

	//Handle <r> <g> <b>
	else if (stmt->args->num == 3)
	{
		uint8_t r, g, b;

		//Get red
		ValueObject *r_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *r_int = castIntegerExplicit(r_node, scope);
		r = (uint8_t)getInteger(r_int);
		deleteValueObject(r_node);
		deleteValueObject(r_int);

		//Get green
		ValueObject *g_node = interpretExprNode(stmt->args->exprs[1], scope);
		ValueObject *g_int = castIntegerExplicit(g_node, scope);
		g = (uint8_t)getInteger(g_int);
		deleteValueObject(g_node);
		deleteValueObject(g_int);

		//Get blue
		ValueObject *b_node = interpretExprNode(stmt->args->exprs[2], scope);
		ValueObject *b_int = castIntegerExplicit(b_node, scope);
		b = (uint8_t)getInteger(b_int);
		deleteValueObject(b_node);
		deleteValueObject(b_int);

		//Do the work
		led_set_all(r, g, b);
	}

	//Error
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to BLINKIES. Expected format: BLINK <rgb> or BLINK <r> <g> <b>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

ReturnObject *lulz_interpret_line_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num >= 5)
	{
		int16_t x0 = 0, x1 = 0, y0 = 0, y1 = 0;
		uint16_t color = 0;

		for (uint32_t n = 0; n < stmt->args->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
			ValueObject *use = castIntegerExplicit(val, scope);

			if (!val || !use)
			{
				deleteValueObject(val);
				deleteValueObject(use);
				return NULL;
			}

			switch (n)
			{
			case 0:
				x0 = (int16_t)getInteger(use);
				break;
			case 1:
				y0 = (int16_t)getInteger(use);
				break;
			case 2:
				x1 = (int16_t)getInteger(use);
				break;
			case 3:
				y1 = (int16_t)getInteger(use);
				break;
			case 4:
				color = (uint16_t)getInteger(use);
				break;
			}

			deleteValueObject(val);
			deleteValueObject(use);
		}

		gfx_draw_line(x0, y0, x1, y1, color);
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Get current time in millis
 *
 * Format:
 * TIX
 */
ValueObject *lulz_interpret_millis_expr_node(ExprNode *node, ScopeObject *scope)
{
	return createIntegerValueObject(MILLIS());
}

/**
 * Get a BUKKIT of peers
 *
 * Format:
 * BADGEZ?
 *
 * BUKKIT Structure:
 *
 * BUKKIT
 *   + COUNT: Number of peers in the BUKKIT
 *   + PEERS: BUKKIT of peer objects
 *       + NAME: Name of peer
 */
ValueObject *lulz_interpret_peers_expr_node(ExprNode *node, ScopeObject *scope)
{
	peer_t *peers;
	ValueObject *bukkit = createArrayValueObject(scope);

	//Iterate through all peers
	uint16_t i = 0;
	for (peers = peers_hashtable_get(); peers != NULL; peers = peers->hh.next)
	{

		//Create a name value in the peer bukkit
		void *name = util_heap_alloc_ext(6);
		sprintf(name, "%d", i++);
		IdentifierNode *name_node = createIdentifierNode(IT_DIRECT, name, NULL, NULL, 0);
		ValueObject *name_value = createStringValueObject(copyString(peers->name));

		if (!createScopeValue(scope, getArray(bukkit), name_node))
		{
			deleteValueObject(name_value);
			deleteIdentifierNode(name_node);
			return NULL;
		}
		if (!updateScopeValue(scope, getArray(bukkit), name_node, name_value))
		{
			deleteValueObject(name_value);
			deleteIdentifierNode(name_node);
			return NULL;
		}
	}

	return bukkit;
}

/**
 * Execute circle command.
 *
 * Format: DOT <x> <y> <color>
 */
ReturnObject *lulz_interpret_pixel_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num >= 3)
	{
		int16_t x = 0, y = 0;
		color_565_t color = 0;
		for (uint32_t n = 0; n < stmt->args->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
			ValueObject *use = castIntegerExplicit(val, scope);

			if (!val || !use)
			{
				deleteValueObject(val);
				deleteValueObject(use);
				return NULL;
			}

			switch (n)
			{
			case 0:
				x = (int16_t)getInteger(use);
				break;
			case 1:
				y = (int16_t)getInteger(use);
				break;
			case 2:
				color = (color_565_t)getInteger(use);
				break;
			}

			deleteValueObject(val);
			deleteValueObject(use);
		}

		//Move the print to screen cursor
		gfx_draw_pixel(x, y, color);
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * 20hz task while playing bling
 */
void __play_bling_task(void *parameters)
{
	//20 hz timer
	TickType_t start;
	TickType_t ticks = (1000 / 20) / portTICK_PERIOD_MS;
	lulz_callback_data_t *p_bling_data = (lulz_callback_data_t *)parameters;
	uint8_t index = 0;
	int8_t direction = 1;
	float hue = 0;
	float value = 1;

	//Positions storage for roller coaster
	uint8_t positions[LED_PATTERN_ROLLER_COASTER_COUNT];
	for (uint8_t i = 0; i < LED_PATTERN_ROLLER_COASTER_COUNT; i++)
	{
		positions[i] = LED_COUNT - 1 - i;
	}

	//Triple sweep data
	led_triple_sweep_state_t triple_sweep = {
			.direction_red = 1,
			.direction_green = 1,
			.direction_blue = -1,
			.direction_yellow = -1,
			.index_red = 7,
			.index_green = 23,
			.index_blue = 7,
			.index_yellow = 23,
			.red = 255,
			.green = 255,
			.blue = 255,
			.yellow = 255,
	};

	led_pattern_balls_t balls;
	led_pattern_balls_init(&balls);

	while (p_bling_data->running)
	{
		start = xTaskGetTickCount();
		switch (p_bling_data->pattern)
		{
		case 1:
			led_pattern_hue(&hue);
			break;
		case 2:
			led_pattern_polar();
			break;
		case 3:
			led_pattern_sparkle(&index);
			break;
		case 4:
			led_pattern_double_sweep(&index, &hue, &value);
			break;
		case 5:
			led_pattern_flame();
			break;
		case 6:
			led_pattern_balls(&balls);
			break;
		case 7:
			led_pattern_running_lights(255, 0, 0, &index);
			break;
		case 8:
			led_pattern_running_lights(0, 255, 0, &index);
			break;
		case 9:
			led_pattern_running_lights(0, 0, 255, &index);
			break;
		case 10:
			led_pattern_running_lights(255, 255, 0, &index);
			break;
		case 11:
			led_pattern_kitt((int8_t *)&index, &direction);
			break;
		case 12:
			led_pattern_rainbow(&hue, 3);
			break;
		case 13:
			led_pattern_roller_coaster(positions, util_hsv_to_rgb(0, 1, 1));
			break;
		case 14:
			led_pattern_roller_coaster(positions, util_hsv_to_rgb(0.3, 1, 1));
			break;
		case 15:
			led_pattern_roller_coaster(positions, util_hsv_to_rgb(0.7, 1, 1));
			break;
		case 16:
			led_pattern_roller_coaster(positions, util_hsv_to_rgb(0.16, 1, 1));
			break;
		case 17:
			led_pattern_triple_sweep(&triple_sweep);
			break;
		}
		vTaskDelayUntil(&start, ticks);
	}

	led_clear();
	free(p_bling_data);
	vTaskDelete(NULL);
}

/**
 * Play bling on the screen
 *
 * Format: RICK ROLL WIT "/sdcard/rickroll.raw" <callback> <data> <loop>
 */
ReturnObject *lulz_interpret_play_bling_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num == 3)
	{
		char *path = NULL;
		uint8_t pattern = 0;
		bool loop = false;

		ValueObject *path_node = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *pattern_node = interpretExprNode(stmt->args->exprs[1], scope);
		ValueObject *loop_node = interpretExprNode(stmt->args->exprs[2], scope);
		ValueObject *path_str = castStringImplicit(path_node, scope);
		ValueObject *pattern_int = castIntegerExplicit(pattern_node, scope);
		ValueObject *loop_bool = castBooleanExplicit(loop_node, scope);

		path = copyString(getString(path_str));
		pattern = getInteger(pattern_int);
		loop = loop_bool->data.i;

		//Create bling data, this will be free-ed later when the task completes
		lulz_callback_data_t *p_bling_data = util_heap_alloc_ext(sizeof(lulz_callback_data_t));
		p_bling_data->pattern = pattern;
		p_bling_data->running = true;

		//Start background task for LEDs
		if (pattern > 0)
		{
			xTaskCreate(__play_bling_task, strrchr(path, '/'), 5000, p_bling_data, TASK_PRIORITY_PLAY_BLING, NULL);
		}

		//Play raw file in foreground
		gfx_play_raw_file(path, 0, 0, LCD_WIDTH, LCD_HEIGHT, NULL, loop, NULL);
		DELAY(50);
		p_bling_data->running = false;

		//Cleanup
		free(path);
		deleteValueObject(path_node);
		deleteValueObject(pattern_node);
		deleteValueObject(loop_node);
		deleteValueObject(path_str);
		deleteValueObject(pattern_int);
		deleteValueObject(loop_bool);
	}
	else
	{
		ESP_LOGE(TAG, "Invalid arguments to RICK ROLL WIT . Valid format is RICK ROLL WIT <file> <pattern> <loop>");
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

ReturnObject *lulz_interpret_print_screen_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	for (uint32_t n = 0; n < stmt->args->num; n++)
	{
		ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
		ValueObject *use = castStringExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}

		//Actually print to the screen
		gfx_print(getString(use));

		deleteValueObject(val);
		deleteValueObject(use);
	}
	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Push the screen buffer to the LCD
 *
 * Syntax:
 * LOLOLOL
 */
ReturnObject *lulz_interpret_push_buffer_stmt_node(StmtNode *node, ScopeObject *scope)
{
	gfx_push_screen_buffer();
	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

/**
 * Interpret RGB to 565 command.
 *
 * Format: ROYGEEBIF2 <r> <g> <b>
 */
ValueObject *lulz_interpret_rgb_to_565_expr_node(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	uint8_t r = 0, g = 0, b = 0;
	color_565_t color = 0;

	if (expr->expr1 && expr->expr2 && expr->expr3)
	{

		ValueObject *rval = interpretExprNode(expr->expr1, scope);
		ValueObject *gval = interpretExprNode(expr->expr2, scope);
		ValueObject *bval = interpretExprNode(expr->expr3, scope);
		ValueObject *ruse = castIntegerImplicit(rval, scope);
		ValueObject *guse = castIntegerImplicit(gval, scope);
		ValueObject *buse = castIntegerImplicit(bval, scope);

		if (!rval || !gval || !bval || !ruse || !guse || !buse)
		{
			deleteValueObject(rval);
			deleteValueObject(gval);
			deleteValueObject(bval);
			deleteValueObject(ruse);
			deleteValueObject(guse);
			deleteValueObject(buse);
			return NULL;
		}

		r = getInteger(ruse);
		g = getInteger(guse);
		b = getInteger(buse);

		deleteValueObject(rval);
		deleteValueObject(gval);
		deleteValueObject(bval);
		deleteValueObject(ruse);
		deleteValueObject(guse);
		deleteValueObject(buse);

		//Generate a 565 color, if they gave us bad data, they get a bad color
		color = util_rgb_to_565_discreet(r, g, b);
	}

	return createIntegerValueObject(color);
}

static ReturnObject *__interpret_rect_stmt_node(StmtNode *node, ScopeObject *scope, bool filled)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	if (stmt->args->num >= 5)
	{
		int16_t x = 0, y = 0, w = 0, h = 0;
		uint16_t color = 0;
		for (uint32_t n = 0; n < stmt->args->num; n++)
		{
			ValueObject *val = interpretExprNode(stmt->args->exprs[n], scope);
			ValueObject *use = castIntegerImplicit(val, scope);

			if (!val || !use)
			{
				deleteValueObject(val);
				deleteValueObject(use);
				return NULL;
			}

			switch (n)
			{
			case 0:
				x = (int16_t)getInteger(use);
				break;
			case 1:
				y = (int16_t)getInteger(use);
				break;
			case 2:
				w = (int16_t)getInteger(use);
				break;
			case 3:
				h = (int16_t)getInteger(use);
				break;
			case 4:
				color = (uint16_t)getInteger(use);
				break;
			}

			deleteValueObject(val);
			deleteValueObject(use);
		}

		//Actually draw the rectangle
		if (filled)
		{
			gfx_fill_rect(x, y, w, h, color);
		}
		else
		{
			gfx_draw_rect(x, y, w, h, color);
		}
	}

	ReturnObject *ret = createReturnObject(RT_DEFAULT, NULL);
	return ret;
}

ReturnObject *lulz_interpret_rect_stmt_node(StmtNode *node, ScopeObject *scope)
{
	return __interpret_rect_stmt_node(node, scope, false);
}

/**
 * Get a random number below max (e.g., 100 will return a random number 0 to 99)
 * Format: CRAZY GO NUTS <max>
 */
ValueObject *lulz_interpret_random_expr_node(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1)
	{

		//Evaluate the expressio
		ValueObject *val = interpretExprNode(expr->expr1, scope);
		ValueObject *int_val = castIntegerExplicit(val, scope);

		if (!val || !int_val)
		{
			deleteValueObject(val);
			deleteValueObject(int_val);
			return NULL;
		}

		//Get the max value
		uint32_t max = getInteger(int_val);
		//Generate the random value
		uint32_t r = util_random(0, max);

		//Build return object
		ValueObject *ret = createIntegerValueObject(r);

		//Cleanup
		deleteValueObject(val);
		deleteValueObject(int_val);

		//If we got a valid return value earlier, return it otherwise fall out
		//of if block and return nil
		if (ret)
		{
			return ret;
		}
	}
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to CRAZY GO NUTS");
	}

	return createNilValueObject();
}

ReturnObject *lulz_interpret_rect_filled_stmt_node(StmtNode *node, ScopeObject *scope)
{
	return __interpret_rect_stmt_node(node, scope, true);
}

ReturnObject *lulz_interpret_run_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;
	if (stmt->args->num == 1)
	{
		ValueObject *val = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *use = castStringExplicit(val, scope);
		char *file_name = getString(use);
		lolcode_execute(file_name, true);
	}
	else
	{
		ESP_LOGE(TAG, "Exactly one argument expected to TROLL");
	}

	return createReturnObject(RT_DEFAULT, NULL);
	;
}

/**
 * Get the unique device serial number
 * Format: WHOAMI?
 */
ValueObject *lulz_interpret_serial_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	return createIntegerValueObject(util_serial_get());
}

/**
 * Get badge state value
 * Format: OHIMEMBER <key>
 */
ValueObject *lulz_interpret_state_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1)
	{
		char *key = NULL;
		char *value = NULL;

		//Evaluate the expressio
		ValueObject *val = interpretExprNode(expr->expr1, scope);
		ValueObject *use = castStringExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}

		//Get the key from LULZCODE
		key = getString(use);
		if (strcmp(key, "brightness") == 0)
		{
			value = util_heap_alloc_ext(4);
			sprintf(value, "%d", state_brightness_get());
		}
		else if (strcmp(key, "name") == 0)
		{
			value = util_heap_alloc_ext(STATE_NAME_LENGTH + 1);
			state_name_get(value);
		}
		else if (strcmp(key, "botnet_level") == 0)
		{
			value = util_heap_alloc_ext(4);
			sprintf(value, "%d", state_botnet_get()->level);
		}
		else if (strcmp(key, "botnet_experience") == 0)
		{
			value = util_heap_alloc_ext(6);
			snprintf(value, 6, "%d", state_botnet_get()->experience);
		}
		else if (strcmp(key, "botnet_name") == 0)
		{
			value = util_heap_alloc_ext(BOTNET_ID_LENGTH + 1);
			snprintf(value, BOTNET_ID_LENGTH + 1, "%02x", state_botnet_get()->botnet_id);
		}
		else if (strcmp(key, "botnet_points") == 0)
		{
			value = util_heap_alloc_ext(6);
			sprintf(value, "%d", state_botnet_get()->points);
		}
		else if (strcmp(key, "voltage") == 0)
		{
			value = util_heap_alloc_ext(8);
			sprintf(value, "%2.1f", battery_voltage());
		}
		else if (strcmp(key, "score_pong") == 0)
		{
			value = util_heap_alloc_ext(6);
			sprintf(value, "%d", state_score_pong_get());
		}
		else if (strcmp(key, "score_ski") == 0)
		{
			value = util_heap_alloc_ext(6);
			sprintf(value, "%d", state_score_ski_get());
		}

		//Build return object
		ValueObject *ret = NULL;
		//If value is not null, lets return it
		//If valid result from badge state, create a return value
		if (value)
		{
			//Return a copy of the value, otherwise lulzcode will try to free the
			//global value off the heap later
			ret = createStringValueObject(copyString(value));
		}
		else
		{
			ESP_LOGE(TAG, "Unknown badge state key '%s' passed to OHIMEMBER.", key);
		}

		//Cleanup
		deleteValueObject(val);
		deleteValueObject(use);
		if (value)
			free(value);

		//If we got a valid return value earlier, return it otherwise fall out
		//of if block and return nil
		if (ret)
		{
			return ret;
		}
	}
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to OHIMEMBER");
	}

	return createNilValueObject();
}

/**
 * Set badge state value
 * Format: MEMBER <key> <value>
 */
ReturnObject *lulz_interpret_state_set_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;

	//Must be exactly two arguments
	if (stmt->args->num == 2)
	{
		ValueObject *key = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *value = interpretExprNode(stmt->args->exprs[1], scope);
		ValueObject *keystr = castStringExplicit(key, scope);

		if (strcmp(getString(keystr), "name") == 0)
		{
			ValueObject *valuestr = castStringExplicit(value, scope);
			state_name_set(getString(valuestr));
			state_save_indicate();
			deleteValueObject(valuestr);
		}
		else if (strcmp(getString(keystr), "brightness") == 0)
		{
			ValueObject *valueint = castIntegerExplicit(value, scope);
			state_brightness_set((uint8_t)MIN(getInteger(valueint), UINT8_MAX));
			state_save_indicate();
			deleteValueObject(valueint);

			util_update_global_brightness();
		}
		else if (strcmp(getString(keystr), "score_pong") == 0)
		{
			ValueObject *valueint = castIntegerExplicit(value, scope);
			state_score_pong_set((uint32_t)MIN(getInteger(valueint), UINT32_MAX));
			state_save_indicate();
			deleteValueObject(valueint);
		}
		else
		{
			ESP_LOGE(TAG, "Unknown MEMBER key value.");
		}

		deleteValueObject(key);
		deleteValueObject(value);
		deleteValueObject(keystr);
	}
	else
	{
		ESP_LOGE(TAG, "MEMBER expects two arguments.");
	}

	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Get char at index
 * Format: CUT <YARN> <NUMBR>
 */
ValueObject *lulz_interpret_string_get_at_index(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1 && expr->expr2)
	{
		char *str = NULL;
		uint32_t index = 0;

		//Evaluate the expression
		ValueObject *str_node = interpretExprNode(expr->expr1, scope);
		ValueObject *index_node = interpretExprNode(expr->expr2, scope);
		ValueObject *str_val = castStringExplicit(str_node, scope);
		ValueObject *index_val = castIntegerExplicit(index_node, scope);

		if (!str_node || !index_node || !str_val || !index_val)
		{
			deleteValueObject(str_node);
			deleteValueObject(index_node);
			deleteValueObject(str_val);
			deleteValueObject(index_val);
			return NULL;
		}

		str = getString(str_val);
		index = getInteger(index_val);

		char c = 0;

		if (index < strlen(str))
		{
			c = str[index];
		}

		//Build return object
		ValueObject *ret = createIntegerValueObject((uint8_t)c);

		//Cleanup
		deleteValueObject(str_node);
		deleteValueObject(index_node);
		deleteValueObject(str_val);
		deleteValueObject(index_val);

		//		if (str)
		//			free(str);

		return ret;
	}
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to CUT");
	}

	return createNilValueObject();
}

/**
 * Get char at index
 * Format: CUT <YARN> <NUMBR>
 */
ValueObject *lulz_interpret_string_set_at_index(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1 && expr->expr2 && expr->expr3)
	{
		char *str = NULL;
		uint32_t index = 0;
		char *insert = NULL;

		//Evaluate the expression
		ValueObject *str_node = interpretExprNode(expr->expr1, scope);
		ValueObject *index_node = interpretExprNode(expr->expr2, scope);
		ValueObject *insert_node = interpretExprNode(expr->expr3, scope);

		ValueObject *str_val = castStringExplicit(str_node, scope);
		ValueObject *index_val = castIntegerExplicit(index_node, scope);
		ValueObject *insert_val = castStringExplicit(insert_node, scope);

		if (!str_node || !index_node || !insert_node || !str_val || !index_val || !insert_val)
		{
			deleteValueObject(str_node);
			deleteValueObject(index_node);
			deleteValueObject(insert_node);
			deleteValueObject(str_val);
			deleteValueObject(index_val);
			deleteValueObject(insert_val);
			return NULL;
		}

		str = getString(str_val);
		index = getInteger(index_val);
		insert = getString(insert_val);

		ValueObject *ret = NULL;

		//Make sure valid index. We may consider expanding the string length with spaces or null
		//characters if index > strlen - may be a bad idea though
		if (insert != NULL && (index + strlen(insert)) <= strlen(str))
		{
			memcpy(str + index, insert, strlen(insert));

			//Build return object
			ret = createStringValueObject(copyString(str));
		}
		else
		{
			ret = createNilValueObject();
			ESP_LOGD(TAG, "Invalid index passed to YO");
		}

		//Cleanup
		deleteValueObject(str_node);
		deleteValueObject(index_node);
		deleteValueObject(insert_node);
		deleteValueObject(str_val);
		deleteValueObject(index_val);
		deleteValueObject(insert_val);

		return ret;
	}
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to YO. Expected YO <YARN> <NUMBR> <LETR>");
	}

	return createNilValueObject();
}

/**
 * Get string length
 * Format: KOUNT <YARN>
 */
ValueObject *lulz_interpret_string_length(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1)
	{
		//Evaluate the expression
		ValueObject *str_node = interpretExprNode(expr->expr1, scope);
		ValueObject *str_val = castStringExplicit(str_node, scope);

		if (!str_node || !str_val)
		{
			deleteValueObject(str_node);
			deleteValueObject(str_val);
			return NULL;
		}

		ValueObject *ret = createIntegerValueObject(strlen(getString(str_val)));

		//Cleanup
		deleteValueObject(str_node);
		deleteValueObject(str_val);

		return ret;
	}
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to KOUNT");
	}

	return createNilValueObject();
}

/**
 * Return a substring of a YARN
 *
 * Format:
 * GNAW <YARN> <Start> <End>
 */
ValueObject *lulz_interpret_string_substring(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1 && expr->expr2 && expr->expr3)
	{
		char *str = NULL;
		uint32_t start_index = 0;
		uint32_t end_index = 0;

		//Evaluate the expression
		ValueObject *str_node = interpretExprNode(expr->expr1, scope);
		ValueObject *start_index_node = interpretExprNode(expr->expr2, scope);
		ValueObject *end_index_node = interpretExprNode(expr->expr3, scope);

		ValueObject *str_val = castStringExplicit(str_node, scope);
		ValueObject *start_index_val = castIntegerExplicit(start_index_node, scope);
		ValueObject *end_index_val = castIntegerExplicit(end_index_node, scope);

		if (!str_node || !start_index_node || !str_val || !start_index_val)
		{
			deleteValueObject(str_node);
			deleteValueObject(start_index_node);
			deleteValueObject(end_index_node);
			deleteValueObject(str_val);
			deleteValueObject(start_index_val);
			deleteValueObject(end_index_val);
			return NULL;
		}

		str = getString(str_val);

		//Get indices ensuring safety
		start_index = MAX(getInteger(start_index_val), 0);
		end_index = MIN(getInteger(end_index_val), strlen(str));
		int32_t length = end_index - start_index;

		//Make sure we have valid length
		ValueObject *ret = NULL;
		if (length > 0)
		{
			char *substring = (char *)util_heap_alloc_ext(length + 1);
			snprintf(substring, length + 1, "%s", str + start_index);
			ret = createStringValueObject(substring);
		}
		else
		{
			ESP_LOGE(TAG, "Unable to GNAW on YARN, invalid parameters");
			ret = createStringValueObject(copyString(""));
		}

		//Cleanup
		deleteValueObject(str_node);
		deleteValueObject(start_index_node);
		deleteValueObject(end_index_node);
		deleteValueObject(str_val);
		deleteValueObject(start_index_val);
		deleteValueObject(end_index_val);

		return ret;
	}
	else
	{
		ESP_LOGE(TAG, "Invalid argument(s) to GNAW. Expected GNAW <YARN> <NUMBR> <NUMBR>");
	}

	return createNilValueObject();
}

/**
 * @brief Interpret LULZCODE SYSTUM command
 * Format:
 * 	SYSTUM "<command>"
 */
ReturnObject *lulz_interpret_system_call_stmt_node(StmtNode *node, ScopeObject *scope)
{
	lulz_generic_stmt_node *stmt = (lulz_generic_stmt_node *)node->stmt;
	if (stmt->args->num == 1)
	{
		ValueObject *val = interpretExprNode(stmt->args->exprs[0], scope);
		ValueObject *use = castStringExplicit(val, scope);
		char *command = getString(use);
		if (strcmp(command, "ABOUT") == 0)
		{
			ui_about();
		}
		else if (strcmp(command, "ADDON_START") == 0)
		{
			addons_start();
		}
		else if (strcmp(command, "ADDON_STOP") == 0)
		{
			addons_stop();
		}
		else if (strcmp(command, "INTERRUPTS_ON") == 0)
		{
			ui_allow_interrupt(true);
		}
		else if (strcmp(command, "INTERRUPTS_OFF") == 0)
		{
			ui_allow_interrupt(false);
		}
		else if (strcmp(command, "BOTNET") == 0)
		{
			botnet_ui_main();
		}
		else if (strcmp(command, "NAME") == 0)
		{
			ui_pick_name();
		}
		else if (strcmp(command, "POST") == 0)
		{
			post_screen();
		}
		else if (strcmp(command, "SHOUTS") == 0)
		{
			ui_shouts();
		}
		// else if (strcmp(command, "SYNC_SD") == 0)
		// {
		// 	sync_start();
		// }
		else if (strcmp(command, "STATE_RESET") == 0)
		{
			unlink(STATE_FILE);
			esp_restart();
		}
#ifdef CONFIG_BADGE_TYPE_MASTER
		else if (strcmp(command, "CHEAT_LEVEL_UP") == 0)
		{
			botnet_level_up();
		}
#endif
		else if (strcmp(command, "SKI") == 0)
		{
			ski();
		}
		else if (strcmp(command, "TILT_ON") == 0)
		{
			state_tilt_set(true);
		}
		else if (strcmp(command, "TILT_OFF") == 0)
		{
			state_tilt_set(false);
		}
		else if (strcmp(command, "TIME") == 0)
		{
			ui_time();
		}
		else
		{
			ESP_LOGE(TAG, "Invalid system call '%s'", command);
		}
	}
	else
	{
		ESP_LOGE(TAG, "TROLL expects one argument");
	}

	return createReturnObject(RT_DEFAULT, NULL);
}

/**
 * Get the height of a string
 * Format: HOW BIG <YARN>
 */
ValueObject *lulz_interpret_text_height_expr_node(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1)
	{
		char *text = NULL;

		//Evaluate the expressio
		ValueObject *val = interpretExprNode(expr->expr1, scope);
		ValueObject *use = castStringExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}

		//Get the string to measure the height of
		text = getString(use);

		//Calculate the height
		int16_t x, y;
		uint16_t width = 0;
		uint16_t height = 0;
		gfx_text_bounds(text, 0, 0, &x, &y, &width, &height);
		ValueObject *ret = createIntegerValueObject(height);

		//Cleanup
		deleteValueObject(val);
		deleteValueObject(use);

		//If we got a valid return value earlier, return it otherwise fall out
		//of if block and return nil
		if (ret)
		{
			return ret;
		}
	}

	ESP_LOGE(TAG, "Invalid or missing expression in HOW BIG");
	return createIntegerValueObject(0);
}

/**
 * Get the width of a string
 * Format: HOW SPREAD <YARN>
 */
ValueObject *lulz_interpret_text_width_expr_node(ExprNode *node, ScopeObject *scope)
{
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1)
	{
		char *text = NULL;

		//Evaluate the expression
		ValueObject *val = interpretExprNode(expr->expr1, scope);
		ValueObject *use = castStringExplicit(val, scope);

		if (!val || !use)
		{
			deleteValueObject(val);
			deleteValueObject(use);
			return NULL;
		}

		//Get the string to measure the height of
		text = getString(use);

		//Calculate the width
		int16_t x, y;
		uint16_t width = 0;
		uint16_t height = 0;
		gfx_text_bounds(text, 0, 0, &x, &y, &width, &height);
		ValueObject *ret = createIntegerValueObject(width);

		//Cleanup
		deleteValueObject(val);
		deleteValueObject(use);

		//If we got a valid return value earlier, return it otherwise fall out
		//of if block and return nil
		if (ret)
		{
			return ret;
		}
	}

	ESP_LOGE(TAG, "Invalid or missing expression in HOW SPREAD");
	return createIntegerValueObject(0);
}

/**
 * Get the current unlock state
 * Format: L33T
 */
ValueObject *lulz_interpret_unlock_get_expr_node(ExprNode *node, ScopeObject *scope)
{
	return createIntegerValueObject(state_unlock_get());
}

/**
 * Validates an unlock code. True if successful
 * Format: CHEEZBURGER "code"
 */
ValueObject *lulz_interpret_unlock_validate_expr_node(ExprNode *node, ScopeObject *scope)
{
	bool valid = false;
	lulz_generic_expr_node *expr = (lulz_generic_expr_node *)node->expr;

	if (expr->expr1)
	{

		//Evaluate the expression
		ValueObject *value_node = interpretExprNode(expr->expr1, scope);
		ValueObject *str_node = castStringExplicit(value_node, scope);

		if (!value_node || !str_node)
		{
			deleteValueObject(value_node);
			deleteValueObject(str_node);
			return NULL;
		}

		//Get the code as a string
		const char *code = getString(str_node);

		//Validate the unlock code
		valid = util_validate_unlock(code);

		//Cleanup
		deleteValueObject(value_node);
		deleteValueObject(str_node);
	}
	else
	{
		ESP_LOGE(TAG, "Invalid or missing expression in CHEEZBURGER");
	}
	return createBooleanValueObject(valid);
}

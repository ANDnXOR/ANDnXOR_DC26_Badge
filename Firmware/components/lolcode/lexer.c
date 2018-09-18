#include <stdbool.h>

#include "system.h"

#include "lexer.h"

const static char *TAG = "MRMEESEEKS::lulzcode::lexer";

/**
 * Creates a lexeme.
 *
 * \param [in] image The string that identifies the lexeme.
 *
 * \param [in] fname The name of the file containing the lexeme.
 *
 * \param [in] line  The line number the lexeme occurred on.
 *
 * \return A new lexeme with the desired properties.
 *
 * \retval NULL Memory allocation failed.
 */
Lexeme *createLexeme(char *image, const char *fname, unsigned int line)
{
	Lexeme *ret = util_heap_alloc_ext(sizeof(Lexeme));
	if (!ret) {
		perror("malloc");
		return NULL;
	}
	ret->image = util_heap_alloc_ext(sizeof(char) * (strlen(image) + 1));
	if (!(ret->image)) {
		free(ret);
		perror("malloc");
		return NULL;
	}
	strcpy(ret->image, image);
	/**
	 * \note \a fname is not copied because it only one copy is stored for
	 * all lexemes from the same file.  This is simply to avoid large
	 * numbers of lexemes storing duplicate file name strings.
	 */
	ret->fname = fname;
	ret->line = line;
#ifdef DEBUG
	fprintf(stderr, "Creating lexeme [%s]\n", image);
#endif
	return ret;
}

/**
 * Deletes a lexeme.
 *
 * \param [in,out] lexeme The lexeme to delete.
 */
void deleteLexeme(Lexeme *lexeme)
{
	if (!lexeme)
		return;
	free(lexeme->image);
	/**
	 * \note We do not free the file name because it is shared between many
	 * lexemes and is freed by whomever created the file name string.
	 */
	free(lexeme);
}

/**
 * Creates a list of lexemes.
 *
 * \return An empty lexeme list.
 *
 * \retval NULL Memory allocation failed.
 */
LexemeList *createLexemeList(void)
{
	LexemeList *p = util_heap_alloc_ext(sizeof(LexemeList));
	if (!p) {
		perror("malloc");
		return NULL;
	}
	p->num = 0;
	p->lexemes = NULL;
	return p;
}

/**
 * Adds a lexeme to a list of lexemes.
 *
 * \param [in,out] list The list of lexemes to add \a lexeme to.
 *
 * \param [in] lexeme The lexeme to add to \a list.
 *
 * \post \a lexeme will be added to the end of \a list and the size of \a list
 * will be updated.
 *
 * \return A pointer to the added lexeme (will be the same as \a lexeme).
 *
 * \retval NULL Memory allocation failed.
 */
Lexeme *addLexeme(LexemeList *list, Lexeme *lexeme)
{
	unsigned int newsize;
	void *mem = NULL;
	if (!list)
		return NULL;
	newsize = list->num + 1;
	mem = util_heap_realloc_ext(list->lexemes, sizeof(Lexeme *) * newsize);
	if (!mem) {
		perror("realloc addLexeme()");
		return NULL;
	}
	list->lexemes = mem;
	list->lexemes[list->num] = lexeme;
	list->num = newsize;
	return lexeme;
}

/**
 * Deletes a list of lexemes.
 *
 * \param [in,out] list The lexeme list to delete.
 *
 * \post The memory at \a list and all of its members will be freed.
 */
void deleteLexemeList(LexemeList *list)
{
	unsigned int n;
	if (!list)
		return;
	for (n = 0; n < list->num; n++)
		deleteLexeme(list->lexemes[n]);
	free(list->lexemes);
	free(list);
}

/**
 * Scans a buffer, removing unnecessary characters and grouping characters into
 * lexemes.  Lexemes are strings of characters separated by whitespace (although
 * newline characters are considered separate lexemes).  String literals are
 * handled a bit differently:  Starting at the first quotation character,
 * characters are collected until either a non-escaped quotation character is
 * read (i.e., a quotation character not preceded by a colon which itself is not
 * preceded by a colon) or a newline or carriage return character is read,
 * whichever comes first.  This handles the odd (but possible) case of strings
 * such as "::" which print out a single colon.  Also handled are the effects of
 * commas, ellipses, bangs (!), and array accesses ('Z).
 *
 * \param [in] buffer The characters to turn into lexemes.
 *
 * \param [in] size The number of characters in \a buffer.
 *
 * \param [in] fname The name of the file \a buffer was read from.
 *
 * \param [in] is_include True if scanning an include file
 *
 * \return A list of lexemes created from the contents of \a buffer.
 */
LexemeList *lulz_scan_buffer(const char *buffer, unsigned int size, const char *fname, bool is_include) {
	const char *start = buffer;
	LexemeList *list = NULL;
	unsigned int line = 1;
	list = createLexemeList();
	if (!list)
		return NULL;
	while (start < buffer + size) {

		char *temp = NULL;
		unsigned int len = 1;
		/* Comma (,) is a soft newline */
		if (*start == ',') {
			Lexeme *lex = createLexeme("\n", fname, line);
			if (!lex) {
				deleteLexemeList(list);
				return NULL;
			}
			if (!addLexeme(list, lex)) {
				deleteLexeme(lex);
				deleteLexemeList(list);
				return NULL;
			}
			start++;
			continue;
		}
		/* Bang (!) is its own lexeme */
		if (*start == '!') {
			Lexeme *lex = createLexeme("!", fname, line);
			if (!lex) {
				deleteLexemeList(list);
				return NULL;
			}
			if (!addLexeme(list, lex)) {
				deleteLexeme(lex);
				deleteLexemeList(list);
				return NULL;
			}
			start++;
			continue;
		}
		/* Apostrophe Z ('Z) is its own lexeme */
		if (!strncmp(start, "'Z", 2)) {
			Lexeme *lex = createLexeme("'Z", fname, line);
			if (!lex) {
				deleteLexemeList(list);
				return NULL;
			}
			if (!addLexeme(list, lex)) {
				deleteLexeme(lex);
				deleteLexemeList(list);
				return NULL;
			}
			start += 2;
			continue;
		}

		/* Skip over leading whitespace */
		while (isspace((unsigned char )*start)) {
			unsigned int newline = 0;
			/* Newline is its own lexeme */
			if (!strncmp(start, "\r\n", 2)) {
				newline = 1;
				start++;
			}
			else if (*start == '\r' || *start == '\n') {
				newline = 1;
			}
			if (newline) {
				Lexeme *lex = createLexeme("\n", fname, line);
				if (!lex) {
					deleteLexemeList(list);
					return NULL;
				}
				if (!addLexeme(list, lex)) {
					deleteLexeme(lex);
					deleteLexemeList(list);
					return NULL;
				}
				line++;
			}
			start++;
			continue;
		}
		/* Skip over ellipses (...) and newline */
		if ((!strncmp(start, "\xE2\x80\xA6\r\n", 5) && (start += 5))
				|| (!strncmp(start, "\xE2\x80\xA6\r", 4) && (start += 4))
				|| (!strncmp(start, "\xE2\x80\xA6\n", 4) && (start += 4))
				|| (!strncmp(start, "...\r\n", 5) && (start += 5))
				|| (!strncmp(start, "...\r", 4) && (start += 4))
				|| (!strncmp(start, "...\n", 4) && (start += 4))) {
			const char *test = start;
			/* Make sure next line is not empty */
			while (*test && isspace((unsigned char )*test)) {
				if (*test == '\r' || *test == '\n') {
					lulz_error(LX_LINE_CONTINUATION, fname, line);
					deleteLexemeList(list);
					return NULL;
				}
				test++;
			}
			continue;
		}
		/* Skip over comments */
		if ((list->num == 0
				|| *(list->lexemes[list->num - 1]->image) == '\n')
				&& !strncmp(start, "OBTW", 4)) {
			start += 4;
			while (strncmp(start, "TLDR", 4)) {
				if ((!strncmp(start, "\r\n", 2) && (start += 2))
						|| (*start == '\r' && start++)
						|| (*start == '\n' && start++))
					line++;
				else
					start++;
			}
			start += 4;
			/* Must end in newline */
			while (*start && isspace((unsigned char )*start) && *start != '\r' && *start != '\n')
				start++;
			if (start == buffer || *start == ',' || *start == '\r' || *start == '\n')
				continue;
			lulz_error(LX_MULTIPLE_LINE_COMMENT, fname, line);
			deleteLexemeList(list);
			return NULL;
		}
		if (!strncmp(start, "BTW", 3)) {
			start += 3;
			while (*start && *start != '\r' && *start != '\n')
				start++;
			continue;
		}

		/* LULZCODE addition, parse FREND preprocessor include recursively */
		if (strncmp(start, "FREND", 5) == 0) {
			char *fname = util_heap_alloc_ext(128);
			memset(fname, '\0', 128);

			start = strchr(start, '"') + 1;
			unsigned int i = 0;
			while (*start != '"' && i < 127) {
				fname[i] = *start;
				start++;
				i++;
			}

			//Advance the current position to the next line
			start = strchr(start, '\n') + 1;
			line++;
			util_heap_stats_dump();

			//Limit the size of FRENDs
			uint32_t fsize = util_file_size(fname);
			if (fsize > 500000) {
				ESP_LOGE(TAG, "FREND '%s' too big.", fname);
				free(fname);
				return false;
			}

			//Read file to include
			FILE *ffd = fopen(fname, "r");
			if (ffd == NULL) {
				ESP_LOGE(TAG, "Failed to open file for reading");
				free(fname);
				return false;
			}

			//File exists, load it
			else {
				ESP_LOGD(TAG, "Parsing FREND '%s' Size = %d bytes", fname, fsize);
				char *inc = (char *) util_heap_alloc_ext(fsize + 1);
				memset(inc, 0, fsize + 1);
				i = 0;
				while (i < fsize + 1) {
					int c = fgetc(ffd);
					if (c == EOF) {
						break;
					}
					inc[i++] = c;
				}
				fclose(ffd);

				LexemeList *inc_lexemes = lulz_scan_buffer(inc, i, "FREND", true);
				for (i = 0; i < inc_lexemes->num; i++) {
					addLexeme(list, inc_lexemes->lexemes[i]);
				}

				ESP_LOGD(TAG, "Done parsing FREND '%s', %d lexemes added", fname, inc_lexemes->num);
//				util_heap_stats_dump();

				free(fname);
				free(inc);
				free(inc_lexemes->lexemes);
				free(inc_lexemes);
				taskYIELD();
			}

			continue;
		}

		/* We have removed or processed any leading characters at this
		 * point */
		if (!*start)
			break;
		if (*start == '"') {
			/* Find the end of the string, watching for escape sequences */
			while ((start[len]
					&& *(start + len) != '\r'
					&& *(start + len) != '\n'
					&& *(start + len) != '"')
					|| (*(start + len) == '"'
							&& *(start + len - 1) == ':'
							&& *(start + len - 2) != ':'))
				len++;
			if (*(start + len) == '"')
				len++;
			/* Make sure this is the end of the token */
			if (start[len] && !isspace((unsigned char )start[len])
					&& *(start + len) != ','
					&& *(start + len) != '!'
					&& strncmp(start + len, "'Z", 2)
					&& strncmp(start + len, "...", 3)
					&& strncmp(start + len, "\xE2\x80\xA6", 3)) {
				lulz_error(LX_EXPECTED_TOKEN_DELIMITER, fname, line);
				deleteLexemeList(list);
				return NULL;
			}
		}
		else {
			/* Scan for the end of the token */
			while (start[len] && !isspace((unsigned char )start[len])
					&& *(start + len) != ','
					&& *(start + len) != '!'
					&& strncmp(start + len, "'Z", 2)
					&& strncmp(start + len, "...", 3)
					&& strncmp(start + len, "\xE2\x80\xA6", 3))
				len++;
		}
		temp = util_heap_alloc_ext(sizeof(char) * (len + 1));
		if (!temp) {
			perror("malloc");
			deleteLexemeList(list);
			return NULL;
		}
		strncpy(temp, start, len);
		temp[len] = '\0';
		Lexeme *lex = createLexeme(temp, fname, line);
		if (!lex) {
			free(temp);
			deleteLexemeList(list);
			return NULL;
		}
		if (!addLexeme(list, lex)) {
			free(temp);
			deleteLexeme(lex);
			deleteLexemeList(list);
			return NULL;
		}
		free(temp);
		start += len;
	}

	//Finally add an EOF lexeme if this is the outer-most buffer
	if (!is_include) {
		/* Create an end-of-file lexeme */
		Lexeme *lex = createLexeme("$", fname, line);
		if (!lex) {
			deleteLexemeList(list);
			return NULL;
		}
		if (!addLexeme(list, lex)) {
			deleteLexeme(lex);
			deleteLexemeList(list);
			return NULL;
		}
	}
	return list;
}

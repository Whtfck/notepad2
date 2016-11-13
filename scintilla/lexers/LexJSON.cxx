// Lexer for JSON.

#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

static inline bool IsJsonOp(int ch) {
	return ch == '{' || ch == '}' || ch == '[' || ch == ']' || ch == ':' || ch == ',' || ch == '+' || ch == '-';
}

#define MAX_WORD_LENGTH	7
static void ColouriseJSONDoc(Sci_PositionU startPos, Sci_Position length, int initStyle, WordList *[], Accessor &styler) {
	const bool fold = styler.GetPropertyInt("fold", 1) != 0;

	int state = initStyle;
	int chNext = styler[startPos];
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	Sci_PositionU endPos = startPos + length;
	if (endPos == (Sci_PositionU)styler.Length())
		++endPos;

	Sci_Position lineCurrent = styler.GetLine(startPos);
	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent-1) >> 16;
	int levelNext = levelCurrent;
	char buf[MAX_WORD_LENGTH + 1] = {0};
	int wordLen = 0;

	for (Sci_PositionU i = startPos; i < endPos; i++) {
		int ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		const bool atLineStart = i == (Sci_PositionU)styler.LineStart(lineCurrent);
		if (atEOL || i == endPos-1) {
			if (fold) {
				int levelUse = levelCurrent;
				int lev = levelUse | levelNext << 16;
				if (levelUse < levelNext)
					lev |= SC_FOLDLEVELHEADERFLAG;
				if (lev != styler.LevelAt(lineCurrent)) {
					styler.SetLevel(lineCurrent, lev);
				}
				levelCurrent = levelNext;
			}
			lineCurrent++;
		}

		switch (state) {
		case SCE_C_OPERATOR:
			styler.ColourTo(i - 1, state);
			state = SCE_C_DEFAULT;
			break;
		case SCE_C_NUMBER:
			if (!(iswordchar(ch) || ((ch == '+' || ch == '-') && IsADigit(chNext)))) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_IDENTIFIER:
			if (!iswordstart(ch)) {
				buf[wordLen] = 0;
				if (!(strcmp(buf, "true") && strcmp(buf, "false") && strcmp(buf, "null"))) {
					styler.ColourTo(i - 1, SCE_C_WORD);
				}
				state = SCE_C_DEFAULT;
				wordLen = 0;
			} else if (wordLen < MAX_WORD_LENGTH) {
				buf[wordLen++] = (char)ch;
			}
			break;
		case SCE_C_STRING:
			if (atLineStart) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			} else if (ch == '\\' && (chNext == '\\' || chNext == '\"')) {
				i++;
				ch = chNext;
				chNext = styler.SafeGetCharAt(i + 1);
			} else if (ch == '\"') {
				int pos = i + 1;
				while (IsASpace(styler.SafeGetCharAt(pos++)));
				if (styler[pos-1] == ':') {
					styler.ColourTo(i, SCE_C_LABEL);
				} else {
					styler.ColourTo(i, SCE_C_STRING);
				}
				state = SCE_C_DEFAULT;
				continue;
			}
			break;
#if 1
		case SCE_C_COMMENTLINE:
			if (atLineStart) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_DEFAULT;
			}
			break;
		case SCE_C_COMMENT:
			if (ch == '*' && chNext == '/') {
				i++;
				chNext = styler.SafeGetCharAt(i + 1);
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
				levelNext--;
				continue;
			}
			break;
#endif
		}

		if (state == SCE_C_DEFAULT) {
#if 1
			if (ch == '/' && chNext == '/') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_COMMENTLINE;
			} else if (ch == '/' && chNext == '*') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_COMMENT;
				levelNext++;
				i++;
				chNext = styler.SafeGetCharAt(i + 1);
			} else
#endif
			if (ch == '\"') {
				styler.ColourTo(i - 1, state);
				state = SCE_C_STRING;
			} else if (IsADigit(ch) || (ch == '.' && IsADigit(chNext))) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_NUMBER;
			} else if (iswordstart(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_IDENTIFIER;
				buf[wordLen++] = (char)ch;
			} else if (IsJsonOp(ch)) {
				styler.ColourTo(i - 1, state);
				state = SCE_C_OPERATOR;
				if (ch == '{' || ch == '[') {
					levelNext++;
				} else if (ch == '}' || ch == ']') {
					levelNext--;
				}
			}
		}
	}

	// Colourise remaining document
	styler.ColourTo(endPos - 1, state);
}

LexerModule lmJSON(SCLEX_JSON, ColouriseJSONDoc, "json");

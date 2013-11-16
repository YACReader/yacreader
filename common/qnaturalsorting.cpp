/* This file contains parts of the KDE libraries
   Copyright (C) 1999 Ian Zepp (icszepp@islc.net)
   Copyright (C) 2006 by Dominic Battre <dominic@battre.de>
   Copyright (C) 2006 by Martin Pool <mbp@canonical.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "qnaturalsorting.h"

//from KDE
/*
int naturalCompare(const QString &_a, const QString &_b, Qt::CaseSensitivity caseSensitivity)
{
	// This method chops the input a and b into pieces of
	// digits and non-digits (a1.05 becomes a | 1 | . | 05)
	// and compares these pieces of a and b to each other
	// (first with first, second with second, ...).
	//
	// This is based on the natural sort order code code by Martin Pool
	// http://sourcefrog.net/projects/natsort/
	// Martin Pool agreed to license this under LGPL or GPL.

	// FIXME: Using toLower() to implement case insensitive comparison is
	// sub-optimal, but is needed because we compare strings with
	// localeAwareCompare(), which does not know about case sensitivity.
	// A task has been filled for this in Qt Task Tracker with ID 205990.
	// http://trolltech.com/developer/task-tracker/index_html?method=entry&id=205990
	QString a;
	QString b;
	if (caseSensitivity == Qt::CaseSensitive) {
		a = _a;
		b = _b;
	} else {
		a = _a.toLower();
		b = _b.toLower();
	}

	const QChar* currA = a.unicode(); // iterator over a
	const QChar* currB = b.unicode(); // iterator over b

	if (currA == currB) {
		return 0;
	}

	const QChar* begSeqA = currA; // beginning of a new character sequence of a
	const QChar* begSeqB = currB;

	while (!currA->isNull() && !currB->isNull()) {
		if (currA->unicode() == QChar::ObjectReplacementCharacter) {
			return 1;
		}

		if (currB->unicode() == QChar::ObjectReplacementCharacter) {
			return -1;
		}

		if (currA->unicode() == QChar::ReplacementCharacter) {
			return 1;
		}

		if (currB->unicode() == QChar::ReplacementCharacter) {
			return -1;
		}

		// find sequence of characters ending at the first non-character
		while (!currA->isNull() && !currA->isDigit() && !currA->isPunct() && !currA->isSpace()) {
			++currA;
		}

		while (!currB->isNull() && !currB->isDigit() && !currB->isPunct() && !currB->isSpace()) {
			++currB;
		}

		// compare these sequences
		const QStringRef& subA(a.midRef(begSeqA - a.unicode(), currA - begSeqA));
		const QStringRef& subB(b.midRef(begSeqB - b.unicode(), currB - begSeqB));
		const int cmp = QStringRef::localeAwareCompare(subA, subB);
		if (cmp != 0) {
			return cmp < 0 ? -1 : +1;
		}

		if (currA->isNull() || currB->isNull()) {
			break;
		}

		// find sequence of characters ending at the first non-character
		while (currA->isPunct() || currA->isSpace() || currB->isPunct() || currB->isSpace()) {
			if (*currA != *currB) {
				return (*currA < *currB) ? -1 : +1;
			}
			++currA;
			++currB;
		}

		// now some digits follow...
		if ((*currA == '0') || (*currB == '0')) {
			// one digit-sequence starts with 0 -> assume we are in a fraction part
			// do left aligned comparison (numbers are considered left aligned)
			while (1) {
				if (!currA->isDigit() && !currB->isDigit()) {
					break;
				} else if (!currA->isDigit()) {
					return +1;
				} else if (!currB->isDigit()) {
					return -1;
				} else if (*currA < *currB) {
					return -1;
				} else if (*currA > *currB) {
					return + 1;
				}
				++currA;
				++currB;
			}
		} else {
			// No digit-sequence starts with 0 -> assume we are looking at some integer
			// do right aligned comparison.
			//
			// The longest run of digits wins. That aside, the greatest
			// value wins, but we can't know that it will until we've scanned
			// both numbers to know that they have the same magnitude.

			bool isFirstRun = true;
			int weight = 0;
			while (1) {
				if (!currA->isDigit() && !currB->isDigit()) {
					if (weight != 0) {
						return weight;
					}
					break;
				} else if (!currA->isDigit()) {
					if (isFirstRun) {
						return *currA < *currB ? -1 : +1;
					} else {
						return -1;
					}
				} else if (!currB->isDigit()) {
					if (isFirstRun) {
						return *currA < *currB ? -1 : +1;
					} else {
						return +1;
					}
				} else if ((*currA < *currB) && (weight == 0)) {
					weight = -1;
				} else if ((*currA > *currB) && (weight == 0)) {
					weight = + 1;
				}
				++currA;
				++currB;
				isFirstRun = false;
			}
		}

		begSeqA = currA;
		begSeqB = currB;
	}

	if (currA->isNull() && currB->isNull()) {
		return 0;
	}

	return currA->isNull() ? -1 : + 1;
}

*/
static inline QChar getNextChar(const QString &s, int location)
{
	return (location < s.length()) ? s.at(location) : QChar();
}

int naturalCompare(const QString &s1, const QString &s2,  Qt::CaseSensitivity cs)
{
	for (int l1 = 0, l2 = 0; l1 <= s1.count() && l2 <= s2.count(); ++l1, ++l2) {
		// skip spaces, tabs and 0's
		QChar c1 = getNextChar(s1, l1);
		while (c1.isSpace())
			c1 = getNextChar(s1, ++l1);
		QChar c2 = getNextChar(s2, l2);
		while (c2.isSpace())
			c2 = getNextChar(s2, ++l2);

		if (c1.isDigit() && c2.isDigit()) {
			while (c1.digitValue() == 0)
				c1 = getNextChar(s1, ++l1);
			while (c2.digitValue() == 0)
				c2 = getNextChar(s2, ++l2);

			int lookAheadLocation1 = l1;
			int lookAheadLocation2 = l2;
			int currentReturnValue = 0;
			// find the last digit, setting currentReturnValue as we go if it isn't equal
			for (
				QChar lookAhead1 = c1, lookAhead2 = c2;
				(lookAheadLocation1 <= s1.length() && lookAheadLocation2 <= s2.length());
				lookAhead1 = getNextChar(s1, ++lookAheadLocation1),
				lookAhead2 = getNextChar(s2, ++lookAheadLocation2)
				) {
				bool is1ADigit = !lookAhead1.isNull() && lookAhead1.isDigit();
				bool is2ADigit = !lookAhead2.isNull() && lookAhead2.isDigit();
				if (!is1ADigit && !is2ADigit)
					break;
				if (!is1ADigit)
					return -1;
				if (!is2ADigit)
					return 1;
				if (currentReturnValue == 0) {
					if (lookAhead1 < lookAhead2) {
						currentReturnValue = -1;
					} else if (lookAhead1 > lookAhead2) {
						currentReturnValue = 1;
					}
				}
			}
			if (currentReturnValue != 0)
				return currentReturnValue;
		}

		if (cs == Qt::CaseInsensitive) {
			if (!c1.isLower()) c1 = c1.toLower();
			if (!c2.isLower()) c2 = c2.toLower();
		}
		int r = QString::localeAwareCompare(c1, c2);
		if (r < 0)
			return -1;
		if (r > 0)
			return 1;
	}
	// The two strings are the same (02 == 2) so fall back to the normal sort
	return QString::compare(s1, s2, cs);
}
bool naturalSortLessThanCS( const QString &left, const QString &right )
{
  return (naturalCompare( left, right, Qt::CaseSensitive ) < 0);
}

bool naturalSortLessThanCI( const QString &left, const QString &right )
{
	return (naturalCompare( left, right, Qt::CaseInsensitive ) < 0);
}

bool naturalSortLessThanCIFileInfo(const QFileInfo & left,const QFileInfo & right)
{
	return naturalSortLessThanCI(left.fileName(),right.fileName());
}

bool naturalSortLessThanCILibraryItem(LibraryItem * left, LibraryItem * right)
{
	return naturalSortLessThanCI(left->name,right->name);
}

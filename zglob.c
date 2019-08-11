/* ZGlob: Utilities for easy matching of strings using * and ? wildcards.

Copyright (C) 2004-2019 by Adam D. Moss <c@yotes.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include <string.h>
#include "zglob.h"

/*#define TESTAPP*/

/*#define zdprintf printf*/

#if __STDC_VERSION__ >= 199901L
	#define zdprintf(...) /**/
#else
	#define zdprintf /**/
#endif


/* returns offset in haystack at which needle was found, or -1 on failure */
static int
zglob_strloc(const char *haystack, const char *needle, size_t maxchars) {
#if 1
  /* return first match */
  int i = 0;
  while (haystack[i] != '\0') {
    if (strncmp(needle, &haystack[i], maxchars) == 0) {
      return i;
    }
    ++i;
  }
  return -1; /* not found */
#else
  /* return last match */
  int i = strlen(haystack);
  while (i >= 0) {
    if (strncmp(needle, &haystack[i], maxchars) == 0) {
      return i;
    }
    --i;
  }
  return -1; /* not found */
#endif
}


int
zglob(const char *const str,
      const char *const wildcard) {
  int wildpos = 0;
  int stroff = 0;
  char wildc;
  zdprintf(" [%s:%s]", str, wildcard);
  while ((wildc = wildcard[wildpos]) != '\0') {
  	int use_as_literal = (wildc == '\\'); /*escaped*/
  	if (use_as_literal) wildc = wildcard[++wildpos];
  	/* a trailing '\' in wildcard is treated as a literal \ rather than an escape' */
  	if (wildc == '\0') return ('\\' == str[stroff] && '\0' == str[stroff+1]);
    if (wildc == '?' && !use_as_literal) {
      /* happily eat any char except end-of-string */
      if (str[stroff] == '\0') {
        zdprintf("(?eos)");
        return 0;
      }
      ++stroff;
    } else if (wildc == '*' && !use_as_literal) {
      int litlen;
      while (wildcard[wildpos+1] == '*' ||
             wildcard[wildpos+1] == '?') {
        /* eat str chars, if appropriate */
        if (wildcard[wildpos+1] == '?') {
          if (str[stroff] == '\0') {
            zdprintf("(?eos2)");
            return 0;
          }
          ++stroff;          
        }
        /* compound multiple '*' */
        ++wildpos;
      }

      if (wildcard[wildpos+1] == '\0') {
        return 1; /* '*' at end of wildcard matches remainder of str */
      }

      /* find length of literal following '*' */
      litlen = 0;
      while (wildcard[wildpos + 1 + litlen] != '*' &&
             wildcard[wildpos + 1 + litlen] != '\0' &&
             wildcard[wildpos + 1 + litlen] != '?') {
        ++litlen;
      }

      if (litlen) {
        /* look for each successive whole match in str after stroff,
           recurse into each one */
        int foundoff = zglob_strloc(&str[stroff],
                                    &wildcard[wildpos + 1],
                                    litlen);
        int ofoundoff = foundoff;
        int foresult;
        if (foundoff < 0) {
          zdprintf("(nfo)");
          return 0; /* no match at all */
        }
        foresult = foundoff;
        while (foresult >= 0) {
          if (zglob(&str[stroff+ foundoff+1],
                    &wildcard[wildpos])) {
            zdprintf("(RYAY)");
            return 1;
          }
          foresult = zglob_strloc(&str[stroff+ foundoff+1],
                                  &wildcard[wildpos + 1],
                                  litlen);
          if (foresult >= 0) {
            foundoff = foundoff + foresult + 1;
          }
        }
        stroff += ofoundoff + litlen;
        wildpos += litlen;
      } else {
        zdprintf("(0lit)");
      }
    } else {
      if (str[stroff] == '\0') {
        zdprintf("(strshort:%c)", wildc);
        return 0; /* ran out of chars to match */
      } else {
        if (str[stroff] != wildc) {
          zdprintf("(unint)");
          return 0; /* uninteresting chars failed to match */
        } else {
          ++stroff;
        }
      }
    }

    ++wildpos;
  }
  
  if (str[stroff] == '\0') {
    return 1; /* str ended same time as wildcard, so match. */
  }
  zdprintf("(fall)");
  return 0; /* not matched */
}


#ifdef TESTAPP
#include <stdio.h>
#define TESTRESULT(STR, GLOB, EXPECT) \
	printf("%s: string=\"%s\" glob=\"%s\" expected=%d\n", \
		((EXPECT)==zglob(STR,GLOB))?(successes++,"pass"):(failures++,"FAIL"), \
		(STR), (GLOB), (EXPECT)	);
int
main(void) {
  int successes = 0, failures = 0;
  printf("zglob self-tests:\n");
  TESTRESULT("a", "a", 1);
  TESTRESULT("a", "a", 1);
  TESTRESULT("a", "*", 1);
  TESTRESULT("a", "*a", 1);
  TESTRESULT("a", "a*", 1);
  TESTRESULT("a", "**a", 1);
  TESTRESULT("a", "a**", 1);
  TESTRESULT("a", "**a**", 1);

  TESTRESULT("", "\\", 0);
  TESTRESULT("\\", "\\", 1);
  TESTRESULT("a\\", "\\", 0);
  TESTRESULT("a\\", "a\\", 1);
  TESTRESULT("\\", "a\\", 0);
  TESTRESULT("\\", "", 0);

  TESTRESULT("*", "*", 1);
  TESTRESULT("a", "\\*", 0);
  TESTRESULT("*", "\\*", 1);
  TESTRESULT("a", "a\\*", 0);
  TESTRESULT("a", "\\*a", 0);

  TESTRESULT("?", "?", 1);
  TESTRESULT("?", "\\?", 1);
  TESTRESULT("?", "*", 1);
  TESTRESULT("?", "\\*", 0);
  TESTRESULT("\\?", "?", 0);
  TESTRESULT("\\?", "\\?", 0);
  TESTRESULT("\\?", "\\\\?", 1);
  TESTRESULT("\\?", "*", 1);
  TESTRESULT("\\?", "\\*", 0);
  TESTRESULT("*", "?", 1);

  TESTRESULT("a", "\\a", 1);
  TESTRESULT("b", "\\a", 0);
  TESTRESULT("a\\*", "a\\*", 0);
  TESTRESULT("a*", "a*", 1);
  TESTRESULT("a*", "a\\*", 1);
  TESTRESULT("a\\*", "a\\\\*", 1);

  TESTRESULT("\\a", "\\\\a", 1);
  TESTRESULT("\\", "\\\\", 1);
  TESTRESULT("\\", "\\\\\\", 0);
  TESTRESULT("\\\\", "\\\\\\\\", 1);
  TESTRESULT("\\\\", "\\\\*", 1);
  TESTRESULT("\\\\", "*\\\\", 1);

  TESTRESULT("abcd", "abcd", 1);
  TESTRESULT("abcd", "*bcd", 1);
  TESTRESULT("abcd", "a*cd", 1);
  TESTRESULT("abcd", "ab*d", 1);
  TESTRESULT("abcd", "abc*", 1);

  TESTRESULT("abcd", "*abcd", 1);
  TESTRESULT("abcd", "a*bcd", 1);
  TESTRESULT("abcd", "ab*cd", 1);
  TESTRESULT("abcd", "abc*d", 1);
  TESTRESULT("abcd", "abcd*", 1);

  TESTRESULT("abcd", "*cd", 1);
  TESTRESULT("abcd", "a*d", 1);
  TESTRESULT("abcd", "ab*", 1);
  TESTRESULT("abcd", "*abcd", 1);
  TESTRESULT("abcd", "abcd*", 1);

  TESTRESULT("", "", 1);
  TESTRESULT("", "*", 1);

  TESTRESULT("abcd", "abc", 0);
  TESTRESULT("abcd", "*dd", 0);
  TESTRESULT("abcd", "???dd", 0);
  TESTRESULT("abcd", "aa*", 0);
  TESTRESULT("abcd", "aa???", 0);
  TESTRESULT("abcd", "bcd", 0);
  TESTRESULT("", "a", 0);
  TESTRESULT("a", "", 0);

  TESTRESULT("a", "?", 1);
  TESTRESULT("abcd", "????", 1);
  TESTRESULT("abcd", "a???", 1);
  TESTRESULT("abcd", "?b??", 1);
  TESTRESULT("abcd", "??c?", 1);
  TESTRESULT("abcd", "???d", 1);
  TESTRESULT("abcd", "?bc?", 1);

  TESTRESULT("abcd", "a*?d", 1);
  TESTRESULT("abcd", "a?*d", 1);
  TESTRESULT("abc", "a*?", 1);
  TESTRESULT("abc", "a?*", 1);
  TESTRESULT("ab", "a*?", 1);
  TESTRESULT("ab", "?*?", 1);
  TESTRESULT("ab", "a?*", 1);
  TESTRESULT("abc", "*?c", 1);
  TESTRESULT("abc", "?*c", 1);
  TESTRESULT("abc", "*b?", 1);
  TESTRESULT("abc", "?b*", 1);
  TESTRESULT("a", "?*", 1);
  TESTRESULT("a", "*?", 1);

  TESTRESULT("", "?", 0);
  TESTRESULT("a", "?*?", 0);
  TESTRESULT("a", "?*????????????", 0);
  TESTRESULT("abcd", "???", 0);
  TESTRESULT("abc", "????", 0);
  TESTRESULT("abc", "abc?", 0);
  TESTRESULT("abcd", "?b?", 0);
  TESTRESULT("abcd", "??c", 0);
  TESTRESULT("abcd", "??d", 0);

  /* these ones require recursion for correct results */
  TESTRESULT("aaabbbccc", "a*b*c", 1);
  TESTRESULT("aaabbbcbc", "a*b*c", 1);
  TESTRESULT("aaabbbcdc", "a*b*c", 1);
  TESTRESULT("aaabbbcbcd", "a*b*cd", 1);
  TESTRESULT("aaabbbcdcd", "a*b*cd", 1);
  TESTRESULT("abcabcabc", "a*b*c", 1);
  TESTRESULT("abbxbbcb", "a*b*", 1);
  TESTRESULT("abbxbbcb", "a*b*cb", 1);
  TESTRESULT("abbxbbcbb", "a*b*bb", 1);
  TESTRESULT("abbxbbcbb", "a*bb*bb", 1);
  TESTRESULT("a", "*?*", 1);

  printf("\n%d tests passed, %d tests failed\n", successes, failures);
  if (failures)
  {
  	printf("\n*** %d TEST FAILURE(S) ***\n", failures);
  	return 1;
  }
  return 0;
}
#endif /* TESTAPP */

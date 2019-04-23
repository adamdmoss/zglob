/* ZGlob: Utilities for easy matching of strings using * and ? wildcards.
   Adam D. Moss <adam@gimp.org> <aspirin@icculus.org>

Copyright (C) 2004 Adam D. Moss (the "Author").  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is fur-
nished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CON-
NECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the Author of the
Software shall not be used in advertising or otherwise to promote the sale,
use or other dealings in this Software without prior written authorization
from the Author.
*/
#include <string.h>

#include "zglob.h"

/*#define TESTAPP*/

/*#define zdprintf printf*/
#define zdprintf


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
    if (wildc == '?') {
      /* happily eat any char except end-of-string */
      if (str[stroff] == '\0') {
        zdprintf("(?eos)");
        return 0;
      }
      ++stroff;
    } else if (wildc == '*') {
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
           recurse into each one! */
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
int
main(void) {
  printf("%d / ", zglob("a", "a") );
  printf("%d / ", zglob("a", "*") );
  printf("%d / ", zglob("a", "*a") );
  printf("%d / ", zglob("a", "a*") );
  printf("%d / ", zglob("a", "**a") );
  printf("%d / ", zglob("a", "a**") );
  printf("%d / ", zglob("a", "**a**") );
  printf("\n");
  printf("%d / ", zglob("abcd", "abcd") );
  printf("%d / ", zglob("abcd", "*bcd") );
  printf("%d / ", zglob("abcd", "a*cd") );
  printf("%d / ", zglob("abcd", "ab*d") );
  printf("%d / ", zglob("abcd", "abc*") );
  printf("\n");
  printf("%d / ", zglob("abcd", "*abcd") );
  printf("%d / ", zglob("abcd", "a*bcd") );
  printf("%d / ", zglob("abcd", "ab*cd") );
  printf("%d / ", zglob("abcd", "abc*d") );
  printf("%d / ", zglob("abcd", "abcd*") );
  printf("\n");
  printf("%d / ", zglob("abcd", "*cd") );
  printf("%d / ", zglob("abcd", "a*d") );
  printf("%d / ", zglob("abcd", "ab*") );
  printf("%d / ", zglob("abcd", "*abcd") );
  printf("%d / ", zglob("abcd", "abcd*") );
  printf("%d / ", zglob("", "") );
  printf("\n");
  printf("%d / ", !zglob("abcd", "abc") );
  printf("%d / ", !zglob("abcd", "*dd") );
  printf("%d / ", !zglob("abcd", "bcd") );
  printf("%d / ", !zglob("", "a") );
  printf("%d / ", !zglob("a", "") );
  printf("\n");
  printf("%d / ", zglob("a", "?") );
  printf("%d / ", zglob("abcd", "????") );
  printf("%d / ", zglob("abcd", "a???") );
  printf("%d / ", zglob("abcd", "?b??") );
  printf("%d / ", zglob("abcd", "??c?") );
  printf("%d / ", zglob("abcd", "???d") );
  printf("%d / ", zglob("abcd", "?bc?") );
  printf("\n");
  printf("%d / ", zglob("abcd", "a*?d") );
  printf("%d / ", zglob("abcd", "a?*d") );
  printf("%d / ", zglob("abc", "a*?") );
  printf("%d / ", zglob("abc", "a?*") );
  printf("%d / ", zglob("ab", "a*?") );
  printf("%d / ", zglob("ab", "?*?") );
  printf("%d / ", zglob("ab", "a?*") );
  printf("%d / ", zglob("abc", "*?c") );
  printf("%d / ", zglob("abc", "?*c") );
  printf("%d / ", zglob("abc", "*b?") );
  printf("%d / ", zglob("abc", "?b*") );
  printf("%d / ", zglob("a", "?*") );
  printf("%d / ", zglob("a", "*?") );
  printf("\n");
  printf("%d / ", !zglob("", "?") );
  printf("%d / ", !zglob("a", "?*?") );
  printf("%d / ", !zglob("a", "?*????????????") );
  printf("%d / ", !zglob("abcd", "???") );
  printf("%d / ", !zglob("abc", "????") );
  printf("%d / ", !zglob("abc", "abc?") );
  printf("%d / ", !zglob("abcd", "?b?") );
  printf("%d / ", !zglob("abcd", "??c") );
  printf("%d / ", !zglob("abcd", "??d") );
  printf("\n");
  /* these ones require recursion for correct results! */
  printf("%d / ", zglob("aaabbbccc", "a*b*c") );
  printf("%d / ", zglob("aaabbbcbc", "a*b*c") );
  printf("%d / ", zglob("aaabbbcdc", "a*b*c") );
  printf("%d / ", zglob("aaabbbcbcd", "a*b*cd") );
  printf("%d / ", zglob("aaabbbcdcd", "a*b*cd") );
  printf("%d / ", zglob("abcabcabc", "a*b*c") );
  printf("%d / ", zglob("abbxbbcb", "a*b*") );
  printf("%d / ", zglob("abbxbbcb", "a*b*cb") );
  printf("%d / ", zglob("abbxbbcbb", "a*b*bb") );
  printf("%d / ", zglob("abbxbbcbb", "a*bb*bb") );
  printf("%d / ", zglob("a", "*?*") );

}
#endif /* TESTAPP */

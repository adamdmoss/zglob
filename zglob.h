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
#ifndef _Z_GLOB_H
#define _Z_GLOB_H

/* wildcard note: '*' matches 0 or more times, '?' matches exactly once.
   '\' escapes any character in wildcard string so '?' and '*' can be used as
   literal matches, i.e. zglob("ab","\\?b")==0 */

/* Returns 1 if 'str' matches 'wildcard', otherwise 0. */
int zglob(const char *const str,
          const char *const wildcard);

#endif /* _Z_GLOB_H */

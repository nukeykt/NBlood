/* Extended Module Player
 * Copyright (C) 1996-2021 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "format.h"

#ifndef LIBXMP_NO_PROWIZARD
#include "loaders/prowizard/prowiz.h"
#endif

extern const struct format_loader libxmp_loader_xm;
extern const struct format_loader libxmp_loader_mod;
extern const struct format_loader libxmp_loader_it;
extern const struct format_loader libxmp_loader_s3m;
extern const struct format_loader libxmp_loader_mtm;

#ifndef LIBXMP_NO_PROWIZARD
extern const struct pw_format *const pw_formats[];
#endif

extern const struct format_loader *const format_loaders[];
#ifdef EDUKE32_DISABLE
const struct format_loader *const format_loaders[NUM_FORMATS + 2] = {
#else
const struct format_loader *const format_loaders[] = {
#endif // EDUKE32_DISABLE
	&libxmp_loader_xm,
	&libxmp_loader_mod,
	&libxmp_loader_it,
	&libxmp_loader_s3m,
	&libxmp_loader_mtm,
#ifndef LIBXMP_NO_PROWIZARD
	&libxmp_loader_pw,
#endif
	NULL
};

#ifdef EDUKE32_DISABLE
static const char *_farray[NUM_FORMATS + NUM_PW_FORMATS + 1] = { NULL };
#else
static const char *_farray[sizeof(format_loaders)/sizeof(struct format_loader *)] = { NULL };
#endif // EDUKE32_DISABLE

const char *const *format_list(void)
{
	int count, i;

	if (_farray[0] == NULL) {
		for (count = i = 0; format_loaders[i] != NULL; i++) {
#ifndef LIBXMP_NO_PROWIZARD
			if (strcmp(format_loaders[i]->name, "prowizard") == 0) {
				int j;

				for (j = 0; pw_formats[j] != NULL; j++) {
					_farray[count++] = pw_formats[j]->name;
				}
				continue;
			}
#endif
			_farray[count++] = format_loaders[i]->name;
		}

		_farray[count] = NULL;
	}

	return _farray;
}

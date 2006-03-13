/*
 *  scelib - Simpliest C Extension Library
 *  Copyright (C) 2005-2006 Richard 'riri' GILL <richard@houbathecat.info>
 *  * Some ideas taken and adapted from libslack:
 *  * Copyright (C) 1999-2004 raf <raf@raf.org>
 *  * , uClibc:
 *  * Copyright (C) 2002, 2003 Manuel Novoa III
 *	* and snprintf function in tarball from:
 *	* Copyright 1999, Mark Martinec <mark.martinec@ijs.si>. All rights reserved.
 *
 *  vaprint.c - own implementation of vasprintf() functions.
 *  See implementation notes that should have been provided with this file.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "scelib.h"
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>



/* ========================================================================= */
/* Constants used in this module                                             */

#define FLAG_JUSTIFY		(1 << 0)
#define FLAG_SIGN			(1 << 1)
#define FLAG_SPACE			(1 << 2)
#define FLAG_ALT			(1 << 3)
#define FLAG_ZERO			(1 << 4)
#define FLAG_UNSIGNED		(1 << 5)
#define FLAG_UPPERCASE		(1 << 6)
#define FLAG_FLT_F			(1 << 7)
#define FLAG_FLT_E			(1 << 8)
#define FLAG_FLT_G			(1 << 9)
#define FLAG_WIDTH			(1 << 10)
#define FLAG_PRECISION		(1 << 11)

/* to avoid using <float.h>, let's define some big values */
#define MAX_INT_PART		99 + 1
#define MAX_FRAC_PART		99 + 1



/* ========================================================================= */
/* Types                                                                     */

/* ------------------------------------------------------------------------- */
/* an argument length modifier                                               */
typedef enum fmtlength {

	lenDefault = 0,
	lenChar,		/* hh */
	lenShort,		/* h */
	lenLong			/* l */

} fmtlength_e;

/* ------------------------------------------------------------------------- */
typedef enum fmtconv {

	convNone = 0,
	convChar,
	convInt,
	convLong,
	convDouble,
	convString,
	convPointer

} fmtconv_e;

/* ------------------------------------------------------------------------- */
/* a single argument value passed by va_list argument pointer                */
typedef union fmtarg {

	char c;				/* for char and % */
	double d;			/* for numeric values */
	void *p;			/* for string, pointer and result (n) */

} fmtarg_u;

/* ------------------------------------------------------------------------- */
/* type fmtspec_t                                                            */

typedef struct fmtspec {

	int flags;			/* flags defined as FLAG_* constants */
	int width;			/* width value */
	int precision;		/* precision value */
	int base;			/* numeric base output */
	int fmtconv;		/* the original conversion character */

	size_t fmtlen;		/* length of this specifier format string */
	size_t outlen;		/* length of the resulting string */

	fmtlength_e len;	/* length modifier */
	fmtconv_e conv;		/* conversion specifier */
	fmtarg_u arg;		/* the actual argument */

} fmtspec_t;

/* ------------------------------------------------------------------------- */
/* type fmtctx_t                                                             */
/* Represents the formating context, incorporing informations on the input   */
/* format buffer, the format state during the parse, the extracted format    */
/* specifiers information, and the resulting output buffer                   */

typedef struct fmtctx {

	char *fmt;
	size_t fmtlen;

	fmtspec_t *specs;	/* array of format specifiers */
	size_t nbspecs;		/* number of format specifiers */

	char *outbuf;		/* output buffer */
	size_t outlen;		/* length of output buffer (excluded '\0') */

} fmtctx_t;



/* ========================================================================= */
/* Static global const variables                                             */

/* list of supported 'flags', following the FLAG_* constants above */
static const char format_flags[] = "-+ #0";

/* list of supported conversion characters */
static const char format_conv[] = "diouxXfFeEgGcspn%";



/* ========================================================================= */
/* Static functions declaration                                              */

/* ------------------------------------------------------------------------- */
/* free_ctx()                                                                */
/* free all memory allocated for an fmtctx_t variable.                       */
/* also returns the saved errno value, because this function should not      */
/* reset it                                                                  */

static int free_ctx(fmtctx_t *pctx, int free_buffer);

/* ------------------------------------------------------------------------- */
/* analyze_format()                                                          */
/* extract argument specifiers informations from the format string, to get   */
/* flags, width, precision, width and/or precision argument check, qualifier */
/* and argument index (if implemented)                                       */

static int analyze_format(fmtctx_t *pctx, va_list ap);

/* ------------------------------------------------------------------------- */
/* format_output()                                                           */
/* use all preceding collected informations (specifiers and arguments) to    */
/* make the output string.                                                   */

static int format_output(fmtctx_t *pctx);

/* ------------------------------------------------------------------------- */
/* log_10()                                                                  */
/* find the integral part of the log in base 10.                             */
/* Note: this not a real log10() with an int output, just an approximation   */
/* for the integer part of x in :                                            */
/* 10^x ~= r                                                                 */
/* log_10(200) = 2; log_10(250) = 2;                                         */

static int log_10(double r);

/* ------------------------------------------------------------------------- */
/* pow_10()                                                                  */
/* find the nth power of 10                                                  */

static double pow_10(int n);

/* ------------------------------------------------------------------------- */
/* integral()                                                                */
/* do the job of the standard modf C function, but with that, we're not      */
/* required to link against the math library.                                */

static double integral(double real, double *fp);

/* ------------------------------------------------------------------------- */
/* roundto()                                                                 */
/* round a floating point value to the precision, which is the number of     */
/* digits after the decimal point if dec=1, or the number of significant     */
/* digits of the whole number if dec=0.                                      */

static double roundto(double val, int precision, int decimal);

/* ------------------------------------------------------------------------- */
/* num2str()                                                                 */
/* extract in string format the integral and fractional parts of a number.   */
/* the integral string is assumed to be at least MAX_INT_PART long, the      */
/* fractional one MAX_FRAC_PART long, or null.                               */
/* return > 0 if positive value, < 0 if negative one, = 0 if zero one        */
static int num2str(double number, int base, int precision, char *integral_part, char *fraction_part);

/* ------------------------------------------------------------------------- */
/* fmt_integer()                                                             */
/* format the integer number, and assign output only if asked (to be able to */
/* compute the final size before the allocation                              */
static size_t fmt_integer(fmtspec_t *spec, char **out);

/* ------------------------------------------------------------------------- */
/* fmt_floating()                                                            */
/* format the floating point number, and assign output only if asked (to be  */
/* able to compute the final size before the allocation                      */
static size_t fmt_floating(fmtspec_t *spec, char **out);

/* ------------------------------------------------------------------------- */
/* fmt_exponent()                                                            */
/* format the floating point number in scientific format, and assign output  */
/* only if asked (to be able to compute the final size before the allocation */

static size_t fmt_exponent(fmtspec_t *spec, char **out);

/* ------------------------------------------------------------------------- */
/* fmt_string()                                                              */
/* format the string value, and assign output only if asked (to be able to   */
/* compute the final size before the allocation                              */
static size_t fmt_string(fmtspec_t *spec, char **out);

#define myswap(type, a, b)	\
{							\
	type t;					\
	t = (a);				\
	(a) = (b);				\
	(b) = t;				\
}

/* ========================================================================= */
/* Public functions definitions                                              */

/* ------------------------------------------------------------------------- */
/* cmd_create()                                                              */

char *vaprint(char **dest, size_t *dlen, const char *fmt, va_list ap) {

	fmtctx_t ctx;		/* the format context */
	int s_errno;		/* saved errno */
	char *ptr;			/* various strings pointer */

	/* prepare output parameter for null result */
	mem_init(dest, NULL);
	mem_init(dlen, 0);

	/*	format context initialization, if fmt is null,
		use an empty static string  */
	memset(&ctx, 0, sizeof(fmtctx_t));
	ctx.fmt = (char *) ((fmt) ? fmt : "");
	ctx.fmtlen = strlen(ctx.fmt);

	/*	fast method to find how many specs are there, we also check that
		there's at least one character after each spec */
	ptr = ctx.fmt;
	while ((ptr = strchr(ptr, '%'))) {
		/* check this's not the end of the string */
		if (!(*++ptr)) {
			errno = EINVAL;
			return NULL;
		}

		++ptr;
		++ctx.nbspecs;
	}

	/* no spec defined, we just have to copy the input string */
	if (! ctx.nbspecs) {
		ctx.outlen = ctx.fmtlen;
		ctx.outbuf = mem_new(char, ctx.outlen + 1);
		if (!ctx.outbuf) {
			return NULL;
		}
		strcpy(ctx.outbuf, ctx.fmt);
		mem_init(dest, ctx.outbuf);
		mem_init(dlen, ctx.outlen);
		return ctx.outbuf;
	}

	/* allocate room for the spec structures */
	ctx.specs = mem_new(fmtspec_t, ctx.nbspecs);
	if (!ctx.specs) {
		errno = free_ctx(&ctx, 1);
		return NULL;
	}

	/* extract format string informations */
	if ((s_errno = analyze_format(&ctx, ap))) {
		errno = free_ctx(&ctx, 1);
		return NULL;
	}

	/* do the format thing */
	s_errno = format_output(&ctx);
	errno = free_ctx(&ctx, (s_errno) ? 1 : 0);

	/* finish returning results */
	mem_init(dest, ctx.outbuf);
	mem_init(dlen, ctx.outlen);
	return ctx.outbuf;

}

/* ========================================================================= */
/* Static functions definitions                                              */

/* ------------------------------------------------------------------------- */
/* free_ctx()                                                                */

static int free_ctx(fmtctx_t *pctx, int free_buffer) {

	int s_errno = errno;

	if (pctx->specs) {
		free(pctx->specs);
	}

	if (pctx->outbuf && free_buffer) {
		free(pctx->outbuf);
		pctx->outlen = 0;
	}

	return s_errno;

}

/* ------------------------------------------------------------------------- */
/* analyze_format()                                                          */

static int analyze_format(fmtctx_t *pctx, va_list ap) {

	char *ptr;
	int i;
	char *fmt = pctx->fmt;
	fmtspec_t *spec = NULL;
	int curarg = 0;

	while ((fmt = strchr(fmt, '%'))) {

		/* skip the specifier marker '%' */
		++fmt;

		/*	we already have allocated the spec structures. If this's the first
			loop, initialize our pointer, use next one otherwise */
		if (!spec) {
			spec = pctx->specs;
		}
		else {
			++spec;
		}

		/* 'not found' flag for width and precision */
		spec->width = -1;
		spec->precision = -1;
		++(spec->fmtlen);	/* why ???? */

		/* flags
		 * we scan for the current character in the set of defined flags
		 * to set specifier flag field accordingly, going through until
		 * the next character isn't one of the predefined flags
		 */
		while (strchr(format_flags, *fmt)) {
			ptr = (char *) format_flags;
			i = 1;
			do {
				if (*fmt == *ptr) {		/* found this one */
					spec->flags |= i;	/* add the indexed flag */
					break;				/* stop this iteration */
				}
				i <<= 1;				/* go next */
			}
			while (*ptr++);

			/* next format character */
			++fmt;
			++(spec->fmtlen);
		}

		/* width
		 * if width is specified as '*', get the width from the next
		 * argument (we update the next argument position accordingly),
		 * else, go through the characters until they're digits, and
		 * build the resulting number by base 10 additions.
		 */
		if (*fmt == '*') {
			/* assign current arg to width value, next for argument */
			spec->flags |= FLAG_WIDTH;
			++(spec->fmtlen);
			++fmt;
		}
		else if (isdigit(*fmt)) {
			/*	iteration until the next format string character is a
				digit, to compute the field width in base 10 */
			i = 0;
			do {
				i = (i * 10) + (*fmt - '0');
				++fmt;
				++(spec->fmtlen);
			}
			while (isdigit(*fmt));
			spec->width = (i > 0) ? i : -i;
		}

		/* precision
		 * if we find a dot, next is the precision.
		 * it works just like the width algorithm
		 */
		if (*fmt == '.') {
			++fmt;
			++(spec->fmtlen);
			if (*fmt == '*') {
				/* assign current arg to precision value, next for arg */
				spec->flags |= FLAG_PRECISION;
				++(spec->fmtlen);
				++fmt;
			}
			else if (isdigit(*fmt)) {
				/*	iteration until next format string character is a
					digit, to compute the field precision in base 10 */
				i = 0;
				do {
					i = (i * 10) + (*fmt - '0');
					++fmt;
					++(spec->fmtlen);
				}
				while (isdigit(*fmt));
				spec->precision = (i > 0) ? i : -i;
			}
			else {
				/* only '.' specified as precision, in this case the precision
				 * should be taken as zero
				 */
				spec->precision = 0;
			}
		}

		/* if vaarg width and/or precision, get'em */
		if (spec->flags & FLAG_WIDTH) {
			spec->width = va_arg(ap, int);
			if (spec->width < 0) {
				spec->width = - spec->width;
				spec->flags |= FLAG_JUSTIFY;
			}
		}
		if (spec->flags & FLAG_PRECISION) {
			spec->precision = va_arg(ap, int);
			if (spec->precision < 0) {
				spec->precision = -1;
			}
		}

		/* if the 'plus' flag is set, the 'space' flag is ignored */
		if ((spec->flags & FLAG_SPACE) && (spec->flags & FLAG_SIGN)) {
			spec->flags &= ~FLAG_SPACE;
		}
		/* if the 'minus' flag is set, the 'zero' flag is ignored */
		if ((spec->flags & FLAG_ZERO) && (spec->flags & FLAG_JUSTIFY)) {
			spec->flags &= ~FLAG_ZERO;
		}

		/* length modifiers
		 * can be specified by one or two letters.
		 * when we're done, update the index 'i' to break the loop
		 */
		i = 0;
		while (i++ < 2) {
			switch (*fmt) {
				case 'h':
					/* "h" is short, "hh" is char */
					spec->len = (spec->len == lenShort) ? lenChar : lenShort;
					break;
				case 'l':
					/* "l" is long, "ll" is long long */
					spec->len = lenLong;
					++i;
					break;
				default:
					i = 3;
			}
			if (i == 3) {
				break;
			}
			if (i <= 2)	{ /* caught another char than length modifiers one */
				++fmt;
				++(spec->fmtlen);
			}
		}

		/* conversion specifier */
		if (!strchr(format_conv, *fmt)) {
			/* none of the supported conversion specifiers defined */
			return EINVAL;
		}

		spec->fmtconv = *fmt;	/* store the conversion specifier */
		spec->base = 10;		/* use decimal numeric base by default */

		/* basic int output */
		if (strchr("di", *fmt)) {
			spec->conv = convInt;

			if (spec->len == lenLong) {
				spec->conv = convLong;
				spec->arg.d = (long) va_arg(ap, long);
			}
			else if (spec->len == lenChar) {
				spec->arg.d = (char) va_arg(ap, int);
			}
			else if (spec->len == lenShort) {
				spec->arg.d = (short) va_arg(ap, int);
			}
			else {
				spec->arg.d = (int) va_arg(ap, int);
			}

			/* adapt flags according to ISO defines */
			if (spec->precision < 0) {
				/* default precision of 1 */
				spec->precision = 1;
			}
			else if (spec->precision == 0 && (spec->flags & FLAG_ZERO)) {
				/* if precision=0 and zero flag, flag ignored */
				spec->flags &= ~FLAG_ZERO;
			}
			if (spec->arg.d == 0) {
				/* if zero value, # flag ignored */
				spec->flags &= ~FLAG_ALT;
			}

			/* calculate room for output */
			spec->outlen = fmt_integer(spec, 0);
		}

		/* other bases decimal output */
		else if (strchr("oxXup", *fmt)) {
			spec->conv = convInt;
			spec->flags |= FLAG_UNSIGNED;
			spec->flags &= ~(FLAG_SPACE|FLAG_SIGN);

			if (spec->len == lenLong) {
				spec->conv = convLong;
				spec->arg.d = (unsigned long) va_arg(ap, unsigned long);
			}
			else if (spec->len == lenChar) {
				spec->arg.d = (unsigned char) va_arg(ap, unsigned char);
			}
			else if (spec->len == lenShort) {
				spec->arg.d = (unsigned short) va_arg(ap, unsigned int);
			}
			else {
				spec->arg.d = (unsigned int) va_arg(ap, unsigned int);
			}
			if (*fmt == 'o') {
				spec->base = 8;
			}
			else if (*fmt == 'X' || *fmt == 'P') {
				spec->flags |= FLAG_UPPERCASE;
				spec->base = 16;
			}
			else if (*fmt == 'x' || *fmt == 'p') {
				spec->base = 16;
			}

			/* adapt flags according to ISO defines */
			if (spec->precision < 0) {
				/* default precision of 1 */
				spec->precision = 1;
			}
			else if (spec->precision == 0 && (spec->flags & FLAG_ZERO)) {
				/* if precision=0 and zero flag, flag ignored */
				spec->flags &= ~FLAG_ZERO;
			}
			if (spec->arg.d == 0 && spec->base != 8) {
				/* if zero value, # flag ignored */
				spec->flags &= ~FLAG_ALT;
			}

			/* calculate room for output */
			spec->outlen = fmt_integer(spec, 0);
		}

		/* floating point output */
		else if (strchr("fFeEgG", *fmt)) {
			spec->conv = convDouble;
			spec->arg.d = va_arg(ap, double);
			if (strchr("FEG", *fmt)) {
				spec->flags |= FLAG_UPPERCASE;
			}

			switch (*fmt) {
				case 'F':
				case 'f':
					/* adapt flags according to ISO defines */
					if (spec->precision < 0) {
						spec->precision = 6;
					}
					spec->flags |= FLAG_FLT_F;
					spec->outlen = fmt_floating(spec, 0);
					break;
				case 'E':
				case 'e':
					/* adapt flags according to ISO defines */
					if (spec->precision < 0) {
						spec->precision = 6;
					}
					spec->flags |= FLAG_FLT_E;
					spec->outlen = fmt_exponent(spec, 0);
					break;
				case 'G':
				case 'g':
					/* adapt flags according to ISO defines */
					if (spec->precision == 0) {
						spec->precision = 1;
					}
					else if (spec->precision < 0) {
						/* but trailing zeroes removed */
						spec->precision = 6;
					}
					spec->flags |= FLAG_FLT_G;
					i = log_10(spec->arg.d);
					/* for '%g|%G' ANSI: use f if exponent is in the range or
					 * [-4,p] exclusively else use %e|%E
					 */
					spec->outlen = (i > -4 && i < spec->precision) ?
						fmt_floating(spec, 0) : fmt_exponent(spec, 0);
					break;
			}
		}

		/* character output or '%' */
		else if (*fmt == 'c' || *fmt == '%') {
			spec->conv = convChar;
			spec->arg.c = (*fmt == 'c') ? (char) va_arg(ap, int) : *fmt;
			spec->outlen = 1;
			++fmt;	/* skip the second '%' */
		}

		/* string output */
		else if (*fmt == 's') {
			spec->conv = convString;
			spec->arg.p = (void *) va_arg(ap, char *);
			spec->outlen = fmt_string(spec, 0);
		}

		/* number of transpositions at this time */
		else if (*fmt == 'n') {
			spec->conv = convPointer;
			spec->arg.p = (void *) va_arg(ap, int *);
		}

		/* go ahead next specifier (and argument) */
		++curarg;
		++spec->fmtlen;

	}

	return 0;

}

/* ------------------------------------------------------------------------- */
/* format_output()                                                           */

static int format_output(fmtctx_t *pctx) {

	return 0;

}

/* ------------------------------------------------------------------------- */
/* log_10()                                                                  */

static int log_10(double r) {

	int i = 0;
	double result = 1;

	if (r < 0) {
		r = -r;
	}
	else if (r == 0) {
		return 0;
	}

	if (r < 1) {
		while (result >= r) {
			result *= .1;
			++i;
		}
		return -i;
	}

    while (result <= r) {
		result *= 10;
		i++;
	}
	return (i - 1);

}

/* ------------------------------------------------------------------------- */
/* pow_10()                                                                  */

static double pow_10(int n) {

	int i;
	double dpow;

	if (n < 0) {
		for (i = 1, dpow = 1, n = -n; i <= n; ++i) {
			dpow *= 0.1;
		}
	}
	else {
		for (i = 1, dpow = 1; i <= n ; ++i) {
			dpow *= 10.0;
		}
	}
	return dpow;

}

/* ------------------------------------------------------------------------- */
/* integral()                                                                */

static double integral(double real, double *fp) {

	double dpow, sub, idx;
	int ilog;
	double ip = 0;

	if (real == 0) {
		mem_init(fp, 0);
		return 0;
	}

	/* negative ? */
	if (real < 0) {
		real = -real;
	}

	/* a fraction ? */
	if (real < 1) {
		mem_init(fp, real);
		return 0;
	}

	/* process extraction */
	for (ilog = log_10(real); ilog >= 0; --ilog) {
		dpow = pow_10(ilog);
		sub = (real - ip) / dpow;

		idx = 0;
		while (idx + 1 <= sub) {
			++idx;
		}

		ip += idx * dpow;
	}

	mem_init(fp, real - ip);
	return ip;

}

/* ------------------------------------------------------------------------- */
/* roundto()                                                                 */

static double roundto(double val, int precision, int decimal) {

	double ip, fp;
	int digits;
	double rounded;
/*	double conv;*/

	ip = integral(val, &fp);
	digits = ((decimal) ? precision : precision - 1 - log_10(ip));
/*	conv = (fp * pow_10(digits)) + ((val < 0) ? -0.5 : 0.5);*/
/*	conv = (fp * pow_10(digits)) + 0.5;*/
/*	rounded = (double) (int) conv;*/
/*	rounded = (val < 0) ? ceil(conv) : floor(conv);*/
	rounded = (val < 0) ? ceil(fp * pow_10(digits)) : floor(fp * pow_10(digits));
/*	rounded = (double)((int)(((val < 0) ? -0.5 : 0.5) + conv));*/
	fp = rounded * pow_10(-digits);
/*	fp = (((double)((int)(((val < 0) ? -0.5 : 0.5) + (fp * pow_10(digits)))))
		* pow_10(-digits));*/
	return (ip + fp);

}

/* ------------------------------------------------------------------------- */
/* num2str()                                                                 */

static int num2str(double number, int base, int precision, char *integral_part, char *fraction_part) {

	register int i, j;
	double ip, fp; /* integer and fraction part */
	double fraction, round_prec, sign;
	int ch, digits = MAX_INT_PART - 1;
	char frac_part[MAX_FRAC_PART];

	/* taking care of the obvious case: 0.0 */
	if (number == 0) {
		integral_part[0] = '0';
		integral_part[1] = '\0';
		frac_part[0] = '0';
		frac_part[1] = '\0';
		if (fraction_part) {
			strcpy(fraction_part, frac_part);
		}
		return 0;
	}

	round_prec = pow_10(-6);

	/* for negative numbers */
	if ((sign = number) < 0) {
		number = -number;
	}

	ip = integral(number, &fraction);
	number = ip;
	/* do the integral part */
	if ( ip == 0) {
		integral_part[0] = '0';
		i = 1;
	}
	else {
		for ( i = 0; i < digits && number != 0; ++i) {
			number /= base;
			ip = integral(number, &fp);
			ch = (int) ((fp + round_prec) * base); /* force to round */
			integral_part[i] = (ch <= 9) ? ch + '0' : ch + 'a' - 10;
			if (! isxdigit(integral_part[i])) {
				/* bail out overflow !! */
				break;
			}
			number = ip;
		}
	}

	/* Oh No !! out of bound, ho well fill it up ! */
	if (number != 0) {
		for (i = 0; i < digits; ++i) {
			integral_part[i] = '9';
		}
	}
	integral_part[i] = '\0';

	/* reverse every thing */
	for ( i--, j = 0; j < i; j++, i--) {
		myswap(int, integral_part[i], integral_part[j]);
	}

	/* the fractionnal part */
	for (i = 0, fp = fraction; precision > 0 && i < MAX_FRAC_PART;
		i++, precision--) {
		frac_part[i] = (int) ((fp + round_prec) * 10 + '0');
		if (! isdigit(frac_part[i])) {
			/* underflow ? */
			break;
		}
		fp = (fp * 10) - (double) (long) ((fp + round_prec) * 10);
	}
	frac_part[i] = '\0';
	if (fraction_part) {
		strcpy(fraction_part, frac_part);
	}

	return (sign > 0) ? 1 : -1;

}

/* ------------------------------------------------------------------------- */
/* fmt_integer()                                                             */

static size_t fmt_integer(fmtspec_t *spec, char **out) {

	char ipart[MAX_INT_PART];
	char *tmp, *outstr;
	int len, spadlen, zpadlen, baselen = 0;
	char sign = 0;

	outstr = (out) ? *out : 0;

	if (spec->arg.d == 0) {
		if (spec->precision != 0 ||
			(spec->base == 8 && (spec->flags & FLAG_ALT))) {
			strcpy(ipart, "0");
		}
		else {
			ipart[0] = '\0';
		}
	}
	else {
		num2str(spec->arg.d, spec->base, 0, ipart, 0);
	}
	tmp = ipart;
	len = (int) strlen(tmp);

	/* calculate extra characters */
	if (!(spec->flags & FLAG_UNSIGNED)) {
		if (spec->arg.d < 0) {
			sign = '-';
		}
		else if (spec->flags & FLAG_SIGN) {
			sign = '+';
		}
		else if (spec->flags & FLAG_SPACE) {
			sign = ' ';
		}
		if (sign) {
			++len;
		}
	}

	if (spec->base != 10 && (spec->flags & FLAG_ALT)) {
		/* '0' for octal, '0x' for hexa */
		if (spec->base == 16) {
			baselen = 2;
		}
		else if (spec->arg.d != 0) {
			++len;
		}
	}

	/* adding padding characters */
	zpadlen = spec->precision - len + (sign ? 1 : 0);
	if (zpadlen < 0) {
		zpadlen = 0;
	}

	spadlen = spec->width - (len + zpadlen + baselen);
	if (spadlen < 0) {
		spadlen = 0;
	}

	len += spadlen + zpadlen + baselen;

	if (spec->flags & FLAG_ZERO) {
		zpadlen = max(zpadlen, spadlen);
		spadlen = 0;
	}

	if (spec->flags & FLAG_JUSTIFY) {
		spadlen = -spadlen;	/* < 0 => right pad */
	}

	if (outstr) {
		while (spadlen > 0) {
			*outstr++ = ' ';
			--spadlen;
		}

		if (sign) {
			*outstr++ = sign;
		}

		if (spec->arg.d != 0 && !sign &&
			(spec->base != 10) && (spec->flags & FLAG_ALT)) {
			*outstr++ = '0';
			if (spec->base == 16) {
				*outstr++ = (spec->flags & FLAG_UPPERCASE) ? 'X' : 'x';
			}
		}

		while (zpadlen > 0) {
			*outstr++ = '0';
			--zpadlen;
		}

		while (*tmp) {
			*outstr++ = (spec->flags & FLAG_UPPERCASE) ? toupper(*tmp) : *tmp;
			++tmp;
		}

		while (spadlen++ < 0) {
			*outstr++ = ' ';
		}

	}
	return len;

}

/* ------------------------------------------------------------------------- */
/* fmt_floating()                                                            */

static size_t fmt_floating(fmtspec_t *spec, char **out) {

	char ipart[MAX_INT_PART], fpart[MAX_FRAC_PART];
	char *outstr, *itmp, *ftmp;
	double val;
	char sign = 0, point = 0;
	int ilen, flen;
	int len, spadlen, zpadlen;

	outstr = (out) ? *out : 0;

	/* round the value to precision */
	val = roundto(spec->arg.d, spec->precision,
		(spec->flags & FLAG_FLT_G) ? 0 : 1);
	num2str(val, 10, spec->precision, ipart, fpart);
	itmp = ipart; ilen = (int) strlen(ipart);
	ftmp = fpart; flen = (int) strlen(fpart);
	if (flen == 1 && *ftmp == '0') {
		ftmp[--flen] = '\0';
	}

	if ((spec->flags & FLAG_FLT_F) && spec->precision > 0) {
		point = '.';
		while (flen < spec->precision) {
			ftmp[flen++] = '0';
			ftmp[flen] = '\0';
		}

	}
	if (spec->flags & FLAG_FLT_G) {
		if (flen > 0) {
			point = '.';
			if (!(spec->flags & FLAG_ALT)) {
				while (flen > 0 && ftmp[flen-1] == '0') {
					ftmp[--flen] = '\0';
				}
			}
		}
	}
	if (!flen && (spec->flags & FLAG_ALT)) {
		point = '.';
	}

	len = ilen + flen + ((point) ? 1 : 0);

	if (spec->arg.d < 0) {
		sign = '-';
	}
	else if (spec->flags & FLAG_SIGN) {
		sign = '+';
	}
	else if (spec->flags & FLAG_SPACE) {
		sign = ' ';
	}
	if (sign) {
		++len;
	}

	zpadlen = spec->precision - len + (sign ? 1 : 0);
	if (zpadlen < 0) {
		zpadlen = 0;
	}

	spadlen = spec->width - (len + zpadlen);
	if (spadlen < 0) {
		spadlen = 0;
	}

	len += spadlen + zpadlen;
	if (spec->flags & FLAG_ZERO) {
		zpadlen = max(zpadlen, spadlen);
		spadlen = 0;
	}
	if (spec->flags & FLAG_JUSTIFY) {
		spadlen = -spadlen;
	}

	if (outstr) {
		while (spadlen > 0) {
			*outstr++ = ' ';
			--spadlen;
		}

		if (sign) {
			*outstr++ = sign;
		}

		while (zpadlen > 0) {
			*outstr++ = '0';
			--zpadlen;
		}

		while (*itmp) {
			*outstr++ = *itmp++;
		}

		if (point) {
			*outstr++ = '.';
			while (*ftmp) {
				*outstr++ = *ftmp++;
			}
		}

		while (spadlen++ < 0) {
			*outstr++ = ' ';
		}
	}

	return len;

}

/* ------------------------------------------------------------------------- */
/* fmt_exponent()                                                            */

static size_t fmt_exponent(fmtspec_t *spec, char **out) {

	char tmp[512];
	char *outstr, *ptr;
	double val;
	char sign = 0;
	int len, spadlen, zpadlen;

	outstr = (out) ? *out : 0;

	/* calculate sign characters */
	if (spec->arg.d < 0) {
		sign = '-';
		val = -spec->arg.d;
	}
	else {
		val = spec->arg.d;
		if (spec->flags & FLAG_SIGN) {
			sign = '+';
		}
		else if (spec->flags & FLAG_SPACE) {
			sign = ' ';
		}
	}

	/* basic floating point output */
	if (spec->flags & FLAG_UPPERCASE) {
		len = sprintf(tmp,
			(spec->flags & FLAG_FLT_G) ? "%.*G" : "%.*E",
			spec->precision, val);
	}
	else {
		len = sprintf(tmp,
			(spec->flags & FLAG_FLT_G) ? "%.*g" : "%.*e",
			spec->precision, val);
	}
	ptr = tmp;

	if (!strchr(tmp, '.') && (spec->flags & FLAG_ALT)) {
		/* adding '.' before the 'e' */
		ptr = strrchr(tmp, (spec->flags & FLAG_UPPERCASE) ? 'E' : 'e');
		if (ptr) {
			memmove(ptr+1, ptr, strlen(ptr));
			*ptr = '.';
			tmp[++len] = '\0';
		}
		else {
			tmp[len++] = '.';
			tmp[len] = '\0';
		}
		ptr = tmp;
	}
	if (sign) {
		++len;
	}

	/* adding padding characters */
	spadlen = spec->width - len;
	if (spadlen < 0) {
		spadlen = 0;
	}
	len += spadlen;
	if (spec->flags & FLAG_ZERO) {
		zpadlen = spadlen;
		spadlen = 0;
	}
	else {
		zpadlen = 0;
	}
	if (spec->flags & FLAG_JUSTIFY) {
		spadlen = -spadlen;
	}

	if (outstr) {
		while (spadlen > 0) {
			*outstr++ = ' ';
			--spadlen;
		}

		if (sign) {
			*outstr++ = sign;
		}

		while (zpadlen > 0) {
			*outstr++ = '0';
			--zpadlen;
		}

		while (*ptr) {
			*outstr++ = *ptr++;
		}

		while (spadlen++ < 0) {
			*outstr++ = ' ';
		}
	}

	return len;

#if 0
	char ipart[MAX_INT_PART], fpart[MAX_FRAC_PART];
	char tmp[MAX_INT_PART + MAX_FRAC_PART + 1];
	char *itmp, *ftmp;
	double val;
	int width, j;
	size_t i;

	j = log_10(spec->arg.d);
	val = spec->arg.d / pow_10(j);	/* get the mantissa */

	/* round up/down near the precision */
	val += ((val < 0) ? - 0.5 : 0.5) * pow_10(-spec->precision);

	num2str(val, 10, spec->precision, ipart, fpart);
	itmp = ipart;
	ftmp = fpart;

	/* calculate how much padding need */
	/* 1 for unit, 1 for the '.', 1 for 'e|E',
	* 1 for '+|-', 3 for 'exp' */
	width = spec->width -
		((val > 0 && !(spec->flags & FLAG_JUSTIFY)) ? 1 : 0) -
		((spec->flags & FLAG_SPACE) ? 1 : 0) - spec->precision - 7;

	/* PAD_RIGHT(spec); */
	/* PUT_PLUS(val, spec); */
	/* PUT_SPACE(val, spec); */
	while (*itmp) {/* the integral */
		/*PUT_CHAR(*tmp, spec);*/
		++itmp;
	}

	if (spec->precision != 0 || (spec->flags & FLAG_ALT)) {
		/* PUT_CHAR('.', spec); */  /* the '.' */
	}

	/* smash the trailing zeros */
	if (spec->flags & FLAG_FLT_G) {
		for (i = strlen(ftmp) - 1; i >= 0 && ftmp[i] == '0'; --i) {
			ftmp[i] = '\0';
		}
	}

	for (; *ftmp; ++ftmp) {
		/* PUT_CHAR(*ftmp, spec); */ /* the fraction */
	}

	/* the exponent put the 'e|E' */
	if (spec->flags & FLAG_UPPERCASE) {
		/* PUT_CHAR('E', spec); */
	}
	else {
		/* PUT_CHAR('e', spec); */
	}

	if (j > 0) {  /* the sign of the exp */
		/* PUT_CHAR('+', spec); */
	}
	else {
		/* PUT_CHAR('-', spec); */
		j = -j;
	}

	num2str(j, 10, 0, ipart, 0);
	if (j < 9) {  /* need to pad the exponent with 0 '000' */
		/* PUT_CHAR('0', spec);
		PUT_CHAR('0', spec); */
	}
	else if (j < 99) {
		/* PUT_CHAR('0', spec); */
	}

	while (*itmp) { /* the exponent */
		/* PUT_CHAR(*itmp, spec); */
		++itmp;
	}

	/*PAD_LEFT(spec);*/

	if (out) {
		*out = str_dup(tmp);
	}
	return strlen(tmp);
#endif

}

/* ------------------------------------------------------------------------- */
/* fmt_string()                                                              */

static size_t fmt_string(fmtspec_t *spec, char **out) {

	char *tmp;
	char *outstr;
	int i, len, padlen;

	tmp = (char *) spec->arg.p;
	outstr = (out) ? *out : 0;

	/* if null string, output a predefined string according to the precision:
	 * if precision is not given or greater than the length of our 'case'
	 * string, output this one "(null)", else output nothing
	 */
	if (!tmp) {
		tmp = (spec->precision == -1 || spec->precision >= 6) ? "(null)" : "";
	}

	/* compute the output length */
	for (len = 0; (spec->precision == -1 || len < spec->precision) && tmp[len];
		++len) {
	}
	padlen = spec->width - len;
	if (padlen < 0) {
		padlen = 0;
	}
	len += padlen;
	if (spec->flags & FLAG_JUSTIFY) {
		padlen = -padlen;	/* < 0 => right pad */
	}

	/* output only if requested */
	if (outstr) {
		/* do left padding if needed */
		while (padlen-- > 0) {
			*outstr++ = ' ';
		}

		/* do the copy, precision max if given */
		i = 0;
		while (*tmp && (spec->precision == -1 || (i < spec->precision))) {
			*outstr++ = *tmp++;
		}

		/* do right padding if needed */
		while (padlen++ < 0) {
			*outstr++ = ' ';
		}
	}

	return len;

}

/*	scelib - Simple C Extension Library
 *  Copyright (C) 2005-2007 Richard 'riri' GILL <richard@houbathecat.info>
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
 */
/** @file
 *	@brief Command line utility.
 *
 *	Almost every command line program needs to parse its supplied arguments.
 *	The Windows world tends to use one big string, letting programmers
 *	implement their own parser. In the Unix world, the getopt() API is the
 *	reference. None of them are easy to use and functional.
 *
 *	The SCElib command line parsing utility tries to give programmers a quick
 *	and natural way to manage command line options, via an opaque command line
 *	object and a callback system.
 *
 *	First create a cmdline_t object, then assign options with their
 *	restrictions, and call cmdline_parse(). For each discovered option
 *	(previously defined or not), the parser will call your callback function.
 *
 *	@todo	Add options description support for a <em>print usage</em> utility.
 */
#ifndef __SCELIB_CMDLINE_H
#define __SCELIB_CMDLINE_H

#include "defs.h"
#include "platform.h"

SCELIB_BEGIN_CDECL

/** Command line flags enumeration.
 *
 *	These flags control the command line object behaviour.
 */
enum cmdline_flags
{
	/** Indicates that the @a -- will be treated as an <em>end of options</em>
	 *	marker.\ Subsequent command line parameters will all be reported as
	 *	simple arguments, even if they conform to the option style.
	 */
	CMDF_ENDMARK	= 0x0001,

	/** The @a -x (a dash and one letter or digit) is reported as an option.
	 */
	CMDF_DASHSHORT	= 0x0002,

	/** The @a -long (a dash and several letters and/or digits) is reported as
	 *	an option.
	 */
	CMDF_DASHLONG	= 0x0004,

	/** The @a --long (a double dash and several letters and/or digits) is
	 *	reported as an option.
	 */
	CMDF_DDASHLONG	= 0x0008,

	/** The @a /x (a slash and one letter or digit) is reported as an option.
	 */
	CMDF_SLASHSHORT	= 0x0010,

	/** The @a /long (a slash and several letters and/or digits) is reported as
	 *	an option.
	 */
	CMDF_SLASHLONG	= 0x0020,

	/** @a option=arg (argument separated from the option by an equal sign) is
	 *	reported as an option for short (one letter or digit) options.
	 */
	CMDF_EQUALSHORT	= 0x0100,

	/** @a option=arg (argument separated from the option by an equal sign) is
	 *	reported as an option for long (several letters and/or digits) options.
	 */
	CMDF_EQUALLONG	= 0x0200,

	/** @a option @a arg (argument separated from the option by a space) is
	 *	reported as an option for short (one letter or digit) options.
	 */
	CMDF_SPACESHORT	= 0x0400,

	/** @a option @a arg (argument separated from the option by a space) is
	 *	reported as an option for long (several letters and/or digits) options.
	 */
	CMDF_SPACELONG	= 0x0800,

	/** Report only defined options as such ones.\ Non defined options aren't
	 *	reported, but simple arguments (or options after the end-of-options
	 *	marker if the @ref CMDF_ENDMARK is set) are.
	 */
	CMDF_ONLYDEFS	= 0x1000
};

/** Shortcut for GNU style set of flags.
 *
 *	Use this define to set the various flags needed to handle GNU style
 *	options, aka @c -s, @c -s @c arg, @c --long or @c --long=arg.
 *
 *	Note that this define doesn't include the @ref CMDF_ENDMARK flag.
 */
#define CMDF_GNUSTYLE	\
	CMDF_DASHSHORT|CMDF_DDASHLONG|CMDF_SPACESHORT|CMDF_EQUALLONG

/** Shortcut for Windows style set of flags.
 *
 *	Use this define to set the various flags needed to handle options as most
 *	of Windows programs do, aka @c /s, @c /s @c arg, @c /long, @c /long @c arg.
 */
#define CMDF_WINSTYLE	\
	CMDF_SLASHSHORT|CMDF_SLASHLONG|CMDF_SPACESHORT|CMDF_SPACELONG

/**	Shortcut for Java style set of flags.
 *
 *	This's not really the convention, but many programs coming from the Java
 *	and <em>new technologies</em> worlds use styles like @c -s, @c -s=arg,
 *	@c -long or @c -long=arg.
 */
#define CMDF_JAVASTYLE	CMDF_DASHSHORT|CMDF_DASHLONG|CMDF_SPACESHORT|CMDF_SPACELONG

#if !defined(DOXYGEN_PARSING)
#if PLATFORM_IS(WINDOWS)
#define CMDF_STDSTYLE	CMDF_WINSTYLE
#else
#define CMDF_STDSTYLE	CMDF_GNUSTYLE
#endif
#else	/* DOXYGEN_PARSING */
/**	Shortcut for platform dependent style.
 *
 *	This define will correspond to @ref CMDF_GNUSTYLE on *nix and
 *	@ref CMDF_WINSTYLE on Windows.
 */
#define CMDF_STDSTYLE
#endif

/** The command line object.
 *
 *	This opaque structure is a handle to a created command line object, by
 *	which you can use the scelib command line utility. You'll find it as the
 *	first parameter of each function.
 */
typedef struct cmdline_type *cmdline_t;

/**	Option or argument reference in callback.
 *
 *	For each call of your callback function, a new cmdparsed_t object will be
 *	given as option information. In this structure, you'll find generic options
 *	fields.
 *
 *	When the parser looks for options and find one, it checks its style and
 *	report it really as an option if the style is correct, or as a simple
 *	argument if not.
 */
typedef struct cmdparsed_type
{
	/** Position on command line.
	 *
	 *	See it as an index to @c argv.
	 */
	int opt_pos;

	/** Actual option name, without prefix.
	 *
	 *	It's NULL if the command line parameter is not an option (argument
	 *	alone).
	 */
	char *opt_name;

	/** Argument, if specified, NULL otherwise.
	 *
	 *	Note that @e bad-style options will be treated as arguments, and will
	 *	be placed in this field.
	 */
	char *opt_arg;

	/** Reference to option id, -1 if not a defined one.
	 *	You can detect if the call is for defined options or not, by checking
	 *	the @a optid field. @a optid is the index to the defined option in the
	 *	@ref cmdline_t object, or -1 if the parser didn't detect one for this
     *	command line argument.
 	 */
	int optid;
} cmdparsed_t;

/** Prototype for command line callbacks.
 *
 *	Your callback(s) must have this prorotype. For each call, you'll have
 *	access to the command line object (to retrieve option information for
 *	example), a simple @ref cmdparsed_t structure pointer, in which you'll find
 *	all revelant information about the actual argument, and a pointer to user
 *	data you've previously passed to the parser function (this's a quick way to
 *	keep state information, you can use it to store you're detected options for
 *	example, without the need of global variables - global variables are devil).
 */
typedef int (*cmdline_cb_t)(cmdline_t, cmdparsed_t *, void *);

/** Create a command line utility object.
 *
 *	This function creates a command line object and return it. The callback
 *	parameter is a default one, when no defined option was found or no specific
 *	callback function has been declared for a defined option. You can pass NULL
 *	as callback parameter to avoid being called in another way than for a
 *	specific option.
 *
 *	@param[in] flags	OR-ed list of @ref cmdline_flags "command line flags"
 *	@param[in] callback	global callback
 *	@return	a new @ref cmdline_t object
 */
cmdline_t cmdline_create(int flags, cmdline_cb_t callback);

/**	Destroys a previously allocated cmdline_t object.
 *
 *	It frees all internal datas too. Call it after parsing if you don't need
 *	any reference to the command line object.
 *
 *	@param[in] cl		the object to destroy
 *	@return 0 if all is ok, -1 otherwise (see @c errno).
 */
int cmdline_destroy(cmdline_t cl);

/** The command line parsing handler.
 *
 *	Its scheme is quiet simple. It navigate through the actual arguments passed
 *	on the command line, and check for each if an option was defined. If it's
 *	not the case and a default callback function was specified (with
 *	@ref cmdline_create()), it'll call it.
 *
 *	If a defined option is detected, it'll call the option specific callback
 *	like for non defined options. You can use a specific callback for each
 *	option, use a generic one, or a shared one for some options, at your
 *	choice, you can completly manage the way you handle options.
 *
 *	In this callback, you've to return a value: 0 to indicate to continue the
 *	parsing (for example if you detected an error) or a non zero value to stop.
 *	The parser will return the last called callback return value. As the parser
 *	returns -1 in case of internal error, you should avoid using this value in
 *	callback returns (use instead a positive value).
 *
 *	@param[in] cl		the command line object
 *	@param[in] argc		number of command line arguments
 *	@param[in] argv		vector to command line strings
 *	@param[in] userdata	optional user data forwarded to callbacks
 *	@return the last callback return value, or -1 if any internal error occured
 */
int cmdline_parse(cmdline_t cl, int argc, char **argv, void* userdata);

/** Add a defined option to the command line object.
 *
 *	When you have a @ref cmdline_t object, you can assign to it options that
 *	will be checked by the parser. The options representation is internal to
 *	the utility, which great simplifies the interface.
 *
 *	Defining options isn't mandatory (this utility can be used just as a quick
 *	command line callback scheme), but you'll miss all its interest.
 *
 *	@param[in] cl		the command line object
 *	@param[in] sname	the short name (one letter or digit), or 0 if you don't
 *						want this option to have a short name
 *	@param[in] lname	the long name (several letters and/or digits), or NULL
 *						if you don't want this option to have a long name
 *	@param[in] callback	callback specific to this defined option
 *	@return the option index in the @ref cmdline_t set. This index if the one
 *			used to initialize the @a optid field in the @ref cmdparsed_t
 *			structure.
 */
int cmdline_addopt(cmdline_t cl, char sname, char *lname,
				   cmdline_cb_t callback);

/** Add a defined option to the command line object if the predicate is true.
 *
 *	This function acts like @ref cmdline_addopt(), but have an additional
 *	parameter @a pred which tells if the option should be really defined or
 *	not (usefull for platform dependant options). The option won't be added
 *	if @a pred is 0.
 *
 *	@see cmdline_addopt()
 */
int cmdline_addopt_if(cmdline_t cl, char sname, char *lname,
					  cmdline_cb_t callback, int pred);

/** Get information on a defined option.
 *
 *	This function returns the information of the option pointed by the
 *	@ref cmdline_t object and the @a optid option index.
 *
 *	Don't use buffers or free returned buffers. The returned pointers point to
 *	allocated memory locations. For example:
 *	@code
 *	char sname;
 *	char *lname;
 *	cmdline_getopt(cl, 0, &sname, &lname);
 *	// do anything
 *	cmdline_getopt(cl, 1, &sname, &lname);
 *	@endcode
 *
 *	@param[in] cl		the command line object
 *	@param[in] optid	the option index
 *	@param[out] sname	returned short name (0 if no short name)
 *	@param[out] lname	returned long name (NULL if no long name)
 *	@return 0 if all is Ok or -1 when an error occurred (essentialy EINVAL for
 *			bad option referencing).
 */
int cmdline_getopt(cmdline_t cl, int optid, char *sname, char **lname);

SCELIB_END_CDECL

#endif /* __SCELIB_CMDLINE_H */
/* vi:set ts=4 sw=4: */

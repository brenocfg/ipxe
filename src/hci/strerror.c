#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <gpxe/errortab.h>

/** @file
 *
 * Error descriptions.
 *
 * The error numbers used by Etherboot are a superset of those defined
 * by the PXE specification version 2.1.  See errno.h for a listing of
 * the error values.
 *
 * To save space in ROM images, error string tables are optional.  Use
 * the ERRORMSG_XXX options in config.h to select which error string
 * tables you want to include.  If an error string table is omitted,
 * strerror() will simply return the text "Error 0x<errno>".
 *
 */

static struct errortab errortab_start[0]
	__table_start ( struct errortab, errortab );
static struct errortab errortab_end[0]
	__table_end ( struct errortab, errortab );

/**
 * Find error description
 *
 * @v errno		Error number
 * @v mask		Mask of bits that we care about
 * @ret errortab	Error description, or NULL
 */
static struct errortab * find_error ( int errno, int mask ) {
	struct errortab *errortab;

	for ( errortab = errortab_start ; errortab < errortab_end ;
	      errortab++ ) {
		if ( ( ( errortab->errno ^ errno ) & mask ) == 0 )
			return errortab;
	}

	return NULL;
}

/**
 * Find closest error description
 *
 * @v errno		Error number
 * @ret errortab	Error description, or NULL
 *
 * 
 */
static struct errortab * find_closest_error ( int errno ) {
	struct errortab *errortab;

	/* First, look for an exact match */
	if ( ( errortab = find_error ( errno, 0x7fffffff ) ) != NULL )
		return errortab;

	/* Second, try masking off the gPXE-specific bit and seeing if
	 * we have an entry for the generic POSIX error message.
	 */
	if ( ( errortab = find_error ( errno, 0x0000ffff ) ) != NULL )
		return errortab;

	/* Lastly, try masking off the POSIX bits and seeing if we
	 * have a match just based on the PXENV component.  This
	 * allows us to report errors from underlying PXE stacks.
	 */
	if ( ( errortab = find_error ( ( errno & 0x000000ff ),
				       0xffff00ff ) ) != NULL )
		return errortab;

	return NULL;
}

/**
 * Retrieve string representation of error number.
 *
 * @v errno/rc		Error number or return status code
 * @ret strerror	Pointer to error text
 *
 * If the error is not found in the linked-in error tables, generates
 * a generic "Error 0x<errno>" message.
 *
 * The pointer returned by strerror() is valid only until the next
 * call to strerror().
 *
 */
const char * strerror ( int errno ) {
	static char errbuf[64];
	struct errortab *errortab;

	/* Allow for strerror(rc) as well as strerror(errno) */
	if ( errno < 0 )
		errno = -errno;

	/* Find the error description, if one exists */
	errortab = find_closest_error ( errno );

	/* Construct the error message */
	if ( errortab ) {
		snprintf ( errbuf, sizeof ( errbuf ), "%s (%#08x)",
			   errortab->text, errno );
	} else {
		snprintf ( errbuf, sizeof ( errbuf ), "Error %#08x", errno );
	}

	return errbuf;
}

/** The most common errors */
struct errortab common_errors[] __errortab = {
	{ 0, "No error" },
	{ ENOMEM, "Out of memory" },
	{ EINVAL, "Invalid argument" },
	{ ENOSPC, "No space left on device" },
	{ EIO, "Input/output error" },
	{ EACCES, "Permission denied" },
	{ ENOENT, "File not found" },
	{ ENETUNREACH, "Network unreachable" },
	{ ETIMEDOUT, "Connection timed out" },
};

#ifndef _CONST_H
#define _CONST_H

/**************************************************************************** 
 *
 * This header file contains the global constant & macro definitions.
 * 
 ****************************************************************************/

/* Maxi number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 20

#define UPROCMAX 3  /* number of usermode processes (not including master proc
		       and system daemons */

#define	HIDDEN static
#define	TRUE 	1
#define	FALSE	0
#define ON 	1
#define OFF 	0
#define EOS '\0'

#define DEV_PER_INT 8 /* Maximum number of devices per interrupt line */

#define CR 0x0a   /* carriage return as returned by the terminal */

#endif

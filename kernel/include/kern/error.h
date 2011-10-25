/*
 * System call error constants.
 */
#ifndef __ERROR_H
#define __ERROR_H

#define EOK (0) /* success */
#define EFAIL (-1) /* unknown failure */
#define ENOSYS (-2) /* no such system call */
#define EINVAL (-3) /* invalid argument */
#define ENOMEM (-4) /* not enough memory */
#define ENOEVT (-5) /* interrupt already waited on */
#define EM2BIG (-6) /* message is too big */
#define ENOSEND (-7) /* destination process is dead */
#define ENOREPL (-8) /* process is not expecting reply */
#define EN2BIG (-9) /* name is too big */

#endif


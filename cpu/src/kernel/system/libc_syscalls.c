/*----------------------------------------------------------------------

                     This file is part of Freetribe

                https://github.com/bangcorrupt/freetribe

                                License

                   GNU AFFERO GENERAL PUBLIC LICENSE
                      Version 3, 19 November 2007

                           AGPL-3.0-or-later

 Freetribe is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
                  (at your option) any later version.

     Freetribe is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty
        of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
          See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.

                       Copyright bangcorrupt 2023

----------------------------------------------------------------------*/

/**
 * @file    libc_syscalls.c
 *
 * @brief   Implementation of Newlib stub functions.
 */

/*----- Includes -----------------------------------------------------*/

#include <sys/stat.h>

#include <stdbool.h>
#include <stdint.h>

#include "svc_midi.h"

#include <unistd.h>

#include <errno.h>

/// TODO: This is suggested Embcosm docs, but breaks build in Debian container.
/// https://www.embecosm.com/appnotes/ean9/ean9-howto-newlib-1.0.html
//
// #undef errno
// extern int errno;

/*----- Macros -------------------------------------------------------*/

/*----- Typedefs -----------------------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

void *_sbrk(int incr) {

    // Defined by the linker.
    extern uint8_t _heap_end;

    static uint8_t *heap_end;

    uint8_t *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_heap_end;
    }
    prev_heap_end = heap_end;

    /// TODO: Check for collision.
    //
    // if (heap_end + incr > stack_ptr) {
    //     // write(1, "Heap and stack collision\n", 25);
    //     // abort();
    // }

    heap_end += incr;
    return (void *)prev_heap_end;
}

int _write(int file, char *buffer, int length) {

    int result = 0;

    // We only handle stdout and stderr.
    if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {

        errno = EBADF;
        result = -1;
    }

    while (length--) {

        svc_midi_send_byte(*buffer++);
        result++;
    }

    return result;
}

int _close(int file) { return -1; }

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) { return 1; }

int _lseek(int file, int ptr, int dir) { return 0; }

int _read(int file, char *ptr, int len) { return 0; }

int _getpid(void) { return 1; }

int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

void _exit(int status) {

    while (true)
        ;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/

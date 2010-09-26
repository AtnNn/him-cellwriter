
/*

cellwriter -- a character recognition input method
Copyright (C) 2007 Michael Levin <risujin@risujin.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "config.h"
#include "common.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/*
        Single-instance checks
*/

static SingleInstanceFunc on_dupe;
static int fifo;
static char *path;

static gboolean check_dupe(void)
{
        ssize_t len;
        char buf[2];

        if (fifo <= 0 || !on_dupe)
                return FALSE;
        len = read(fifo, buf, 1);
        buf[1] = 0;
        if (len > 0)
                on_dupe(buf);
        return TRUE;
}

void single_instance_cleanup(void)
{
        if (fifo > 0)
                close(fifo);
        if (path && unlink(path) == -1)
                log_errno("Failed to unlink program FIFO");
}

int single_instance_init(SingleInstanceFunc func, const char *str)
{
        on_dupe = func;
        path = g_build_filename(g_get_home_dir(), "." PACKAGE, "fifo", NULL);

        /* If we can open the program FIFO in write-only mode then we must
           have a reader process already running. We send it a one-byte junk
           message to wake it up and quit. */
        if ((fifo = open(path, O_WRONLY | O_NONBLOCK)) > 0) {
                write(fifo, str, 1);
                close(fifo);
                return TRUE;
        }

        /* The FIFO can be left over from a previous instance if the program
           crashes or is killed */
        if (g_file_test(path, G_FILE_TEST_EXISTS)) {
                g_debug("Program FIFO exists but is not opened on "
                        "read-only side, deleting\n");
                single_instance_cleanup();
        }

        /* Otherwise, create a read-only FIFO and poll for input */
        fifo = 0;
        if (mkfifo(path, S_IRUSR | S_IWUSR)) {
                log_errno("Failed to create program FIFO");
                return FALSE;
        }
        if ((fifo = open(path, O_RDONLY | O_NONBLOCK)) == -1) {
                log_errno("Failed to open FIFO for reading");
                return FALSE;
        }

        /* Setup the polling function */
        g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE, 1000,
                           (GSourceFunc)check_dupe, NULL, NULL);

        return FALSE;
}


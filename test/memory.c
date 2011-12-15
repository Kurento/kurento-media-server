#include <glib.h>
#include <string.h>
#include <stdlib.h>

#define PROC "/proc/self/status"

int
get_data_memory() {
	int mem;

	GIOChannel *chan;
	GError *err= NULL;
	GIOStatus st;
	char *line = NULL;
	char *vmdata = NULL;
	size_t len;

	chan = g_io_channel_new_file(PROC, "r", &err);
	if (chan == NULL && err != NULL) {
		g_printerr("%s:%d: %s", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}

	g_assert(chan != NULL);

	/* Read memory size data from /proc/pid/status */
	while (!vmdata) {

		st = g_io_channel_read_line(chan, &line, &len, NULL, &err);

		if (st != G_IO_STATUS_NORMAL && err != NULL) {
			g_printerr("%s:%d: %s", __FILE__, __LINE__, err->message);
			g_error_free(err);
		}

		g_assert(st == G_IO_STATUS_NORMAL);

		if (!strncmp(line, "VmData:", 7)) {
			vmdata = g_strdup(&line[7]);
		}

		g_free(line);
	}

	g_io_channel_shutdown(chan, FALSE, NULL);
	g_io_channel_unref(chan);

	/* Get rid of " kB\n"*/
	len = strlen(vmdata);
	vmdata[len - 4] = 0;

	mem = atoi(vmdata);

	g_free(vmdata);

	return mem;
}

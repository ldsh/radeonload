/*
    Copyright (C) 2012 Lauri Kasanen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "radeonload.h"
#include <getopt.h>

const void *area;

void die(const char * const why) {
	puts(why);
	exit(1);
}

static void version() {
	printf("RadeonLoad %s\n", VERSION);
	exit(1);
}

static void help(const char * const me, const unsigned int ticks) {
	printf(_("\n\tRadeonLoad for R600 and above.\n\n"
		"\tUsage: %s [-ch] [-b bus] [-d file] [-l limit] [-t ticks]\n\n"
		"-b --bus 3		Pick card from this PCI bus\n"
		"-c --color		Enable colors\n"
		"-d --dump file		Dump data to this file, - for stdout\n"
		"-l --limit 3		Quit after dumping N lines, default forever\n"
		"-t --ticks 50		Samples per second (default %u)\n"
		"\n"
		"-h --help		Show this help\n"
		"-v --version		Show the version\n"),
		me, ticks);
	die("");
}

unsigned int readgrbm() {

	const void *ptr = area + 0x10;

	const unsigned int *inta = ptr;

	return *inta;
}

int main(int argc, char **argv) {

	unsigned int ticks = 120;
	unsigned char color = 0;
	unsigned char bus = 0;
	unsigned int limit = 0;
	char *dump = NULL;

	// Translations
#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain("radeonload", "/usr/share/locale");
	textdomain("radeonload");
#endif

	// opts
	const struct option opts[] = {
		{"bus", 1, 0, 'b'},
		{"color", 0, 0, 'c'},
		{"dump", 1, 0, 'd'},
		{"help", 0, 0, 'h'},
		{"limit", 1, 0, 'l'},
		{"ticks", 1, 0, 't'},
		{"version", 0, 0, 'v'},
		{0, 0, 0, 0}
	};

	while (1) {
		int c = getopt_long(argc, argv, "b:cd:hl:t:v", opts, NULL);
		if (c == -1) break;

		switch(c) {
			case 'h':
			case '?':
				help(argv[0], ticks);
			break;
			case 't':
				ticks = atoi(optarg);
			break;
			case 'c':
				color = 1;
			break;
			case 'b':
				bus = atoi(optarg);
			break;
			case 'v':
				version();
			break;
			case 'l':
				limit = atoi(optarg);
			break;
			case 'd':
				dump = optarg;
			break;
		}
	}

	// init
	const unsigned int pciaddr = init_pci(bus);

	const int family = getfamily(pciaddr);
	if (!family)
		puts(_("Unknown Radeon card. <= R500 won't work, new cards might."));

	const char * const cardname = family_str[family];

	initbits(family);

	// runtime
	collect(&ticks);

	if (dump)
		dumpdata(ticks, dump, limit);
	else
		present(ticks, cardname, color);

	munmap((void *) area, MMAP_SIZE);
	return 0;
}

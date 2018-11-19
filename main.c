#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static char *argv0;

int verbose;

static void
usage()
{
	fprintf(stderr, "usage: %s [ -v ] file\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	argv0 = argv[0];
	argc--;
	argv++;

	while (argc > 1) {
		if (strncmp(argv[0], "-v", 2) == 0)
			verbose = 1;
		else
			usage();
		argc--;
		argv++;
	}

	if (argc != 1)
		usage();

	if (decode(argv[0]) < 0)
		return 1;

	return 0;
}

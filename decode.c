#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>

#include "bele.h"
#include "dat.h"
#include "fns.h"

extern int verbose;

uint8_t *buf;

int
unpackhdr(Hdr *hdr, uint8_t *buf, unsigned int size)
{
	uint8_t *p;

	if (size < HdrSize)
		return -1;

	p = buf;

	memmove(hdr->filename, p, sizeof(hdr->filename));
	if (hdr->filename[sizeof(hdr->filename) - 1] != '\0'){
		fprintf(stderr, "bad filename\n");
		return -1;
	}

	p += sizeof(hdr->filename);
	p += le32get(p, &hdr->filepointer);
	p += le32get(p, &hdr->maximumsize);
	p += le32get(p, &hdr->isreadonly);
	p += le32get(p, &hdr->encryptedsize);
	p += le32get(p, &hdr->fileposition);

	return p-buf;
}

int
readhdr(FILE *f, Hdr *hdr)
{
	uint8_t buf[HdrSize];
	int n;

	if (fread(buf, HdrSize, 1, f) != 1) {
		if (feof(f))
			return 0;
		return -1;
	}

	if ((n = uodemo_decrypt(buf, sizeof(buf))) < 0) {
		fprintf(stderr, "error: uodemo_decrypt\n");
		return -1;
	}

	if ((n = unpackhdr(hdr, buf, sizeof(buf))) < 0)
		return -1;

	return n;
}

void
cleanname(char *name)
{
	char *p;

	while((p = strstr(name, "../"))) {
		memmove(p, p + 3, strlen(p + 3) + 1);
	}
}

int
readdata(FILE *f, Hdr *hdr, int nhdr)
{
	char *dir;
	struct stat sb;
	char name[MAX_PATH];
	FILE *ff;
	uint32_t size;
	unsigned int i, bsize;
	int n;

	if (fseek(f, (nhdr * HdrSize) + hdr->filepointer, SEEK_SET) < 0)
		return -1;

	cleanname(hdr->filename);

	memmove(name, hdr->filename, sizeof(name));

	dir = dirname(name);
	if (dir == NULL)
		return -1;

	if (stat(dir, &sb) != 0) {
		if (mkdir(dir, 0775) < 0) {
			if (errno != EEXIST) {
				fprintf(stderr, "error: mkdir: %s\n", strerror(errno));
				return -1;
			}
		}
	} else if (!S_ISDIR(sb.st_mode)) {
		fprintf(stderr, "error: file %s exists and is not a directory\n", dir);
		return -1;
	}

	ff = fopen(hdr->filename, "wb");
	if (f == NULL) {
			fprintf(stderr, "error: fopen: %s\n", strerror(errno));
			return -1;
	}

	if (fread(buf, hdr->encryptedsize, 1, f) != 1) {
		if (feof(f))
			return 0;
		return -1;
	}

	bsize = BlkSize;
	for(i = 0; i < hdr->encryptedsize; i += bsize) {
		if((hdr->encryptedsize - i) < bsize)
			bsize = hdr->encryptedsize - i;
		if ((n = uodemo_decrypt(buf+ i, bsize)) < 0) {
			fprintf(stderr, "error: uodemo_decrypt\n");
			return -1;
		}
	}

	le32get(buf+hdr->encryptedsize-4, &size);

	if (size > 0) {
		if (fwrite(buf, size, 1, ff) != 1)
			return -1;
	}

	if (fclose(ff) != 0) {
			fprintf(stderr, "error: fclose: %s\n", strerror(errno));
			return -1;
	}

	return 0;
}

FILE*
fdup(FILE *f)
{
	int ofd, nfd;

	ofd = fileno(f);
	if (ofd < 0)
		return NULL;

	nfd = dup(ofd);
	if (nfd < 0)
		return NULL;

	return fdopen(nfd, "rb");
}

int
readfile(FILE *f)
{
	Hdr hdrs[3000], *hdr;
	unsigned int i, nhdr;
	int n;

	for (i = 0; i < sizeof(hdrs); i++) {
		hdr = &hdrs[i];

		n = readhdr(f, hdr);
		if (n < 0)
			return -1;

		if (verbose) {
			printf("filename %s\n", hdr->filename);
			printf("filepointer %d\n", hdr->filepointer);
			printf("maximumsize %d\n", hdr->maximumsize);
			printf("isreadonly %d\n", hdr->isreadonly);
			printf("encryptedsize %d\n", hdr->encryptedsize);
			printf("fileposition %d\n", hdr->fileposition);
		}

		if (strcmp(hdr->filename, "@@@.@@@") == 0)
			break;
	}

	nhdr = i + 1;

	buf = malloc(MaxFileSize);
	if (buf == NULL)
		return -1;

	for (i = 0; i < nhdr - 1; i++) {
		hdr = &hdrs[i];

		n = readdata(f, hdr, nhdr);
		if (n < 0)
			return -1;
	}

	free(buf);

	return 0;
}

int
decode(char *name)
{
	FILE *f;

	f = fopen(name, "rb");
	if (f == NULL) {
		fprintf(stderr, "error: fopen: %s\n", strerror(errno));
		return -1;
	}

	if (readfile(f) < 0)
		return -1;

	if (fclose(f) != 0) {
		fprintf(stderr, "error: fclose: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

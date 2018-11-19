typedef struct Hdr Hdr;

enum {
	HdrSize = 280,
	BlkSize = 0x1000,
	MaxFileSize = 2 * 1024 * 1024,
};

#define MAX_PATH 260

struct Hdr {
	char filename[MAX_PATH];	/* 260 is MAX_PATH in Windows */
	uint32_t filepointer;		/* this pointer is relative from the beginning of the data area */
	uint32_t maximumsize;		/* if set, the file can grow until this size is reached */
	uint32_t isreadonly;		/* 0: writeable, != 0: read-only */
	uint32_t encryptedsize;		/* encrypted file size (not the actual file size) */
	uint32_t fileposition;		/* internally used by uodemo.exe, not relevant for reading from the archive */
};

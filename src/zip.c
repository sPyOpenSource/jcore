#include "all.h"

/*
 * Read a zipfile with uncompressed entries
 */


#define LREC_SIZE 30
#define CREC_SIZE 46
#define ECREC_SIZE 22


#define LOCAL_HEADER_SIGNATURE 0x04034b50
#define L_VERSION_NEEDED_TO_EXTRACT_0 4
#define L_VERSION_NEEDED_TO_EXTRACT_1 5
#define L_GENERAL_PURPOSE_BIT_FLAG 6
#define L_COMPRESSION_METHOD 8
#define L_LAST_MOD_FILE_TIME 10
#define L_LAST_MOD_FILE_DATE 12
#define L_CRC32 14
#define L_COMPRESSED_SIZE 18
#define L_UNCOMPRESSED_SIZE 22
#define L_FILENAME_LENGTH 26
#define L_EXTRA_FIELD_LENGTH 28

#define CENTRAL_HEADER_SIGNATURE 0x02014b50
#define C_SIGNATURE 0
#define C_VERSION_MADE_BY_0 4
#define C_VERSION_MADE_BY_1 5
#define C_VERSION_NEEDED_TO_EXTRACT_0 7
#define C_VERSION_NEEDED_TO_EXTRACT_1 8
#define C_GENERAL_PURPOSE_BIT_FLAG 9
#define C_COMPRESSION_METHOD 10
#define C_LAST_MOD_FILE_TIME 12
#define C_LAST_MOD_FILE_DATE 14
#define C_CRC32 16
#define C_COMPRESSED_SIZE 20
#define C_UNCOMPRESSED_SIZE 24
#define C_FILENAME_LENGTH 28
#define C_EXTRA_FIELD_LENGTH 30
#define C_COMMENT_LENGTH 32
#define C_DISK_NUMBER_START 34
#define C_INTERNAL_FILE_ATTRIBUTES 36
#define C_EXTERNAL_FILE_ATTRIBUTES 38
#define C_RELATIVE_OFFSET_LOCAL_HEADER 42
#define C_FILENAME 46

#define END_CENTRAL_DIR_SIGNATURE 0x06054b50
#define NUMBER_THIS_DISK 4
#define NUM_DISK_WITH_START_CENTRAL_DIR 6
#define NUM_ENTRIES_CENTRL_DIR_THS_DISK 8
#define TOTAL_ENTRIES_CENTRAL_DIR 10
#define SIZE_CENTRAL_DIRECTORY 12
#define OFFSET_START_CENTRAL_DIRECTORY 16
#define ZIPFILE_COMMENT_LENGTH 20


static char *zip;
static jint count;
static jint dirofs;
static char *mempos;
static char *dirbuf;
static jint current;

static void zseek(jint pos)
{
	if (pos < 0)
		sys_panic("Seeking to negative position ");
	mempos = zip + pos;
}

static void zread(char *buf, jint len)
{
	memcpy(buf, mempos, len);
}

void zip_reset()
{
	mempos = zip;
	dirofs = 0;
	current = 0;
}

void zip_init(char *zipstart, jint ziplen)
{
	jint signature, dir_size;
	char buffer[ECREC_SIZE];
	int len = ziplen;
	zip = zipstart;
	mempos = zip;
	zseek(len - ECREC_SIZE);
	zread(buffer, ECREC_SIZE);
	signature = makelong(buffer, 0);
	if (signature != END_CENTRAL_DIR_SIGNATURE) {
		sys_panic("Wrong signature=0x%lx\n", signature);
		return;
	}
	count = makeword(buffer, TOTAL_ENTRIES_CENTRAL_DIR);
	/*Debug.out.println("count="+count); */
	dir_size = makelong(buffer, SIZE_CENTRAL_DIRECTORY);
	/*Debug.out.println("dir_size="+dir_size); */
	zseek(len - (dir_size + ECREC_SIZE));
	dirbuf = jxmalloc(dir_size MEMTYPE_OTHER);
	zread(dirbuf, dir_size);
	/*
	   Debug.out.println("nr disk = " + makeword(dirbuf, NUMBER_THIS_DISK));
	   Debug.out.println("nr disk cdir = " + makeword(dirbuf, NUM_DISK_WITH_START_CENTRAL_DIR));
	   Debug.out.println("num entries = " + makeword(dirbuf,NUM_ENTRIES_CENTRL_DIR_THS_DISK));
	 */
	dirofs = 0;
	current = 0;
}

int zip_next_entry(zipentry * entry)
{
	jint signature, filename_length, cextra_length, comment_length, local_header_offset, filestart;
	char header[LREC_SIZE];
	if (current == count)
		return -1;
	current++;
	//printf("** %d\n", current);
	signature = makelong(dirbuf, dirofs + 0);
	//printf("Signature: 0x%lx\n", signature);
	if (signature != CENTRAL_HEADER_SIGNATURE) {
		sys_panic("wrong central header signature 0x%lx\n", signature);
		return -1;
	}
	entry->uncompressed_size = makelong(dirbuf, dirofs + C_UNCOMPRESSED_SIZE);
	/* compr. method ? */

	filename_length = makeword(dirbuf, dirofs + C_FILENAME_LENGTH);
	cextra_length = makeword(dirbuf, dirofs + C_EXTRA_FIELD_LENGTH);
	comment_length = makeword(dirbuf, dirofs + C_COMMENT_LENGTH);
	local_header_offset = makelong(dirbuf, dirofs + C_RELATIVE_OFFSET_LOCAL_HEADER);

	/* read local header */
	zseek(local_header_offset);
	zread(header, LREC_SIZE);
	signature = makelong(header, 0);
	if (signature != LOCAL_HEADER_SIGNATURE) {
		sys_panic("wrong local header signature");
		return -1;
	}

	filestart = local_header_offset + LREC_SIZE + makeword(header, L_FILENAME_LENGTH) + makeword(header, L_EXTRA_FIELD_LENGTH);

	makestring(entry->filename, dirbuf, dirofs + C_FILENAME, filename_length);
	entry->data = zip + filestart;

	entry->compression_method = makeword(dirbuf, dirofs + C_COMPRESSION_METHOD);
	if (entry->compression_method != 0) {
		printf("Filename: %s\n", entry->filename);
		sys_panic("Compression not supported.");
		return -1;
	}

	dirofs += C_FILENAME + filename_length + cextra_length + comment_length;
	return 0;
}

static jint makeword(char *b, int offset)
{
	return (((jint) (b[offset + 1]) << 8) | ((jint) (b[offset]) & 0xff));
}

static jint makelong(char *b, int offset)
{
	return ((((jint) b[offset + 3]) << 24) & 0xff000000)
	     | ((((jint) b[offset + 2]) << 16) & 0x00ff0000)
	     | ((((jint) b[offset + 1]) << 8)  & 0x0000ff00)
	     |   ((jint) b[offset + 0]         & 0xff);
}

static char *makestring(char *buf, char *b, int offset, int len)
{
	strncpy(buf, b + offset, len);
	buf[len] = '\0';
}

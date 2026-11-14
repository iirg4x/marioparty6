#ifndef MP6_C_FILE_STRUC_H
#define MP6_C_FILE_STRUC_H
#include "stddef.h"
/* Stream state and callback contracts for file I/O. */
/* Identify a file in the underlying I/O callbacks. */
typedef unsigned long __file_handle;
/* Represent a saved file position. */
typedef unsigned long fpos_t;

/* Mark a stream failed and discard its active buffered interval. */
#define set_error(file) \
    do { \
        (file)->file_state.error = 1; \
        (file)->buffer_length = 0; \
    } while (0)

/* Distinguish closed, disk, console and string streams. */
enum __file_kinds {
	__closed_file, __disk_file, __console_file, __string_file, __unavailable_file
};

/* Track whether a stream uses byte or wide-character I/O. */
enum __file_orientation {
	/* 0x0 */ UNORIENTED,
	/* 0x1 */ CHAR_ORIENTED,
	/* 0x2 */ WIDE_ORIENTED
};

/* Stream-mode bitfields pack access, buffering, backing kind, orientation and translation. */
typedef struct _file_modes {
	// Opening-mode selector retained for the stream.
	unsigned int open_mode : 2;
	// Access-mode bits used to allow reading, writing or appending.
	unsigned int io_mode : 3;
	// Selector for unbuffered, line-buffered or fully buffered transfers.
	unsigned int buffer_mode : 2;
	// Kind of backing stream used by the file operations.
	unsigned int file_kind : 3;
	// Byte or wide-character orientation selected by stream I/O.
	unsigned int file_orientation : 2;
	// Binary-mode flag controlling character translation.
	unsigned int binary_io : 1;
} file_modes;

/* Select read, write, update or append access. */
enum __io_modes {
	__read       = 1, __write      = 2, __read_write = 3, __append     = 4
};

/* Track the current direction and pushback state. */
enum __io_states {
	__neutral, __writing, __reading, __rereading
};

/* Report completion, an I/O error or end of file. */
enum __io_results {
	__no_io_error, __io_error, __io_EOF
};

/* State packs direction and ownership bits before the EOF and error indicator bytes. */
typedef struct _file_states {
	// Current read, write or pushback direction.
	unsigned int io_state : 3;
	// Ownership flag indicating whether the stream releases its buffer.
	unsigned int free_buffer : 1;
	// End-of-file indicator set by input operations.
	unsigned char eof;
	// Error indicator retained until the stream state is cleared.
	unsigned char error;
} file_states;

/* Run while a callback waits for an I/O operation. */
typedef void (*__idle_proc)(void);
/* Seek the underlying handle and return the resulting position. */
typedef int (*__pos_proc)(__file_handle file, fpos_t* position, int mode, __idle_proc idle_proc);
/* Transfer bytes through the underlying file handle. */
typedef int (*__io_proc)(__file_handle file, unsigned char* buff, size_t* count,
    __idle_proc idle_proc);
/* Close the underlying file handle. */
typedef int (*__close_proc)(__file_handle file);

/* Stream record stores modes, character buffers, byte-buffer state, callbacks and a file link. */
typedef struct _FILE {
	// Backing I/O handle supplied to the file callbacks.
	__file_handle handle;
	// Opening, buffering and orientation settings for this stream.
	file_modes file_mode;
	// Current transfer direction and persistent stream indicators.
	file_states file_state;

	/* Track record ownership and single-character buffering. */
	unsigned char is_dynamically_allocated;
	// Single-byte storage used by unbuffered character I/O.
	char char_buffer;
	// Additional character-storage byte in the stream record.
	char char_buffer_overflow;
	// Two-byte storage for characters pushed back onto the input stream.
	char ungetc_buffer[2];
	// Two wide-character slots retained for input pushback.
	wchar_t ungetc_wide_buffer[2];

	/* Track the file position and the active buffered interval. */
	unsigned long position;
	// Base address of the stream's byte buffer.
	unsigned char* buffer;
	// Allocated byte extent of the stream buffer.
	unsigned long buffer_size;
	// Current byte position within the buffered interval.
	unsigned char* buffer_ptr;
	// Bytes remaining in the active transfer interval.
	unsigned long buffer_length;
	// Alignment value used when positioning buffered transfers.
	unsigned long buffer_alignment;
	// Buffered length retained while pushback changes the active interval.
	unsigned long save_buffer_length;
	// File offset corresponding to the start of buffered data.
	unsigned long buffer_position;

	/* Dispatch positioning, transfer, close and idle operations. */
	__pos_proc position_fn;
	// Callback that fills input bytes from the backing handle.
	__io_proc read_fn;
	// Callback that sends buffered bytes to the backing handle.
	__io_proc write_fn;
	// Callback used to release the backing file handle.
	__close_proc close_fn;
	// Callback invoked while a backing I/O operation waits.
	__idle_proc idle_fn;
	// Link connecting this stream to the file-record chain.
	struct _FILE* next_file_struct;
} FILE;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define _IOFBF 2
#define _IOLBF 1
#define _IONBF 0
#ifdef __cplusplus
extern "C" {
#endif
extern FILE __files[];
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace std { using ::FILE; }
#endif
#endif

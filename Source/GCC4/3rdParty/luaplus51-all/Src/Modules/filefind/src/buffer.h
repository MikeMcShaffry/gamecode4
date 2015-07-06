#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

#define BUFFER_STATIC_SIZE 256

typedef void* (*buffer_Alloc)(void* userData, void* ptr, unsigned int size);

typedef struct _buffer {
	char *buffer;
	char static_buffer[BUFFER_STATIC_SIZE];
	size_t pos;
	size_t buffsize;
	buffer_Alloc alloc_function;
	void* user_data;
} BUFFER;

void buffer_init(BUFFER* buff);
void buffer_initwithalloc(BUFFER* buff, buffer_Alloc alloc, void* userData);

#define buffer_ptr(buff)	((buff)->buffer)
#define buffer_size(buff)	((buff)->buffsize)
#define buffer_pos(buff)	((buff)->pos)
#define buffer_posptr(buff)	((buff)->buffer + (buff)->pos)

#define buffer_reset(buff) ((buff)->pos = 0)


void buffer_openspacehelper(BUFFER *buff, size_t amount);
void buffer_resize(BUFFER* buff, size_t size);

#define buffer_free(buff)	buffer_resize((buff), 0)

#define buffer_openspace(buff, amount) \
  if (((size_t)(amount) + (buff)->pos) > (buff)->buffsize) \
    buffer_openspacehelper((buff), (amount));

#define buffer_addchar(buff, c) { buffer_openspace((buff), 1); (buff)->buffer[(buff)->pos] = (c); (buff)->pos++; }
#define buffer_addstring(buff, str, len) { size_t _len = (len); buffer_openspace((buff), _len); memcpy((buff)->buffer + (buff)->pos, str, _len); (buff)->pos += _len; }
#define buffer_putchar(buff, c) (buff)->buffer[(buff)->pos] = (c)
#define buffer_putstring(buff, str, len) { size_t _len = (len); buffer_openspace((buff), _len); memcpy((buff)->buffer + (buff)->pos, str, _len); }
#define buffer_setpos(buff, newpos) (buff)->pos = newpos;
#define buffer_deltapos(buff, delta) (buff)->pos += delta
#define buffer_isempty(buff) ((buff)->pos == 0)

#define buffer_getchar(buff) ((buff)->pos + 1 < (buff)->buffsize ? (buff)->buffer[(buff)->pos++] : EOF)

#endif /* BUFFER_H */

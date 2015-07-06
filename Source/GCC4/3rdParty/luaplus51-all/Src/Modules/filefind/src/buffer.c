#include "buffer.h"
#include <string.h>
#include <stdlib.h>

void* buffer_DefaultAllocFunction(void* userData, void* ptr, unsigned int size)
{
	(void)userData;

	if (size == 0) {
		free(ptr);
		return NULL;
	} else {
		return realloc(ptr, size);
	}
}


void buffer_init(BUFFER* buff) {
	buffer_initwithalloc( buff, NULL, NULL );
}


void buffer_initwithalloc(BUFFER* buff, buffer_Alloc alloc, void* userData) {
	buff->buffer = (char*)&buff->static_buffer;
	buff->pos = 0;
	buff->buffsize = BUFFER_STATIC_SIZE;
	buff->alloc_function = alloc ? alloc : buffer_DefaultAllocFunction;
	buff->user_data = userData;
}


void buffer_resize(BUFFER* buff, size_t size) {
    if (size == 0) {
		if (buff->buffer != (char*)&buff->static_buffer) {
			buff->alloc_function(buff->user_data, buff->buffer, 0);
			buff->buffer = (char*)&buff->static_buffer;
		}
		return;
    }

    if (size < BUFFER_STATIC_SIZE) {
		buff->buffsize = size;
		buff->pos = buff->pos > size ? size : buff->pos;
		if (buff->buffer != (char*)&buff->static_buffer) {
			buff->alloc_function(buff->user_data, buff->buffer, 0);
            buff->buffer = (char*)&buff->static_buffer;
		}
		return;
    }

	if (buff->buffer == (char*)&buff->static_buffer) {
		buff->buffer = buff->alloc_function(buff->user_data, 0, size);
		memcpy(buff->buffer, &buff->static_buffer, buff->pos);
		buff->buffsize = size;
	} else {
		buff->buffsize = size > buff->buffsize * 2 ? size : buff->buffsize * 2;
		buff->buffer = buff->alloc_function(buff->user_data, buff->buffer, buff->buffsize);
    }
}


void buffer_openspacehelper(BUFFER *buff, size_t amount) {
	if (amount + buff->pos < BUFFER_STATIC_SIZE)
		buffer_resize(buff, BUFFER_STATIC_SIZE);
	else
		buffer_resize(buff, amount + buff->pos);
}



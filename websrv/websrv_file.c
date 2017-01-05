#include "user_interface.h"
#include "string.h"
#include "file.h"
#include "logic.h"

// typedef void (* push_data_chunk)(uint8_t *data, uint16_t buffer_size, uint32_t offset, uint16_t length, bool isFinished);
// typedef void (* request_data_chunk)(uint8_t *data, uint16_t buffer_size, uint32_t offset, uint16_t *length, bool isFinished);

// be nice to implement some cacheing.
uint32_t websrv_file_request_data_size(uint8_t *resource, uint32_t *length)
{
    EspFileDescriptor desc;

    if(file_find(&desc, resource)) 
    {
        return desc.length;
    }
    else
    {
        // 404
    }
    return 0;
}

void websrv_file_request_data_chunk(uint8_t *resource, uint8_t *data, uint16_t buffer_size, uint32_t offset, uint16_t *length)
{
    EspFileDescriptor desc;

    if(file_find(&desc, resource)) 
    {
        uint16_t to_get = min(desc.length - offset, buffer_size);

        file_read(&desc, (void*)data, offset, to_get);
        *length = to_get;
    }
    else
    {
        // 404
    }
}

void websrv_file_request_data_content_type(uint8_t *resource, uint8_t *content_type)
{
    // find content type...
    uint8_t *ext = strrchr(resource, '.');

    if(strcmp(ext, ".htm") == 0 || strcmp(ext, ".html"))
    {
        strcpy(content_type, "text/html");
        return;
    }
    else if(strcmp(ext, ".js") == 0)
    {
        strcpy(content_type, "application/javascript");
        return;
    }
    else if(strcmp(ext, ".css") == 0)
    {
        strcpy(content_type, "text/css");
        return;
    }
    else if(strcmp(ext, ".jpg") == 0)
    {
        strcpy(content_type, "image/jpeg");
        return;
    }
    else if(strcmp(ext, ".gif") == 0)
    {
        strcpy(content_type, "image/gif");
        return;
    }
    else if(strcmp(ext, ".png") == 0)
    {
        strcpy(content_type, "image/png");
        return;
    }
}
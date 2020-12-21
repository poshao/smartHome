#include "sm_http_parse.h"

sm_http_header_t *sm_init_http_header(char *name, char *value)
{
    sm_http_header_t *header;
    header = os_zalloc(sizeof(sm_http_header_t));
    header->name = os_zalloc(os_strlen(name) + 1);
    os_strcpy(header->name, name);
    header->value = os_zalloc(os_strlen(value) + 1);
    os_strcpy(header->value, value);
    return header;
}

void sm_free_http_header(sm_http_header_t *header)
{
    if (!header)
    {
        return;
    }
    if (header->name)
    {
        os_free(header->name);
    }
    if (header->value)
    {
        os_free(header->value);
    }
    os_free(header);
    header = NULL;
}

void sm_http_headers_add(sm_http_headers_t *headers, sm_http_header_t *header)
{
    if (!headers)
        return;
    if (!header)
        return;
    if (!headers->header)
    {
        headers->header = header;
        headers->footer = header;
    }
    else
    {
        headers->footer->next = header;
        headers->footer = header;
    }
}

sm_http_header_t *sm_http_header_get(sm_http_headers_t *headers, char *name)
{
    sm_http_header_t *header;
    header = headers->header;
    while (header)
    {
        if (os_strcmp(header->name, name) == 0)
        {
            return header;
        }
        header = header->next;
    }
    return NULL;
}

void sm_free_http_headers(sm_http_headers_t *headers)
{
    sm_http_header_t *header, *next;
    header = headers->header;
    while (header)
    {
        next = header->next;
        sm_free_http_header(header);
        header = next;
    }
    headers->header = headers->footer = NULL;
    // os_free(headers);
    // headers=NULL;
}

sm_return_t sm_parse_http(sm_http_request_t *r, sm_buf_t *b)
{
    /*
    报文格式: 
    
    METHOD PATH HTTP/1.1
    Content-Type: text/html

    body
    */
    enum
    {
        state_start = 0,
        state_method,
        state_space_after_method,
        state_space_after_url,
        state_proto_start,
        state_header_start,
        state_header_value_start,
        state_body_start

    } state;

    char *p, ch, *m, *o;
    char t[100] = {0};

    state = r->state;

    p = b->pos;
    while (p < b->last + 1)
    {

        ch = *p;
        // os_printf("state: %d\n", (int)state);
        switch (state)
        {
        case state_start:
            /* code */
            m = p;
            state = state_method;
            break;
        case state_method:
            if (ch == ' ')
            {
                if (p - m == 3)
                {
                    if (m[0] == 'G' && m[1] == 'E' && m[2] == 'T')
                    {
                        r->method = SM_HTTP_GET;
                        os_printf("get\n");
                    }
                }
                else if (p - m == 4)
                {
                    if (m[0] == 'P' && m[1] == 'O' && m[2] == 'S' && m[3] == 'T')
                    {
                        r->method = SM_HTTP_POST;
                        os_printf("post\n");
                    }
                }
                else
                {
                    return SM_INVALID_REQUEST_METHOD;
                }
                m = p + 1;
                state = state_space_after_method;
            }
            break;
        case state_space_after_method:
            if (ch == ' ')
            {
                r->url = os_zalloc(p + 1 - m);
                os_memcpy(r->url, m, p - m);
                os_printf("%s %d\n", r->url, p - m);
                state = state_space_after_url;
            }
            break;
        case state_space_after_url:
            if (ch != ' ')
            {
                state = state_proto_start;
            }
            break;
        case state_proto_start:
            if (ch == 10)
            {
                state = state_header_start;
                m = p + 1;
            }
            break;
        case state_header_start:
            if (ch == 10)
            {
                state = state_body_start;
            }
            else if (ch == ':')
            {
                os_memset(t, 0, 100);
                os_memcpy(t, m, p - m);
                os_printf("key: %s : %d\n", t, p - m);
                state = state_header_value_start;
                m = p + 1;
            }
            break;
        case state_header_value_start:
            if (ch == 10)
            {
                os_memset(t, 0, 100);
                os_memcpy(t, m, p - m - 1);
                os_printf("value: %s : %d\n", t, p - m - 1);
                state = state_header_start;
                m = p + 1;
            }
            break;
        case state_body_start:
            r->body = p;
            r->bodylen += b->last - p + 1;
            os_memset(t, 0, 100);
            os_memcpy(t, p, b->last - p + 1);
            os_printf("body: %s :%d\n", t, b->last - p + 1);
            return SM_OK;
        default:
            break;
        }
        p++;
    }
    return SM_OK;
}
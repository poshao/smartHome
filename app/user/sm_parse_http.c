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

void sm_http_header_set_value(sm_http_header_t *header, char *value)
{
    if (!header)
        return;
    os_free(header->value);
    header->value = NULL;
    header->value = os_zalloc(os_strlen(value) + 1);
    os_strcpy(header->value, value);
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

char *sm_http_header_get_value(sm_http_headers_t *headers, char *name)
{
    sm_http_header_t *header;
    header = sm_http_header_get(headers, name);
    if (header)
        return header->value;
    return NULL;
}

void sm_http_header_set(sm_http_headers_t *headers, char *name, char *value)
{
    sm_http_header_t *header;
    header = sm_http_header_get(headers, name);
    if (!header)
    {
        header = sm_init_http_header(name, value);
        sm_http_headers_add(headers, header);
    }
    else
    {
        sm_http_header_set_value(header, value);
    }
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
}

sm_http_request_t *sm_init_http_request()
{
    sm_http_request_t *request;
    request = os_zalloc(sizeof(sm_http_request_t));
    return request;
}

void sm_free_http_request(sm_http_request_t *r)
{
    if (!r)
        return;
    if (r->url)
    {
        os_free(r->url);
    }
    if (r->body)
    {
        os_free(r->body);
    }
    sm_free_http_headers(&r->headers);
    os_free(r);
    r = NULL;
}

sm_buf_t *sm_init_buf(char *pdata, int len)
{
    sm_buf_t *buf;
    buf = os_zalloc(sizeof(sm_buf_t));
    buf->pos = pdata;
    buf->last = pdata + len - 1;
    return buf;
}

void sm_free_buf(sm_buf_t *buf)
{
    os_free(buf);
    buf = NULL;
}

#define SM_HTTP_NAME_LEN 20
#define SM_HTTP_VALUE_LEN 150
sm_return_t sm_parse_http_request(sm_http_request_t *r, sm_buf_t *b)
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

    char *p, ch, *m;
    char t_name[SM_HTTP_NAME_LEN] = {0};
    char t_value[SM_HTTP_VALUE_LEN] = {0};

    sm_http_header_t *header;

    state = r->state;

    p = b->pos;
    while (p < b->last + 1)
    {
        ch = *p;
        switch (state)
        {
        case state_start:
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
                    }
                }
                else if (p - m == 4)
                {
                    if (m[0] == 'P' && m[1] == 'O' && m[2] == 'S' && m[3] == 'T')
                    {
                        r->method = SM_HTTP_POST;
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
                os_memset(t_name, 0, SM_HTTP_NAME_LEN);
                os_memcpy(t_name, m, p - m);
                state = state_header_value_start;
                m = p + 1;
            }
            break;
        case state_header_value_start:
            if (m == p && ch == ' ')
            {
                m = p + 1;
            }
            else if (ch == 10)
            {
                os_memset(t_value, 0, SM_HTTP_VALUE_LEN);
                os_memcpy(t_value, m, p - m - 1);

                header = sm_init_http_header(t_name, t_value);
                sm_http_headers_add(&r->headers, header);

                state = state_header_start;
                m = p + 1;
            }
            break;
        case state_body_start:
            r->bodylen += b->last - p + 1;
            r->body = os_zalloc(r->bodylen + 1);
            os_memcpy(r->body, p, r->bodylen);
            return SM_OK;
        default:
            break;
        }
        p++;
    }
    return SM_OK;
}

void sm_dump_http_request(sm_http_request_t *r)
{
    if (!r)
        return;
    os_printf("=== dump http_request begin ===\n");
    os_printf("url: %s\n", r->url);

    os_printf("method: ");
    switch (r->method)
    {
    case SM_HTTP_GET:
        os_printf("GET\n");
        break;
    case SM_HTTP_POST:
        os_printf("POST\n");
        break;
    default:
        os_printf("INVALID\n");
        break;
    }

    if (r->headers.header)
    {
        os_printf("headers:\n");
        sm_http_header_t *header;
        header = r->headers.header;
        while (header)
        {
            os_printf("> %s:%s\n", header->name, header->value);
            header = header->next;
        }
    }

    if (r->body)
    {
        os_printf("body:%d\n", r->bodylen);
        os_printf("%s\n", r->body);
    }
    os_printf("=== dump http_request end   ===\n\n");
}

void sm_http_content_type_set(sm_http_response_t *r, sm_content_type_t tp)
{
    if (!r)
        return;

    char content_type[50] = {0};
    switch (tp)
    {
    case SM_CONTENT_TYPE_HTML:
        os_strcpy(content_type, "text/html");
        break;
    case SM_CONTENT_TYPE_JSON:
        os_strcpy(content_type, "application/json");
        break;
    case SM_CONTENT_TYPE_JAVASCRIPT:
        os_strcpy(content_type, "text/javascript");
        break;
    case SM_CONTENT_TYPE_CSS:
        os_strcpy(content_type, "text/css");
        break;
    case SM_CONTENT_TYPE_X_FORM:
        os_strcpy(content_type, "application/x-www-form-unlencoded");
        break;
    default:
        os_strcpy(content_type, "text/plain");
        break;
    }
    sm_http_header_set(&r->headers, "Content-Type", content_type);
}

sm_http_response_t *sm_init_http_response()
{
    sm_http_response_t *r;
    r = os_zalloc(sizeof(sm_http_response_t));
    r->code = 200;
    return r;
}

void sm_free_http_response(sm_http_response_t *r)
{
    if (!r)
        return;
    if (r->body)
    {
        os_free(r->body);
    }
    sm_free_http_headers(&r->headers);
    os_free(r);
    r = NULL;
}

char *sm_get_code_description(int code)
{
    switch (code)
    {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 404:
        return "Not Found";
    default:
        // 500
        return "Internal Server Error";
        break;
    }
}

sm_return_t sm_build_http_response(sm_http_response_t *r, sm_buf_t *buf)
{
    /** 报文结构
     *  HTTP/1.1 200 OK
     *  Content-Length:10
     *   
     *  1234567890
     * 
     */

    char line[200] = {0};
    char *p;
    int len = 0;

    sm_http_header_t *header;

    // if (r->bodylen > 0)
    // {
    os_sprintf(line, "%d", r->bodylen);
    sm_http_header_set(&r->headers, "Content-Length", line);
    // }

    p = buf->pos;
    len = os_sprintf(line, "HTTP/1.1 %d %s\r\n", r->code, sm_get_code_description(r->code));
    if (p + len < buf->last)
    {
        os_memcpy(p, line, len);
        p += len;
    }
    else
    {
        return SM_NO_MEM;
    }

    header = r->headers.header;
    while (header)
    {
        len = os_sprintf(line, "%s: %s\r\n", header->name, header->value);
        if (p + len < buf->last)
        {
            os_memcpy(p, line, len);
            p += len;
        }
        else
        {
            return SM_NO_MEM;
        }
        // p+=len;
        // os_printf("%s: %s\r\n",header->name,header->value);
        header = header->next;
    }
    len = os_sprintf(line, "\r\n");
    if (p + len < buf->last)
    {
        os_memcpy(p, line, len);
        p += len;
    }
    else
    {
        return SM_NO_MEM;
    }
    // p += len;

    // os_printf("\r\n");

    // send body
    // len = os_sprintf(line, "%s", r->body);
    // len=r->bodylen;
    if (r->bodylen)
        if (p + r->bodylen < buf->last)
        {
            os_memcpy(p, r->body, r->bodylen);
            p += r->bodylen;
        }
        else
        {
            return SM_NO_MEM;
        }
    buf->last = p;
    return SM_OK;
    // return out;
    // os_printf("%s",r->body);
}
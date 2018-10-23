/* request.h
*/

#ifndef __REQUEST__
#define __REQUEST__ 
#include "tools/utils.h"

struct http_request {
    char * method;
    char * url;
    char * version;
    struct Map * headers; 
    char * body; 
};


void parse_request(
    struct http_request * request,  
    char * http_data);



#endif

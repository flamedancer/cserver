/* request.h
*/
struct http_request {
    char * method;
    char * url;
    char * version;
};


void parse_request(
    struct http_request * request,  
    char * http_data);

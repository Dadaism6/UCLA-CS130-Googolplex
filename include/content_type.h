#ifndef CONTENT_TYPE_H
#define CONTENT_TYPE_H


#include <string>
#include <map>

struct content_type {
    std::map<std::string, std::string> content_type_map = 
    {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".txt", "text/plain"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".zip", "application/zip"}
    };

    std::string content_type_str = "text/plain";
    static content_type get_content_type(std::string extension);

};

#endif // CONTENT_TYPE_H
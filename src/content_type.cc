#include "content_type.h"

content_type content_type::get_content_type(std::string extension)
{
    content_type content_type_;
    if (content_type_.content_type_map.find(extension) != content_type_.content_type_map.end()) {
        content_type_.content_type_str = content_type_.content_type_map.at(extension);
    }
    return content_type_;
}
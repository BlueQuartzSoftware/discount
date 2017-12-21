#include <iostream>

#include "mkdio.h"

int main(int argc, char const *argv[])
{
    std::string text = "Hello **DISCOUNT**!";
    std::cout << "TEXT: " << text << std::endl;
    mkd_flag_t flag = MKD_1_COMPAT;
    MMIOT *doc = mkd_string(text.c_str(), text.size(), flag);
    if (!mkd_compile(doc, flag)) {
        std::cerr << "Markdown Compilation Error!" << std::endl;
        mkd_cleanup(doc);
        return 1;
    }
    char *res = nullptr;
    int size = mkd_document(doc, &res);
    if (size) {
        std::string html(res, res + size);
        std::cout << "HTML: " << html << std::endl;
    }
    mkd_cleanup(doc);
    return 0;
}

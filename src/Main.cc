#include "Solver.h"

#include <cstdio>
#include <sstream>
#include <string>

int from_hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    throw 42;
}

numlin::Grid<int> parse_url(const std::string& url) {
    // TODO: add error check
    int idx = url.find("numlin/") + 7;

    std::istringstream iss(url.substr(idx));
    int height, width;
    char buf;
    iss >> width >> buf >> height >> buf;

    numlin::Grid<int> ret(height, width, -1);
    int pos = 0;
    while (iss >> buf) {
        if (buf >= 'g') {
            pos += buf - 'f';
        } else if (buf == '-') {
            char a, b;
            iss >> a >> b;
            ret.at(pos / width, pos % width) = from_hex(a) * 16 + from_hex(b) - 1;
            pos += 1;
        } else {
            ret.at(pos / width, pos % width) = from_hex(buf) - 1;
            pos += 1;
        }
    }
    return ret;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <problem url>\n", argv[0]);
        return 0;
    }

    numlin::Grid<int> problem = parse_url(argv[1]);
    numlin::Solve(problem);
}

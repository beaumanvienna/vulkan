#pragma once

#include <string>

// Image class for sheet
class Image {
public:
    Image(std::string filename, std::string name, size_t tx, size_t ty, size_t tw, size_t th, size_t r);
    ~Image();

    size_t getTx();
    size_t getTy();
    size_t getTw();
    size_t getTh();
    size_t getR();
    std::string getFilename();
    std::string getName();
private:
    std::string filename;
    std::string name;
    size_t tx;
    size_t ty;
    size_t tw;
    size_t th;
    size_t r;
};

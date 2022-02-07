#include "Image.h"

Image::Image(std::string filename, std::string name, size_t tx, size_t ty, size_t tw, size_t th, size_t r) {
    this->filename = filename;
    this->name = name;
    this->tx = tx;
    this->ty = ty;
    this->tw = tw;
    this->th = th;
    this->r = r;
}

Image::~Image() {
}

std::string Image::getName() {
    return name;
}

size_t Image::getTx() {
    return tx;
}
size_t Image::getTy() {
    return ty;
}
size_t Image::getTw() {
    return tw;
}
size_t Image::getTh() {
    return th;
}

size_t Image::getR() {
    return r;
}

std::string Image::getFilename() {
    return filename;
}

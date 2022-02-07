#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "MaxRectsBinPack.h"
#include "Image.h"

namespace fs = std::filesystem;

const char* toStr(size_t value) {
    std::string str = std::to_string(value);
    return str.c_str();
}

std::vector<std::string> getListFiles(std::string dirName) 
{
    std::vector<std::string> list;
    for (const auto& entry : fs::directory_iterator(dirName))
    {   
        auto p = entry.path();
        list.push_back(p.generic_string());
    }
    
    return list;
}

rbp::MaxRectsBinPack::FreeRectChoiceHeuristic chooseBestHeuristic(std::vector<sf::Texture*> *rects, size_t texWidth, size_t texHeight) {
    rbp::MaxRectsBinPack pack;
    std::vector<rbp::MaxRectsBinPack::FreeRectChoiceHeuristic> listHeuristics;
    listHeuristics.push_back(rbp::MaxRectsBinPack::RectBestAreaFit);
    listHeuristics.push_back(rbp::MaxRectsBinPack::RectBestLongSideFit);
    listHeuristics.push_back(rbp::MaxRectsBinPack::RectBestShortSideFit);
    listHeuristics.push_back(rbp::MaxRectsBinPack::RectBottomLeftRule);
    listHeuristics.push_back(rbp::MaxRectsBinPack::RectContactPointRule);

    rbp::MaxRectsBinPack::FreeRectChoiceHeuristic res;
    float max = 0;

    for (auto& heu : listHeuristics) {
        pack.Init(texWidth, texHeight);

        for (size_t j = 0; j < rects->size(); j++) {
            pack.Insert(rects->at(j)->getSize().x, rects->at(j)->getSize().y, heu);
        }

        if (pack.Occupancy() > max) {
            max = pack.Occupancy();
            res = heu;
        }
    }
    return res;
}


int main(int argc, char** argv) {
    std::string folderName;
    if (argc != 2) 
    {
        folderName = "../../resources/atlas/images/";
        std::cout << "using " << folderName << " as image folder" << std::endl;
    }
    else
    {
        folderName = argv[1];
    }
    
    const int SPRITESHEET_WIDTH  = 4096;
    const int SPRITESHEET_HEIGHT = 4096;
    
    std::vector<sf::Texture*> imgTex; // images textures
    std::vector<std::string> imgTexID; // name of the images
    std::vector<Image> images; // xml data of the images
    std::string filename = "../../resources/atlas/atlas.png"; // filename of sprite sheet
    std::string filenameH = "../../resources/atlas/atlas.h"; // filename of header file
    std::string filenameCPP = "../../resources/atlas/atlas.cpp"; // filename of cpp file
    sf::Vector2i size(SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT); // size of the sprite sheet

    sf::RenderTexture rend; // texture to render the sprite sheet
    rend.create(size.x, size.y);

    rbp::MaxRectsBinPack pack(size.x, size.y); //pack of image

    // list all filenames in the folder images
    std::vector<std::string> listAll = getListFiles(folderName);
    
    if (!listAll.size())
    {
        std::cout << "No files found in " << folderName << std::endl;
        return -1;
    }
    for (auto name : listAll)
    {
        std::cout << "file: " << name << std::endl;
    }

    // load all the images
    for (auto& img : listAll) {
        sf::Texture *texP = new sf::Texture();
        texP->loadFromFile(img);
        imgTex.push_back(texP);
        img = img.substr(img.find_last_of("/")+1);
        img = img.substr(0,img.find_last_of("."));
        imgTexID.push_back(img);
    }

    float rotation = 0;

    // choose the best heuristic
    const rbp::MaxRectsBinPack::FreeRectChoiceHeuristic best1 = chooseBestHeuristic(&imgTex, size.x, size.y);

    for (size_t i = 0; i < imgTex.size(); i++) {
        // insert the image into the pack
        rbp::Rect packedRect = pack.Insert(imgTex[i]->getSize().x, imgTex[i]->getSize().y, best1);

        if (packedRect.height <= 0) {
            std::cout << "Error: The pack is full\n";
        }

        sf::Sprite spr(*imgTex[i]); // sprite to draw on the rendertexture

        // if the image is rotated
        if (imgTex[i]->getSize().x == packedRect.height && packedRect.width != packedRect.height) {
            rotation = 90; // set the rotation for the xml data

            // rotate the sprite to draw
            size_t oldHeight = spr.getTextureRect().height;
            spr.setPosition((float) packedRect.x, (float) packedRect.y);
            spr.rotate(rotation);
            spr.setPosition(spr.getPosition().x + oldHeight, spr.getPosition().y);
        }
        else { // if there is no rotation
            rotation = 0;
            spr.setPosition((float) packedRect.x, (float) packedRect.y);
        }

        rend.draw(spr); // draw the sprite on the sprite sheet
        // save data of the image for the xml file
        images.push_back(Image(filename, imgTexID[i], packedRect.x, packedRect.y, packedRect.width, packedRect.height, (size_t)rotation));
    }
    
    std::ofstream atlas_h;
    atlas_h.open (filenameH);
    atlas_h << "#pragma once" << std::endl;
    atlas_h << "" << std::endl;
    atlas_h << "// IMAGES" << std::endl;
    
    int n = 0;
    for (auto image : images)
    {
        std::string name = image.getName();
        name = name.substr(0, name.find_last_of("."));
        atlas_h << "#define " << name << " " << n << std::endl;
        
        n++;
    }
    atlas_h.close();
    
    std::ofstream atlas_cpp;
    atlas_cpp.open (filenameCPP);
    atlas_cpp << "const AtlasImage images[" << images.size() << "] = {" << std::endl;
    atlas_cpp << "" << std::endl;
    
    int width, height;
    n = 0;
    for (auto image : images)
    {
        std::string name = image.getName();
        name = name.substr(0, name.find_last_of("."));

        width  = image.getTw();
        height = image.getTh();

        float u1, v1, u2, v2;
        u1 = static_cast<float>(image.getTx()) / SPRITESHEET_WIDTH;
        v1 = static_cast<float>(image.getTy()) / SPRITESHEET_HEIGHT;
        u2 = u1 + static_cast<float>(image.getTw()) / SPRITESHEET_WIDTH;
        v2 = v1 + static_cast<float>(image.getTh()) / SPRITESHEET_HEIGHT;
        
        v1 = 1.0f - v1;
        v2 = 1.0f - v2;
        
        int rotation = image.getR();
        atlas_cpp << "    {" << u1 << ", " << v1 << ", " << u2 << ", " << v2 << ", " << width << ", " << height << ", " << rotation  << ", \"" << name << "\" }," << std::endl;
        
        n++;
    }

    atlas_cpp << "    };" << std::endl;
    atlas_cpp << "const Atlas atlas = {" << std::endl;
    atlas_cpp << "  images, " << images.size() << "," << std::endl;
    atlas_cpp << "};" << std::endl;
    atlas_cpp.close();

    rend.display(); // render the texture properly

    // free the memory of the images
    for (auto& tex : imgTex) {
        delete(tex);
    }

    // save the sprite sheet
    sf::Texture tex = rend.getTexture();
    sf::Image img = tex.copyToImage(); // need to create an image to save a file
    img.saveToFile(filename);


    // see the occupancy of the packing
    std::cout << "pack1 : " << pack.Occupancy() << "%\n";
    
    // SFML code the create a window and diplay the sprite sheet
    sf::RenderWindow window(sf::VideoMode(size.x, size.y), "Sprite sheets generator");
    sf::Sprite spr(tex);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);
        window.draw(spr);
        window.display();
        sf::sleep(sf::milliseconds(10));
    }
}

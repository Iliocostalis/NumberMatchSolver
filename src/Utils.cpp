#include <Utils.h>
#include <stb_image_write.h>

void saveImage(const std::string& name, const Image& image)
{
    std::string path = "../images/out/" + name;
    stbi_write_png(path.c_str(), image.width, image.height, 1, image.pixels, 0);
}
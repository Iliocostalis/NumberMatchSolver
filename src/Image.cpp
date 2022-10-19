#include <Image.h>
#include <Utils.h>

#if DEFINED(LINUX)
#include <cmath>
#include <Debugging.h>

Image::Image(int width, int height, uint8_t* pixels) : width(width), height(height), pixels(pixels)
{
    createdBuffer = false;
}

Image::Image(int width, int height) : width(width), height(height)
{
    pixels = new uint8_t[width*height];
    createdBuffer = true;
}

Image::Image()
{
    createdBuffer = false;
}

Image::Image(const Image& other)
{
    width = other.width;
    height = other.height;
    pixels = new uint8_t[width*height];
    std::copy(other.pixels, other.pixels + width*height, pixels);
    createdBuffer = true;
}

Image::~Image()
{
    if(createdBuffer)
        delete[] pixels;
}

uint8_t Image::getValue(int x, int y) const
{
    ASSERT(x >= 0 && x < width && y >= 0 && y < height);
    return pixels[y * width + x];
}

void Image::setValue(int x, int y, uint8_t value)
{
    ASSERT(x >= 0 && x < width && y >= 0 && y < height);
    pixels[y * width + x] = value;
}


void Image::scale(Image* out)
{
    Image scaledHalf(width, out->height);

    int scale = width / out->width;

    for(int y = 0; y < out->height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            uint8_t val = 255;

            for(int v = 0; v < scale; ++v)
            {
                val = std::min(val, getValue(x,y*scale+v));

                //uint8_t nVal = getValue(x,y*scale+v);
                //if(nVal + v*3 < val)
                //    val = nVal;
            }

            scaledHalf.setValue(x,y,val);
        }
    }
    
    for(int y = 0; y < out->height; ++y)
    {
        for(int x = 0; x < out->width; ++x)
        {
            uint8_t val = 255;

            for(int v = 0; v < scale; ++v)
            {
                val = std::min(val, scaledHalf.getValue(x*scale+v,y));

                //uint8_t nVal = scaledHalf.getValue(x*scale+v,y);
                //if(nVal + v*3 < val)
                //    val = nVal;
            }

            out->setValue(x,y,val);
        }
    }
}

void Image::scaleNearest(Image* out)
{
    //Image scaledHalf(width, out->height);

    float scaleX = (float)width / (float)out->width;
    float scaleY = (float)height / (float)out->height;

    for(int y = 0; y < out->height; ++y)
    {
        for(int x = 0; x < out->width; ++x)
        {
            int val = (int)getValue((int)(x * scaleX + 0.5f), (int)(y * scaleY + 0.5f));
            //val = std::max(0, std::min(255, (val - 100) * 4));
            out->setValue(x, y, (uint8_t)val);
        }
    }
}

void Image::lineFilter(Image* out)
{
    Image tmp(width, height);
    Position<int> tmpPos;
    for(int y = 3; y < height-3; ++y)
    {
        tmpPos.y = y;
        for(int x = 3; x < width-3; ++x)
        {
            tmpPos.x = x;
            int val1 = getPixelOnLineScoreSmall(*this, tmpPos, Direction::UP);
            int val2 = getPixelOnLineScoreSmall(*this, tmpPos, Direction::RIGHT);
            int val = std::max(val1, val2);

            val = std::min(255, std::max(val, 0));
            val = 255-val;

            tmp.setValue(x,y,(uint8_t)val);
        }
    }

    // stage 2

    std::array<uint8_t, 5> buf;
    for(int y = 3; y < height-3; ++y)
    {
        for(int x = 3; x < width-3; ++x)
        {
            buf[0] = tmp.pixels[(y-2) * width + x];
            buf[1] =  tmp.pixels[(y-1) * width + x];
            buf[2] =  tmp.pixels[(y) * width + x];
            buf[3] =  tmp.pixels[(y+1) * width + x];
            buf[4] =  tmp.pixels[(y+2) * width + x];
            int medianUp = median5(buf, 3);
            //int averageUp = (buf[0] + buf[1] + buf[2] + buf[3] + buf[4]) / 5;

            buf[0] = tmp.pixels[(y) * width + x-2];
            buf[1] =  tmp.pixels[(y) * width + x-1];
            buf[2] =  tmp.pixels[(y) * width + x];
            buf[3] =  tmp.pixels[(y) * width + x+1];
            buf[4] =  tmp.pixels[(y) * width + x+2];
            int medianLeft = median5(buf, 3);
            //int averageLeft = (buf[0] + buf[1] + buf[2] + buf[3] + buf[4]) / 5;


            //out.setValue(x,y,tmp.getValue(x,y));

            out->setValue(x,y, (uint8_t)std::min(medianLeft, medianUp));
            //out->setValue(x,y, std::min(averageLeft, averageUp));
        }
    }    
}


////////////////////////////////////////ImageRGB/////////////////////////////////////////

ImageRGB::ImageRGB(int width, int height, uint8_t* pixels) : Image(width, height, pixels)
{
    createdBuffer = false;
}

ImageRGB::ImageRGB(int width, int height)
{
    this->width = width;
    this->height = height;
    pixels = new uint8_t[width*height*3];
    createdBuffer = true;
}

ImageRGB::~ImageRGB()
{
    if(createdBuffer)
        delete[] pixels;
}

uint8_t ImageRGB::getValue(int x, int y, int rgb) const
{
    ASSERT(x >= 0 && x < width && y >= 0 && y < height);
    return pixels[(y * width + x)*3 + rgb];
}

void ImageRGB::setValue(int x, int y, int rgb, uint8_t value)
{
    ASSERT(x >= 0 && x < width && y >= 0 && y < height);
    pixels[(y * width + x)*3 + rgb] = value;
}

void ImageRGB::scaleAndBW(Image* imageOut)
{
    float scaleX = (float)width / (float)imageOut->width;
    float scaleY = (float)height / (float)imageOut->height;

    for(int y = 0; y < imageOut->height; ++y)
    {
        for(int x = 0; x < imageOut->width; ++x)
        {
            int val = (int)getValue((int)(x * scaleX + 0.5f), (int)(y * scaleY + 0.5f), 1);
            //val = std::max(0, std::min(255, (val - 100) * 4));
            //val = (int)(std::pow((float)val / 255.0f, 2.2f) * 255.0f);
            imageOut->setValue(x, y, (uint8_t)val);
        }
    }
}

void ImageRGB::toBlackWhite(Image* imageOut)
{
    float dev = 1.0f/255.0f;
    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            //int bw = ((int)getValue(x, y, 0) + (int)getValue(x, y, 1) + (int)getValue(x, y, 2))/3;
            //float val = 0.2126f * std::pow((float)getValue(x, y, 0)*dev, 2.2f) + 0.7152f * std::pow((float)getValue(x, y, 1)*dev, 2.2f) + 0.0722f * std::pow((float)getValue(x, y, 2)*dev, 2.2f);
            //float val = 0.2126f * (float)getValue(x, y, 0)*dev + 0.7152f * (float)getValue(x, y, 1)*dev + 0.0722f * (float)getValue(x, y, 2)*dev;
            
            int bw = (int)getValue(x, y, 1);
            imageOut->setValue(x, y, (uint8_t)bw);
            //imageOut->setValue(x, y, (uint8_t) (val*255.0f));
        }
    }
}
#endif
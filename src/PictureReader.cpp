#include <PictureReader.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <Utils.h>
#include <AverageValues.h>
#include <BlockToColorThreshold.h>
#include <cmath>


void PictureReader::readImage(ImageRGB& imageRGB)
{
    Image image(imageRGB.width, imageRGB.height);
    imageRGB.toBlackWhite(&image);
    //saveImage("bw.png", image);

    // resize to max 1000 width
    int newWidth = image.width;
    int scale = 1;
    while(newWidth > 1000) newWidth /= 2;

    //newWidth = 900;
    int newHeight = image.height / (image.width / newWidth);

//#define FILTER_FULL_SIZE

    Image imageScaled(newWidth, newHeight);

#ifdef FILTER_FULL_SIZE
    Image imageFiltered(image.width, image.height);
#else
    Image imageFiltered(newWidth, newHeight);
#endif

#ifdef FILTER_FULL_SIZE
    image.lineFilter(&imageFiltered);
    imageFiltered.scale(&imageScaled);
#else
    image.scale(&imageScaled);
    imageScaled.lineFilter(&imageFiltered);
#endif

    saveImage("imageFiltered.png", imageFiltered);
    saveImage("imageScaled.png", imageScaled);

    PlayField playFiled = findPlayField(imageFiltered);
    findNumbers(imageScaled, playFiled);

    saveImage("fin.png", imageFiltered);
}




std::array<Position, 3> findPixelsOnLine(Image& image, Position pos, Direction direction, int maxMove)
{
    int moveX = 0;
    int moveY = 0;
    Direction scoreDirection;

    switch (direction)
    {
        case Direction::UP:
            scoreDirection = Direction::RIGHT;
            moveY = 1;
            break;

        case Direction::DOWN:
            scoreDirection = Direction::LEFT;
            moveY = -1;
            break;

        case Direction::RIGHT:
            scoreDirection = Direction::UP;
            moveX = 1;
            break;

        case Direction::LEFT:
            scoreDirection = Direction::DOWN;
            moveX = -1;
            break;
    }

    std::array<Position, 3> positions{pos, pos, pos};
    std::array<int, 3> scores{0, 0, 0};
    int lastAddedIndex = 0;
    int lastAddedIndexScores = 0;
    int minSpace = 5;

    for(int i = 0; i < maxMove; ++i)
    {
        int score = getPixelOnLineScoreSmall(image, pos, scoreDirection);

        if(i - lastAddedIndex < minSpace)
        {
            if(score > scores[lastAddedIndexScores])
            {
                scores[lastAddedIndexScores] = score;
                positions[lastAddedIndexScores] = pos;
                lastAddedIndex = i;
            }
        }
        else
        {
            for(int a = 0; a < 3; ++a)
            {
                if(score > scores[a])
                {
                    scores[a] = score;
                    positions[a] = pos;
                    lastAddedIndex = i;
                    lastAddedIndexScores = a;
                    break;
                }
            }
        }

        pos.x += moveX;
        pos.y += moveY;
    }

    return positions;
}

Position findPixelOnLine(Image& image, Position pos, Direction direction)
{
    int maskUD[] = { 1, -2,  1,
                     1, -2,  1,
                     1, -2,  1};

    int maskLR[] = {  1,  1,  1,
                     -2, -2, -2,
                      1,  1,  1};

    int* mask;
    if(direction == Direction::UP || direction == Direction::DOWN)
        mask = maskUD;
    else
        mask = maskLR;

    int moveX = 0;
    int moveY = 0;
    if(direction == Direction::UP)
        moveX = -1;
    else if(direction == Direction::DOWN)
        moveX = 1;
    else if(direction == Direction::LEFT)
        moveY = -1;
    else
        moveY = 1;

    Position bestLine(pos);
    int bestLineCount = 0;

    int bestVal = 0;
    int steps = 0;
    int maxSteps = image.width/6;
    while(steps < maxSteps)
    {
        int val = 0;
        int valB = 0;
        for (int x = 0; x < 3; x++)
        {
            for (int y = 0; y < 3; y++)
            {
                val += image.getValue(pos.x + x - 1, pos.y + y - 1) * mask[y * 3 + x];
                valB += image.getValue(pos.x + x - 1, pos.y + y - 1) * maskLR[y * 3 + x];
            }
        }

        std::cout << (int)image.getValue(pos.x, pos.y) << std::endl;
        std::cout << val << std::endl;
        std::cout << valB << std::endl;
        std::cout << std::endl;
        if(val > bestVal && val > 0)
        {
            bestVal = val;
            bestLine = pos;

            //image.setValue(pos.x, pos.y, 255);
            //image.setValue(pos.x+1, pos.y, 255);
            //image.setValue(pos.x+1, pos.y+1, 255);
            //image.setValue(pos.x, pos.y+1, 255);
        }

        ++steps;
        pos.x += moveX;
        pos.y += moveY;
    }

    //image.setValue(bestLine.x, bestLine.y, 255);
    //image.setValue(bestLine.x+1, bestLine.y, 255);
    //image.setValue(bestLine.x+1, bestLine.y+1, 255);
    //image.setValue(bestLine.x, bestLine.y+1, 255);

    return bestLine;
}


std::vector<Position> followLine(Image& image, Direction direction, int stepSize, Position pos)
{
    int moveX = 0;
    int moveY = 0;

    AverageValues average(4);

    switch (direction)
    {
        case Direction::UP:
            moveY = -stepSize;
            break;
        case Direction::DOWN:
            moveY = stepSize;
            break;
        case Direction::LEFT:
            moveX = -stepSize;
            break;
        case Direction::RIGHT:
            moveX = stepSize;
            break;
    }
        
    
    int lr = 1;

    bool oneExtraChance = true;
    std::vector<Position> line;
    Position tmpLine;

    BlockToColorThreshold blockToColorThreshold(2, 5, direction);

    do
    {
        pos.x += moveX;
        pos.y += moveY;

        int lineOffsetX = 0;
        int lineOffsetY = 0;
        int hitCount = 0;
        
        average.addValue(blockToColorThreshold.getColorThreshold(image, pos));
        int colorThreshold = average.getValue();

        if(moveX != 0)
            for(int i = -lr; i <= lr; ++i)
            {
                int val = image.getValue(pos.x, pos.y + i);
                if(val <= colorThreshold)
                {
                    ++hitCount;
                    lineOffsetY += i;
                }
            }
        else
            for(int i = -lr; i <= lr; ++i)
            {
                int val = image.getValue(pos.x + i, pos.y);
                if(val <= colorThreshold)
                {
                    ++hitCount;
                    lineOffsetX += i;
                }
            }

        if(hitCount)
        {
            if(!oneExtraChance)
            {
                oneExtraChance = true;
                line.push_back(tmpLine);
            }
            line.push_back(pos);
        }
        else
        {
            if(oneExtraChance)
            {
                tmpLine = pos;
                oneExtraChance = false;
            }
            else
                break;
        }
        
        hitCount = std::max(hitCount, 1);
        
        if(lineOffsetX < 0)
            pos.x -= (-lineOffsetX)/hitCount;
        else
            pos.x += lineOffsetX/hitCount;

        if(lineOffsetY < 0)
            pos.y -= (-lineOffsetY)/hitCount;
        else
            pos.y += lineOffsetY/hitCount;

        //std::cout << "color: " << (int)pixels[pos.y * width + pos.x] << std::endl;
    }
    while(true);

    //std::cout << "end pos: x:" << pos.x << "y:" << pos.y << std::endl;
    average.reset();
    return line;
}

std::vector<Position> findLine(Image& image, Direction direction, int stepSize, Position pos)
{
    pos = findPixelOnLine(image, pos, direction);
    return followLine(image, direction, stepSize, pos);
}

float findSquareSize(Image& image, Position start, Position end, Direction direction)
{
    float x = start.x;
    float y = start.y;

    float moveX = end.x - start.x;
    float moveY = end.y - start.y;

    float max = std::max(std::abs(moveX), std::abs(moveY));

    moveX /= max;
    moveY /= max;

    //move start back
    x -= moveX * image.width / 6;
    y -= moveY * image.width / 6;

    std::vector<int> colors;

    Position tmpLine;

    Position pos;
    std::vector<int> values;
    values.reserve(image.width);

    do
    {
        x += moveX;
        y += moveY;
        pos.x = (int)x;
        pos.y = (int)y;

        int val = getPixelOnLineScore(image, pos, direction);
        values.push_back(val);
    } while (pos.x > 10 && pos.x < image.width - 10 && pos.y > 10 && pos.y < image.height - 10);

    blurVector(values);
    blurVector(values);
    auto highPoints = getHighPoints(values);

    std::vector<int> highPointValue;
    highPointValue.reserve(highPoints.size());

    for(auto& v : highPoints)
        highPointValue.push_back(values[v]);

    float minVal = 0;
    float maxVal = 0;
    float scale = 0;
    /*auto highPointBlured = getFrequenzyOfValues(highPointValue, 24, &minVal, &maxVal, &scale);
    blurVector(highPointBlured);
    blurVector(highPointBlured);
    //auto highPointBlured = blurFreqenzyOfValues(highPointValue, 16, &minVal, &maxVal, &scale);

    bool foundMax = false;
    float last = highPointBlured[0];
    int cutIndex = 0;
    for(int i = 1; i < highPointBlured.size()-1; ++i)
    {
        if(!foundMax)
        {
            if(highPointBlured[i] > highPointBlured[i-1] && highPointBlured[i] >= highPointBlured[i+1])
                foundMax = true;
        }
        else
        {
            if(highPointBlured[i+1] >= highPointBlured[i]) // starts increasing
            {
                cutIndex = i;
                break;
            }
        }
    }

    int cutHeight = frequenzyIndexToValue(scale, minVal, cutIndex);*/
    int cutHeight = 800;

    std::vector<int> highPointIndexCapped;
    highPointIndexCapped.reserve(highPoints.size());

    for(int i = 0; i < highPoints.size(); ++i)
        if(values[highPoints[i]] > cutHeight)
            highPointIndexCapped.push_back(highPoints[i]);


    std::vector<int> highPointSpace;
    highPointSpace.resize(highPointIndexCapped.size() - 1);

    for(int i = 0; i < highPointIndexCapped.size() - 1; ++i)
        highPointSpace[i] = highPointIndexCapped[i+1] - highPointIndexCapped[i];


    float mostFreqentSpace = findMostFreqentValue(highPointSpace);

    /*int lastBorderIndex = 0;
    for(auto& v : highPointSpace)
    {
        // 20% tolerance
        if(std::abs(v - mostFreqentSpace) > (float)mostFreqentSpace * 0.2f)
            break;

        ++lastBorderIndex;
    }
    
    Position borderPos;
    borderPos.x = start.x + highPointIndexCapped[lastBorderIndex] * moveX;
    borderPos.y = start.y + highPointIndexCapped[lastBorderIndex] * moveY;
    */
    
    return mostFreqentSpace;
}

Position centerOnLine(Image& image, const Position& pos, Direction direction)
{
    int bestScoreUp = 0;
    int moveX = 0;
    int moveY = 0;

    if(direction == Direction::UP || direction == Direction::DOWN)
        moveY = 1;
    else
        moveX = 1;

    Position bestPos(pos);
    for(int i = -2; i <= 2; ++i)
    {
        Position p;
        p.x = pos.x + i * moveX;
        p.y = pos.y + i * moveY;
        int score = getPixelOnLineScore(image, p, direction);

        if(score > bestScoreUp)
        {
            bestScoreUp = score;
            bestPos = p;
        }
    }

    return bestPos;
}

PlayField PictureReader::findPlayField(Image& image)
{
    /*
    int stepSize = 2;
    std::vector<Position> up;
    std::vector<Position> down;
    std::vector<Position> left;
    std::vector<Position> right;
    int trys = 0;
    Position pos;
    pos.x = image.width/2;
    pos.y = image.height/2;
    do
    {
        up = findLine(image, Direction::UP, stepSize, pos);
        down = findLine(image, Direction::DOWN, stepSize, pos);
        left = findLine(image, Direction::LEFT, stepSize, pos);
        right = findLine(image, Direction::RIGHT, stepSize, pos);
        ++trys;

        pos.x += image.width/16;
        pos.y += image.height/8;
        if(trys > 3)
            return;
    }
    while(up.size() < 8 || left.size() < 8 || left.size() < 8 || left.size() < 8);

    
    float squareUp = findSquareSize(image, up[3], up[up.size()-4], Direction::UP);
    float squareRight = findSquareSize(image, right[3], right[right.size()-4], Direction::RIGHT);
    float squareLeft = findSquareSize(image, left[3], left[left.size()-4], Direction::LEFT);
    float squareDown = findSquareSize(image, down[3], down[down.size()-4], Direction::DOWN);

    float squareHeight = (squareUp + squareDown) * 0.5f; 
    float squareWidth = (squareLeft + squareRight) * 0.5f;

    bool error = false;

    if(std::abs(squareDown - squareUp) > 2.0f)
        error = true;

    if(std::abs(squareLeft - squareRight) > 2.0f)
        error = true;
        */


















    int stepSize = 3;
    PlayField playField;

    Position pos;
    pos.x = image.width/2 - image.width/6;
    pos.y = image.height/2 + image.height/6;
    int maxSteps = image.width/3;
    std::array<Position, 3> upPoints = findPixelsOnLine(image, pos, Direction::RIGHT, maxSteps);

    std::vector<Position> upLine1 = followLine(image, Direction::UP, stepSize, upPoints[0]);
    std::vector<Position> upLine2 = followLine(image, Direction::UP, stepSize, upPoints[1]);
    std::vector<Position> upLine3 = followLine(image, Direction::UP, stepSize, upPoints[2]);

    std::vector<Position>& upLineLongest = upLine1.size() > upLine2.size() ? (upLine1.size() > upLine3.size() ? upLine1 : upLine3) : (upLine2.size() > upLine3.size() ? upLine2 : upLine3);


    if(upLineLongest.size() < 10)
        return playField;

    Position lineEndPoint = centerOnLine(image, upLineLongest.back(), Direction::LEFT);

    std::vector<Position> lineLeft = followLine(image, Direction::LEFT, stepSize, lineEndPoint);
    std::vector<Position> lineRight = followLine(image, Direction::RIGHT, stepSize, lineEndPoint);

    Position lineTopLeft = centerOnLine(image, lineLeft.back(), Direction::LEFT);
    Position lineTopRight = centerOnLine(image, lineRight.back(), Direction::RIGHT);

    std::vector<Position> lineLeftDown = followLine(image, Direction::DOWN, stepSize, lineTopLeft);
    std::vector<Position> lineRightDown = followLine(image, Direction::DOWN, stepSize, lineTopRight);

    int playfieldWidth = lineRightDown[3].x - lineLeftDown[3].x;
    int squareWidth = playfieldWidth / 9;


    int centerIndex = (squareWidth / stepSize) / 2;

    playField.squareWidth = squareWidth;
    playField.squareHeight = squareWidth;

    playField.topLeft.x = lineLeftDown[centerIndex].x;
    playField.topLeft.y = lineLeft[lineLeft.size()-1-centerIndex].y;

    playField.topRight.x = lineRightDown[centerIndex].x;
    playField.topRight.y = lineRight[lineRight.size()-1-centerIndex].y;

    showPoint(image, playField.topLeft);
    showPoint(image, playField.topRight);

    for(auto&p : upLine1)
        image.setValue(p.x, p.y, 255);
    for(auto&p : upLine2)
        image.setValue(p.x, p.y, 255);
    for(auto&p : upLine3)
        image.setValue(p.x, p.y, 255);

    for(auto&p : lineLeft)
        image.setValue(p.x, p.y, 255);
    for(auto&p : lineRight)
        image.setValue(p.x, p.y, 255);
    for(auto&p : lineLeftDown)
        image.setValue(p.x, p.y, 255);
    for(auto&p : lineRightDown)
        image.setValue(p.x, p.y, 255);


    return playField;
}

void PictureReader::findNumbers(const Image& image, const PlayField& playField)
{

}
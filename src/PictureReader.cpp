#include <PictureReader.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <Utils.h>
#include <AverageValues.h>
#include <BlockToColorThreshold.h>
#include <cmath>
#include <NumberGame.h>

#define COLOR_BLACK 100

void PictureReader::readImage(ImageRGB& imageRGB)
{
    //Image image(imageRGB.width, imageRGB.height);
    //imageRGB.toBlackWhite(&image);

    int newWidth = 600;
    int newHeight = imageRGB.height / (imageRGB.width / newWidth);

    //saveImage("bw.png", image);

    // resize to max 1000 width
    //int newWidth = image.width;
    //int scale = 1;
    //while(newWidth > 1000) newWidth /= 2;
    //int newHeight = image.height / (image.width / newWidth);

    Image imageScaled(newWidth, newHeight);
    Image imageFiltered(newWidth, newHeight);

    imageRGB.scaleAndBW(&imageScaled);
    //image.scale(&imageScaled);
    imageScaled.lineFilter(&imageFiltered);

    saveImage("imageFiltered.png", imageFiltered);
    saveImage("imageScaled.png", imageScaled);

    PlayField playFiled = findPlayField(imageFiltered);
    std::vector<int> numbers;
    NeuralNetwork nn;
    findNumbers(imageScaled, imageFiltered, playFiled, &numbers, nn);

    saveImage("fin.png", imageFiltered);
}

void PictureReader::getNumbers(ImageRGB& imageRGB, std::vector<int>* numbers, const NeuralNetwork& nn)
{
    Image image(imageRGB.width, imageRGB.height);
    imageRGB.toBlackWhite(&image);

    //int newWidth = 600;
    //int newHeight = imageRGB.height / (imageRGB.width / newWidth);

    // resize to max 1000 width
    int newWidth = image.width;
    int scale = 1;
    while(newWidth > 1000) newWidth /= 2;
    int newHeight = image.height / (image.width / newWidth);

    Image imageScaled(newWidth, newHeight);
    Image imageFiltered(newWidth, newHeight);
    saveImage("imageScaled.png", imageScaled);

    //imageRGB.scaleAndBW(&imageScaled);
    image.scale(&imageScaled);
    imageScaled.lineFilter(&imageFiltered);

    PlayField playFiled = findPlayField(imageFiltered);
    findNumbers(imageScaled, imageFiltered, playFiled, numbers, nn);
}

void PictureReader::getNumberImagesForNN(ImageRGB& imageRGB, std::list<Image>* out, const std::vector<int>& numbers)
{
    Image image(imageRGB.width, imageRGB.height);
    imageRGB.toBlackWhite(&image);

    //int newWidth = 600;
    //int newHeight = imageRGB.height / (imageRGB.width / newWidth);

    // resize to max 1000 width
    int newWidth = image.width;
    int scale = 1;
    while(newWidth > 1000) newWidth /= 2;
    int newHeight = image.height / (image.width / newWidth);

    Image imageScaled(newWidth, newHeight);
    Image imageFiltered(newWidth, newHeight);

    //imageRGB.scaleAndBW(&imageScaled);
    image.scale(&imageScaled);
    imageScaled.lineFilter(&imageFiltered);

    //saveImage("imageFiltered.png", imageFiltered);
    //saveImage("imageScaled.png", imageScaled);

    PlayField playFiled = findPlayField(imageFiltered);
    getNumberImages(imageScaled, imageFiltered, playFiled, out, numbers);

    //saveImage("fin.png", imageFiltered);
}

std::array<Position<int>, 3> findPixelsOnLine(Image& image, Position<int> pos, Direction direction, int maxMove)
{
    Position<int> move(0,0);
    Direction scoreDirection;

    switch (direction)
    {
        case Direction::UP:
            scoreDirection = Direction::RIGHT;
            move.y = 1;
            break;

        case Direction::DOWN:
            scoreDirection = Direction::LEFT;
            move.y = -1;
            break;

        case Direction::RIGHT:
            scoreDirection = Direction::UP;
            move.x = 1;
            break;

        case Direction::LEFT:
            scoreDirection = Direction::DOWN;
            move.x = -1;
            break;
    }

    std::array<Position<int>, 3> positions{pos, pos, pos};
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

        pos = pos + move;
    }

    return positions;
}

std::vector<Position<int>> followLine(Image& image, Direction direction, int stepSize, Position<int> pos)
{
    Position<int> move(0,0);

    AverageValues average(4);

    switch (direction)
    {
        case Direction::UP:
            move.y = -stepSize;
            break;
        case Direction::DOWN:
            move.y = stepSize;
            break;
        case Direction::LEFT:
            move.x = -stepSize;
            break;
        case Direction::RIGHT:
            move.x = stepSize;
            break;
    }
        
    
    int lr = 1;

    bool oneExtraChance = true;
    std::vector<Position<int>> line;
    Position<int> tmpLine;

    BlockToColorThreshold blockToColorThreshold(2, 5, direction);

    do
    {
        pos = pos + move;

        Position<int> lineOffset(0,0);
        int hitCount = 0;
        
        average.addValue(blockToColorThreshold.getColorThreshold(image, pos));
        int colorThreshold = average.getValue();

        if(move.x != 0)
            for(int i = -lr; i <= lr; ++i)
            {
                int val = image.getValue(pos.x, pos.y + i);
                if(val <= colorThreshold)
                {
                    ++hitCount;
                    lineOffset.y += i;
                }
            }
        else
            for(int i = -lr; i <= lr; ++i)
            {
                int val = image.getValue(pos.x + i, pos.y);
                if(val <= colorThreshold)
                {
                    ++hitCount;
                    lineOffset.x += i;
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
        
        if(lineOffset.x < 0)
            pos.x -= (-lineOffset.x)/hitCount;
        else
            pos.x += lineOffset.x/hitCount;

        if(lineOffset.y < 0)
            pos.y -= (-lineOffset.y)/hitCount;
        else
            pos.y += lineOffset.y/hitCount;

        //std::cout << "color: " << (int)pixels[pos.y * width + pos.x] << std::endl;
    }
    while(true);

    //std::cout << "end pos: x:" << pos.x << "y:" << pos.y << std::endl;
    average.reset();
    return line;
}

/*
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
    ///auto highPointBlured = getFrequenzyOfValues(highPointValue, 24, &minVal, &maxVal, &scale);
    ///blurVector(highPointBlured);
    ///blurVector(highPointBlured);
    /////auto highPointBlured = blurFreqenzyOfValues(highPointValue, 16, &minVal, &maxVal, &scale);
///
    ///bool foundMax = false;
    ///float last = highPointBlured[0];
    ///int cutIndex = 0;
    ///for(int i = 1; i < highPointBlured.size()-1; ++i)
    ///{
    ///    if(!foundMax)
    ///    {
    ///        if(highPointBlured[i] > highPointBlured[i-1] && highPointBlured[i] >= highPointBlured[i+1])
    ///            foundMax = true;
    ///    }
    ///    else
    ///    {
    ///        if(highPointBlured[i+1] >= highPointBlured[i]) // starts increasing
    ///        {
    ///            cutIndex = i;
    ///            break;
    ///        }
    ///    }
    ///}
///
    ///int cutHeight = frequenzyIndexToValue(scale, minVal, cutIndex);
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

    ///int lastBorderIndex = 0;
    ///for(auto& v : highPointSpace)
    ///{
    ///    // 20% tolerance
    ///    if(std::abs(v - mostFreqentSpace) > (float)mostFreqentSpace * 0.2f)
    ///        break;
///
    ///    ++lastBorderIndex;
    ///}
    ///
    ///Position borderPos;
    ///borderPos.x = start.x + highPointIndexCapped[lastBorderIndex] * moveX;
    ///borderPos.y = start.y + highPointIndexCapped[lastBorderIndex] * moveY;
    
    return mostFreqentSpace;
}
*/

Position<int> centerOnLine(const Image& image, const Position<int>& pos, Direction direction)
{
    int bestScoreUp = 0;
    Position<int> move(0, 0);

    if(direction == Direction::UP || direction == Direction::DOWN)
        move.y = 1;
    else
        move.x = 1;

    Position<int> bestPos(pos);
    for(int i = -2; i <= 2; ++i)
    {
        Position<int> p;
        p = pos + move * i;
        
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
    int stepSize = 3;
    PlayField playField{};

    Position<int> pos;
    pos.x = image.width/2 - image.width/6;
    pos.y = image.height/2 + image.height/6;
    int maxSteps = image.width/3;

    std::array<Position<int>, 3> upPoints = findPixelsOnLine(image, pos, Direction::RIGHT, maxSteps);

    std::vector<Position<int>> upLine1 = followLine(image, Direction::UP, stepSize, upPoints[0]);
    std::vector<Position<int>> upLine2 = followLine(image, Direction::UP, stepSize, upPoints[1]);
    std::vector<Position<int>> upLine3 = followLine(image, Direction::UP, stepSize, upPoints[2]);

    std::vector<Position<int>>& upLineLongest = upLine1.size() > upLine2.size() ? (upLine1.size() > upLine3.size() ? upLine1 : upLine3) : (upLine2.size() > upLine3.size() ? upLine2 : upLine3);


    if(upLineLongest.size() < 10)
        return playField;

    Position<int> lineEndPoint = centerOnLine(image, upLineLongest.back(), Direction::LEFT);

    std::vector<Position<int>> lineLeft = followLine(image, Direction::LEFT, stepSize, lineEndPoint);
    std::vector<Position<int>> lineRight = followLine(image, Direction::RIGHT, stepSize, lineEndPoint);

    Position<int> lineTopLeft = centerOnLine(image, lineLeft.back(), Direction::LEFT);
    Position<int> lineTopRight = centerOnLine(image, lineRight.back(), Direction::RIGHT);

    std::vector<Position<int>> lineLeftDown = followLine(image, Direction::DOWN, stepSize, lineTopLeft);
    std::vector<Position<int>> lineRightDown = followLine(image, Direction::DOWN, stepSize, lineTopRight);

    int moveStepsInward = 2;
    int playfieldWidth = lineRightDown[moveStepsInward].x - lineLeftDown[moveStepsInward].x;
    int squareWidth = playfieldWidth / GAME_WIDTH;


    playField.squareWidth = squareWidth;
    playField.squareHeight = squareWidth;

    playField.topLeft.x = lineLeftDown[moveStepsInward].x;
    playField.topLeft.y = lineLeft[lineLeft.size()-1-moveStepsInward].y;

    playField.topRight.x = lineRightDown[moveStepsInward].x;
    playField.topRight.y = lineRight[lineRight.size()-1-moveStepsInward].y;

    playField.botLeft = lineLeftDown.back();
    playField.botRight = lineRightDown.back();

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

int getCellStatus(Image& number, int size, int* outColorWhite)
{
    int averageOutside = 0;
    int countOutside = 0;
    int averageInside = 0;
    int countInside = 0;
    int border = size / 6;
    for(int ys = 2; ys < size-2; ys+=1)
    {
        for(int xs = 2; xs < size-2; xs+=1)
        {
            if(xs > border && xs < size-2-border && ys > border && ys < size-2-border) // inside
            {
                countInside += 1;
                averageInside += number.getValue(xs, ys);
            }
            else
            {
                countOutside += 1;
                averageOutside += number.getValue(xs, ys);
            }
        }
    }
    countOutside = std::max(1, countOutside);
    countInside = std::max(1, countInside);
    averageOutside /= countOutside;
    averageInside /= countInside;

    *outColorWhite = std::min(255, averageOutside+10);

    int diffAv = averageOutside - averageInside;

    if(diffAv >= 14)
        return 1;

    if(diffAv >= 2)
        return 0;

    return -1;
}

Position<int> centerOnCorner(Image& imageFiltered, const Position<int>& position, const Position<int>& move)
{
    Position movedDown(position);
    Position movedRight(position);
    movedDown.y += move.y;
    movedRight.x += move.x;

    Position centerDown = centerOnLine(imageFiltered, movedDown, Direction::LEFT);
    Position centerRight = centerOnLine(imageFiltered, movedRight, Direction::DOWN);

    return Position<int>(centerDown.x, centerRight.y);
}

void PictureReader::findNumbers(const Image& image, Image& imageFiltered, const PlayField& playField, std::vector<int>* numbers, const NeuralNetwork& nn)
{
    Position<float> moveVectorPerCell(0.f, 0.f);
    moveVectorPerCell = (Position<float>)(playField.topRight - playField.topLeft) / (float)GAME_WIDTH;

    Position<int> topLeft(playField.topLeft);
    Position<int> topRight(playField.topRight);

    Position<float> leftDown(playField.botLeft - playField.topLeft);
    leftDown = leftDown / std::abs(leftDown.y);

    Position<float> rightDown(playField.botRight - playField.topRight);
    rightDown = rightDown / std::abs(rightDown.y);

    int cutOutside = 3;
    Image singleNumber(moveVectorPerCell.x, moveVectorPerCell.x);
    Image singleNumberScaled(NN_IMAGE_SIZE, NN_IMAGE_SIZE);
    for(int y = 0; y < GAME_HEIGHT_VISIBLE; ++y)
    {
        if(y != 0)
        {
            topLeft = topLeft + leftDown * moveVectorPerCell.x;
            topLeft = centerOnCorner(imageFiltered, topLeft, Position(moveVectorPerCell.x/2, moveVectorPerCell.x/2));

            topRight = topRight + rightDown * moveVectorPerCell.x;
            topRight = centerOnCorner(imageFiltered, topRight, Position(-moveVectorPerCell.x/2, moveVectorPerCell.x/2));
        }

        //showPoint(imageFiltered, topLeft);
        //showPoint(imageFiltered, topRight);

        moveVectorPerCell = (Position<float>)(topRight - topLeft) / (float)GAME_WIDTH;

        int size = (int)moveVectorPerCell.x-cutOutside*2;

        for(int x = 0; x < GAME_WIDTH; ++x)
        {
            Position<int> pos;
            pos = topLeft + (Position<int>)(moveVectorPerCell * (float)x);

            int minX = 1000;
            int maxX = 0;
            int minY = 1000;
            int maxY = 0;
            
            for(int ys = 0; ys < size; ++ys)
            {
                for(int xs = 0; xs < size; ++xs)
                {
                    int value = image.getValue(xs+pos.x+cutOutside, ys+pos.y+cutOutside);
                    singleNumber.setValue(xs, ys, value);
                    if(value < COLOR_BLACK)
                    {
                        minX = std::min(xs, minX);
                        maxX = std::max(xs, maxX);
                        minY = std::min(ys, minY);
                        maxY = std::max(ys, maxY);
                    }
                }
            }

            int colorWhite;
            int cs = getCellStatus(singleNumber, size, &colorWhite);

            if(cs == 0)
            {
                std::cout << "x";
                numbers->push_back(FIELD_GRAY);
                continue;
            }
            else if(cs == -1)
            {
                std::cout << std::endl;
                return;
            }

            int centerOffsetX = (int)((float)(minX + maxX) / 2.0f + 0.5f - (float)size/2 + 100.0f) - 100;
            int centerOffsetY = (int)((float)(minY + maxY) / 2.0f + 0.5f - (float)size/2 + 100.0f) - 100;
            
            // center and scale
            Image& imageOut = singleNumberScaled;
            float scale = (float)size / (float)NN_IMAGE_SIZE;
            for(int ys = 0; ys < NN_IMAGE_SIZE; ++ys)
            {
                for(int xs = 0; xs < NN_IMAGE_SIZE; ++xs)
                {
                    int xx = (int)((float)xs*scale + 0.5f);
                    int yy = (int)((float)ys*scale + 0.5f);
                    int ox = std::max(0, std::min(size-1, xx+centerOffsetX));
                    int oy = std::max(0, std::min(size-1, yy+centerOffsetY));
                    int value = singleNumber.getValue(ox, oy);
                    imageOut.setValue(xs, ys, value);
                }
            }

            int number = nn.run(imageOut);
            std::cout << number;

            numbers->push_back(number);
            //std::string name = std::string("number_s_t") + std::string("_y") + std::to_string(y) + std::string("_x") + std::to_string(x) + std::string(".png");
            //saveImage(name, imageOut);
        }
        std::cout << std::endl;
    }
}

void PictureReader::getNumberImages(const Image& image, Image& imageFiltered, const PlayField& playField, std::list<Image>* out, const std::vector<int>& numbers)
{
    Position<float> moveVectorPerCell(0.f, 0.f);
    moveVectorPerCell = (Position<float>)(playField.topRight - playField.topLeft) / (float)GAME_WIDTH;

    Position<int> topLeft(playField.topLeft);
    Position<int> topRight(playField.topRight);

    Position<float> leftDown(playField.botLeft - playField.topLeft);
    leftDown = leftDown / std::abs(leftDown.y);

    Position<float> rightDown(playField.botRight - playField.topRight);
    rightDown = rightDown / std::abs(rightDown.y);

    int cutOutside = 3;
    Image singleNumber(moveVectorPerCell.x, moveVectorPerCell.x);
    Image singleNumberScaled(NN_IMAGE_SIZE, NN_IMAGE_SIZE);
    int imageCount = 0;
    for(int y = 0; y < 11; ++y)
    {
        if(y != 0)
        {
            topLeft = topLeft + leftDown * moveVectorPerCell.x;
            topLeft = centerOnCorner(imageFiltered, topLeft, Position(moveVectorPerCell.x/2, moveVectorPerCell.x/2));

            topRight = topRight + rightDown * moveVectorPerCell.x;
            topRight = centerOnCorner(imageFiltered, topRight, Position(-moveVectorPerCell.x/2, moveVectorPerCell.x/2));
        }

        moveVectorPerCell = (Position<float>)(topRight - topLeft) / (float)GAME_WIDTH;
        int size = (int)moveVectorPerCell.x-cutOutside*2;

        for(int x = 0; x < 9; ++x)
        {
            if(imageCount >= numbers.size())
                return;

            Position<int> pos;
            pos = topLeft + (Position<int>)(moveVectorPerCell * (float)x);

            int minX = 1000;
            int maxX = 0;
            int minY = 1000;
            int maxY = 0;
            int count = 0;
            for(int ys = 0; ys < size; ++ys)
            {
                for(int xs = 0; xs < size; ++xs)
                {
                    int value = image.getValue(xs+pos.x+cutOutside, ys+pos.y+cutOutside);
                    singleNumber.setValue(xs, ys, value);
                    if(value < COLOR_BLACK)
                    {
                        minX = std::min(xs, minX);
                        maxX = std::max(xs, maxX);
                        minY = std::min(ys, minY);
                        maxY = std::max(ys, maxY);
                        count += 1;
                    }
                }
            }

            if(count < 50)
                continue;

            int centerOffsetX = (int)((float)(minX + maxX) / 2.0f + 0.5f - (float)size/2 + 100.0f) - 100;
            int centerOffsetY = (int)((float)(minY + maxY) / 2.0f + 0.5f - (float)size/2 + 100.0f) - 100;
            
            Image& imageOut = out->emplace_back(NN_IMAGE_SIZE, NN_IMAGE_SIZE);
            float scale = (float)size / (float)NN_IMAGE_SIZE;
            for(int ys = 0; ys < NN_IMAGE_SIZE; ++ys)
            {
                for(int xs = 0; xs < NN_IMAGE_SIZE; ++xs)
                {
                    int xx = (int)((float)xs*scale + 0.5f);
                    int yy = (int)((float)ys*scale + 0.5f);
                    int ox = std::max(0, std::min(size-1, xx+centerOffsetX));
                    int oy = std::max(0, std::min(size-1, yy+centerOffsetY));
                    int value = singleNumber.getValue(ox, oy);
                    imageOut.setValue(xs,ys, value);
                }
            }

            //std::string name = std::string("number_s_n") + std::to_string(numbers[y * 9 + x]) + std::string("_y") + std::to_string(y) + std::string("_x") + std::to_string(x) + std::string(".png");
            //saveImage(name, imageOut);

            imageCount += 1;
        }
    }
}
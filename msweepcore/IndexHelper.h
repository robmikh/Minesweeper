#pragma once
struct IndexHelper
{
    int width;
    int height;

    IndexHelper(int width, int height)
    {
        this->width = width;
        this->height = height;
    }

    int ComputeIndex(int x, int y) 
    {
        return x * height + y;
    }

    int ComputeXFromIndex(int index)
    {
        return index / height;
    }

    int ComputeYFromIndex(int index)
    {
        return index % height;
    }

    bool IsInBounds(int x, int y)
    {
        return (x >= 0 && x < width) && (y >= 0 && y < height);
    }
};
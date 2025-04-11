// Mock implementation of OpenCV core for testing
#pragma once
#ifndef OPENCV2_CORE_HPP
#define OPENCV2_CORE_HPP

#include <vector>

namespace cv {
    class Mat {
    public:
        Mat() = default;
        Mat(int rows, int cols, int type) : rows(rows), cols(cols) {}
        int rows = 0;
        int cols = 0;
        void create(int rows, int cols, int type) { this->rows = rows; this->cols = cols; }
        bool empty() const { return rows == 0 || cols == 0; }
    };
    
    template<typename T>
    class Point_ {
    public:
        T x, y;
        Point_() : x(0), y(0) {}
        Point_(T x, T y) : x(x), y(y) {}
    };
    
    using Point2f = Point_<float>;
    
    template<typename T>
    class Rect_ {
    public:
        T x, y, width, height;
        Rect_() : x(0), y(0), width(0), height(0) {}
        Rect_(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}
    };
    
    using Rect = Rect_<int>;
}

#endif

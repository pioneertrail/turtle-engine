// Mock OpenCV header for building without actual OpenCV
#pragma once

#include <vector>
#include <string>
#include <memory>

namespace cv {
    class Mat {
    public:
        Mat() {}
        Mat(int rows, int cols, int type) {}
        Mat(const Mat& other) {}
        
        int rows = 0;
        int cols = 0;
    };
    
    enum ImreadModes {
        IMREAD_COLOR = 1,
        IMREAD_GRAYSCALE = 0
    };
    
    Mat imread(const std::string& filename, int mode = IMREAD_COLOR) {
        return Mat();
    }
    
    void imshow(const std::string& winname, const Mat& mat) {}
    
    int waitKey(int delay = 0) { return 0; }
    
    template<typename T>
    class Point_ {
    public:
        Point_() : x(0), y(0) {}
        Point_(T x, T y) : x(x), y(y) {}
        
        T x, y;
    };
    
    typedef Point_<int> Point;
    
    template<typename T>
    class Rect_ {
    public:
        Rect_() : x(0), y(0), width(0), height(0) {}
        Rect_(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}
        
        T x, y, width, height;
    };
    
    typedef Rect_<int> Rect;
}

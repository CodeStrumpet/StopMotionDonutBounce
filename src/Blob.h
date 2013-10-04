//
//  blob.h
//  watercolorwalls
//
//  Created by Matthew Thomas on 2/27/13.
//
//

#ifndef __watercolorwalls__blob__
#define __watercolorwalls__blob__

#include <iostream>

#include "ofMain.h"

class Blob {
public:
    Blob();
    Blob(int index_);
    ~Blob();

    int getIndex();
    void setLastFrameArea(float area);
    float getLastFrameArea();
    ofPoint getCentroid();
    void setCentroid(ofPoint newCentroid);
    unsigned long long getTimestamp();
    void setTimestamp(unsigned long long newTimestamp);
    
private:
    int index;
    float lastFrameArea;
    ofPoint centroid;
    unsigned long long timestamp;
};

#endif /* defined(__watercolorwalls__blob__) */

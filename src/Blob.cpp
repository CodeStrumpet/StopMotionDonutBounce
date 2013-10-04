//
//  blob.cpp
//  watercolorwalls
//
//  Created by Matthew Thomas on 2/27/13.
//
//

#include "blob.h"

Blob::Blob() {
    index = -1;
}

Blob::Blob(int index_) :
index (index_)
{
    index = index_;
}

Blob::~Blob() {
}


int Blob::getIndex()
{
    return index;
}


void Blob::setLastFrameArea(float area) {
    lastFrameArea = area;
}

float Blob::getLastFrameArea() {
    return lastFrameArea;
}

ofPoint Blob::getCentroid() {
    return centroid;
}

void Blob::setCentroid(ofPoint newCentroid) {
    centroid = newCentroid;
}

unsigned long long Blob::getTimestamp() {
    return timestamp;
}

void Blob::setTimestamp(unsigned long long newTimestamp) {
    timestamp = newTimestamp;
}






#ifndef _TEST_APP
#define _TEST_APP

#include <tr1/unordered_map>

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxOpenNI.h"
#include "ofxBlobTracker.h"
#include "Blob.h"

typedef std::tr1::unordered_map< int, Blob > hashmap;


#define CAM_WIDTH      640
#define CAM_HEIGHT     480

typedef enum MotionState {
    MotionStateNoObjects = 0,
    MotionStateMotion,
    MotionStateStill
} MotionState;


class testApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();
    
    // Input
    void keyPressed(int key);
    void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
    
    void blobAdded(ofxBlob &_blob);
    void blobMoved(ofxBlob &_blob);
    void blobDeleted(ofxBlob &_blob);

private:
    
    // Image sources
	ofVideoGrabber              vidGrabber;
    ofxOpenNI                   openNI;
    
    // Image containers
	ofxCvGrayscaleImage         colorImage;  // It is so ugly that "colorImage" is actually a grayscale image right now. sorry...
	ofxCvGrayscaleImage         threImg;
	ofxCvGrayscaleImage         bgImg;
	
    // Blob tracking
    ofxBlobTracker              blobTracker;
    int                         minArea;    
	int                         threshold;
	bool                        bLearnBackground;
    hashmap                     cachedBlobs;
    
    // Serial
    ofSerial                    serial;
    bool                        serialInited;
    
    // Timing
    unsigned long long          lastSerialOutTime;
    
    // drawing
    ofTrueTypeFont              largeMessageFont;
    ofTrueTypeFont              smallMessageFont;
    ofTrueTypeFont              instructionsFont;
    
    MotionState                 currMotionState;
    
};

#endif

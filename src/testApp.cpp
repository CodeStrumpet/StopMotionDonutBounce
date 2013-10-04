#include "testApp.h"

#include <math.h>       /* sqrt */

#define USE_OPENNI 1
#define SERIAL_INTERVAL_MILLIS 100
#define BLOB_MOVEMENT_DIST_THRESHOLD .2
#define BLOB_MOVEMENT_TIME_THRESHOLD 2000

#define SERIAL_ADDRESS "/dev/tty.usbmodem1d1121" //"/dev/tty.usbmodem1411" 
#define ONI_FILE "test_stopmotion.oni" //"test_with_donut.oni"

void testApp::setup(){
    
	ofBackground(255, 255, 255);
    ofSetFrameRate(60);
    
    ofAddListener(blobTracker.blobAdded, this, &testApp::blobAdded);
    ofAddListener(blobTracker.blobMoved, this, &testApp::blobMoved);
    ofAddListener(blobTracker.blobDeleted, this, &testApp::blobDeleted);
	
    colorImage.allocate(CAM_WIDTH, CAM_HEIGHT);
    threImg.allocate(CAM_WIDTH, CAM_HEIGHT);
    bgImg.allocate(CAM_WIDTH, CAM_HEIGHT);
	
    if (USE_OPENNI) {
        // setup openni
        openNI.setup();        
        //openNI.startPlayer(ONI_FILE);
        
        openNI.addDepthGenerator();
        openNI.addImageGenerator();
        openNI.setRegister(true);
        //openNI.setMirror(true);
        openNI.addUserGenerator();
        openNI.setMaxNumUsers(2);
        openNI.setDepthColoring(COLORING_GREY);
        openNI.start();
        
    } else {
        vidGrabber.setDeviceID(0);
        vidGrabber.initGrabber(CAM_WIDTH, CAM_HEIGHT);        
    }
	
	threshold = 10;
    minArea = 2000;
	bLearnBackground = true;
    
    
    // setup serial
    serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	
	int baud = 9600;
	//serial.setup(0, baud); //open the first device
	serialInited = serial.setup(SERIAL_ADDRESS, baud); 
    
    // initialize lastSerialOutTime to current time
    lastSerialOutTime = ofGetElapsedTimeMillis();
    
    largeMessageFont.loadFont("DroidSerif-Regular.ttf", 30);
    smallMessageFont.loadFont("DroidSerif-Regular.ttf", 20);
    instructionsFont.loadFont("DroidSerif-Regular.ttf", 40);
}

void testApp::update(){
    
    if (USE_OPENNI) {
        openNI.update();
    } else {
        vidGrabber.update();
    }
	
	if (vidGrabber.isFrameNew() || openNI.isNewFrame()) {
        

        if (USE_OPENNI && openNI.isDepthOn()) {            
            
            int numUsers = openNI.getNumTrackedUsers();
            if (false && numUsers > 0) { // Disabled!!!
                
                ofxOpenNIUser & user = openNI.getTrackedUser(0);
                colorImage.setFromPixels(user.getMaskPixels());
            } else {                
                colorImage.setFromPixels(openNI.getDepthPixels().getChannel(0));                
            }
            
        } else if (vidGrabber.isInitialized()) {
            colorImage.setFromPixels(vidGrabber.getPixels(), CAM_WIDTH, CAM_HEIGHT);
        }
        
        //colorImage.mirror(false, true);
		
        if(bLearnBackground)
        {
            bgImg = colorImage;
            bLearnBackground = false;
        }
		
        threImg = colorImage;
        threImg.absDiff(bgImg);
        //threImg.blur(5);
        threImg.threshold(threshold);
        
        blobTracker.update(threImg, threshold, minArea, 640*480);
        
        

        
        
        /*
         
         dictionary <int blobID, ofPoint centroid, time >
         
         cache first centroid, recache if centroid is more than X distance away
         
         
         
         */
        
        
       
        MotionState motionState = MotionStateNoObjects;
        
        bool isStopped = true; // if any blob is in motion, this will be set to false
        
        if (blobTracker.size() > 0) {
            
            motionState = MotionStateMotion;
            
            for (int i = 0; i < blobTracker.size(); i++) {
                
                int blobId = blobTracker[i].id;
                
                // blob does not yet exist
                if (!cachedBlobs.count(blobId)) {
                    Blob blob = Blob(blobId);
                    blob.setCentroid(blobTracker[i].centroid);
                    blob.setTimestamp(ofGetElapsedTimeMillis());
                    cachedBlobs[blobId] = blob;
                } else {
                    
                    Blob cachedBlob = cachedBlobs[blobId];
                    
                    float movementAmount = sqrt(pow(blobTracker[i].centroid.x - cachedBlob.getCentroid().x, 2) +
                                                pow(blobTracker[i].centroid.y - cachedBlob.getCentroid().y, 2));
                    
                    if (ofGetElapsedTimeMillis() - lastSerialOutTime > SERIAL_INTERVAL_MILLIS) {
                        //cout << "movementAmount: " << movementAmount << "  elapsedTime: " << ofGetElapsedTimeMillis() - cachedBlob.getTimestamp() << "\n";
                    }
                    
                    // we declare still state if blob has not moved and it has been enough time since cached position
                    if (movementAmount < BLOB_MOVEMENT_DIST_THRESHOLD && ofGetElapsedTimeMillis() - cachedBlob.getTimestamp() > BLOB_MOVEMENT_TIME_THRESHOLD) {
                        
                        // at least one blob is still, don't do anything (setting isStopped to true here will not guarantee no movement from all blobs)
                        
                    } else {
                        isStopped = false;
                        
                        // recache position and time if blob has moved more than threshold distance
                        if (movementAmount > BLOB_MOVEMENT_DIST_THRESHOLD) {
                            cachedBlob.setCentroid(blobTracker[i].centroid);
                            cachedBlob.setTimestamp(ofGetElapsedTimeMillis());
                            
                            cachedBlobs[blobId] = cachedBlob;
                        }
                    }
                }                
                
                //cout << "blob " << blobTracker[i].id << ":  {age: " << blobTracker[i].age << "}, {sitting: " << blobTracker[i].sitting << "}, {maccel: " << blobTracker[i].maccel << "}\n";
            }
            
            if (isStopped) {
                motionState = MotionStateStill;
            } else {
                motionState = MotionStateMotion;
            }            
        }
        
        currMotionState = motionState;
        
        if (ofGetElapsedTimeMillis() - lastSerialOutTime > SERIAL_INTERVAL_MILLIS) {
            
            // print curr values
            for (int i = 0; i < blobTracker.size(); i++) {
                
                //cout << "blob " << blobTracker[i].id << ":  {age: " << blobTracker[i].age << "}, {sitting: " << blobTracker[i].sitting << "}, {maccel: " << blobTracker[i].maccel << "}\n";
            }

            
            //cout << "motionState: " << motionState << "\n";
            
            lastSerialOutTime = ofGetElapsedTimeMillis();
            
            char motionMode = 'd';
            if (currMotionState == MotionStateNoObjects) {
                motionMode = 'a';
            } else if (currMotionState == MotionStateMotion) {
                motionMode = 'b';
            } else if (currMotionState == MotionStateStill) {
                motionMode = 'c';
            }
            
            // if condition is met...
            if (serialInited) {
                serial.writeByte(motionMode);                
            }

            
        }
	}
    
}

void testApp::draw(){

	
    ofSetColor(255, 255, 255);
    ofSetLineWidth(1);
    
    colorImage.draw(CAM_WIDTH, 0);
        
	
    ofSetHexColor(0x1826B0);
    
    threImg.draw(0, 0);
    
    blobTracker.draw(0, 0);
    
	
	// Physics

    // Infos
    ofSetColor(0, 0, 0);
    ofDrawBitmapString("FPS : " + ofToString(ofGetFrameRate()), 10, CAM_HEIGHT + 20);
    
    string motionString = "";
    string modifierString = "";
    
    if (currMotionState == MotionStateNoObjects) {
        motionString = "No objects";
        modifierString = "(Donut is lonely)";
    } else if (currMotionState == MotionStateMotion) {
        motionString = "Moving objects";
        modifierString = "(Donut is intrigued)";
    } else if (currMotionState == MotionStateStill) {
        motionString = "Non-moving objects";
        modifierString = "(Angry donut)";
    }
    
    ofSetHexColor(0xDD3311);
    largeMessageFont.drawString(motionString + " " + modifierString, 100, CAM_HEIGHT + 100);
    //ofSetHexColor(0xAA3388);
    //smallMessageFont.drawString(modifierString, 100, CAM_HEIGHT + 250);
    
    
    ofSetHexColor(0x000000);
    instructionsFont.drawString("This is an emotional donut\nStand still to make it angry ;)!", 100, CAM_HEIGHT + 250);
    
    
}


void testApp::blobAdded(ofxBlob &_blob){
    //ofLog(OF_LOG_NOTICE, "Blob ID " + ofToString(_blob.id) + " added" );
}

void testApp::blobMoved(ofxBlob &_blob){
    //ofLog(OF_LOG_NOTICE, "Blob ID " + ofToString(_blob.id) + " moved" );
}

void testApp::blobDeleted(ofxBlob &_blob){
    //ofLog(OF_LOG_NOTICE, "Blob ID " + ofToString(_blob.id) + " deleted" );
}

void testApp::keyPressed(int key){
	
	switch (key) {
            
        case 'p':
            openNI.startPlayer(ONI_FILE);
            break;
        case '1':
            if (minArea > 250) {
                minArea -= 5;
            }            
            break;
        case '2':
            minArea += 5;
            break;
		case ' ':
			bLearnBackground = true;
			break;
        case 'q':
            serial.close();
            serialInited = false;
            break;
        case 's':
            //serial.setup(0, baud); //open the first device
            serialInited = serial.setup(SERIAL_ADDRESS, 9600);
            break;
    }
    
}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ) {
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {
	
}


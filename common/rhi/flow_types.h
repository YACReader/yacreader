// Shared types for YACReader Flow implementations (GL and RHI)
#ifndef __YACREADER_FLOW_TYPES_H
#define __YACREADER_FLOW_TYPES_H

enum Performance {
    low = 0,
    medium,
    high,
    ultraHigh
};

// Cover Vector
struct YACReader3DVector {
    float x;
    float y;
    float z;
    float rot;
};

struct Preset {
    /*** Animation Settings ***/
    // sets the speed of the animation
    float animationStep;
    // sets the acceleration of the animation
    float animationSpeedUp;
    // sets the maximum speed of the animation
    float animationStepMax;
    // sets the distance of view
    float animationFadeOutDist;
    // sets the rotation increasion
    float preRotation;
    // sets the light strenght on rotation
    float viewRotateLightStrenght;
    // sets the speed of the rotation
    float viewRotateAdd;
    // sets the speed of reversing the rotation
    float viewRotateSub;
    // sets the maximum view angle
    float viewAngle;

    /*** Position Configuration ***/
    // the X Position of the Coverflow
    float cfX;
    // the Y Position of the Coverflow
    float cfY;
    // the Z Position of the Coverflow
    float cfZ;
    // the X Rotation of the Coverflow
    float cfRX;
    // the Y Rotation of the Coverflow
    float cfRY;
    // the Z Rotation of the Coverflow
    float cfRZ;
    // sets the rotation of each cover
    float rotation;
    // sets the distance between the covers
    float xDistance;
    // sets the distance between the centered and the non centered covers
    float centerDistance;
    // sets the pushback amount
    float zDistance;
    // sets the elevation amount
    float yDistance;

    float zoom;
};

extern struct Preset defaultYACReaderFlowConfig;
extern struct Preset presetYACReaderFlowClassicConfig;
extern struct Preset presetYACReaderFlowStripeConfig;
extern struct Preset presetYACReaderFlowOverlappedStripeConfig;
extern struct Preset pressetYACReaderFlowUpConfig;
extern struct Preset pressetYACReaderFlowDownConfig;

#endif // __YACREADER_FLOW_TYPES_H

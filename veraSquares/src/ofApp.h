#pragma once
#include "ofMain.h"

// A3 dimensions
#define A3_WIDTH  3508
#define A3_HEIGHT 4961

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);

    // --- Grid creation ---
    void generateMolnarGrid();
    ofPolyline drawScribblySquare(float x, float y, float size, float offsetRange);
    void createCornerLines(float cornerLength);

    // --- Scale & Save ---
    vector<ofPolyline> scaleAndTranslatePolylines(
        const vector<ofPolyline>& src,
        float scale,
        float offsetX,
        float offsetY
    );
    bool savePolylinesToSVG(const string& filename,
        const vector<ofPolyline>& polylines,
        int svgWidth,
        int svgHeight,
        float strokeWidth,
        const string& color);

    // --- Storage ---
    vector<ofPolyline> molnarGridSquares;   // <--- declare it here
    vector<ofPolyline> cornerLines;

    // --- Grid Params ---
    int   gridRows = 6;
    int   gridCols = 6;
    float cellSize = 80.0f;

    // --- Styles ---
    float offsetRange = 5.0f;
    float lineWidth = 1.5f;
};

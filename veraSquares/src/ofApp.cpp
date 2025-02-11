#include "ofApp.h"
#include <algorithm>

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetFrameRate(60);
    ofBackground(255);

    createCornerLines(50.0f);
    generateMolnarGrid();  
}

//--------------------------------------------------------------
void ofApp::update() {
    // no-op
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(255);

    // Draw scribbly squares
    ofSetColor(0);
    ofSetLineWidth(lineWidth);
    for (auto& poly : molnarGridSquares) {
        poly.draw();
    }

    // Draw corner lines
    for (auto& c : cornerLines) {
        c.draw();
    }

    // Basic instructions
    ofSetColor(0);
    ofDrawBitmapString("Press 's' to save to A3", 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 's') {
        float scaleX = (float)A3_WIDTH / (float)ofGetWidth();
        float scaleY = (float)A3_HEIGHT / (float)ofGetHeight();
        float uniformScale = std::min(scaleX, scaleY);

        float offsetX = (A3_WIDTH - ofGetWidth() * uniformScale) * 0.5f;
        float offsetY = (A3_HEIGHT - ofGetHeight() * uniformScale) * 0.5f;

        // Scale squares + corners
        auto scaledSquares = scaleAndTranslatePolylines(molnarGridSquares, uniformScale, offsetX, offsetY);
        auto scaledCorners = scaleAndTranslatePolylines(cornerLines, uniformScale, offsetX, offsetY);

        // Merge
        vector<ofPolyline> merged;
        merged.reserve(scaledSquares.size() + scaledCorners.size());
        merged.insert(merged.end(), scaledSquares.begin(), scaledSquares.end());
        merged.insert(merged.end(), scaledCorners.begin(), scaledCorners.end());

        // Save
        bool ok = savePolylinesToSVG("molnarGrid_A3.svg",
            merged,
            A3_WIDTH,
            A3_HEIGHT,
            lineWidth,
            "black");
        if (ok) {
            ofLogNotice() << "Saved molnarGrid_A3.svg!";
        }
    }
}

//--------------------------------------------------------------
void ofApp::generateMolnarGrid() {
    molnarGridSquares.clear();

    float totalW = gridCols * cellSize;
    float totalH = gridRows * cellSize;

    // Center the entire grid
    float startX = (ofGetWidth() - totalW) * 0.5f;
    float startY = (ofGetHeight() - totalH) * 0.5f;

    // Set up ranges:
    float minOffsetRange = 20.0f;   // or whatever minimum wiggle you like
    float maxOffsetRange = 30.0f;  // or your maximum wiggle

    // We'll have (gridRows × gridCols) cells total
    int totalCells = gridRows * gridCols;

    // Start a global scribble count at 1
    int scribbleCount = 1;

    // Iterate rows top->bottom
    for (int r = 0; r < gridRows; r++) {
        // Iterate columns right->left
        for (int c = gridCols - 1; c >= 0; c--) {
            float x = startX + c * cellSize;
            float y = startY + r * cellSize;

            // 1) The number of repeats for this cell
            int repeats = scribbleCount;

            // 2) Map scribbleCount in [1..totalCells] to offset range
            //    fraction goes from ~0.0 at scribbleCount=1 up to 1.0 at scribbleCount=totalCells
            float fraction = float(scribbleCount - 1) / float(totalCells - 1);
            float thisOffset = ofMap(fraction, 0, 1,
                minOffsetRange, maxOffsetRange, true);

            // Draw that many scribbles in this cell
            for (int i = 0; i < repeats; i++) {
                ofPolyline poly = drawScribblySquare(x, y, cellSize, thisOffset);
                molnarGridSquares.push_back(poly);
            }

            // Move on to the next cell
            scribbleCount++;
        }
    }
}


//--------------------------------------------------------------
ofPolyline ofApp::drawScribblySquare(float x, float y, float size, float offsetRange) {
    auto randOff = [offsetRange]() {
        return ofRandom(-offsetRange, offsetRange);
    };

    glm::vec3 topLeft(x + randOff(), y + randOff(), 0);
    glm::vec3 topRight(x + size + randOff(), y + randOff(), 0);
    glm::vec3 bottomRight(x + size + randOff(), y + size + randOff(), 0);
    glm::vec3 bottomLeft(x + randOff(), y + size + randOff(), 0);

    ofPolyline sq;
    sq.addVertex(topLeft);
    sq.addVertex(topRight);
    sq.addVertex(bottomRight);
    sq.addVertex(bottomLeft);
    sq.close();
    return sq;
}


//--------------------------------------------------------------
void ofApp::createCornerLines(float cornerLength) {
    cornerLines.clear();

    float w = ofGetWidth();
    float h = ofGetHeight();

    // top-left
    {
        ofPolyline horz, vert;
        horz.addVertex(glm::vec3(0, 0, 0));
        horz.addVertex(glm::vec3(cornerLength, 0, 0));

        vert.addVertex(glm::vec3(0, 0, 0));
        vert.addVertex(glm::vec3(0, cornerLength, 0));

        cornerLines.push_back(horz);
        cornerLines.push_back(vert);
    }
    // top-right
    {
        ofPolyline horz, vert;
        horz.addVertex(glm::vec3(w - cornerLength, 0, 0));
        horz.addVertex(glm::vec3(w, 0, 0));

        vert.addVertex(glm::vec3(w, 0, 0));
        vert.addVertex(glm::vec3(w, cornerLength, 0));

        cornerLines.push_back(horz);
        cornerLines.push_back(vert);
    }
    // bottom-left
    {
        ofPolyline horz, vert;
        horz.addVertex(glm::vec3(0, h, 0));
        horz.addVertex(glm::vec3(cornerLength, h, 0));

        vert.addVertex(glm::vec3(0, h - cornerLength, 0));
        vert.addVertex(glm::vec3(0, h, 0));

        cornerLines.push_back(horz);
        cornerLines.push_back(vert);
    }
    // bottom-right
    {
        ofPolyline horz, vert;
        horz.addVertex(glm::vec3(w - cornerLength, h, 0));
        horz.addVertex(glm::vec3(w, h, 0));

        vert.addVertex(glm::vec3(w, h - cornerLength, 0));
        vert.addVertex(glm::vec3(w, h, 0));

        cornerLines.push_back(horz);
        cornerLines.push_back(vert);
    }
}

//--------------------------------------------------------------
vector<ofPolyline> ofApp::scaleAndTranslatePolylines(
    const vector<ofPolyline>& src,
    float scale,
    float offsetX,
    float offsetY)
{
    vector<ofPolyline> result;
    result.reserve(src.size());

    for (const auto& poly : src) {
        ofPolyline newPoly;
        for (auto& pt : poly.getVertices()) {
            float nx = pt.x * scale + offsetX;
            float ny = pt.y * scale + offsetY;
            newPoly.addVertex(glm::vec3(nx, ny, 0));
        }
        result.push_back(newPoly);
    }
    return result;
}

//--------------------------------------------------------------
bool ofApp::savePolylinesToSVG(const string& filename,
    const vector<ofPolyline>& polylines,
    int svgWidth,
    int svgHeight,
    float strokeWidth,
    const string& color)
{
    ofFile file(filename, ofFile::WriteOnly);
    if (!file) {
        ofLogError("savePolylinesToSVG") << "Could not open " << filename;
        return false;
    }

    file << "<svg version=\"1.1\" baseProfile=\"full\" "
        << "xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << svgWidth << "\" "
        << "height=\"" << svgHeight << "\">\n";
    file << "<g fill=\"none\" stroke=\"" << color
        << "\" stroke-width=\"" << strokeWidth << "\">\n";

    for (const auto& poly : polylines) {
        const auto& verts = poly.getVertices();
        if (verts.size() < 2) continue;

        file << "  <path d=\"M " << verts[0].x << " " << verts[0].y;
        for (size_t i = 1; i < verts.size(); i++) {
            file << " L " << verts[i].x << " " << verts[i].y;
        }
        file << " Z\"/>\n";
    }

    file << "</g>\n</svg>\n";
    file.close();

    ofLogNotice("savePolylinesToSVG")
        << "Saved " << filename << " with " << polylines.size() << " polylines.";
    return true;
}

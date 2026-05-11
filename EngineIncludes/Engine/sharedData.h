#ifndef SHAREDDATA_H
#define SHAREDDATA_H

class sharedData
{
    public:
        unsigned int rectVAO = 0, rectVBO = 0;
        unsigned int circleVAO = 0, circleVBO = 0;
        unsigned int mainShaderID = 0, pointShaderID = 0;
        static constexpr int circleNumVertices = 33;
        static constexpr int circleNumIndices = (circleNumVertices - 1) * 3;
        void initVAOs();
    private:
        void createCirclePoints();
        const float rectVertices[16] = {
            1 / 2.0f,  1 / 2.0f, 1.0f, 1.0f,  // top right
            -1 / 2.0f,  1 / 2.0f, 0.0f, 1.0f,   // top left
            -1 / 2.0f, -1 / 2.0f, 0.0f, 0.0f, // bottom left 
            1 / 2.0f, -1 / 2.0f, 1.0f, 0.0f// bottom right
        };
        const int rectIndices[6] = { 
            0, 1, 3,  
            1, 2, 3    
        }; 

        float circleVertices[circleNumVertices * 4];
        unsigned int circleIndices[circleNumIndices];
};

#endif
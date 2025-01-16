#ifndef SHAREDDATA_H
#define SHAREDDATA_H

class sharedData
{
    public:
        unsigned int rectVAO = 0, rectVBO = 0;
        void initVAOs();
    private:
        const float rectVertices[8] = {
        1 / 2.0f,  1 / 2.0f,  // top right
        -1 / 2.0f,  1 / 2.0f,   // top left
        -1 / 2.0f, -1 / 2.0f,  // bottom left 
        1 / 2.0f, -1 / 2.0f  // bottom right
    };
    const int rectIndices[6] = { 
                            0, 1, 3,  
                            1, 2, 3    
                        }; 
};

#endif
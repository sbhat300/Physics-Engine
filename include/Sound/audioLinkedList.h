#ifndef AUDIOLINKEDLIST_H
#define AUDIOLINKEDLIST_H

#include <Sound/audioClip.h>

class audioLinkedList
{

    public:
        struct audioNode
        {
            audioClip* data;
            audioNode* next;
        };
        audioNode* head;

        audioLinkedList();
        void addNode(audioClip* data);
        void popNode(audioNode* prevNode);
        void destroy();
    private:

};

#endif
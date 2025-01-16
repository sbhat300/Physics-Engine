#include "Sound/audioLinkedList.h"

audioLinkedList::audioLinkedList()
{
    head = nullptr;
}

void audioLinkedList::addNode(audioClip* data)
{
    head = new audioNode{data, head};
}

void audioLinkedList::popNode(audioNode* prevNode)
{
    if(prevNode == nullptr)
    {
        audioNode* prevHead = head;
        head = head->next;
        delete prevHead;
        return;
    }

    audioNode* deleteNode = prevNode->next;
    prevNode->next = deleteNode->next;
    delete deleteNode;
}

void audioLinkedList::destroy()
{
    audioNode* prevHead;
    while(head != nullptr)
    {
        prevHead = head;
        head = head->next;
        delete prevHead;
    }
}
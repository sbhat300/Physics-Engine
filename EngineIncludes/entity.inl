#ifndef ENTITY_INL
#define ENTITY_INL
#include "entity.h" //for intellisense

template <typename T>
void entity::addScript() {
    T* s = new T();
    s->parent = this;
    s->start();
    scripts.push_back(s);
    contain[3] = true;
}

#endif
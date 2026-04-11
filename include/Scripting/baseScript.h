#ifndef BASESCRIPT_H
#define BASESCRIPT_H

#include <entity.h>

class baseScript
{
    public:
        entity* parent;

        virtual void start() {};
        virtual void update() {};
        virtual void fixedUpdate() {};
        virtual ~baseScript() = default;
};

#endif
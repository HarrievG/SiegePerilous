#pragma once

#include "PhysicsShapeCreator.h"
#include <string>
#include <memory>
#include <map>

namespace SiegePerilous
{
    class ShapeFactory
    {
    public:
        void registerCreator(const std::string& type, std::unique_ptr<PhysicsShapeCreator> creator);
        const PhysicsShapeCreator* create(const std::string& type) const;

    private:
        std::map<std::string, std::unique_ptr<PhysicsShapeCreator>> m_creators;
    };
}

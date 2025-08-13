#include "ShapeFactory.h"

namespace SiegePerilous
{
    void ShapeFactory::registerCreator(const std::string& type, std::unique_ptr<PhysicsShapeCreator> creator)
    {
        m_creators[type] = std::move(creator);
    }

    const PhysicsShapeCreator* ShapeFactory::create(const std::string& type) const
    {
        auto it = m_creators.find(type);
        if (it != m_creators.end())
        {
            return it->second.get();
        }
        return nullptr;
    }
}

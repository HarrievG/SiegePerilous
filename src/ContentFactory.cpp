#include "ContentFactory.h"

namespace SiegePerilous
{
    void ContentFactory::RegisterItem(const std::string& type, std::unique_ptr<ContentItem> creator)
    {
        m_creators[type] = std::move(creator);
    }

    const ContentItem* ContentFactory::Get(const std::string& type) const
    {
        auto it = m_items.find(type);
        if (it != m_items.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

	void TiledContent::load( glz::json_t &json ) const {

	}

}

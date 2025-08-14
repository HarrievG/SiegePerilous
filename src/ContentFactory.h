#pragma once

#include <string>
#include <memory>
#include <map>
#include <glaze/glaze.hpp>
#include "include/Declarations.h"


namespace SiegePerilous
{
	using namespace System;
	class ContentItem {
	public:
		virtual ~ContentItem( ) = default;
		virtual void Load( glz::json_t &json ) = 0;
	};


    class ContentFactory
    {
    public:
        void RegisterItem(const std::string& type, std::unique_ptr<ContentItem> creator);
        const ContentItem* Get(const std::string& type) const;

    private:
        std::map<std::string, std::unique_ptr<ContentItem>> m_items;
    };


	// PhysicsShapeCreator for ASE sprites
	class TileContent : public ContentItem {
	public:
		void load( glz::json_t &json ) const override;
		UInt8 m_tileType = 0;
	};

}

#include "ContentFactory.h"

namespace SiegePerilous
{
	namespace Content {

		fs::path snip_first_folder( fs::path &p ) {
			fs::path snipped_part;
			auto it = p.begin( );
			auto end = p.end( );

			if ( it == end ) {
				return snipped_part; // Path is empty, return empty
			}

			// Find the iterator position of the first actual directory component, skipping roots
			auto first_dir_it = it;
			while ( first_dir_it != end && ( *first_dir_it == p.root_name( ) || *first_dir_it == p.root_directory( ) ) ) {
				++first_dir_it;
			}

			// If we found a directory component to snip...
			if ( first_dir_it != end ) {
				// 1. Store the part we are about to snip.
				snipped_part = *first_dir_it;

				// 2. Rebuild the path from the remaining components.
				fs::path new_path;
				auto rebuild_it = first_dir_it;
				++rebuild_it; // Start rebuilding from the component *after* the snipped one

				while ( rebuild_it != end ) {
					new_path /= *rebuild_it;
					++rebuild_it;
				}

				// 3. Assign the new, shorter path back to the original reference.
				p = new_path;
			} else {
				// The path consists only of a root (e.g., "/" or "C:/").
				// There's no folder to snip, so we can clear the path.
				p.clear( );
			}

			return snipped_part;
		}


		std::map<std::type_index, std::any>                 content_caches;
		std::map<std::type_index, std::filesystem::path>    content_caches_path;
		std::map<std::type_index, std::atomic<bool>>        content_caches_dirty;
	}
	
    //void ContentFactory::RegisterItem(const std::string& type, std::unique_ptr<ContentItem> creator)
    //{
    //    m_creators[type] = std::move(creator);
    //}
	//
    //const ContentItem* ContentFactory::Get(const std::string& type) const
    //{
    //    auto it = m_items.find(type);
    //    if (it != m_items.end())
    //    {
    //        return it->second.get();
    //    }
    //    return nullptr;
    //}
	//
	//void TiledContent::load( glz::json_t &json ) const {
	//
	//}

}

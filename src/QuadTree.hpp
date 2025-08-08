#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath> // For std::abs

namespace QuadTree {
	// Represents a 2D point
	struct Point {
		float x, y;
	};

	// Represents an Axis-Aligned Bounding Box (AABB)
	struct AABB {
		Point center;
		Point half_dim;

		bool contains( const Point &p ) const {
			return ( p.x >= center.x - half_dim.x &&
				p.x <= center.x + half_dim.x &&
				p.y >= center.y - half_dim.y &&
				p.y <= center.y + half_dim.y );
		}

		bool intersects( const AABB &other ) const {
			return ( std::abs( center.x - other.center.x ) <= ( half_dim.x + other.half_dim.x ) ) &&
				( std::abs( center.y - other.center.y ) <= ( half_dim.y + other.half_dim.y ) );
		}
	};

	template <typename T>
	class QuadTree {
	private:
		// A node can hold up to 4 objects before it splits.
		static constexpr int MAX_OBJECTS = 4;
		// The deepest level a node can be.
		static constexpr int MAX_LEVELS = 8;

		int level;
		AABB boundary;
		std::vector<std::pair<Point, T>> objects;
		std::unique_ptr<QuadTree<T>> nodes[4];

		// Splits the node into 4 sub-nodes
		void split( ) {
			float sub_width = boundary.half_dim.x / 2.0f;
			float sub_height = boundary.half_dim.y / 2.0f;
			float x = boundary.center.x;
			float y = boundary.center.y;

			// Child node order: 0:SE, 1:SW, 2:NW, 3:NE
			nodes[0] = std::make_unique<QuadTree<T>>( level + 1, AABB{ {x + sub_width, y - sub_height}, {sub_width, sub_height} } );
			nodes[1] = std::make_unique<QuadTree<T>>( level + 1, AABB{ {x - sub_width, y - sub_height}, {sub_width, sub_height} } );
			nodes[2] = std::make_unique<QuadTree<T>>( level + 1, AABB{ {x - sub_width, y + sub_height}, {sub_width, sub_height} } );
			nodes[3] = std::make_unique<QuadTree<T>>( level + 1, AABB{ {x + sub_width, y + sub_height}, {sub_width, sub_height} } );
		}

		// Determine which node the object belongs to. -1 means object cannot completely fit
		// within a child node and is part of the parent node.
		int get_index( const Point &p ) {
			int index = -1;
			double vertical_midpoint = boundary.center.y;
			double horizontal_midpoint = boundary.center.x;

			// Object can completely fit within the top quadrants
			bool top_quadrant = ( p.y > vertical_midpoint );
			// Object can completely fit within the bottom quadrants
			bool bottom_quadrant = ( p.y < vertical_midpoint );

			// Object can completely fit within the left quadrants
			if ( p.x < horizontal_midpoint ) {
				if ( top_quadrant ) {
					index = 2; // Top left
				} else if ( bottom_quadrant ) {
					index = 1; // Bottom left
				}
			}
			// Object can completely fit within the right quadrants
			else if ( p.x > horizontal_midpoint ) {
				if ( top_quadrant ) {
					index = 3; // Top right
				} else if ( bottom_quadrant ) {
					index = 0; // Bottom right
				}
			}
			return index;
		}

	public:
		QuadTree( int p_level, const AABB &p_boundary ) : level( p_level ), boundary( p_boundary ) {
			nodes[0] = nullptr;
			nodes[1] = nullptr;
			nodes[2] = nullptr;
			nodes[3] = nullptr;
		}

		void insert( const Point &p, const T &value ) {
			if ( !boundary.contains( p ) ) {
				return;
			}

			if ( nodes[0] != nullptr ) {
				int index = get_index( p );
				if ( index != -1 ) {
					nodes[index]->insert( p, value );
					return;
				}
			}

			objects.emplace_back( p, value );

			if ( objects.size( ) > MAX_OBJECTS && level < MAX_LEVELS ) {
				if ( nodes[0] == nullptr ) {
					split( );
				}

				size_t i = 0;
				while ( i < objects.size( ) ) {
					int index = get_index( objects[i].first );
					if ( index != -1 ) {
						nodes[index]->insert( objects[i].first, objects[i].second );
						objects.erase( objects.begin( ) + i );
					} else {
						i++;
					}
				}
			}
		}

		std::vector<T> query( const AABB &range ) {
			std::vector<T> found;
			if ( !boundary.intersects( range ) ) {
				return found;
			}

			for ( const auto &obj : objects ) {
				if ( range.contains( obj.first ) ) {
					found.push_back( obj.second );
				}
			}

			if ( nodes[0] != nullptr ) {
				for ( int i = 0; i < 4; ++i ) {
					auto child_found = nodes[i]->query( range );
					found.insert( found.end( ), child_found.begin( ), child_found.end( ) );
				}
			}
			return found;
		}
	};
}
#include "World.h"
#include "tiled_data.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "ShapeFactory.h"
#include "ChainShapeCreator.h"
#include "aseprite_data.h"
#include "ContentFactory.h"
#include "ContentFactory.h"
#include "Sprite.h"

SiegePerilous::WorldState::WorldState( ) : m_isInitialized( false ), m_isRunning( false ), m_debugDraw( nullptr ), m_camera( nullptr ), m_shapeFactory(std::make_unique<ShapeFactory>()) {
	physicsState.worldId = B2_NULL_ID;
	physicsState.groundId = B2_NULL_ID;
	physicsState.bodyId = B2_NULL_ID;
	audioState.stream_in = nullptr;
	audioState.stream_out = nullptr;
	currentTime = 0.0;
	accumulator = 0.0;
}

SiegePerilous::WorldState::~WorldState( ) {
	if ( m_isInitialized ) {
		Shutdown( );
	}

	if ( m_debugDraw ) {
		delete m_debugDraw;
		m_debugDraw = nullptr;
	}
}

void SiegePerilous::WorldState::SetRenderer( SDL_Renderer *renderer ) {
	if ( renderer ) {
		m_debugDraw = new b2SDLDraw( renderer, &m_camera );
		uint32_t flags = 0;
		//flags |= b2_drawShapes;
		//flags |= b2_drawJoints;
		//flags |= b2_drawAABBs;
		//flags |= b2_drawMass;
		//flags |= b2_drawTransforms;
		m_debugDraw->SetFlags( flags );
		m_camera.UpdateRenderer( renderer );
	}
}

bool SiegePerilous::WorldState::Initialise( ) {
	if ( m_isInitialized ) {
		return true; // Already initialized
	}
	Content::content_cache_item<SiegePerilous::AseSprite>( "sprites" );

	b2WorldDef worldDef = b2DefaultWorldDef( );
	worldDef.gravity = b2Vec2( { 0.0f, -10.f } );
	physicsState.worldId = b2CreateWorld( &worldDef );

	//b2BodyDef groundBodyDef = b2DefaultBodyDef( );
	//groundBodyDef.position = b2Vec2( { 0.0f, 0.0f } );
	//physicsState.groundId = b2CreateBody( physicsState.worldId, &groundBodyDef );
	//b2Polygon groundBox = b2MakeOffsetBox( 25.0f, 1.0f, { 0.0f,0.0f }, b2MakeRot( degToRad( 10 ) ) );
	//b2ShapeDef groundShapeDef = b2DefaultShapeDef( );
	//b2CreatePolygonShape( physicsState.groundId, &groundShapeDef, &groundBox );

	b2BodyDef bodyDef = b2DefaultBodyDef( );
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Vec2( { 0.0f, 10.0f } );
	physicsState.bodyId = b2CreateBody( physicsState.worldId, &bodyDef );
	b2Circle circle = { {0.0f, 0.0f}, 0.5f };
	b2ShapeDef shapeDef = b2DefaultShapeDef( );
	shapeDef.density = 1.0f;
	shapeDef.material.friction = 0.3f;
	b2CreateCircleShape( physicsState.bodyId, &shapeDef, &circle );


	m_isInitialized = true;

	currentTime = SDL_GetTicks( ) / 1000.0;
	accumulator = 0.0;

	m_shapeFactory->registerCreator("b2ChainShape", std::make_unique<ChainShapeCreator>());
	//this is stupid, an ase is an sprite!
	//m_shapeFactory->registerCreator( "aseSprite", std::make_unique<AsepriteShapeCreator>( ) );

	m_map = Tiled::load_map_with_deps( "tileMaps/main_menu.json" );

	CreatePhysicsBodiesFromMap();

	std::filesystem::path relPath = fileSystem->RelativeToOSPath( "main_menu.json" );
	std::string relPathStr = relPath.string();
	if ( m_map ) {
		// Iterate through each tileset defined in the map
		for ( const auto &tileset : m_map->tilesets ) {

			// 1. Handle the main tileset image (the spritesheet)
			if ( tileset.image ) {
				std::string image_path = *tileset.image;

				SDL_Surface *surface = IMG_Load( fileSystem->RelativeToOSPath( image_path ).string().c_str() );
				if ( surface ) {
					SDL_Texture *texture = SDL_CreateTextureFromSurface( m_camera.GetRenderer( ), surface );
					if ( texture ) {
						// Store the main tileset texture with its firstgid as the key.
						// This is used for rendering tiles from the spritesheet.
						m_tileset_textures[tileset.firstgid] = texture;
					} else {
						std::cerr << "Failed to create texture from " << image_path << "! SDL_Error: " << SDL_GetError( ) << std::endl;
					}
					SDL_DestroySurface( surface );
				} else {
					std::cerr << "Failed to load tileset image " << image_path << "! IMG_Error: " /*<< IMG_GetError( )*/ << std::endl;
				}
			}

			// 2. Handle individual tile images within the tileset
			if ( tileset.tiles ) {
				for ( const auto &tile : *tileset.tiles ) {

					// This tile has an animation defined.
					if ( tile.animation ) {
						uint32_t base_gid = tileset.firstgid + tile.id;

						// 1. Store the animation definition.
						Tiled::AnimationDefinition def;
						def.base_gid = base_gid;
						def.frames = *tile.animation;
						m_map->m_animation_definitions[base_gid] = def;

						// 2. Create an active state for this animation.
						Tiled::ActiveAnimationState state;
						state.definition = &m_map->m_animation_definitions[base_gid];
						m_map->m_active_animations[base_gid] = state;
					}
					
					if ( tile.image ) {
						// This specific tile has its own image file.
						std::string image_path = *tile.image;

						if ( fileSystem->RelativeToOSPath( image_path ).extension( ) == ".aseprite" 
							||fileSystem->RelativeToOSPath( image_path ).extension( ) == ".ase" )
						{
							
							//move to sprite.h as LoadExternalTileSourceASE( const std::string &image_path )
							auto spritePath = fs::path( image_path );
							spritePath.replace_extension( "" ); // Aseprite files are usually exported to PNGs
							spritePath = spritePath / "sprite.json";

							auto spriteX = SiegePerilous::Content::Load<AseSprite>( spritePath );
							auto spriteY = SiegePerilous::Content::Load<AseSprite>( spritePath );
							std::cout << ">>   Found external tile source : Tile[" << tile.id << "] Loading from '" << fileSystem->RelativeToOSPath( spritePath ) << "'" << std::endl;

							size_t map_file_size = 0;
							auto sprite_buffer_data = fileSystem->ReadFile( spritePath );
							ASE::aseprite_file_t sprite;
							if ( auto &buffer = sprite_buffer_data ) {
								auto err = glz::read < glz::opts{ .error_on_unknown_keys = false } > ( sprite, *buffer );
								if ( err ) {
									std::cerr << "Error: Failed to sprite JSON from '" << spritePath << "'." << std::endl;
									continue;
								}
								std::cout << "     Successfully loaded and parsed '" << spritePath << "' (" << buffer->size( ) << " bytes)." << std::endl;
							}
							for (auto & layer : sprite.layers)
							{
								int cellIdx = 1;
								for ( auto &cell : layer.cells ) {
									
									if (cell.data)
									{
										std::string &data_string = *cell.data;
										ASE::cell_user_data_physics_t celldata;

										auto error = glz::read_json( celldata, data_string );
										if ( error ) {
											std::cerr << "     Error parsing intermediate physics data in cell ["<< cellIdx <<"] : "
												<< glz::format_error( error, data_string ) << std::endl;
											cellIdx ++;
											continue;
										}

										if ( celldata.polygonCollisionShape ) {
											std::cout << "     polygonCollisionShape with [" << celldata.polygonCollisionShape.value().size() << "] points found for  '" << spritePath << " in cell " << cellIdx << std::endl;
											//process polyshape with a factory entry.
										}
									}

									cellIdx ++;
								}
							}
						}

						else if ( SDL_Surface *surface = IMG_Load( fileSystem->RelativeToOSPath( image_path ).string( ).c_str( ) ) ) {
							SDL_Texture *texture = SDL_CreateTextureFromSurface( m_camera.GetRenderer( ), surface );
							if ( texture ) {
								// Calculate the Global ID (GID) for this tile
								uint32_t gid = tileset.firstgid + tile.id;
								// Store the individual tile texture using its GID as the key.
								m_tileset_textures[gid] = texture;
							} else {
								std::cerr << "Failed to create texture for tile from " << image_path << "! SDL_Error: " << SDL_GetError( ) << std::endl;
							}
							SDL_DestroySurface( surface );							
						} else {
							std::cerr << "Failed to load individual tile image " << image_path << "! IMG_Error: " /*<< IMG_GetError( ) */<< std::endl;
						}
					}
				}
			}
		}
	}

	return true;
}

void SiegePerilous::WorldState::Shutdown( ) {
	if ( m_isInitialized ) {
		b2DestroyWorld( physicsState.worldId );
		delete m_debugDraw;
		m_debugDraw = nullptr;

		for ( auto const &[key, val] : m_tileset_textures ) {
			SDL_DestroyTexture( val );
		}
		m_tileset_textures.clear( );

		m_isInitialized = false;
	}
}

void SiegePerilous::WorldState::Update( ) {
	double newTime = SDL_GetTicks( ) / 1000.0;
	double frameTime = newTime - currentTime;
	currentTime = newTime;
	accumulator += frameTime;

	if (m_map)
	{
		double frameTime_ms = frameTime * 1000;
		for ( auto &[gid, anim_state] : m_map->m_active_animations ) {
			anim_state.time_accumulator_ms += frameTime_ms;

			const auto &current_frame_def = anim_state.definition->frames[anim_state.current_frame_index];

			// Check if it's time to advance to the next frame.
			if ( anim_state.time_accumulator_ms >= current_frame_def.duration ) {
				anim_state.time_accumulator_ms -= current_frame_def.duration;
				anim_state.current_frame_index++;

				// Loop the animation --> only if defined, check tile data for this!!!!
				if ( anim_state.current_frame_index >= anim_state.definition->frames.size( ) ) {
					anim_state.current_frame_index = 0;
				}
			}
		}
	}

	const float timeStep = 1.0f / 30.0;
	while ( accumulator >= timeStep ) {
		b2World_Step( physicsState.worldId, timeStep, 4 );
		accumulator -= timeStep;
	}
}

void SiegePerilous::WorldState::Draw( ) {
	if ( m_map ) {
		SDL_Renderer *renderer = m_camera.GetRenderer( );
		for ( auto &layerRefPtr : m_map->GetLayersOfType( "tilelayer", true, true ) ) {
			Tiled::Layer &layer = *layerRefPtr;
			if ( layer.type == "tilelayer" && layer.visible && layer.width.has_value( ) && layer.height.has_value( ) ) {
				for ( int y = 0; y < *layer.height; ++y ) {
					for ( int x = 0; x < *layer.width; ++x ) {
						uint32_t raw_gid = layer.decoded_data[y * ( *layer.width ) + x];
						if ( raw_gid == 0 ) {
							continue;
						}

						static const uint32_t FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
						static const uint32_t FLIPPED_VERTICALLY_FLAG = 0x40000000;
						static const uint32_t FLIPPED_DIAGONALLY_FLAG = 0x20000000;
						static const uint32_t ALL_FLAGS_MASK = ( FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG );

						// 1. Extract flags and clean the GID
						const bool flipped_horizontally = ( raw_gid & FLIPPED_HORIZONTALLY_FLAG );
						const bool flipped_vertically = ( raw_gid & FLIPPED_VERTICALLY_FLAG );
						const bool flipped_diagonally = ( raw_gid & FLIPPED_DIAGONALLY_FLAG );
						const uint32_t gid = raw_gid & ~ALL_FLAGS_MASK; // Use this clean GID for lookups



						//////////////////////////////////////////////////////////////////////////
						///tile animation
						uint32_t render_gid = gid;

						// Check if the tile from the map is a base tile for an animation.
						auto anim_it = m_map->m_active_animations.find( gid );
						if ( anim_it != m_map->m_active_animations.end( ) ) {
							// It is an animated tile. Get the GID of the current frame.
							const Tiled::ActiveAnimationState &anim_state = anim_it->second;
							const Tiled::Frame &current_frame = anim_state.definition->frames[anim_state.current_frame_index];

							// The tileset's firstgid is needed to resolve the local tileid to a global id.
							const Tiled::Tileset *found_tileset = nullptr;
							for ( const auto &ts : m_map->tilesets ) {
								if ( gid >= ts.firstgid ) {
									if ( !found_tileset || ts.firstgid > found_tileset->firstgid ) {
										found_tileset = &ts;
									}
								}
							}
							if ( found_tileset ) {
								render_gid = found_tileset->firstgid + current_frame.tileid;
							}
						}
						//////////////////////////////////////////////////////////////////////////
						
						// 2. Determine the SDL_RendererFlip value and rotation angle.
						// Tiled's diagonal flip is a 90-degree rotation plus a horizontal flip.
						// The other flags are applied on top of that.
						SDL_FlipMode flip = SDL_FLIP_NONE;
						double angle = 0.0;

						if (flipped_diagonally) {
							angle = 90.0;
							flip = SDL_FLIP_HORIZONTAL;
						}
						if (flipped_horizontally) {
							// XOR the flip flag. If it was already set by the diagonal flip, it will be cleared, and vice-versa.
							flip = (SDL_FlipMode)(flip ^ SDL_FLIP_HORIZONTAL);
						}
						if (flipped_vertically) {
							flip = (SDL_FlipMode)(flip ^ SDL_FLIP_VERTICAL);
						}

						// The destination rectangle on the screen
						SDL_FRect dest_rect = {
							x * m_map->tilewidth + layer.offsetx,
							y * m_map->tileheight + layer.offsety,
							m_map->tilewidth ,
							m_map->tileheight
						};

						// Case 1: Check if this GID corresponds to an individual tile image
						auto it = m_tileset_textures.find( render_gid );
						if ( it != m_tileset_textures.end( ) ) {
							SDL_Texture *texture = it->second;
							
							//hmmm i dont remeber why i need to do this only on Y..
							//dest_rect.x +=  -texture->w + m_map->tilewidth;
							dest_rect.y += -texture->h + m_map->tileheight;

							dest_rect.w = texture->w;
							dest_rect.h = texture->h;
							SDL_RenderTexture( renderer, texture, nullptr, &dest_rect );
						}
						// Case 2: It's a tile from a larger tileset spritesheet
						else {
							const Tiled::Tileset *found_tileset = nullptr;
							for ( const auto &ts : m_map->tilesets ) {
								
								SDL_assert(ts.firstgid >= 0);

								if ( gid >= ts.firstgid ) {
									if ( !found_tileset || ts.firstgid > found_tileset->firstgid ) {
										found_tileset = &ts;
									}
								}
							}

							if ( found_tileset ) {
								auto ts_it = m_tileset_textures.find( found_tileset->firstgid );
								if ( ts_it != m_tileset_textures.end( ) ) {
									SDL_Texture *texture = ts_it->second;

									uint32_t local_id = render_gid - found_tileset->firstgid;
									int tile_width = found_tileset->tilewidth.value_or( 0 );
									int tile_height = found_tileset->tileheight.value_or( 0 );
									int columns = found_tileset->columns.value_or( 1 );

									if ( columns > 0 && tile_width > 0 && tile_height > 0 ) {

										SDL_FRect src_rect;

										src_rect.x = static_cast<float>(( local_id % columns ) * tile_width );
										src_rect.y = static_cast<float>(( local_id / columns ) * tile_height );
										src_rect.w = static_cast<float>( tile_width );
										src_rect.h = static_cast<float>( tile_height );

										SDL_RenderTextureRotated(renderer, texture, &src_rect, &dest_rect, angle, nullptr, flip);
									}
								}
							}
						}
					}
				}
			}
		}
	}


	if ( m_debugDraw ) {
		b2World_Draw( physicsState.worldId, m_debugDraw->getDebugDrawPtr( ) );
	}
}

void SiegePerilous::WorldState::Start( ) {
	m_isRunning = true;
}

void SiegePerilous::WorldState::Stop( ) {
	m_isRunning = false;
}

void SiegePerilous::WorldState::CreatePhysicsBodiesFromMap()
{
    if (!m_map)
    {
        return;
    }

    for (const auto &layer : m_map->GetLayersOfType("objectgroup", true, true))
    {
		bool isPhysicsLayer = layer->class_property.value_or( ""  ) == "physics";

		if ( isPhysicsLayer ) {
			if ( layer->objects ) {
				for ( const auto &object : *layer->objects ) {
					const PhysicsShapeCreator *creator = m_shapeFactory->create( object.type );
					if ( creator ) {
						// Create the physics body using the creator
						creator->create( physicsState.worldId, object,layer->offsetx,layer->offsety);
					}
				}
			}
		}
    }
}
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_assert.h>
#include "SiegePerilous.h"
#include "tiled_data.h"
#include "World.h"
#include "FileSystem.h"

static SDL_Window *window		= nullptr;
static SDL_Renderer *renderer	= nullptr;

static std::string config_path = "../../../config.json";

static SiegePerilous::WorldState worldState;

static unsigned int WINDOW_SIZE_X = 1920;
static unsigned int WINDOW_SIZE_Y = 1080;

SDL_AppResult SDL_AppInit( void **appstate, int argc, char **argv ) {
	SDL_SetHint( SDL_HINT_MAIN_CALLBACK_RATE, "60" );

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	//////////////////////////////////////////////////////////////////////////
	//Load config
	size_t config_file_size = 0;
	char *config_buffer = ( char * ) SDL_LoadFile( config_path.c_str( ), &config_file_size );

	if ( !config_buffer ) {
		std::cerr << "Error: Could not load config file '" << config_path << "'. SDL_Error: " << SDL_GetError( ) << std::endl;
		return SDL_APP_FAILURE;
	}
	std::cout << "Successfully loaded '" << config_path << "' (" << config_file_size << " bytes)." << std::endl;

	//This should turn into GlobalConfig struct which contains all the global settings for the game, and the file system config
	FileSystemConfig fileSystemConfig;
	auto err = glz::read < glz::opts{ .error_on_unknown_keys = false } > ( fileSystemConfig, std::string_view( config_buffer, config_file_size ) );
	
	SDL_assert( fileSystem );

	fileSystem->Init(fileSystemConfig.basePath,fileSystemConfig.savePath,
					fileSystemConfig.mainGameName,fileSystemConfig.baseGameName );
	//////////////////////////////////////////////////////////////////////////

	SDL_AudioDeviceID *devices;
	SDL_AudioSpec outspec;
	SDL_AudioSpec inspec;
	SDL_AudioDeviceID device;
	SDL_AudioDeviceID want_device = SDL_AUDIO_DEVICE_DEFAULT_RECORDING;
	const char *devname = NULL;
	int i;

	if ( argc > 2 ) {
		devname = argv[2];
	}

	SDL_Log( "Using audio driver: %s", SDL_GetCurrentAudioDriver( ) );

	devices = SDL_GetAudioRecordingDevices( NULL );
	for ( i = 0; devices[i] != 0; i++ ) {
		const char *name = SDL_GetAudioDeviceName( devices[i] );
		SDL_Log( " Recording device #%d: '%s'", i, name );
		if ( devname && ( SDL_strcmp( devname, name ) == 0 ) ) {
			want_device = devices[i];
		}
	}

	if ( devname && ( want_device == SDL_AUDIO_DEVICE_DEFAULT_RECORDING ) ) {
		SDL_LogWarn( SDL_LOG_CATEGORY_APPLICATION, "Didn't see a recording device named '%s', using the system default instead.", devname );
		devname = NULL;
	}

	SDL_Log( "Opening default playback device..." );
	device = SDL_OpenAudioDevice( SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL );
	if ( !device ) {
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't open an audio device for playback: %s!", SDL_GetError( ) );
		SDL_free( devices );
		return SDL_APP_FAILURE;
	}
	SDL_PauseAudioDevice( device );
	SDL_GetAudioDeviceFormat( device, &outspec, NULL );
	worldState.audioState.stream_out = SDL_CreateAudioStream( &outspec, &outspec );
	if ( !worldState.audioState.stream_out ) {
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't create an audio stream for playback: %s!", SDL_GetError( ) );
		SDL_free( devices );
		return SDL_APP_FAILURE;
	} else if ( !SDL_BindAudioStream( device, worldState.audioState.stream_out ) ) {
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't bind an audio stream for playback: %s!", SDL_GetError( ) );
		SDL_free( devices );
		return SDL_APP_FAILURE;
	}

	SDL_Log( "Opening recording device %s%s%s...",
		devname ? "'" : "",
		devname ? devname : "[[default]]",
		devname ? "'" : "" );

	device = SDL_OpenAudioDevice( want_device, NULL );
	if ( !device ) {
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't open an audio device for recording: %s!", SDL_GetError( ) );
		SDL_free( devices );
		return SDL_APP_FAILURE;
	}
	SDL_free( devices );
	SDL_PauseAudioDevice( device );
	SDL_GetAudioDeviceFormat( device, &inspec, NULL );
	worldState.audioState.stream_in = SDL_CreateAudioStream( &inspec, &inspec );
	if ( !worldState.audioState.stream_in ) {
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't create an audio stream for recording: %s!", SDL_GetError( ) );
		return SDL_APP_FAILURE;
	} else if ( !SDL_BindAudioStream( device, worldState.audioState.stream_in ) ) {
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't bind an audio stream for recording: %s!", SDL_GetError( ) );
		return SDL_APP_FAILURE;
	}

	SDL_SetAudioStreamFormat( worldState.audioState.stream_in, NULL, &outspec );

	if ( !SDL_CreateWindowAndRenderer( "Siege Perilous", WINDOW_SIZE_X, WINDOW_SIZE_Y, SDL_WINDOW_RESIZABLE, &window, &renderer ) ) {
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't create SDL window and renderer: %s", SDL_GetError( ) );
		return SDL_APP_FAILURE;
	}

	worldState.SetRenderer( renderer );
	worldState.Initialise( );
	worldState.Start( );

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate( void *appstate ) {
	worldState.Update( );

	Uint64 ticks = SDL_GetTicks( );
	//Uint8 r = (ticks / 20) % 255;
	//Uint8 g = (ticks / 20 + 85) % 255;
	//Uint8 b = (ticks / 20 + 170) % 255;
	SDL_SetRenderDrawColor( renderer, 128, 0, 128, 255 );
	SDL_RenderClear( renderer );

	worldState.Draw( );

	SDL_RenderPresent( renderer );

	while ( SDL_GetAudioStreamAvailable( worldState.audioState.stream_in ) > 0 ) {
		Uint8 buf[1024];
		const int br = SDL_GetAudioStreamData( worldState.audioState.stream_in, buf, sizeof( buf ) );
		if ( br < 0 ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Failed to read from input audio stream: %s", SDL_GetError( ) );
			return SDL_APP_FAILURE;
		} else if ( !SDL_PutAudioStreamData( worldState.audioState.stream_out, buf, br ) ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Failed to write to output audio stream: %s", SDL_GetError( ) );
			return SDL_APP_FAILURE;
		}
	}

	if ( !worldState.IsRunning( ) ) {
		return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent( void *appstate, SDL_Event *event ) {
	if ( event->type == SDL_EVENT_WINDOW_RESIZED ) {
		worldState.GetCamera( ).UpdateRenderer( renderer );
	}
	if ( event->type == SDL_EVENT_QUIT ) {
		worldState.Stop( );
	} else if ( event->type == SDL_EVENT_KEY_DOWN ) {
		if ( event->key.key == SDLK_ESCAPE ) {
			worldState.Stop( );
		}
	} else if ( event->type == SDL_EVENT_MOUSE_BUTTON_DOWN ) {
		if ( event->button.button == 1 ) {
			SDL_PauseAudioStreamDevice( worldState.audioState.stream_out );
			SDL_FlushAudioStream( worldState.audioState.stream_out );
			SDL_ResumeAudioStreamDevice( worldState.audioState.stream_in );
		}
	} else if ( event->type == SDL_EVENT_MOUSE_BUTTON_UP ) {
		if ( event->button.button == 1 ) {
			SDL_PauseAudioStreamDevice( worldState.audioState.stream_in );
			SDL_FlushAudioStream( worldState.audioState.stream_in );
			SDL_ResumeAudioStreamDevice( worldState.audioState.stream_out );
		}
	}
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit( void *appstate, SDL_AppResult result ) {
	const SDL_AudioDeviceID devid_in = SDL_GetAudioStreamDevice( worldState.audioState.stream_in );
	const SDL_AudioDeviceID devid_out = SDL_GetAudioStreamDevice( worldState.audioState.stream_out );
	SDL_CloseAudioDevice( devid_in );
	SDL_CloseAudioDevice( devid_out );
	SDL_DestroyAudioStream( worldState.audioState.stream_in );
	SDL_DestroyAudioStream( worldState.audioState.stream_out );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	worldState.Shutdown( );
	SDL_Quit( );
}
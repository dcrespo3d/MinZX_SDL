#include <iostream>
#include <iomanip>
#include <vector>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

#include "SDL.h"
#include "minzx.h"

int main(int argc, char *argv[])
{
	MinZX minZX;
	minZX.init();

	SDL_Init(SDL_INIT_EVERYTHING);

	unsigned vsyncFlag = 0;
//	unsigned vsyncFlag = SDL_RENDERER_PRESENTVSYNC;

	SDL_Window* window = SDL_CreateWindow
		(
		"SDL2",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1280, 960,
		SDL_WINDOW_SHOWN
		);

	SDL_Renderer* renderer = SDL_CreateRenderer
		(
		window,
		-1,
		SDL_RENDERER_ACCELERATED | vsyncFlag
		);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	std::cout << "Renderer name: " << info.name << std::endl;
	std::cout << "Texture formats: " << std::endl;
	for (Uint32 i = 0; i < info.num_texture_formats; i++)
	{
		std::cout << SDL_GetPixelFormatName(info.texture_formats[i]) << std::endl;
	}

	const unsigned int texWidth = 320;
	const unsigned int texHeight = 240;
	SDL_Texture* texture = SDL_CreateTexture
		(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		texWidth, texHeight
		);

	std::vector< unsigned char > pixels(texWidth * texHeight * 4, 0);

	SDL_Event event;
	bool running = true;
	bool useLocktexture = false;

	unsigned int frames = 0;
	Uint64 start = SDL_GetPerformanceCounter();
	
	while (running)
	{

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		while (SDL_PollEvent(&event))
		{
			if ((SDL_QUIT == event.type) ||
				(SDL_KEYDOWN == event.type && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode))
			{
				running = false;
				break;
			}
			if (SDL_KEYDOWN == event.type && SDL_SCANCODE_L == event.key.keysym.scancode)
			{
				useLocktexture = !useLocktexture;
				std::cout << "Using " << (useLocktexture ? "SDL_LockTexture() + memcpy()" : "SDL_UpdateTexture()") << std::endl;
			}
		}

		// splat down some random pixels
		for (unsigned int i = 0; i < 1000; i++)
		{
			const unsigned int x = rand() % texWidth;
			const unsigned int y = rand() % texHeight;

			const unsigned int offset = (texWidth * 4 * y) + x * 4;
			pixels[offset + 0] = rand() % 256;        // b
			pixels[offset + 1] = rand() % 256;        // g
			pixels[offset + 2] = rand() % 256;        // r
			pixels[offset + 3] = SDL_ALPHA_OPAQUE;    // a
		}

		minZX.update(&pixels[0]);

		if (useLocktexture)
		{
			unsigned char* lockedPixels = nullptr;
			int pitch = 0;
			SDL_LockTexture
				(
				texture,
				NULL,
				reinterpret_cast< void** >(&lockedPixels),
				&pitch
				);
			std::memcpy(lockedPixels, pixels.data(), pixels.size());
			SDL_UnlockTexture(texture);
		}
		else
		{
			SDL_UpdateTexture
				(
				texture,
				NULL,
				pixels.data(),
				texWidth * 4
				);
		}

		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		SDL_Delay(19);

		frames++;
		const Uint64 end = SDL_GetPerformanceCounter();
		const static Uint64 freq = SDL_GetPerformanceFrequency();
		const double seconds = (end - start) / static_cast< double >(freq);
		if (seconds > 2.0)
		{
			std::cout
				<< frames << " frames in "
				<< std::setprecision(1) << std::fixed << seconds << " seconds = "
				<< std::setprecision(1) << std::fixed << frames / seconds << " FPS ("
				<< std::setprecision(3) << std::fixed << (seconds * 1000.0) / frames << " ms/frame)"
				<< std::endl;
			start = end;
			frames = 0;
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

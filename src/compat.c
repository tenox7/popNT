#if defined(_MSC_VER) && (_MSC_VER < 1200)
#define __forceinline __inline
#endif
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_GIF
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_TGA
#include "stb_image.h"

#include "common.h"
#include <math.h>

extern void debug_log(const char* msg);

static SDL_Surface* create_surface_from_pixels(unsigned char* pixels, int w, int h, int channels) {
	SDL_Surface* surface;
	Uint32 rmask, gmask, bmask, amask;
	int bpp;
	int y;

	if (!pixels) return NULL;

	bpp = channels * 8;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000 >> (8 * (4 - channels));
	gmask = 0x00ff0000 >> (8 * (4 - channels));
	bmask = 0x0000ff00 >> (8 * (4 - channels));
	amask = (channels == 4) ? 0x000000ff : 0;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = (channels == 4) ? 0xff000000 : 0;
#endif

	surface = SDL_CreateRGBSurface(0, w, h, bpp, rmask, gmask, bmask, amask);
	if (!surface) {
		stbi_image_free(pixels);
		return NULL;
	}

	SDL_LockSurface(surface);
	for (y = 0; y < h; y++) {
		memcpy((Uint8*)surface->pixels + y * surface->pitch,
		       pixels + y * w * channels,
		       w * channels);
	}
	SDL_UnlockSurface(surface);
	stbi_image_free(pixels);
	return surface;
}

SDL_Surface* IMG_Load(const char* file) {
	int w, h, channels;
	unsigned char* pixels;
	SDL_RWops* rw;
	Sint64 size;
	unsigned char* buf;
	size_t bytesRead;

	rw = SDL_RWFromFile(file, "rb");
	if (!rw) return SDL_LoadBMP(file);

	size = SDL_RWsize(rw);
	if (size <= 0) { SDL_RWclose(rw); return NULL; }

	buf = (unsigned char*)malloc((size_t)size);
	if (!buf) { SDL_RWclose(rw); return NULL; }

	bytesRead = SDL_RWread(rw, buf, 1, (size_t)size);
	SDL_RWclose(rw);

	pixels = stbi_load_from_memory(buf, (int)bytesRead, &w, &h, &channels, 0);
	free(buf);
	return create_surface_from_pixels(pixels, w, h, channels);
}

SDL_Surface* IMG_Load_RW(SDL_RWops* src, int freesrc) {
	int w, h, channels;
	unsigned char* pixels;
	Sint64 size;
	unsigned char* buf;
	size_t bytesRead;

	if (!src) return NULL;

	size = SDL_RWsize(src);
	if (size <= 0) {
		Sint64 pos = SDL_RWtell(src);
		SDL_RWseek(src, 0, RW_SEEK_END);
		size = SDL_RWtell(src);
		SDL_RWseek(src, pos, RW_SEEK_SET);
	}
	if (size <= 0) size = 65536;

	buf = (unsigned char*)malloc((size_t)size);
	if (!buf) { if (freesrc) SDL_RWclose(src); return NULL; }

	bytesRead = SDL_RWread(src, buf, 1, (size_t)size);
	if (freesrc) SDL_RWclose(src);

	pixels = stbi_load_from_memory(buf, (int)bytesRead, &w, &h, &channels, 0);
	free(buf);
	return create_surface_from_pixels(pixels, w, h, channels);
}

int IMG_SavePNG(SDL_Surface* surface, const char* file) {
	return SDL_SaveBMP(surface, file);
}

/* SDL Audio stubs - NT4 SDL2 port has no audio subsystem */
int SDL_OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained) {
	if (obtained) memcpy(obtained, desired, sizeof(*desired));
	return -1;
}
void SDL_PauseAudio(int pause_on) { (void)pause_on; }
void SDL_LockAudio(void) {}
void SDL_UnlockAudio(void) {}
void SDL_CloseAudio(void) {}

/* SDL Haptic stubs */
int SDL_HapticRumbleInit(SDL_Haptic* haptic) { (void)haptic; return -1; }
int SDL_HapticRumblePlay(SDL_Haptic* haptic, float strength, Uint32 length) {
	(void)haptic; (void)strength; (void)length; return -1;
}
void SDL_HapticClose(SDL_Haptic* haptic) { (void)haptic; }
SDL_Haptic* SDL_HapticOpen(int device_index) { (void)device_index; return NULL; }
int SDL_HapticRumbleSupported(SDL_Haptic* haptic) { (void)haptic; return 0; }
SDL_Haptic* SDL_HapticOpenFromJoystick(SDL_Joystick* joystick) { (void)joystick; return NULL; }
int SDL_JoystickIsHaptic(SDL_Joystick* joystick) { (void)joystick; return 0; }

/* SDL Joystick stubs */
int SDL_NumJoysticks(void) { return 0; }
SDL_Joystick* SDL_JoystickOpen(int index) { (void)index; return NULL; }
void SDL_JoystickClose(SDL_Joystick* joystick) { (void)joystick; }
const char* SDL_JoystickName(SDL_Joystick* joystick) { (void)joystick; return ""; }
int SDL_JoystickNumAxes(SDL_Joystick* joystick) { (void)joystick; return 0; }

/* SDL GameController stubs */
SDL_bool SDL_IsGameController(int index) { (void)index; return SDL_FALSE; }
SDL_GameController* SDL_GameControllerOpen(int index) { (void)index; return NULL; }
void SDL_GameControllerClose(SDL_GameController* controller) { (void)controller; }
SDL_GameController* SDL_GameControllerFromInstanceID(SDL_JoystickID id) { (void)id; return NULL; }
int SDL_GameControllerAddMappingsFromRW(SDL_RWops* rw, int freerw) {
	(void)rw; (void)freerw; return 0;
}
const char* SDL_GameControllerName(SDL_GameController* controller) { (void)controller; return ""; }

/* SDL Joystick/GameController Rumble stubs */
int SDL_JoystickRumble(SDL_Joystick* j, Uint16 lo, Uint16 hi, Uint32 ms) {
	(void)j; (void)lo; (void)hi; (void)ms; return -1;
}
int SDL_GameControllerRumble(SDL_GameController* c, Uint16 lo, Uint16 hi, Uint32 ms) {
	(void)c; (void)lo; (void)hi; (void)ms; return -1;
}

/* strnlen - not in MSVC 4.0 CRT */
size_t strnlen(const char* s, size_t maxlen) {
	size_t len = 0;
	while (len < maxlen && s[len]) len++;
	return len;
}

/* strtoimax - C99 function, use strtol */
long strtoimax(const char* nptr, char** endptr, int base) {
	return strtol(nptr, endptr, base);
}

/* log2f - not in MSVC 4.0 */
float log2f(float x) {
	return (float)(log((double)x) / log(2.0));
}

/* powf - not in MSVC 4.0 */
float powf(float base, float exponent) {
	return (float)pow((double)base, (double)exponent);
}

/* WinMain entry point - SDL2 on NT4 doesn't provide SDL2main */
#ifdef _WIN32
#include <windows.h>

int main(int argc, char* argv[]);

extern void debug_log(const char* msg);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
	(void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
	debug_log("WinMain entered");
	return main(__argc, __argv);
}
#endif


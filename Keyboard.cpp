#include "Keyboard.h"

SDL_Keycode Keyboard::KeyMapping[36] = {
		SDLK_TAB,
		 SDLK_1,
		SDLK_q,
		 SDLK_2,
		SDLK_w,
		SDLK_e,
		 SDLK_4,
		SDLK_r,
		 SDLK_5,
		SDLK_t,
		 SDLK_6,
		SDLK_y,
		SDLK_u,
		 SDLK_8,
		SDLK_i,
		 SDLK_9,
		SDLK_o,
		SDLK_p,
		 SDLK_MINUS,
		SDLK_LEFTBRACKET,
		 SDLK_EQUALS,
		SDLK_RIGHTBRACKET,
		 SDLK_BACKSPACE,
		SDLK_BACKSLASH,
		SDLK_z,
		 SDLK_s,
		SDLK_x,
		 SDLK_d,
		SDLK_c,
		SDLK_v,
		 SDLK_g,
		SDLK_b,
		 SDLK_h,
		SDLK_n,
		 SDLK_j,
		SDLK_m,
};

void DrawRectWithBorder(SDL_Renderer* renderer, SDL_Rect area, int t, SDL_Colour inner, SDL_Colour border) {
	SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
	SDL_RenderFillRect(renderer, &area);

	// Border is bigger than the inside of the rectangle
	if (2 * t > area.w || 2 * t > area.h) return;

	SDL_Rect innerArea = { area.x + t, area.y + t, area.w - 2 * t, area.h - 2 * t };
	SDL_SetRenderDrawColor(renderer, inner.r, inner.g, inner.b, inner.a);
	SDL_RenderFillRect(renderer, &innerArea);
}
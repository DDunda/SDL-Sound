#include "IntField.h"

extern Uint32 currentTime;

static std::map<SDL_Keycode, char> numberKeymapping
{
	{SDLK_0,'0'}, // Regular numbers
	{SDLK_1,'1'},
	{SDLK_2,'2'},
	{SDLK_3,'3'},
	{SDLK_4,'4'},
	{SDLK_5,'5'},
	{SDLK_6,'6'},
	{SDLK_7,'7'},
	{SDLK_8,'8'},
	{SDLK_9,'9'},

	{SDLK_KP_0,'0'}, // Keypad numbers
	{SDLK_KP_1,'1'},
	{SDLK_KP_2,'2'},
	{SDLK_KP_3,'3'},
	{SDLK_KP_4,'4'},
	{SDLK_KP_5,'5'},
	{SDLK_KP_6,'6'},
	{SDLK_KP_7,'7'},
	{SDLK_KP_8,'8'},
	{SDLK_KP_9,'9'}
};

void IntField::setAnchor(float aX, float aY) {
	float posX = clickArea.x + clickArea.w * anchorX;
	float posY = clickArea.y + clickArea.h * anchorY;

	anchorX = aX;
	anchorY = aY;

	setPosition(posX, posY);
}

void IntField::setPosition(int x, int y) {
	clickArea.x = x - clickArea.w * anchorX;
	clickArea.y = y - clickArea.h * anchorY;
}

void IntField::recalculateArea() {
	float posX = clickArea.x + clickArea.w * anchorX;
	float posY = clickArea.y + clickArea.h * anchorY;

	clickArea.w = visibleCharacters * dstDigitSize + (visibleCharacters - 1) * digitGap + pad.left + pad.right;
	clickArea.h = dstDigitSize + pad.top + pad.bottom;

	setPosition(posX, posY);
}

void IntField::focus() {
	infocus = true;
	flashCycleStart = currentTime;
}

void IntField::unfocus() {
	infocus = false;

	if (!capturedData.empty()) {
		try
		{
			output = std::stoi(capturedData);
		}
		catch (const std::out_of_range&)
		{
			output = INT_MAX;
			capturedData = std::to_string(output);

			if (caret > capturedData.size())
				caret = capturedData.size();
		}
	}
}

void IntField::update() {
	if (infocus) {
		for (auto keypair : globalKeyboard.keys_keycode) {
			if (keyPressed(keypair.first, &globalKeyboard)) {
				if (numberKeymapping.count(keypair.first)) { // Key just corresponds to a character - no special behaviour
					if (maxData == -1 || capturedData.size() < maxData) {
						if (caret == capturedData.size())
							capturedData += numberKeymapping[keypair.first];
						else
							capturedData.insert(caret, 1, numberKeymapping[keypair.first]);
						flashCycleStart = currentTime;
						caret++;
					}
				}
				else { // Special characters
					switch (keypair.first)
					{
					case SDLK_BACKSPACE:
						if (caret != 0) {
							capturedData.erase(--caret, 1);
							flashCycleStart = currentTime;
						}
						break;

					case SDLK_DELETE:
						if (caret != capturedData.size()) capturedData.erase(caret, 1);
						break;

					case SDLK_LEFT:
						if (caret != 0) {
							caret--;
							flashCycleStart = currentTime;
						}
						break;

					case SDLK_RIGHT:
						if (caret != capturedData.size()) {
							caret++;
							flashCycleStart = currentTime;
						}
						break;

					case SDLK_HOME:
						if (caret != 0) flashCycleStart = currentTime;
						caret = 0;
						break;

					case SDLK_END:
						if (caret != capturedData.size()) flashCycleStart = currentTime;
						caret = capturedData.size();
						break;

					case SDLK_RETURN:
						caret = capturedData.size();
						unfocus();
						break;
					}
				}
			}
		}
	}
}

void IntField::render(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, 45, 40, 38, 255);
	SDL_RenderFillRect(renderer, &clickArea);
	SDL_SetRenderDrawColor(renderer, 206, 228, 234, 255);
	SDL_RenderDrawRect(renderer, &clickArea);

	int start = capturedData.size() - visibleCharacters;
	if (start < 0) start = 0;

	for (int i = start; i < capturedData.size(); i++) {
		SDL_Rect src{
			(capturedData[i] - '0') * srcDigitSize,
			0,
			srcDigitSize,
			srcDigitSize
		};
		SDL_Rect dst{
			clickArea.x + pad.left + (i - start) * (dstDigitSize + digitGap),
			clickArea.y + pad.top,
			dstDigitSize,
			dstDigitSize
		};
		SDL_RenderCopy(renderer, digits, &src, &dst);
	}
	if (infocus && (currentTime - flashCycleStart) % (flashCycle * 2) < flashCycle) {
		if (caret >= visibleCharacters) {
			SDL_RenderDrawLine(renderer,
				clickArea.x + pad.left + (dstDigitSize + digitGap) * visibleCharacters - digitGap / 2,
				clickArea.y + pad.top,
				clickArea.x + pad.left + (dstDigitSize + digitGap) * visibleCharacters - digitGap / 2,
				clickArea.y + pad.top + dstDigitSize
			);
		}
		else {
			SDL_RenderDrawLine(renderer,
				clickArea.x + pad.left + (dstDigitSize + digitGap) * caret - digitGap / 2,
				clickArea.y + pad.top,
				clickArea.x + pad.left + (dstDigitSize + digitGap) * caret - digitGap / 2,
				clickArea.y + pad.top + dstDigitSize
			);
		}
	}
}
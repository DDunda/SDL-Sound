#pragma once
#include<SDL.h>

#include "Input.h"
#include "Sound.h"
#include "AbstractedAccess.h"
#include "RenderableElement.h"

extern SDL_Rect waveformDrawArea;

class Keyboard;

class Key : public Source<float> {
protected:
	Keyboard& parent;

public:
	EasyPointer<fVal> frequency;
	//fadeFilter* sound;
	fadeFilter* sound;
	int noteNum;

	SDL_Keycode key;
	bool active = false;
	bool white = false;

	void SetOctave(int o) {
		frequency->Set(Octaves::OctaveSet[o]->notes[noteNum]);
	}
	Key(Keyboard& p, int octave, int number, SDL_Keycode keycode) : parent(p), noteNum(number), frequency(new fVal(0.0f)) {
		key = keycode;

		SetOctave(octave);

		white = (number <= 4 && !(number & 1)) || (number > 4 && (number & 1));


		dualMultiply* vibrato = new dualMultiply(new propertyWave(5.0f,0.0f,0.99f,1.01f), frequency);
		sound = new fadeFilter(new sineSound(vibrato), 0.01f, 10.0f / (float)SOUND_FREQUENCY);
	}
	~Key() {
		delete sound;
	}
	float Get() {
		return sound->Get();
	};
	void stop() {
		sound->stop();
	}
	void start() {
		sound->start();
	}
	void reset() {
		sound->reset();
	}
};

class Keyboard : public Source<float>, public RenderableElement {
protected:
	int numOctaves = 3;

	void renderBlack(SDL_Renderer* r, bool s, int n) {
		int w_left = renderArea.w * n / (numOctaves * 7);
		int w_right = renderArea.w * (n + 1) / (numOctaves * 7);

		int w_w = w_right - w_left;
		int w_h = renderArea.h;
		int w_x = renderArea.x + w_left;
		int w_y = renderArea.y;

		int b_w = w_w / 2;
		int b_h = w_h / 2;
		int b_x = w_x + w_w * 3 / 4;
		int b_y = w_y;

		SDL_Rect outer{ b_x,b_y,b_w,b_h };
		SDL_SetRenderDrawColor(r, 25, 25, 25, 255);
		SDL_RenderFillRect(r, &outer);

		int border = renderArea.w / ((long long)numOctaves * 7 * 10);
		SDL_Rect inner{ b_x + border, b_y + border, b_w - 2 * border, b_h - 2 * border };
		if (!s) SDL_SetRenderDrawColor(r, 56, 56, 56, 255);
		SDL_RenderFillRect(r, &inner);
	}
	void renderWhite(SDL_Renderer* r, bool s, int n) {
		int left = renderArea.w * n / (numOctaves * 7);
		int right = renderArea.w * (n + 1) / (numOctaves * 7);

		int w = right - left;
		int h = renderArea.h;
		int x = left;
		int y = renderArea.y;
		SDL_Rect outer{ x,y,w,h };
		SDL_SetRenderDrawColor(r, 225, 225, 225, 255);
		SDL_RenderFillRect(r, &outer);

		int border = renderArea.w / ((long long)numOctaves * 7 * 10);
		SDL_Rect inner{ x + border, y + border, w - 2 * border, h - 2 * border };
		if (!s) SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
		SDL_RenderFillRect(r, &inner);
	}

	std::vector<EasyPointer<Key>> keys;

	void MakeKey(int o, int n, SDL_Keycode k) {
		Key* key = new Key(*this, o, n, k);
		keys.push_back(key);
	}

	void SetKeyFrequencies() {
		for (int i = 0; i < numOctaves * 12; i++) {
			keys[i]->SetOctave(i/12 + firstOctave);
		}
	}

	void SetSynths(int synth) {
		synth = ((synth % 5) + 5) % 5;
		if (synth != selectedSynth) {
			// Sine Triangle Square Sawtooth Noise
			selectedSynth = synth;
			switch (selectedSynth)
			{
			case 0:
				for (int i = 0; i < numOctaves * 12; i++) {
					keys[i]->frequency = new fVal(keys[i]->frequency->Get());
					keys[i]->sound->source = new sineSound(keys[i]->frequency);
				}
				break;
			case 1:
				for (int i = 0; i < numOctaves * 12; i++) {
					keys[i]->frequency = new fVal(keys[i]->frequency->Get());
					keys[i]->sound->source = new triangleSound(keys[i]->frequency);
				}
				break;
			case 2:
				for (int i = 0; i < numOctaves * 12; i++) {
					keys[i]->frequency = new fVal(keys[i]->frequency->Get());
					keys[i]->sound->source = new squareSound(keys[i]->frequency);
				}
				break;
			case 3:
				for (int i = 0; i < numOctaves * 12; i++) {
					keys[i]->frequency = new fVal(keys[i]->frequency->Get());
					keys[i]->sound->source = new sawtoothSound(keys[i]->frequency);
				}
				break;
			case 4:
				for (int i = 0; i < numOctaves * 12; i++) {
					keys[i]->frequency = new fVal(keys[i]->frequency->Get());
					keys[i]->sound->source = new noiseSound(keys[i]->frequency);
				}
				break;
			}
		}
	}

public:
	SDL_Rect renderArea;
	static SDL_Keycode KeyMapping[36];
	int firstOctave = 3;
	int selectedSynth = 0;
	
	void update() {
		TryCall(OnUpdate);
		renderArea = { 0,screenHeight / 2, screenWidth, screenHeight / 2 };
		waveformDrawArea = { 0,0,screenWidth, screenHeight / 2 };

		if (keyPressed(SDLK_LEFT)) {
			if (firstOctave != 0) {
				firstOctave--;
				SetKeyFrequencies();
			}
		}

		if (keyReleased(SDLK_RIGHT)) {
			if (firstOctave != 6) {
				firstOctave++;
				SetKeyFrequencies();
			}
		}

		if (keyPressed(SDLK_UP)) {
			SetSynths(selectedSynth + 1);
		}

		if (keyPressed(SDLK_DOWN)) {
			SetSynths(selectedSynth - 1);
		}

		for (auto key : keys) {
			if (keyReleased(key->key)) {
				key->active = false;
				key->stop();
			}
			else if (keyPressed(key->key)) {
				key->active = true;
				key->start();
			}
		}
	};
	void render(SDL_Renderer* r) {
		TryCall(OnRender);

		int whiteNum = 0;
		for (auto key : keys) {
			if (key->white) {
				renderWhite(r, key->active, whiteNum);
				whiteNum++;
			}
		}

		int blackNum = 0;
		for (auto key : keys) {
			if (!key->white) {
				renderBlack(r, key->active, blackNum);

				blackNum++;
				if ((blackNum % 7) == 2 || (blackNum % 7) == 6) blackNum++;
			}
		}
	};

	Keyboard() {
		renderArea = { 0,screenHeight / 2, screenWidth, screenHeight / 2 };

		for (int i = 0; i < numOctaves * 12; i++) {
			MakeKey(firstOctave + i / 12, i % 12, KeyMapping[i]);
		}
	}

	float Get() {
		float v = 0;
		for (int i = 0; i < keys.size(); i++)
			v += keys[i]->Get();
		return v;
	}
};
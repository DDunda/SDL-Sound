#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <ctime>
#include <vector>
#include <map>
#include <atomic>

#define SOUND_FREQUENCY 44100.0

#include "Sound.cpp"
#include "Input.h"
#include "RenderableElement.h"
#include "InteractiveElement.h"
#include "FloatField.h"
#include "TextField.h"
#include "SimpleButton.h"
#include "PianoRoll.h"

#define SOUND_BUFFER_SIZE 441
float soundBuffer[SOUND_BUFFER_SIZE];

void PushAudio(void* userdata, Uint8* stream, int len);

SDL_Texture* num_text;
SDL_Texture* char_text;

struct tone {
	float freq, vol;
};

std::vector<tone> waves{

	// eeee (me)
	/*{108, 0.00794328234724281502065918282836f },
	{218, 1.2589254117941672104239541064e-3f},
	{326, 3.1622776601683793319988935444327e-4f},
	{431, 7.9432823472428150206591828283639e-4f},
	{544, 5.0118723362727228500155418688495e-5f},
	{651, 7.9432823472428150206591828283639e-5f},
	{754, 2.5118864315095801110850320677993e-4f},
	{861, 2.5118864315095801110850320677993e-4f},
	{969, 3.9810717055349725077025230508775e-4f},
	{1066, 7.9432823472428150206591828283639e-5f},
	{1174, 1.5848931924611134852021013733915e-4f},
	{1281, 1.2589254117941672104239541063958e-4f},
	{1335, 6.3095734448019324943436013662234e-5f}*/

	// aaaa (researcher)
	/*	{100,0.245},
		{200,0.15},
		{300,0.1},
		{400,0.165},
		{500,0.215},
		{600,0.3},
		{700,0.14},
		{800,0.1},
		{900,0.14},
		{1000,0.023},
		{1100,0.01},
		{1200,0.01},
		{1300,0.005},
		{1400,0.005},
		{1500,0.012},
		{1600,0.012},*/

		// aaaa (me) (dB)
	/*	{135,-21},
	{270,-27},
	{410,-31},
	{545,-39},
	{680,-29},
	{820,-26},
	{949,-33},
	{1090,-36},
	{1219,-36}*/
	/*{108,-21},
	{216,-30},
	{324,-37},
	{432,-36},
	{540,-41},
	{648,-38},
	{756,-34},
	{864,-32},
	{972,-36},
	{1080,-44},
	{1188,-47},
	{1296,-34},
	{1404,-41},
	{1512,-53},
	{1620,-52},
	{1728,-56},
	{1836,-67},
	{2052,-65},
	{2160,-66},
	{2268,-72},
	{2376,-70},
	{2808,-55},
	{3564,-48},
	{4968,-56},
	{5400,-63},
	{6480,-78},
	{8208,-79},
	{10044,-73},
	{12420,-76},*/
	{100,-23},
	{200,-43},
	{300,-34},
	{400,-35},
	{500,-39},
	{600,-38},
	{700,-35},
	{800,-34},
	{900,-31},
	{1000,-34},
	{1100,-42},
	{1200,-41},
	{1300,-42},
	{1400,-42},
	{1500,-52},
	{1600,-51},
	{1700,-54},
	{1800,-56},
	{1900,-63},
	{2000,-66},
	{2100,-65},
	{2200,-69},
	
};

Uint32 songLen = 0;

float soundMin = 0;
float soundMax = 0;
int samples = 441;

SDL_Rect waveformDrawArea = { 0,0,640,360 };
Uint32 lastTime;
Uint32 currentTime;
float deltaTime;

bool soundRunning = true;
std::atomic<bool> renderRequested(false);

EasyPointer<TextField> fileField;
EasyPointer<FloatField> volField;
EasyPointer<FloatField> freqField;
EasyPointer<SimpleButton> testButton;
EasyPointer<Modulator> freqMod;

EasyPointer<soundMaker> testWave;

EasyPointer<PianoRoll> song;
EasyPointer<fadeFilter> finalFilter;
EasyPointer<Source<float>> waveOutput;
SDL_Renderer* renderer = NULL;

SDL_Window* window = NULL;

void initialiseSDL() {
	std::time_t now = std::time(0);

	srand(now >> 32 ^ (now & 0xFFFFFFFF));

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		exit(-1);
	}

	SDL_AudioSpec config;
	config.freq = SOUND_FREQUENCY;
	config.format = AUDIO_F32;
	config.channels = 1;
	config.callback = PushAudio;
	config.samples = SOUND_BUFFER_SIZE;

	if (SDL_OpenAudio(&config, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}

	screenWidth = 640;
	screenHeight = 360;
	if (SDL_CreateWindowAndRenderer(screenWidth, screenHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE, &window, &renderer) != 0) {
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		exit(-1);
	}

	if (!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF | IMG_INIT_WEBP)) {
		IMG_Quit();
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		exit(-1);
	}
}

// Puts a bunch of sines in a blender
EasyPointer<blendAdd> SetupWaves(std::vector<tone>& waves) {
	blendAdd* soundMerge = new blendAdd();

	for (int i = 0; i < waves.size(); i++) {
		sineSound* sine = new sineSound(new Val<float>(waves[i].freq / SOUND_FREQUENCY));
		volumeFilter* vol = new volumeFilter(sine, new Val<float>(powf(10, (waves[i].vol + 30) / 10.0)));
		soundMerge->addSource(vol);
	}

	return soundMerge;
}

#define epf EasyPointer<Source<float>>
class VoiceFilter : public Source<float> {
public:
	epf vol1;
	epf vol2;
	epf vol3;
	epf freq1;
	epf freq2;
	epf freq3;
	epf spread1;
	epf spread2;
	epf spread3;
	float harmonic;
	VoiceFilter(float h, epf v1, epf v2, epf v3, epf f1, epf f2, epf f3, epf s1, epf s2, epf s3) : harmonic(h), vol1(v1), vol2(v2), vol3(v3), freq1(f1), freq2(f2), freq3(f3), spread1(s1), spread2(s2), spread3(s3) {}
	float GetAmp(float a) {
		return powf(10, a / 10.0);
	}
	float Get() {
		return GetAmp(vol1->Get()) / (powf((harmonic + 1 - freq1->Get()) / spread1->Get(), 2.0) + 1.0) +
			GetAmp(vol2->Get()) / (powf((harmonic + 1 - freq2->Get()) / spread2->Get(), 2.0) + 1.0) +
			GetAmp(vol3->Get()) / (powf((harmonic + 1 - freq3->Get()) / spread3->Get(), 2.0) + 1.0);
	}
	void reset() {
		vol1->reset();
		vol2->reset();
		vol3->reset();
		freq1->reset();
		freq2->reset();
		freq3->reset();
	}
};

EasyPointer<blendAdd> SetupVoice(int base = 100, int harmonics = 50) {
	EasyPointer<FloatField> F1fField = new FloatField();
	EasyPointer<FloatField> F2fField = new FloatField();
	EasyPointer<FloatField> F3fField = new FloatField();

	EasyPointer<FloatField> F1vField = new FloatField();
	EasyPointer<FloatField> F2vField = new FloatField();
	EasyPointer<FloatField> F3vField = new FloatField();

	EasyPointer<FloatField> F1sField = new FloatField();
	EasyPointer<FloatField> F2sField = new FloatField();
	EasyPointer<FloatField> F3sField = new FloatField();

	F1fField->setValue(2.7);
	F2fField->setValue(22.9);
	F3fField->setValue(30.1);

	F1vField->setValue(-4);
	F2vField->setValue(-24);
	F3vField->setValue(-26);

	F1sField->setValue(0.25);
	F2sField->setValue(3);
	F3sField->setValue(2);

	blendAdd* soundMerge = new blendAdd();

	EasyPointer<FloatField> fields[9]{ F1fField, F2fField, F3fField, F1vField, F2vField, F3vField, F1sField, F2sField, F3sField, };
	for (int i = 0; i < 9; ++i) {
		fields[i]->setVisibleCharacters(5);

		fields[i]->setAnchor(0, 0);
		fields[i]->setDigitSize(16);
		fields[i]->setPadding({ 8,10,8,10 });
		fields[i]->setDigitGap(0);
		fields[i]->srcDigitSize = 8;
		fields[i]->setPosition((i / 3) * 120 + 20, (i % 3) * 42 + 20);
		fields[i]->digits = num_text;
	}

	for (int i = 0; i < harmonics; i++) {
		sineSound* sine = new sineSound(new Val<float>(((i + 1) * base) / SOUND_FREQUENCY));
		volumeFilter* vol = new volumeFilter(sine,new VoiceFilter(i,
			F1vField, F2vField, F3vField,
			F1fField, F2fField, F3fField,
			F1sField, F2sField, F3sField
		));
		soundMerge->addSource(vol);
	}

	return soundMerge;
}


void SetupVolField() {
	volField = new FloatField();

	volField->maxData = 8;
	volField->setValue(0.01);

	volField->setAnchor(0, 0);
	volField->setVisibleCharacters(8);
	volField->setDigitSize(16);
	volField->setPadding({ 8,10,8,10 });
	volField->setDigitGap(0);
	volField->srcDigitSize = 8;
	volField->setPosition(25, 67);

	volField->digits = num_text;
}

void SetupFreqField() {
	freqField = new FloatField();

	freqField->maxData = 8;
	freqField->setVisibleCharacters(8);

	freqField->setAnchor(0, 0);
	freqField->setDigitSize(16);
	freqField->setPadding({ 8,10,8,10 });
	freqField->setDigitGap(0);
	freqField->srcDigitSize = 8;
	freqField->setPosition(25, 25);
	freqField->setValue(440);

	freqField->digits = num_text;
}

void SetupFileField() {
	fileField = new TextField();

	fileField->maxData = 20;

	fileField->setAnchor(0, 0);
	fileField->setVisibleCharacters(8);
	fileField->setCharacterSize(16);
	fileField->setPadding({ 8,10,8,10 });
	fileField->setCharacterGap(0);
	fileField->srcCharacterSize = 8;
	fileField->setPosition(25, 109);

	fileField->characters = char_text;

	fileField->OnUnfocus = [](RenderableElement* txt) -> void {
		song->OpenFile(((TextField*)txt)->Get());
	};
}

void SetupPlayButton() {
	testButton = new SimpleButton();

	testButton->setArea(32, 32);
	testButton->setAnchor(0, 0);
	testButton->setPosition(183, 109);

	testButton->OnLeftRelease = [](RenderableElement* txt) -> void {
		if (finalFilter->isStopped())
			finalFilter->start();
		else
			finalFilter->stop();
	};
}

class KeyListener : public Source<float> {
private:
	bool active = false;
	fadeFilter f;
public:
	SDL_Keycode key;
	KeyListener(EasyPointer<Source<float>> s, SDL_Keycode k) : f(s, new Val<float>(0.01), new Val<float>(10/SOUND_FREQUENCY)) {
		key = k;
	}
	void frameUpdate() {
		if (keyReleased(key)) {
			active = false;
			f.stop();
		}
		else if(keyPressed(key)) {
			active = true;
			f.start();
		}
	}
	float Get() {
		return f.Get();
	}
	void reset() {
		f.reset();
	}
};

void MakeKey(blendAdd& b, float f, SDL_Keycode k) {
	//auto vibrato = new dualAdd(new dualMultiply(new sineSound(new Val<float>(2)), new Val<float>(1 / 16.0)), new Val<float>(1 / 2.0));
	//auto sound = new pulseSound(new Val<float>(f),vibrato);
	auto vibrato = new dualMultiply(new dualAdd(new dualMultiply(new sineSound(new Val<float>(5)),new Val<float>(0.01)), new Val<float>(1)), new Val<float>(f));
	auto sound = new sawtoothSound(vibrato);
	b.addSource(new KeyListener(sound,k));
}

class keyBoardRenderer : public RenderableElement {
private:
	void renderBlack(SDL_Renderer* r, bool s, int n) {
		int w_left = area.w * n / keys;
		int w_right = area.w * (n+1) / keys;

		int w_w = w_right - w_left;
		int w_h = area.h;
		int w_x = area.x + w_left;
		int w_y = area.y;

		int b_w = w_w * 0.5;
		int b_h = w_h * 0.5;
		int b_x = w_x + w_w * 0.75;
		int b_y = w_y;

		SDL_Rect outer{ b_x,b_y,b_w,b_h };
		SDL_SetRenderDrawColor(r, 25, 25, 25, 255);
		SDL_RenderFillRect(r, &outer);

		int border = area.w * 0.1 / keys;
		SDL_Rect inner{ b_x + border, b_y + border, b_w - 2 * border, b_h - 2 * border };
		if (!s) SDL_SetRenderDrawColor(r, 56, 56, 56, 255);
		SDL_RenderFillRect(r, &inner);
	}
	void renderWhite(SDL_Renderer* r, bool s, int n) {
		int left = area.w * n / keys;
		int right = area.w * (n + 1) / keys;

		int w = right - left;
		int h = area.h;
		int x = left;
		int y = area.y;
		SDL_Rect outer{ x,y,w,h };
		SDL_SetRenderDrawColor(r, 225, 225, 225, 255);
		SDL_RenderFillRect(r, &outer);

		int border = area.w * 0.1 / keys;
		SDL_Rect inner{ x + border, y + border, w - 2 * border, h - 2 * border };
		if (!s) SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
		SDL_RenderFillRect(r, &inner);
	}
public:
	SDL_Rect area;
	int keys;

	std::vector<SDL_Keycode> whiteKeys;
	std::vector<SDL_Keycode> blackKeys;

	keyBoardRenderer(int k = 14) : keys(k), RenderableElement() {
		area = { 0,screenHeight / 2, screenWidth, screenHeight / 2 };
	}
	void update() {
		TryCall(OnUpdate);
		area = { 0,screenHeight / 2, screenWidth, screenHeight / 2 };
		waveformDrawArea = { 0,0,screenWidth, screenHeight / 2 };
	};
	void render(SDL_Renderer* r) {
		TryCall(OnRender);

		for (int i = 0; i < keys; i++) {
			renderWhite(r, keyDown(whiteKeys[i]), i);
		}

		int keyNum = 0;
		for (int i = 0; i < keys; i++) {
			if ((i % 7) == 2 || (i % 7) == 6) continue;
			renderBlack(r, keyDown(blackKeys[keyNum]), i);
			keyNum++;
		}
	};
};

EasyPointer<keyBoardRenderer> board;

EasyPointer<Source<float>> MakeKeyboard() {
	blendAdd* keyboardBlender = new blendAdd();

	// Black:    1 2   4 5 6   8 9   - = <=   s d   g h j
	// White: TAB q w e r t y u i o p [ ]  \ z x c v b n m

	using namespace noteFrequencies;
	MakeKey(*keyboardBlender, C3 , SDLK_TAB         );
	MakeKey(*keyboardBlender, Cs3, SDLK_1           );
	MakeKey(*keyboardBlender, D3 , SDLK_q           );
	MakeKey(*keyboardBlender, Ds3, SDLK_2           );
	MakeKey(*keyboardBlender, E3 , SDLK_w           );
	MakeKey(*keyboardBlender, F3 , SDLK_e           );
	MakeKey(*keyboardBlender, Fs3, SDLK_4           );
	MakeKey(*keyboardBlender, G3 , SDLK_r           );
	MakeKey(*keyboardBlender, Gs3, SDLK_5           );
	MakeKey(*keyboardBlender, A3 , SDLK_t           );
	MakeKey(*keyboardBlender, As3, SDLK_6           );
	MakeKey(*keyboardBlender, B3,  SDLK_y           );

	MakeKey(*keyboardBlender, C4,  SDLK_u           );
	MakeKey(*keyboardBlender, Cs4, SDLK_8           );
	MakeKey(*keyboardBlender, D4,  SDLK_i           );
	MakeKey(*keyboardBlender, Ds4, SDLK_9           );
	MakeKey(*keyboardBlender, E4,  SDLK_o           );
	MakeKey(*keyboardBlender, F4,  SDLK_p           );
	MakeKey(*keyboardBlender, Fs4, SDLK_MINUS       );
	MakeKey(*keyboardBlender, G4,  SDLK_LEFTBRACKET );
	MakeKey(*keyboardBlender, Gs4, SDLK_EQUALS      );
	MakeKey(*keyboardBlender, A4,  SDLK_RIGHTBRACKET);
	MakeKey(*keyboardBlender, As4, SDLK_BACKSPACE   );
	MakeKey(*keyboardBlender, B4,  SDLK_BACKSLASH   );
	
	MakeKey(*keyboardBlender, C5,  SDLK_z           );
	MakeKey(*keyboardBlender, Cs5, SDLK_s           );
	MakeKey(*keyboardBlender, D5,  SDLK_x           );
	MakeKey(*keyboardBlender, Ds5, SDLK_d           );
	MakeKey(*keyboardBlender, E5,  SDLK_c           );
	MakeKey(*keyboardBlender, F5,  SDLK_v           );
	MakeKey(*keyboardBlender, Fs5, SDLK_g           );
	MakeKey(*keyboardBlender, G5,  SDLK_b           );
	MakeKey(*keyboardBlender, Gs5, SDLK_h           );
	MakeKey(*keyboardBlender, A5,  SDLK_n           );
	MakeKey(*keyboardBlender, As5, SDLK_j           );
	MakeKey(*keyboardBlender, B5,  SDLK_m           );
	
	board = new keyBoardRenderer(21);
	board->blackKeys = {
		SDLK_1, SDLK_2, SDLK_4,     SDLK_5,      SDLK_6,
		SDLK_8, SDLK_9, SDLK_MINUS, SDLK_EQUALS, SDLK_BACKSPACE,
		SDLK_s, SDLK_d, SDLK_g,     SDLK_h,      SDLK_j,
	};
	board->whiteKeys = {
		SDLK_TAB, SDLK_q, SDLK_w, SDLK_e, SDLK_r,           SDLK_t,            SDLK_y,
		SDLK_u,   SDLK_i, SDLK_o, SDLK_p, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, SDLK_BACKSLASH,
		SDLK_z,   SDLK_x, SDLK_c, SDLK_v, SDLK_b,           SDLK_n,            SDLK_m,
	};

	return keyboardBlender;
}

int main(int argc, char* argv[]) {
	initialiseSDL();	

	num_text = IMG_LoadTexture(renderer, "nums.png");
	char_text = IMG_LoadTexture(renderer, "chars.png");

	//SetupFileField();
	//SetupPlayButton();
	SetupVolField();
	//SetupFreqField();
		
/* A simple melody
	C4,E4,A4,B4,
	C4,E4,A4,B4,
	C4,E4,A4,B4,
	C4,E4,A4,B4,

	Bf3,D4,G4,A4,
	Bf3,D4,G4,A4,
	Bf3,D4,G4,A4,
	Bf3,D4,G4,A4,

	Gf3,Bf3,Ef4,F4,
	Gf3,Bf3,Ef4,F4,
	Gf3,Bf3,Ef4,F4,
	Gf3,Bf3,Ef4,F4,

	Df3,F3,Bf3,C4,
	Df3,F3,Bf3,C4,
	Df3,F3,Bf3,C4,
	Df3,C4,Bf3,F3,
*/
	
	//song = new PianoRoll("Melody1.prm");

	//EasyPointer<volumeFilter> volMod = new volumeFilter(song, volField);
	EasyPointer<volumeFilter> volMod = new volumeFilter(MakeKeyboard(), volField);

	finalFilter = new fadeFilter(
		volMod,
		new Val<float>(0.01),
		new Val<float>(0.001)
	);
	waveOutput = finalFilter;

	SDL_PauseAudio(0);

	lastTime = SDL_GetTicks();

	int lastFrameEnd = SDL_GetTicks();

	finalFilter->start();

	while (running || soundRunning) {
		lastTime = currentTime;
		currentTime = SDL_GetTicks();
		deltaTime = (float)(currentTime - lastTime) / 1000.0;
		getEvents();

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		InteractiveElement::UpdateElementFocus();
		RenderableElement::UpdateAllElements();
		Updater::updateAllSources();

		/*for (int i = 0; i < harmonics; i++) {
			*volumes[i]->targetVolume = 
				powf(10,F1vField->output / 10.0) / (powf((i + 1 - F1Field->output) / F1sField->output, 2.0) + 1.0) +
				powf(10,F2vField->output / 10.0) / (powf((i + 1 - F2Field->output) / F2sField->output, 2.0) + 1.0) +
				powf(10,F3vField->output / 10.0) / (powf((i + 1 - F3Field->output) / F3sField->output, 2.0) + 1.0);
		}*/

		if (!running && !finalFilter->isStopped())
			finalFilter->stop();

		if (finalFilter->finished())
			soundRunning = false;

		float mult = deltaTime / 100;
		if (mult < 0) mult = 0;

		soundMax -= mult;
		soundMin += mult;
		
		renderRequested.store(true);
		while (renderRequested.load()) SDL_Delay(1);

		RenderableElement::RenderAllElements(renderer);
		SDL_RenderPresent(renderer);

		int frameEnd = SDL_GetTicks();
		int tmp = 10 - (frameEnd - lastFrameEnd);
		if (tmp > 0)
			SDL_Delay(tmp);
		lastFrameEnd = frameEnd;
	}

	SDL_DestroyTexture(num_text);

	IMG_Quit();

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	/// Close resources ///
	SDL_CloseAudio();
	SDL_Quit();

	/// Exit ///
	return EXIT_SUCCESS;
}


int li = 0;
int soundDrawn = 0;
void PushAudio(void* userdata, Uint8* stream, int len) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	if (waveOutput.isSet()) {
		if (renderRequested.load()) {
			for (int i = 0; i < len / sizeof(float); i++) {
				soundBuffer[i] = waveOutput->Get();
				if (soundBuffer[i] > soundMax) soundMax = soundBuffer[i];
				if (-soundBuffer[i] > soundMax) soundMax = -soundBuffer[i];
				if (soundMax < 0.001) soundMax = 0.001;

				if (renderRequested.load()) {
					int lx = waveformDrawArea.x + (soundDrawn - 1) * waveformDrawArea.w / samples;
					int x = waveformDrawArea.x + soundDrawn * waveformDrawArea.w / samples;
					if (!finalFilter->finished()) {
						float fitted = 1.0 - (soundBuffer[li] / 2.0 + soundMax) / 2.0 / soundMax;
						float nextFitted = 1.0 - (soundBuffer[i] / 2.0 + soundMax) / 2.0 / soundMax;
						SDL_RenderDrawLine(renderer,
							lx,
							waveformDrawArea.y + waveformDrawArea.h * fitted,
							x,
							waveformDrawArea.y + waveformDrawArea.h * nextFitted);
					}
					else {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderDrawLine(renderer, x, waveformDrawArea.y, x, waveformDrawArea.y + waveformDrawArea.h);
						SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
						SDL_RenderDrawPoint(renderer, x, waveformDrawArea.y + waveformDrawArea.h / 2);
					}
					soundDrawn++;
				}

				if (soundDrawn == samples) {
					renderRequested.store(false);
					soundDrawn = 0;
				}
				li = i;
			}
		}
		else {
			for (int i = 0; i < len / sizeof(float); i++) {
				soundBuffer[i] = waveOutput->Get();
				if (soundBuffer[i] > soundMax) soundMax = soundBuffer[i];
				if (soundBuffer[i] < soundMin) soundMin = soundBuffer[i];
				if (soundMin > -0.001) soundMin = -0.001;
				if (soundMax < 0.001) soundMax = 0.001;
				li = i;
			}
		}
	}
	else {
		if (renderRequested.load()) {
			for (int i = 0; i < len / sizeof(float); i++) {
				soundBuffer[i] = 0;
				if (renderRequested.load()) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderDrawLine(renderer, soundDrawn, 0, soundDrawn, 360);
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_RenderDrawPoint(renderer, soundDrawn - 1, 179);
					soundDrawn++;
				}

				if (soundDrawn == samples) {
					renderRequested.store(false);
					soundDrawn = 0;
					break;
				}
				li = i;
			}
		}
		else {
			for (int i = 0; i < len / sizeof(float); i++) {
				soundBuffer[i] = 0;
				li = i;
			}
		}
	}

	SDL_memcpy(stream, soundBuffer, len);
}
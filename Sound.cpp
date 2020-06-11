#pragma once

#ifndef SOUND_FREQUENCY
#define SOUND_FREQUENCY 44100.0
#endif

#include <vector>
#include <queue>
#include "AbstractedAccess.h"

// t: 0-1, progress of one wave
// Returns wave
typedef float(*synth_func)(float t);

namespace noteFrequencies {
	const float
		C0 = 16.35,
		Cs0 = 17.32,
		Df0 = 17.32,
		D0 = 18.35,
		Ds0 = 19.45,
		Ef0 = 19.45,
		E0 = 20.60,
		F0 = 21.83,
		Fs0 = 23.12,
		Gf0 = 23.12,
		G0 = 24.50,
		Gs0 = 25.96,
		Af0 = 25.96,
		A0 = 27.50,
		As0 = 29.14,
		Bf0 = 29.14,
		B0 = 30.87,

		C1 = 32.70,
		Cs1 = 34.65,
		Df1 = 34.65,
		D1 = 36.71,
		Ds1 = 38.89,
		Ef1 = 38.89,
		E1 = 41.20,
		F1 = 43.65,
		Fs1 = 46.25,
		Gf1 = 46.25,
		G1 = 49.00,
		Gs1 = 51.91,
		Af1 = 51.91,
		A1 = 55.00,
		As1 = 58.27,
		Bf1 = 58.27,
		B1 = 61.74,

		C2 = 65.41,
		Cs2 = 69.30,
		Df2 = 69.30,
		D2 = 73.42,
		Ds2 = 77.78,
		Ef2 = 77.78,
		E2 = 82.41,
		F2 = 87.31,
		Fs2 = 92.50,
		Gf2 = 92.50,
		G2 = 98.00,
		Gs2 = 103.83,
		Af2 = 103.83,
		A2 = 110.00,
		As2 = 116.54,
		Bf2 = 116.54,
		B2 = 123.47,

		C3 = 130.81,
		Cs3 = 138.59,
		Df3 = 138.59,
		D3 = 146.83,
		Ds3 = 155.56,
		Ef3 = 155.56,
		E3 = 164.81,
		F3 = 174.61,
		Fs3 = 185.00,
		Gf3 = 185.00,
		G3 = 196.00,
		Gs3 = 207.65,
		Af3 = 207.65,
		A3 = 220.00,
		As3 = 233.08,
		Bf3 = 233.08,
		B3 = 246.94,

		C4 = 261.63,
		Cs4 = 277.18,
		Df4 = 277.18,
		D4 = 293.66,
		Ds4 = 311.13,
		Ef4 = 311.13,
		E4 = 329.63,
		F4 = 349.23,
		Fs4 = 369.99,
		Gf4 = 369.99,
		G4 = 392.00,
		Gs4 = 415.30,
		Af4 = 415.30,
		A4 = 440.00,
		As4 = 466.16,
		Bf4 = 466.16,
		B4 = 493.88,

		C5 = 523.25,
		Cs5 = 554.37,
		Df5 = 554.37,
		D5 = 587.33,
		Ds5 = 622.25,
		Ef5 = 622.25,
		E5 = 659.25,
		F5 = 698.46,
		Fs5 = 739.99,
		Gf5 = 739.99,
		G5 = 783.99,
		Gs5 = 830.61,
		Af5 = 830.61,
		A5 = 880.00,
		As5 = 932.33,
		Bf5 = 932.33,
		B5 = 987.77,

		C6 = 1046.50,
		Cs6 = 1108.73,
		Df6 = 1108.73,
		D6 = 1174.66,
		Ds6 = 1244.51,
		Ef6 = 1244.51,
		E6 = 1318.51,
		F6 = 1396.91,
		Fs6 = 1479.98,
		Gf6 = 1479.98,
		G6 = 1567.98,
		Gs6 = 1661.22,
		Af6 = 1661.22,
		A6 = 1760.00,
		As6 = 1864.66,
		Bf6 = 1864.66,
		B6 = 1975.53,

		C7 = 2093.00,
		Cs7 = 2217.46,
		Df7 = 2217.46,
		D7 = 2349.32,
		Ds7 = 2489.02,
		Ef7 = 2489.02,
		E7 = 2637.02,
		F7 = 2793.83,
		Fs7 = 2959.96,
		Gf7 = 2959.96,
		G7 = 3135.96,
		Gs7 = 3322.44,
		Af7 = 3322.44,
		A7 = 3520.00,
		As7 = 3729.31,
		Bf7 = 3729.31,
		B7 = 3951.07,

		C8 = 4186.01,
		Cs8 = 4434.92,
		Df8 = 4434.92,
		D8 = 4698.63,
		Ds8 = 4978.03,
		Ef8 = 4978.03,
		E8 = 5274.04,
		F8 = 5587.65,
		Fs8 = 5919.91,
		Gf8 = 5919.91,
		G8 = 6271.93,
		Gs8 = 6644.88,
		Af8 = 6644.88,
		A8 = 7040.00,
		As8 = 7458.62,
		Bf8 = 7458.62,
		B8 = 7902.13;
};

class soundMaker : public Source<float> {
public:
	virtual float getSound() {
		return 0;
	};
	float Get() {
		return getSound();
	}
	virtual void reset() {};
};

class filter : public soundMaker {
public:
	EasyPointer<Source<float>> source;
	filter(EasyPointer<Source<float>> sound_src) {
		source = sound_src;
	}
	void linkSource(EasyPointer<Source<float>> sound_src) {
		source = sound_src;
	}
	virtual float getSound() {
		return source->Get();
	}
	virtual void reset() {
		source->reset();
	}
};

class blendFilter : public soundMaker {
public:
	std::vector<EasyPointer<Source<float>>> sources;
	void addSource(EasyPointer<Source<float>> sound_src) {
		sources.push_back(sound_src);
	}
	virtual float getSound() {
		return 0;
	}
	virtual void reset() {
		for (auto src : sources) src->reset();
	}
};

class blendAdd : public blendFilter {
public:
	float getSound() {
		float sound = 0;
		for (auto src : sources) sound += src->Get();
		return sound;
	};
};

class blendMult : public blendFilter {
public:
	float getSound() {
		float sound = 1;
		for (auto src : sources) sound *= src->Get();
		return sound;
	};
};

class propertyWave : public Source<float> {
private:
	float time;
public:
	EasyPointer<Source<float>> min;
	EasyPointer<Source<float>> max;
	EasyPointer<Source<float>> frequency;

	// f, o, min, max 
	propertyWave(EasyPointer<Source<float>> freq, float offset, EasyPointer<Source<float>> min, EasyPointer<Source<float>> max) {
		this->min = min;
		this->max = max;
		frequency = freq;
		time = offset;
	}
	// f, o, min, max 
	propertyWave(float freq, float offset, float min, float max) {
		this->min = new Val<float>(min);
		this->max = new Val<float>(max);
		frequency = new Val<float>(freq);
		time = offset;
	}
	operator float() {
		time += frequency->Get() / SOUND_FREQUENCY;
		time = fmod(time, 1.0f);
		float tmpMin = min->Get();
		return (float)((cos(time * 2 * M_PI) + 1.0f) / 2.0f) * (max->Get() - tmpMin) + tmpMin;
	};
	void reset() {
		time = 0;
		min->reset();
		max->reset();
		frequency->reset();
	}
};

class volumeFilter : public filter {
private:
	float internalVolume;
public:
	EasyPointer<Source<float>> targetVolume;
	volumeFilter(EasyPointer<Source<float>> src, EasyPointer<Source<float>> vol) : filter(src) {
		internalVolume = 0;
		targetVolume = vol;
	}
	float getSound() {
		float tmpVolume = targetVolume->Get();
		float sound = source->Get();
		internalVolume = (internalVolume * 9.0f + tmpVolume) / 10.0f;
		return sound * internalVolume;
	}
	void reset() {
		filter::reset();
		targetVolume->reset();
		internalVolume = targetVolume->Get();
		targetVolume->reset();
	}
};

class Modulator : public Source<float> {
private:
	float time;
public:
	std::vector<float> vals;
	EasyPointer<Source<float>> frequency;
	Modulator(EasyPointer<Source<float>> f) {
		time = 0;
		frequency = f;
	}
	Modulator(float f) {
		time = 0;
		frequency = new Val<float>(f);
	}
	float Get() {
		float tmpFreq = frequency->Get();
		time += tmpFreq / SOUND_FREQUENCY;
		time = fmod(time, 1.0f);
		if (vals.empty()) return 0;
		return vals[floor(time * vals.size())] * tmpFreq / SOUND_FREQUENCY;
	}
	void reset() {
		frequency->reset();
		time = 0;
	}
};

class synthSound : public soundMaker {
private:
	float time;
public:
	synth_func sound;
	EasyPointer<Source<float>> frequency;
	synthSound(synth_func synth, EasyPointer<Source<float>>& f) {
		time = 0;
		frequency = f;
		sound = synth;
	}
	virtual float getSound() {
		time += frequency->Get() / SOUND_FREQUENCY;
		time = fmod(time, 1.0);
		float w = sound(time);
		return w;
	}
	void reset() {
		time = 0;
		frequency->reset();
	}
};

synth_func sineFunc = [](float t) -> float {
	return sin(t * 2 * M_PI);
};

synth_func squareFunc = [](float t) -> float {
	return t < 0.5 ? -1 : 1;
};

synth_func triangleFunc = [](float t) -> float {
	return abs(2.0 - abs(1.0 - 4.0 * t)) - 1.0;
};

synth_func sawtoothFunc = [](float t) -> float {
	return 2 * t - 1.0;
};

class sineSound : public synthSound {
public:
	sineSound(EasyPointer<Source<float>> f) : synthSound(sineFunc, f) {}
};

class squareSound : public synthSound {
public:
	squareSound(EasyPointer<Source<float>> f) : synthSound(squareFunc, f) {}
};

class triangleSound : public synthSound {
public:
	triangleSound(EasyPointer <Source<float>> f) : synthSound(triangleFunc, f) {}
};

class sawtoothSound : public synthSound {
public:
	sawtoothSound(EasyPointer <Source<float>> f) : synthSound(sawtoothFunc, f) {}
};

class noiseSound : public soundMaker {
private:
	float progress = 0;
	float amplitude = 0;

	void setRandomAmplitude() {
		amplitude = roundf(rand() / (float)RAND_MAX) * 2 - 1;
	}
public:
	EasyPointer<Source<float>> frequency;
	
	noiseSound(EasyPointer<Source<float>> f) {
		frequency = f;
	}
	float getSound() {
		progress += frequency->Get() / SOUND_FREQUENCY;
		if (progress > 1) {
			progress = fmodf(progress, 1);
			setRandomAmplitude();
		}
		return amplitude;
	}

	void reset() {
		progress = 0;
		setRandomAmplitude();
		frequency->reset();
	}

};

class fadeFilter : public filter {
private:
	float trail = 0;
	bool stopped = false;
	int resuming = 0;
	std::queue<float> resumeBuffer;
public:
	EasyPointer<Source<float>> finishThreshold;
	EasyPointer<Source<float>> decayRate;

	fadeFilter(EasyPointer<Source<float>> s, EasyPointer<Source<float>> threshold, EasyPointer<Source<float>> decay) : filter(s) {
		finishThreshold = threshold;
		decayRate = decay;
	};
	bool isStopped() {
		return stopped;
	}
	bool finished() {
		return stopped && abs(trail) < finishThreshold->Get();
	}
	float getSound() {
		if (!stopped) {
			if (trail == 1) return source->Get();

			if (trail < 1) trail += decayRate->Get();
			if (trail < 1) {
				if (!resumeBuffer.empty()) {
					float val = resumeBuffer.front();
					resumeBuffer.pop();
					return val * trail;
				}
			}

			trail = 1;
			while (!resumeBuffer.empty()) resumeBuffer.pop();
			return source->Get();
		}
		
		if (trail == 0) return 0;

		if (trail > 0) trail -= decayRate->Get();
		if (trail > 0) {
			float val = source->Get();
			resumeBuffer.push(val);
			return val * trail;
		}

		trail = 0;
		return 0;
	}
	void stop() {
		stopped = true;
	}
	void start() {
		stopped = false;
	}
	void restart() {
		stop();
		finishThreshold->reset();
		decayRate->reset();
		source->reset();
		start();
	}
	void reset() {
		restart();
	}
};
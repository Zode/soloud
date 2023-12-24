// Compressor/Limiter filter

#ifndef SOLOUD_LIMITER_H
#define SOLOUD_LIMITER_H

#include "soloud.h"

#include <vector>

namespace SoLoud
{
	struct Follower
	{
		float Envelope = 0.0f;
		float Attack = 0.0f;
		float Release = 0.0f;
		int Channels = 2;

		bool Initialized = false;

		void Setup( float Attack, float Release, float SampleRate );
		void Process( float* Buffer, unsigned int Samples );
	};
	
	class LimiterInstance : public FilterInstance
	{
	public:
		LimiterInstance( class Limiter* aParent );
		void filterChannel( float* aBuffer, unsigned int aSamples, float aSamplerate, time aTime, unsigned int aChannel, unsigned int aChannels ) override;

		Follower FollowerL;
		Follower FollowerR;
	};

	class Limiter : public Filter
	{
	public:
		Limiter();
		
		int getParamCount() override;
		const char* getParamName( unsigned int aParamIndex ) override;
		unsigned int getParamType( unsigned int aParamIndex ) override;
		float getParamMax( unsigned int aParamIndex ) override;
		float getParamMin( unsigned int aParamIndex ) override;

		FilterInstance* createInstance() override;

		std::vector<float> FloatParameters;

		enum ParameterType
		{
			PreGain = 0,
			PostGain,
			Release,
			Threshold,
		};
	};
}

#endif
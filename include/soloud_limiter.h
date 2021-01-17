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
		virtual void filter( float* aBuffer, unsigned int aSamples, unsigned int aBufferSize, unsigned int aChannels, float aSamplerate, time aTime );
		virtual void filterChannel( float* aBuffer, unsigned int aSamples, float aSamplerate, time aTime, unsigned int aChannel, unsigned int aChannels );

		Follower FollowerL;
		Follower FollowerR;

		std::vector<float*> PreviousBuffers;
		float* ScratchBuffer = nullptr;
		unsigned int BufferSize = 0;
	};

	class Limiter : public Filter
	{
	public:
		Limiter();
		
		virtual int getParamCount();
		virtual const char* getParamName( unsigned int aParamIndex );
		virtual unsigned int getParamType( unsigned int aParamIndex );
		virtual float getParamMax( unsigned int aParamIndex );
		virtual float getParamMin( unsigned int aParamIndex );

		virtual FilterInstance* createInstance();

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
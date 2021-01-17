// Compressor/Limiter filter

#ifndef SOLOUD_LIMITER_H
#define SOLOUD_LIMITER_H

#include "soloud.h"

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
		virtual void filterChannel( float* aBuffer, unsigned int aSamples, float aSamplerate, time aTime, unsigned int aChannel, unsigned int aChannels );

		Follower FollowerL;
		Follower FollowerR;
	};

	class Limiter : public Filter
	{
	public:
		Limiter() = default;
		
		virtual int getParamCount();
		virtual const char* getParamName( unsigned int aParamIndex );
		virtual unsigned int getParamType( unsigned int aParamIndex );
		virtual float getParamMax( unsigned int aParamIndex );
		virtual float getParamMin( unsigned int aParamIndex );

		virtual FilterInstance* createInstance();
	};
}

#endif
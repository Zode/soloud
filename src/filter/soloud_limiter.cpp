// Compressor/Limiter Filter

#include "soloud.h"
#include "soloud_limiter.h"

#include <unordered_map>
#include <string>

namespace SoLoud
{
	struct Parameter
	{
		Parameter( const std::string& Name, const int& Type, const float& Minimum, const float& Maximum, const float& Default )
		{
			this->Name = Name;
			this->Type = Type;
			this->Minimum = Minimum;
			this->Maximum = Maximum;
			this->Default = Maximum;
		}
		
		std::string Name;
		int Type = Filter::FLOAT_PARAM;
		float Minimum = 0.0f;
		float Maximum = 1.0f;
		float Default = 0.0f;
	};
	
	static const std::vector<Parameter> Parameters = {
		{ "PreGain", Filter::FLOAT_PARAM, 0.0f,1.0f, 1.0f },
		{ "PostGain", Filter::FLOAT_PARAM, 0.0f,1.0f, 0.95f },
		// { "Attack", Filter::FLOAT_PARAM, 0.0f,100.0f },
		{ "Release", Filter::FLOAT_PARAM, 0.0f,100.0f, 0.005f },
		{ "Threshold", Filter::FLOAT_PARAM, 0.001f,1.0f, 1.0f }
	};

	Limiter::Limiter()
	{
		FloatParameters.resize( Parameters.size() );
		for( size_t Index = 0; Index < Parameters.size(); Index++ )
		{
			FloatParameters[Index] = Parameters[Index].Default;
		}
	}

	int Limiter::getParamCount()
	{
		return static_cast<int>( Parameters.size() );
	}

	const char* Limiter::getParamName( unsigned int aParamIndex )
	{
		return Parameters[aParamIndex].Name.c_str();
	}

	unsigned int Limiter::getParamType( unsigned int aParamIndex )
	{
		return Parameters[aParamIndex].Type;
	}

	float Limiter::getParamMax( unsigned int aParamIndex )
	{
		return Parameters[aParamIndex].Maximum;
	}

	float Limiter::getParamMin( unsigned int aParamIndex )
	{
		return Parameters[aParamIndex].Minimum;
	}

	FilterInstance* Limiter::createInstance()
	{
		return new LimiterInstance( this );
	}

	void Follower::Setup( float Attack, float Release, float SampleRate )
	{
		this->Attack = Attack * SampleRate;
		this->Release = Release * SampleRate;

		this->Attack = std::expf( std::logf( 0.01f ) / this->Attack );
		this->Release = std::expf( std::logf( 0.01f ) / this->Release );
		
		Initialized = true;

		// Envelope = 0.0f;
	}

	void Follower::Process( float* Buffer, unsigned int Samples )
	{
		float PeakAmplitude = 0.0f;
		for( unsigned int Index = 0; Index < Samples; Index++ )
		{
			const float Amplitude = std::fabs( Buffer[Index] );
			if( Amplitude > PeakAmplitude )
			{
				PeakAmplitude = Amplitude;
			}
		}

		if( PeakAmplitude > Envelope )
		{
			Envelope = Attack * ( Envelope - PeakAmplitude ) + PeakAmplitude;
		}
		else
		{
			Envelope = Release * ( Envelope - PeakAmplitude ) + PeakAmplitude;
		}
		
		Envelope = max( 0.000001f, Envelope );
	}

	LimiterInstance::LimiterInstance( Limiter* aParent )
	{
		initParams( static_cast<int>( Parameters.size() ) );
		for( size_t Index = 0; Index < Parameters.size(); Index++ )
		{
			setFilterParameter( Index, aParent->FloatParameters[Index] );
		}
	}

	void LimiterInstance::filter( float* aBuffer, unsigned aSamples, unsigned aBufferSize, unsigned aChannels, float aSamplerate, time aTime )
	{
		BufferSize = aBufferSize;
		
		if( PreviousBuffers.size() != aChannels )
		{
			for( unsigned int Index = 0; Index < aChannels; Index++ )
			{
				PreviousBuffers.emplace_back( new float[static_cast<unsigned int>( 4096 )] );
				std::memset( PreviousBuffers.back(), 0, sizeof( unsigned int ) * 4096 );
			}
		}

		FilterInstance::filter( aBuffer, aSamples, aBufferSize, aChannels, aSamplerate, aTime );
	}
	
	void LimiterInstance::filterChannel( float* aBuffer, unsigned int aSamples, float aSamplerate, time aTime, unsigned int aChannel, unsigned int aChannels )
	{
		FollowerL.Setup( 0.0f, mParam[Limiter::Release], aSamplerate );
		FollowerR.Setup( 0.0f, mParam[Limiter::Release], aSamplerate );

		// Pre-gain
		for( unsigned int Index = 0; Index < aSamples; Index++ )
		{
			aBuffer[Index] *= mParam[Limiter::PreGain];
		}

		if( aChannel == 0 )
		{
			// Calculate the envelope.
			FollowerL.Process( aBuffer, aSamples );
			const float EnvL = FollowerL.Envelope;

			if( EnvL > 1.0f )
			{
				for( unsigned int Index = 0; Index < aSamples; Index++ )
				{
					// aBuffer[Index] /= EnvL;
					PreviousBuffers[aChannel][Index] /= EnvL;
				}
			}
		}
		else if( aChannel == 1 )
		{
			// Calculate the envelope.
			FollowerR.Process( aBuffer, aSamples );
			const float EnvR = FollowerL.Envelope;

			if( EnvR > 1.0f )
			{
				for( unsigned int Index = 0; Index < aSamples; Index++ )
				{
					// aBuffer[Index] /= EnvR;
					PreviousBuffers[aChannel][Index] /= EnvR;
				}
			}
		}

		if( !ScratchBuffer )
		{
			ScratchBuffer = new float[static_cast<unsigned int>( 4096 )];
			std::memset( ScratchBuffer, 0, sizeof( unsigned int ) * 4096 );
		}
		
		std::memcpy( ScratchBuffer, aBuffer, sizeof( float ) * aSamples );
		std::memcpy( aBuffer, PreviousBuffers[aChannel], sizeof( float ) * aSamples );
		std::memcpy( PreviousBuffers[aChannel], ScratchBuffer, sizeof( float ) * aSamples );

		const float Threshold = mParam[Limiter::Threshold];
		if( Threshold > 0.0f && Threshold < 1.0f )
		{
			const float InverseThreshold = 1.0f / Threshold;
			for( unsigned int Index = 0; Index < aSamples; Index++ )
			{
				float& Signal = aBuffer[Index];
				const float& Amplitude = std::fabs( Signal );
				if( Signal > Threshold )
				{
					Signal = Threshold;
				}
				else if ( Signal < -Threshold )
				{
					Signal = -Threshold;
				}

				Signal *= InverseThreshold;
			}
		}

		// Post-gain
		for( unsigned int Index = 0; Index < aSamples; Index++ )
		{
			PreviousBuffers[aChannel][Index] *= mParam[Limiter::PostGain];
		}
	}
}
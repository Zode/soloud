// Compressor/Limiter Filter

#include "soloud.h"
#include "soloud_limiter.h"

#include <unordered_map>
#include <string>
#include <vector>

namespace SoLoud
{
	struct Parameter
	{
		Parameter( const std::string& Name, const int& Type, const float& Minimum, const float& Maximum )
		{
			this->Name = Name;
			this->Type = Type;
			this->Minimum = Minimum;
			this->Maximum = Maximum;
		}
		
		std::string Name;
		int Type = Filter::FLOAT_PARAM;
		float Minimum = 0.0f;
		float Maximum = 1.0f;
	};
	
	static const std::vector<Parameter> Parameters = {
		{ "Gain", Filter::FLOAT_PARAM, 0.0f,1.0f },
		{ "Attack", Filter::FLOAT_PARAM, 0.0f,100.0f },
		{ "Release", Filter::FLOAT_PARAM, 0.0f,100.0f }
	};

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
		this->Attack = Attack / SampleRate;
		this->Release = Release / SampleRate;
		Initialized = true;

		// Envelope = 0.0f;
	}

	void Follower::Process( float* Buffer, unsigned int Samples )
	{
		float NewEnvelope = 0.0f;
		float CumulativeAmplitude = 0.0f;
		for( unsigned int Index = 0; Index < Samples; Index++ )
		{
			const float& Amplitude = std::fabs( Buffer[Index] );
			/*if( Amplitude > Envelope )
			{
				Envelope += Amplitude + Amplitude * Attack;
			}
			else
			{
				Envelope -= Amplitude + Amplitude * Release;
			}*/
			
			if( Amplitude > NewEnvelope )
			{
				NewEnvelope += Amplitude;
			}

			CumulativeAmplitude += Amplitude;
		}

		// CumulativeAmplitude /= Samples;
		
		if( CumulativeAmplitude > Envelope )
		{
			Envelope += NewEnvelope * CumulativeAmplitude;
		}
		else
		{
			Envelope -= NewEnvelope * 1000.0f;
		}

		if( Envelope < 0.0f )
		{
			Envelope = 0.0f;
		}

		const float Ceiling = 10000.0f;
		if( Envelope > Ceiling )
		{
			Envelope = Ceiling;
		}

		/*const float Ceiling = 5.0f;
		if( Envelope > Ceiling )
		{
			Envelope = Ceiling;
		}
		else if( Envelope < 0.0f )
		{
			Envelope = std::fabs( Envelope );
		}*/
	}

	LimiterInstance::LimiterInstance( Limiter* aParent )
	{
		initParams( static_cast<int>( Parameters.size() ) );
	}
	
	void LimiterInstance::filterChannel( float* aBuffer, unsigned int aSamples, float aSamplerate, time aTime, unsigned int aChannel, unsigned int aChannels )
	{
		if( !FollowerL.Initialized )
		{
			FollowerL.Setup( 1.0f, 1.0f, aSamplerate );
		}

		FollowerL.Setup( 0.0f, 0.1f, aSamplerate );
		FollowerR.Setup( 0.0f, 0.1f, aSamplerate );

		// Pre-gain
		for( unsigned int Index = 0; Index < aSamples; Index++ )
		{
			aBuffer[Index] *= 1000.0f;
		}

		if( aChannel == 0 )
		{
			// Calculate the envelope.
			const float EnvL = FollowerL.Envelope;

			for( unsigned int Index = 0; Index < aSamples; Index++ )
			{
				if( EnvL > 1.0f )
				{
					aBuffer[Index] /= EnvL;
				}
			}

			FollowerL.Process( aBuffer, aSamples );
		}
		else if( aChannel == 1 )
		{
			// Calculate the envelope.
			const float EnvR = FollowerL.Envelope;

			for( unsigned int Index = 0; Index < aSamples; Index++ )
			{
				if( EnvR > 1.0f )
				{
					aBuffer[Index] /= EnvR;
				}
			}

			FollowerR.Process( aBuffer, aSamples );
		}

		// Post-gain
		for( unsigned int Index = 0; Index < aSamples; Index++ )
		{
			// aBuffer[Index] /= 100.0f;
		}

		// Apply the result.
		const float Threshold = 0.05f;
		const float InverseThreshold = 1.0f / Threshold;
		
		//for( unsigned int Index = 0; Index < aSamples; Index++ )
		//{
		//	if( std::fabs( Follower.Envelope ) > 1.0f )
		//	{
		//		aBuffer[Index] /= Follower.Envelope;
		//	}

		//	/*float& Signal = aBuffer[Index];
		//	const float& Amplitude = std::fabs( Signal );
		//	if( Amplitude > Threshold )
		//	{
		//		Signal /= Amplitude * InverseThreshold;
		//	}

		//	Signal *= Threshold;*/
		//}
	}
}
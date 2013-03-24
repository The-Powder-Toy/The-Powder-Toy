#include <math.h>
#include "Sound.h"
#include "Config.h"
#define PI 3.14159265358979
float note_freq[MAX_NOTE];
int note_len[MAX_NOTE];
int note_smp[MAX_NOTE];
void add_note(float f, int l)
{
	int i;
	for(i=0;i<MAX_NOTE;i++)
	{
		if(!note_len[i])
		{
			note_freq[i]=f;
			note_len[i]=l;
			note_smp[i]=0;
			break;
		}
	}
}
#define MIX(A,B,R) (A)*(R)+(B)*(1.0f-(R))
#define NOTE(F,L,S) (sin(2.0f*PI*(F)*(S)/44100.0f)*(1.0f-((float)(S))/(L)))
void create_tone(void *userdata, Uint8 *stream, int l)
{
	int i,k;
	for(;l;l--)
	{
		float smp=0.0f;
		for(i=0;i<MAX_NOTE;i++)
		{
			if(note_len[i])
			{
				float n=0.0f;
				n=NOTE(note_freq[i],note_len[i],note_smp[i])*1024.0f;
				n+=NOTE(note_freq[i]*2,note_len[i],note_smp[i])*256.0f;
				n+=NOTE(note_freq[i]*4,note_len[i],note_smp[i])*64.0f;
				smp=MIX(n,smp,1.0f/(i+1));
				note_smp[i]++;
				if(note_len[i]==note_smp[i])
					note_len[i]=0;
			}
		}
		smp=sqrt(smp);
		if(smp>127.0f)
			*stream=127;
		else if(smp<-128.0f)
			*stream=-128;
		else
			*stream=(signed char)smp;
		stream++;
	}
}

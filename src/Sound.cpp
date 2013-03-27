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
#define SINE(F,S) (sinf(2.0f*PI*(F)*(S)/44100.0f))
#define DECAY(F,S,L) (SINE(F,S)*(1.0f-(S)/(float)(L)))
void create_tone(void *userdata, Uint8 *stream, int l)
{
	int i,k;
	for(;l;l--)
	{
		float smp=0.0f;
		int a=0;
		for(i=0;i<MAX_NOTE;i++)
		{
			if(note_len[i])
			{
				a++;
				smp+=DECAY(note_freq[i],note_smp[i],note_len[i])*1024.0f;
				smp+=DECAY(note_freq[i]*2.0f,note_smp[i],note_len[i])*512.0f;
				smp+=DECAY(note_freq[i]*3.0f,note_smp[i],note_len[i])*341.3f;
				smp+=DECAY(note_freq[i]*4.0f,note_smp[i],note_len[i])*256.0f;
				smp+=DECAY(note_freq[i]*5.0f,note_smp[i],note_len[i])*204.0f;
				note_smp[i]++;
				if(note_len[i]==note_smp[i])
					note_len[i]=0;
			}
		}
		if(!a){
			for(;l;l--)
				*(stream++)=0;
			return;
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

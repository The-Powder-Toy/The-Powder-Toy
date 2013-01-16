#include <math.h>
#include "Sound.h"
#include "Config.h"
#define PI 3.14159265358979
float note_freq[MAX_NOTE];
int note_len[MAX_NOTE];
int note_smp[MAX_NOTE];
void add_note(float f)
{
	int i;
	for(i=0;i<MAX_NOTE;i++)
	{
		if(!note_len[i])
		{
			note_freq[i]=f;
			note_len[i]=22050*3;
			note_smp[i]=0;
			break;
		}
	}
}
void create_tone(void *userdata, Uint8 *stream, int l)
{
	int i,j;
	for(j=0;j<l;j++)
	{
		for(i=0;i<MAX_NOTE;i++)
		{
			if(note_len[i])
			{
				float tmpvar=0;
				tmpvar+=sin(PI*note_freq[i]/22050.0f*note_smp[i])*3.0f/8.0f;
				tmpvar+=sin(2.0f*PI*note_freq[i]/22050.0f*note_smp[i])*1.0f/8.0f;
				tmpvar+=sin(4.0f*PI*note_freq[i]/22050.0f*note_smp[i])*3.0f/8.0f;
				tmpvar+=sin(6.0f*PI*note_freq[i]/22050.0f*note_smp[i])*1.0f/16.0f;
				tmpvar+=sin(0.5f*PI*note_freq[i]/22050.0f*note_smp[i])*1.0f/16.0f;
				*stream+=(tmpvar*16);
				note_smp[i]++;
				if(note_len[i]==note_smp[i])
					note_len[i]=0;
			}
		}
		stream++;
	}
}

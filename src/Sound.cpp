#include <math.h>
#include "Sound.h"
#include "Config.h"

float note_freq[MAX_NOTE];
int note_len[MAX_NOTE];
void add_note(float f)
{
	int i;
	for(i=MAX_NOTE-2;i>0;i--)
	{
		note_freq[i+1]=note_freq[i];
		note_len[i+1]=note_len[i];
	}
	note_freq[0]=f;
	note_len[0]=88200;
}
void create_tone(void *userdata, Uint8 *stream, int l)
{
	int i,j;
	for(j=0;j<l;j++)
	{
		for(i=0;(i<MAX_NOTE)&&(note_len[i]);i++)
		{
			*stream+=(char)(sin(note_freq[i]/22050.0f*(note_len[i]--))*2.0f);
		}
		stream++;
	}
}

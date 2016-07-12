#include "Graphics.h"
#include "font.h"
#include "common/tpt-thread.h"
#ifdef OGLI

static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t TMPMUT = PTHREAD_MUTEX_INITIALIZER;
Graphics::Graphics():
	sdl_scale(1)
{
//	if(gMutex == TMPMUT)
//		pthread_mutex_init (&gMutex, NULL);
	LoadDefaults();
	InitialiseTextures();
	

	
	//Texture for main UI

}

void Graphics::LoadDefaults()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//glOrtho(0, WINDOWW*sdl_scale, 0, WINDOWH*sdl_scale, -1, 1);
	glOrtho(0, WINDOWW*sdl_scale, WINDOWH*sdl_scale, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glRasterPos2i(0, WINDOWH);
	glRasterPos2i(0, 0);
	glPixelZoom(1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Graphics::InitialiseTextures() 
{
	glEnable(GL_TEXTURE_2D);
	
	glGenTextures(1, &vidBuf);
	glBindTexture(GL_TEXTURE_2D, vidBuf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOWW, WINDOWH, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenTextures(1, &textTexture);
	glBindTexture(GL_TEXTURE_2D, textTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glDisable(GL_TEXTURE_2D);
}

void Graphics::DestroyTextures()
{
	//Todo...
}

void Graphics::Acquire()
{
	pthread_mutex_lock(&gMutex);
}

void Graphics::Release()
{
	pthread_mutex_unlock(&gMutex);
}

Graphics::~Graphics()
{
}

void Graphics::Reset()
{
	LoadDefaults();
	DestroyTextures();
	InitialiseTextures();
}

void Graphics::Clear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Graphics::Finalise()
{
    glFlush();
}

#define VIDXRES WINDOWW
#define VIDYRES WINDOWH
#define PIXELMETHODS_CLASS Graphics
#include "OpenGLDrawMethods.inl"
#undef VIDYRES
#undef VIDXRES
#undef PIXELMETHODS_CLASS


#endif

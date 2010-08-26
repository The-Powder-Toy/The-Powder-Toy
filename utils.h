#ifndef UTILS_H
#define UTILS_H

//Signum function
#ifdef WIN32
_inline int sign(float i);
#else
inline int sign(float i);
#endif

#ifdef WIN32
_inline unsigned clamp_flt(float f, float min, float max);
#else
inline unsigned clamp_flt(float f, float min, float max);
#endif

#ifdef WIN32
_inline float restrict_flt(float f, float min, float max);
#else
inline float restrict_flt(float f, float min, float max);
#endif

#endif
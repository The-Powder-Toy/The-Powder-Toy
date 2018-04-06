#ifndef TPT_MATH_
#define TPT_MATH_

#ifndef M_PI
#define M_PI 3.14159265f
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079633f
#endif

namespace tpt
{
	float sin(float angle);

	float cos(float angle);

	float tan(float angle);

	float asin(float angle);

	float acos(float angle);

	float atan(float ratio);

	float atan2(float y, float x);

}
#endif /* TPT_MATH_ */

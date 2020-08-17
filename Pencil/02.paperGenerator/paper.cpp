
#include "paper.h"

float Paper::clamp(float x) {
	if (x < 0) return 0.f;
	if (x > 1.f) return 1.f;
	else return x;
}

float Paper::peak(float pig) {
	return clamp(2.0f * pig);
}

float Paper::valley(float pig) {
	return clamp(2.f * pig - 1.f);
	//return clamp(1.f * pig);
}

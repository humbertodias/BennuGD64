/* pdclib math.h omits BSD/XSI constants such as M_PI. */
#ifndef __BENNUGD_XBOX_MATH_H
#define __BENNUGD_XBOX_MATH_H

#include_next <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif

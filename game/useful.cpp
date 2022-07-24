#include "useful.hpp"

#ifdef DEBUG
	bool enable_breakpoints = false;
#endif

int floor_div(int num, int den) {
	if ((num^den) > 0)
		return num/den;
	else {
		div_t res = div(num, den);
		return res.rem
			? res.quot-1 : res.quot;
	}
}

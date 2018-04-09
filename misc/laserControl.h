#include "Arduino.h"
#include "Utils.h"

// REM: in principle only used by the laserDisplay translation unit,
// but I may use in the main for tests. Therefore, everything should
// have external linkage (declared using the "extern"  keyword).
// Otherwise there will be link errors as there
// will be multiple definitions.

	namespace Lasers {

		//enum laserColor{red, green, blue, magenta, yellow};

		extern void init();


	}
extern void test();

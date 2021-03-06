#ifndef _RENDERER_H_
#define _RENDERER_H_

// REM: in principle only used in the serialCommands translation unit, but I may use it in the main for debug;
// Therefore, I need to declare everything with external linkage (using "extern" keyword in its declaration)

#include "Arduino.h"
#include "Class_P2.h"
#include "scannerDisplay.h"
//#include "hardware.h"
#include "Utils.h"

// namespace DefaultParamRender {
//
//     // Renderer engine:
//     extern const P2 defaultCenter;
//     extern const float defaultAngle;
//     extern const float defaultScaleFactor;
//
//     // Display enging:
// 	extern const bool defaultBlankingFlag; // (true means off or "blankingFlag")
// 	extern const uint32_t defaultDt; // inter-point delay in usec
//
// }


namespace Renderer2D {

	// ======== STATE VARIABLES  ======================================================
	// a) Simplified OpenGL-like "state variables":
	extern P2 center;
	extern float angle;
	extern float scaleFactor;

	// b) Number of points. In the future, it would be more interesting to have a
	// "resolution" variable. The number of points should be always smaller
	// than MAX_NUM_POINTS:
	extern uint16_t sizeBlueprint; // don't forget to set it properly before
	// starting the display engine. Normally there is no pb: it is automatically
	// set while drawing figures, plus it has a default start value of 0 (extern
	// global variable definition)

	extern void clearBlueprint();
	extern uint16_t getSizeBlueprint();

	extern const P2 getLastPoint();

	extern void addToBlueprint(const P2 _newPoint);
	extern void writeInBluePrintArray(uint16_t _index, const P2 &_newPoint);

	extern void renderFigure(); // render with current pose transformation

	//namespace { // "private"
		extern PointBuffer bluePrintArray;
	//}

} // end namespace

#endif

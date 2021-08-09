#include "telemetry.h"

void telemetry_init(){
#if defined TELEMETRY_USE_SPORT
	sport_init();
#else
	//Nothing
#endif
}

bool telemetry_update(){
	bool update = false;

#if defined TELEMETRY_USE_SPORT
	update = sport_update();
#else
	// nothing
#endif
	
	return update;
}
/******************************************************************************

GHAAS Water Balance/Transport Model V3.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2007, University of New Hampshire

MDRainSurfRunoff.c

balazs.fekete@unh.edu

*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <cm.h>
#include <MF.h>
#include <MD.h>

// Output
static int _MDOutRainSurfRunoffID   = MFUnset;

int MDRainSurfRunoffDef () {

	if (_MDOutRainSurfRunoffID != MFUnset) return (_MDOutRainSurfRunoffID);
	
	if ((MDRainInfiltrationDef () == CMfailed) ||
	    ((_MDOutRainSurfRunoffID  = MFVarGetID (MDVarRainSurfRunoff, "mm", MFInput, MFFlux, MFBoundary)) == CMfailed))
		return (CMfailed);
	return (_MDOutRainSurfRunoffID);
}

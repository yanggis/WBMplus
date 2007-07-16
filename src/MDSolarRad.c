/******************************************************************************

GHAAS Water Balance Model Library V1.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2004, University of New Hampshire

MDSolarRad.c

balazs.fekete@unh.edu

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cm.h>
#include <MF.h>
#include <MD.h>

static float _MDSRadISC (int doy) {
	float isc;
	isc = 1.0 - .0167 * cos((double) 0.0172 * (double) (doy - 3));
	return (1367.0 / (isc * isc));
}

static float _MDSRadDEC (int doy) {
	double dec;

	dec = sin (6.224111 + 0.017202 * (double) doy);
	dec = 0.39785 * sin (4.868961 + .017203 * (double) doy + 0.033446 * sin (dec));
	return ((float) asin (dec));
}

static float _MDSRadH (float lat,int doy,float dec) {
	float arg, h;
	
   arg = -tan(dec) * tan(lat);
	if (arg > 1.0) h = 0.0;         /* sun stays below horizon */ 
	else if (arg <  -1.0) h = M_PI; /* sun stays above horizon */
	else h = acos(arg);
	return (h);
}

static int _MDOutSRadDayLengthID = CMfailed;

static void _MDSRadDayLength (int itemID) {
/* daylength fraction of day */
/* Input */
	int doy;   /* day of the year */
	float lat; /* latitude in decimal radians */
/* Local */
	float dec;
/* Output */
	float dayLength;

	doy = MFDateGetDayOfYear ();
	lat = MFModelGetLatitude (itemID) / 180.0 * M_PI;

   dec = _MDSRadDEC (doy);

   if (fabs ((double) lat) > M_PI_2) lat = (M_PI_2 - (double) 0.01) * (lat > 0.0 ? 1.0 : -1.0);

	dayLength = _MDSRadH (lat,doy,dec) / M_PI;
	MFVarSetFloat (_MDOutSRadDayLengthID,itemID,dayLength);
}

int MDSRadDayLengthDef () {
	if (_MDOutSRadDayLengthID != CMfailed) return (_MDOutSRadDayLengthID);

	MFDefEntering ("Day length");
	if ((_MDOutSRadDayLengthID   = MFVarGetID (MDVarSRadDayLength, "1/d", MFOutput, MFState, false)) == CMfailed) return (CMfailed);
	MFDefLeaving ("Day length");
	return (MFVarSetFunction (_MDOutSRadDayLengthID,_MDSRadDayLength));
}

static int _MDOutSRadI0HDayID = CMfailed;

static void _MDSRadI0HDay (int itemID) {
/* daily potential solar radiation from Sellers (1965) */
/* Input */
	int   doy; /* day of the year */
	float lat; /* latitude in decimal degrees */
/* Local */
	float isc, dec, h;
/* Output */
	float i0hDay;

	doy = MFDateGetDayOfYear ();
	lat = MFModelGetLatitude (itemID) / 180.0 * M_PI;

	isc = _MDSRadISC (doy);
   dec = _MDSRadDEC (doy);

   if (fabs ((double) lat) > M_PI_2) lat = (M_PI_2 - (double) 0.01) * (lat > 0.0 ? 1.0 : -1.0);
	h = _MDSRadH (lat,doy,dec);

	i0hDay =	0.000001 * isc * (86400.0 / M_PI) *  (h * sin(lat) * sin(dec) + cos(lat) * cos(dec) * sin(h));
   MFVarSetFloat (_MDOutSRadI0HDayID,itemID,i0hDay);
}

int MDSRadI0HDayDef () {
	if (_MDOutSRadI0HDayID != CMfailed) return (_MDOutSRadI0HDayID);

	MFDefEntering ("I0H Day");
	if ((_MDOutSRadI0HDayID   = MFVarGetID (MDVarSRadI0HDay, "MJ/m2", MFOutput, MFState, false)) == CMfailed) return (CMfailed);
	MFDefLeaving ("I0H Day");
	return (MFVarSetFunction (_MDOutSRadI0HDayID,_MDSRadI0HDay));
}

static int _MDInputID, _MDGrossRadID;

static int _MDOutSolarRadID = CMfailed;

static void _MDSolarRadiationCloud (int itemID) {
/* Input */
	float cloud;
/* Output */
	float solarRad;
/* Local */

	cloud = 0.0;
	if (MFVarTestMissingVal (_MDInputID,  itemID) ||
	    MFVarTestMissingVal (_MDGrossRadID, itemID)) { 
		 MFVarSetMissingVal (_MDOutSolarRadID,   itemID); 
		CMmsgPrint(CMmsgInfo,"Missing Vars in MDSolarRAd ! LAT %f LON %f  \n",MFModelGetLatitude(itemID), MFModelGetLongitude(itemID));
		return;
	}

	solarRad = MFVarGetFloat (_MDGrossRadID, itemID);
	cloud    = MFVarGetFloat(_MDInputID,  itemID) ;
	//if (cloud >100 ) printf ("Error in Cloudcover! VarID %i MDSolarRad!CLD = %f \n",_MDInputID, cloud);
	if (fabs(cloud) > 100.0) printf ("cloud cover item %i  %f VarID %i \n",itemID, cloud, _MDInputID);
	cloud = cloud / 100.0;
		 
   solarRad = solarRad * (0.803 - (0.340 * cloud) - (0.458 * (float) pow ((double) cloud,(double) 2.0)));
	MFVarSetFloat (_MDOutSolarRadID,  itemID, solarRad);
}

static void _MDSolarRadiationSun (int itemID) {
/* Input */
	float sunShine;
/* Output */
	float solarRad;
/* Local */

	if (MFVarTestMissingVal (_MDInputID,  itemID) ||
		 MFVarTestMissingVal (_MDGrossRadID, itemID)) { MFVarSetMissingVal (_MDOutSolarRadID,   itemID); return; }

	solarRad = MFVarGetFloat (_MDGrossRadID, itemID);
	sunShine = MFVarGetFloat (_MDInputID,  itemID) / 100.;
printf ("Hier \n");
	solarRad = solarRad * (0.251 + 0.509 * sunShine);
	MFVarSetFloat (_MDOutSolarRadID,  itemID, solarRad);
}
 

enum { MDhelp, MDinput, MDcloud, MDsun };
enum { MDstandard = 1,  MDOtto = 2 }; 

int MDSolarRadDef () {
	int optID = MDinput;
	const char *optStr, *optName = MDVarSolarRadiation;
	const char *options [] = { MDHelpStr, "input", "cloud", "sun", (char *) NULL };

	if (_MDOutSolarRadID != CMfailed) return (_MDOutSolarRadID);

	MFDefEntering ("Solar Radiation");
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);

	switch (optID) {
		case MDinput: _MDOutSolarRadID = MFVarGetID (MDVarSolarRadiation, "MJ/m^2", MFInput,  MFFlux,  false); break;
		case MDcloud:
		//	printf ("Option: Cloud!!!");
			if (((_MDGrossRadID      = MDGrossRadDef ()) == CMfailed) ||
				 ((_MDInputID         = MFVarGetID (MDVarCloudCover,     "Pro%",      MFInput,  MFState, false)) == CMfailed) ||
			    ((_MDOutSolarRadID   = MFVarGetID (MDVarSolarRadiation, "MJ/m^2", MFOutput, MFFlux,  false)) == CMfailed))
				return (CMfailed);
			_MDOutSolarRadID = MFVarSetFunction (_MDOutSolarRadID,_MDSolarRadiationCloud);
			break;
		case MDsun:
			if (((_MDGrossRadID      = MDGrossRadDef ()) == CMfailed) ||
			    ((_MDInputID         = MFVarGetID (MDVarSunShine,       "%",      MFInput,  MFState, false)) == CMfailed) ||
			    ((_MDOutSolarRadID   = MFVarGetID (MDVarSolarRadiation, "MJ/m^2", MFOutput, MFFlux,  false)) == CMfailed))
				return (CMfailed);
			_MDOutSolarRadID = MFVarSetFunction (_MDOutSolarRadID,_MDSolarRadiationSun);
			break;
		default:
			CMmsgPrint (CMmsgInfo,"Help [%s options]:",optName);
			for (optID = 1;options [optID] != (char *) NULL;++optID) CMmsgPrint (CMmsgInfo," %s",options [optID]);
			CMmsgPrint (CMmsgInfo,"\n");
			return (CMfailed);
	}
	MFDefLeaving ("Solar Radiation");
	return (_MDOutSolarRadID);
}
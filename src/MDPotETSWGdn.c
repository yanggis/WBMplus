/******************************************************************************

GHAAS Water Balance Model Library V1.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2004, University of New Hampshire

MDPotETSWGdn.c

balazs.fekete@unh.edu

*******************************************************************************/

#include<stdio.h>
#include<math.h>
#include<cm.h>
#include<MF.h>
#include<MD.h>

static int _MDInDayLengthID, _MDInI0HDayID;
static int _MDInCParamAlbedoID, _MDInCParamCHeightID, _MDInCParamLWidthID;
static int _MDInCParamRSSID,    _MDInCParamR5ID,      _MDInCParamCDID,     _MDInCParamCRID,  _MDInCParamGLMaxID, _MDInCParamZ0gID;
static int _MDInLeafAreaIndexID, _MDInStemAreaIndexID;

static int _MDInAtMeanID, _MDInAtMinID, _MDInAtMaxID, _MDInSolRadID, _MDInVPressID, _MDInWSpeedID;
static int _MDOutPetID = CMfailed;

static void _MDPotETSWGdn (int itemID) {
/* Input */
	float dayLen;  /* daylength in fraction of day */
 	float i0hDay;  /*  daily potential insolation on horizontal [MJ/m2] */
	float albedo;  /* albedo  */
	float height;  /* canopy height [m] */
	float lWidth;  /* average leaf width [m] */
	float rss;     /* soil surface resistance [s/m] */
	float r5;      /* solar radiation at which conductance is halved [W/m2] */
	float cd;      /* vpd at which conductance is halved [kPa] */
	float cr;      /* light extinction coefficient for projected LAI */
	float glMax;   /* maximum leaf surface conductance for all sides of leaf [m/s] */
	float z0g;     /* z0g       - ground surface roughness [m] */
 	float lai;     /* projected leaf area index */
	float sai;     /* projected stem area index */
	float airT;    /* air temperatur [degree C] */
	float airTMin; /* daily minimum air temperature [degree C] */
	float airTMax; /* daily maximum air temperature [degree C] */
	float solRad;  /* daily solar radiation on horizontal [MJ/m2] */
	float vPress;  /* daily average vapor pressure [kPa] */
	float wSpeed;  /* average wind speed for the day [m/s]  */
	float sHeat = 0.0; /* average subsurface heat storage for day [W/m2] */
/* Local */
	float solNet;  /* average net solar radiation for daytime [W/m2] */
	float airTDtm, airTNtm; /* air temperature for daytime and nighttime [degree C] */
	float uaDtm,   uaNtm;	/* average wind speed for daytime and nighttime [m/s] */
	float lngDtm,	lngNtm;	/* average net longwave radiation for daytime and nighttime [W/m2] */
	float z0;      /* roughness parameter [m]  */
 	float disp;    /* height of zero-plane [m] */
	float z0c;     /* roughness parameter (closed canopy) */
	float dispc;   /* zero-plane displacement (closed canopy) */
	float aa;		/* available energy [W/m2] */
	float asubs;	/* available energy at ground [W/m2] */
	float es;      /* vapor pressure at airT [kPa] */
	float delta;   /* dEsat/dTair [kPa/K] */
 	float dd;      /* vapor pressure deficit [kPa] */
 	float rsc;		/* canopy resistance [s/m] */
	float led, len;	/* daytime and nighttime latent heat [W/m2] */
	float rn;		/* net radiation [W/m2] */
	float rns;		/* net radiation at ground [W/m2] */
	float raa;		/* aerodynamic resistance [s/m] */
	float rac;		/* leaf boundary layer resistance [s/m] */
	float ras;		/* ground aerodynamic resistance  [s/m] */
/* Output */
	float pet;

	if (MFVarTestMissingVal (_MDInDayLengthID,    itemID) ||
		 MFVarTestMissingVal (_MDInI0HDayID,       itemID) ||
		 MFVarTestMissingVal (_MDInCParamAlbedoID, itemID) ||
		 MFVarTestMissingVal (_MDInCParamCHeightID,itemID) ||
		 MFVarTestMissingVal (_MDInCParamLWidthID, itemID) ||
		 MFVarTestMissingVal (_MDInCParamRSSID,    itemID) ||
		 MFVarTestMissingVal (_MDInCParamR5ID,     itemID) ||
		 MFVarTestMissingVal (_MDInCParamCDID,     itemID) ||
		 MFVarTestMissingVal (_MDInCParamCRID,     itemID) ||
		 MFVarTestMissingVal (_MDInCParamGLMaxID,  itemID) ||
		 MFVarTestMissingVal (_MDInCParamZ0gID,    itemID) ||
		 MFVarTestMissingVal (_MDInLeafAreaIndexID,itemID) ||
		 MFVarTestMissingVal (_MDInStemAreaIndexID,itemID) ||
		 MFVarTestMissingVal (_MDInAtMeanID,       itemID) ||
		 MFVarTestMissingVal (_MDInAtMinID,        itemID) ||
		 MFVarTestMissingVal (_MDInAtMaxID,        itemID) ||
		 MFVarTestMissingVal (_MDInSolRadID,       itemID) ||
		 MFVarTestMissingVal (_MDInVPressID,       itemID) ||
		 MFVarTestMissingVal (_MDInWSpeedID,       itemID)) { MFVarSetMissingVal (_MDOutPetID,itemID); return; }

	dayLen  = MFVarGetFloat (_MDInDayLengthID,    itemID);
	i0hDay  = MFVarGetFloat (_MDInI0HDayID,       itemID);
	albedo  = MFVarGetFloat (_MDInCParamAlbedoID, itemID);
	height  = MFVarGetFloat (_MDInCParamCHeightID,itemID);
	lWidth  = MFVarGetFloat (_MDInCParamLWidthID, itemID);
	rss     = MFVarGetFloat (_MDInCParamRSSID,    itemID);
	r5      = MFVarGetFloat (_MDInCParamR5ID,     itemID);
	cd      = MFVarGetFloat (_MDInCParamCDID,     itemID);
	cr      = MFVarGetFloat (_MDInCParamCRID,     itemID);
	glMax   = MFVarGetFloat (_MDInCParamGLMaxID,  itemID);
	z0g     = MFVarGetFloat (_MDInCParamZ0gID,    itemID);
	lai     = MFVarGetFloat (_MDInLeafAreaIndexID,itemID);
	sai     = MFVarGetFloat (_MDInStemAreaIndexID,itemID);
	airT    = MFVarGetFloat (_MDInAtMeanID,       itemID);
	airTMin = MFVarGetFloat (_MDInAtMinID,        itemID);
	airTMax = MFVarGetFloat (_MDInAtMaxID,        itemID);
	solRad  = MFVarGetFloat (_MDInSolRadID,       itemID);
	vPress  = MFVarGetFloat (_MDInVPressID,       itemID);
	wSpeed  = fabs (MFVarGetFloat (_MDInWSpeedID, itemID));
	if (wSpeed < 0.2) wSpeed = 0.2;

	solNet  = (1.0 - albedo) * solRad / (MDConstIGRATE * dayLen);

	z0c     = MDPETlibRoughnessClosed (height,z0g);
	dispc   = height - z0c / 0.3;
	disp    = MDPETlibZPDisplacement (height,lai,sai,z0g);
	z0      = MDPETlibRoughness (disp,height,lai,sai,z0g);

/* daytime */
	if (dayLen > 0.0) {
		airTDtm = airT + ((airTMax - airTMin) / (2 * M_PI * dayLen)) * sin (M_PI * dayLen);
		uaDtm   = wSpeed / (dayLen + (1.0 - dayLen) * MDConstWNDRAT);
		lngDtm  = MDSRadNETLong (i0hDay,airTDtm,solRad,vPress);

		rn      = solNet + lngDtm; 
		aa      = rn - sHeat; 
		rns     = rn * exp (-cr * (lai + sai));
		asubs   = rns - sHeat;
		es      = MDPETlibVPressSat (airTDtm);
		delta   = MDPETlibVPressDelta (airTDtm);
		dd      = es - vPress; 

		rsc     = MDPETlibCanopySurfResistance (airTMin,solRad / dayLen,dd,lai,sai,r5,cd,cr,glMax);
		raa     = MDPETlibBoundaryResistance (uaDtm,height,z0g,z0c,dispc,z0,disp);
		rac     = MDPETlibLeafResistance (uaDtm,height,lWidth,z0g,lai,sai,z0c,dispc);
		ras     = MDPETlibGroundResistance (uaDtm,height,z0g,z0c,dispc,z0,disp);
		led     = MDPETlibShuttleworthWallace (rss,aa,asubs,dd,raa,rac,ras,rsc,delta);
	}
	else led = 0.0;

/* nighttime */
	if (dayLen < 1.0) {
		airTNtm = airT - ((airTMax - airTMin) / (2 * M_PI * (1 - dayLen))) * sin (M_PI * dayLen);
		uaNtm   = MDConstWNDRAT * uaDtm;
		lngNtm  = MDSRadNETLong (i0hDay,airTNtm,solRad,vPress);

		rn = lngNtm;
		aa = rn - sHeat; 
		rns = rn * exp (-cr * (lai + sai));
		asubs = rns - sHeat; 

		es      = MDPETlibVPressSat (airTNtm);
		delta   = MDPETlibVPressDelta (airTNtm);
		dd      = es - vPress; 
		rsc     = 1.0 / (MDConstGLMIN * lai);
		raa     = MDPETlibBoundaryResistance (uaNtm,height,z0g,z0c,dispc,z0,disp);
		rac     = MDPETlibLeafResistance (uaNtm,height,lWidth,z0g,lai,sai,z0c,dispc);
		ras     = MDPETlibGroundResistance (uaNtm,height,z0g,z0c,dispc,z0,disp);
		len     = MDPETlibShuttleworthWallace (rss,aa,asubs,dd,raa,rac,ras,rsc,delta);
	}
	else len = 0.0;

	pet = MDConstEtoM * MDConstIGRATE * (dayLen * led + (1.0 - dayLen) * len);
   MFVarSetFloat (_MDOutPetID,itemID,pet);
}

int MDPotETSWGdnDef () {
	if (_MDOutPetID != CMfailed) return (_MDOutPetID);

	MFDefEntering ("PotET Shuttleworth - Wallace (day-night)");
	if (((_MDInDayLengthID     = MDSRadDayLengthDef ()) == CMfailed) ||
		 ((_MDInI0HDayID        = MDSRadI0HDayDef    ()) == CMfailed) ||
	    ((_MDInCParamAlbedoID  = MDCParamAlbedoDef  ()) == CMfailed) ||
		 ((_MDInCParamCHeightID = MDCParamCHeightDef ()) == CMfailed) ||
		 ((_MDInCParamLWidthID  = MDCParamLWidthDef  ()) == CMfailed) ||
		 ((_MDInCParamRSSID     = MDCParamRSSDef     ()) == CMfailed) ||
		 ((_MDInCParamR5ID      = MDCParamR5Def      ()) == CMfailed) ||
		 ((_MDInCParamCDID      = MDCParamCDDef      ()) == CMfailed) ||
		 ((_MDInCParamCRID      = MDCParamCRDef      ()) == CMfailed) ||
		 ((_MDInCParamGLMaxID   = MDCParamGLMaxDef   ()) == CMfailed) ||
		 ((_MDInCParamZ0gID     = MDCParamZ0gDef     ()) == CMfailed) ||
		 ((_MDInLeafAreaIndexID = MDLeafAreaIndexDef ()) == CMfailed) ||
		 ((_MDInStemAreaIndexID = MDStemAreaIndexDef ()) == CMfailed) ||
		 ((_MDInSolRadID        = MDSolarRadDef      ()) == CMfailed) ||
		 ((_MDInAtMeanID  = MFVarGetID (MDVarAirTemperature, "degC",  MFInput,  MFState, false)) == CMfailed) ||
		 ((_MDInAtMinID   = MFVarGetID (MDVarAirTempMinimum, "degC",  MFInput,  MFState, false)) == CMfailed) ||
		 ((_MDInAtMaxID   = MFVarGetID (MDVarAirTempMaximum, "degC",  MFInput,  MFState, false)) == CMfailed) ||
		 ((_MDInVPressID  = MFVarGetID (MDVarVaporPressure,  "kPa",   MFInput,  MFState, false)) == CMfailed) ||
		 ((_MDInWSpeedID  = MFVarGetID (MDVarWindSpeed,      "m/s",   MFInput,  MFState, false)) == CMfailed) ||
		 ((_MDOutPetID    = MFVarGetID (MDVarPotEvapotrans,  "mm",    MFOutput, MFFlux,  false)) == CMfailed))
		return (CMfailed);
	MFDefLeaving ("PotET Shuttleworth - Wallace (day-night)");
	return(MFVarSetFunction (_MDOutPetID,_MDPotETSWGdn));
}
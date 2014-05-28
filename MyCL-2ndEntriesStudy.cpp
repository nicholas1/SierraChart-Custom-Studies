// The top of every source code file must include this line
#include "sierrachart.h"
#include "MySierraChart.h"
#include "Find2ndEntries.h"


// ************************************************************************************
//
// This code is provided on an "AS IS" basis, without warranty of any kind,
// including without limitation the warranties of merchantability, fitness for a
// particular purpose and non-infringement.
//
// Copyright (c) 2013
// mlwin1@yahoo.com
//
// ************************************************************************************




/*****************************************************************************

	For reference, please refer to the Advanced Custom Study Interface
	and Language documentation on the Sierra Chart website. 
	
*****************************************************************************/

// This line is required. Change the text within the quote
// marks to what you want to name your group of custom studies. 
SCDLLName("MyCL-2ndEntriesStudy") 

char arTempStr[512];
char arTempStr2[512];

inline void EnterLongOrder(SCStudyGraphRef sc)
{
	float& entryPrice  = sc.PersistVars->f1;
	float& vwapAtEntry = sc.PersistVars->f2;
	float& vwapStd1    = sc.PersistVars->f3;
	
	// Create an s_SCNewOrder object. 
	s_SCNewOrder NewOrder;
	NewOrder.OrderQuantity = 1;
	//NewOrder.OrderType = SCT_LIMIT;
	NewOrder.OrderType = SCT_MARKET;
	NewOrder.Price1 = sc.High[sc.Index-1];

	int orderResult;
	
	s_SCPositionData PositionData;
	sc.GetTradePosition(PositionData);
	
	int& lastEntryBar   = sc.PersistVars->i3;
	
	if(PositionData.PositionQuantity == 0 &&
	   sc.Index != lastEntryBar)
	{
		orderResult = sc.BuyEntry(NewOrder);
		if(orderResult > 0)
		{
			lastEntryBar = sc.Index;
			
			// draw arrow
			DrawMarker(sc, sc.Index, sc.Index, sc.Low[sc.Index]-sc.TickSize*8, MARKER_ARROWUP, COLOR_GREEN);
			
			// save study values
			SCFloatArray vwapRef;
			SCFloatArray vwapRefStd;
			//Get the first (0) subgraph from the study the user has selected.
			sc.GetStudyArrayUsingID(sc.Input[2].GetStudyID(), 0, vwapRef);
			sc.GetStudyArrayUsingID(sc.Input[2].GetStudyID(), 1, vwapRefStd);
			entryPrice  = sc.High[sc.Index-1];
			vwapAtEntry = vwapRef[sc.Index];
			vwapStd1    = vwapRefStd[sc.Index];
		}
	}

}

inline void EnterShortOrder(SCStudyGraphRef sc)
{
	float& entryPrice  = sc.PersistVars->f1;
	float& vwapAtEntry = sc.PersistVars->f2;
	float& vwapStd1    = sc.PersistVars->f3;
	
	// Create an s_SCNewOrder object. 
	s_SCNewOrder NewOrder;
	NewOrder.OrderQuantity = 1;
	//NewOrder.OrderType = SCT_LIMIT;
	NewOrder.OrderType = SCT_MARKET;
	NewOrder.Price1 = sc.Low[sc.Index-1];
	
	int orderResult;
	
	s_SCPositionData PositionData;
	sc.GetTradePosition(PositionData);
	
	int& lastEntryBar = sc.PersistVars->i3;
	
	if(PositionData.PositionQuantity == 0 &&
	   sc.Index != lastEntryBar)
	{
		orderResult = sc.SellEntry(NewOrder);
		if(orderResult > 0)
		{
			lastEntryBar = sc.Index;
			
			// draw arrow
			DrawMarker(sc, sc.Index, sc.Index, sc.High[sc.Index]+sc.TickSize*8, MARKER_ARROWDOWN, COLOR_RED);
			
			// save study values
			SCFloatArray vwapRef;
			SCFloatArray vwapRefStd;
			//Get the first (0) subgraph from the study the user has selected.
			sc.GetStudyArrayUsingID(sc.Input[2].GetStudyID(), 0, vwapRef);
			sc.GetStudyArrayUsingID(sc.Input[2].GetStudyID(), 1, vwapRefStd);
			entryPrice  = sc.High[sc.Index-1];
			vwapAtEntry = vwapRef[sc.Index];
			vwapStd1    = vwapRefStd[sc.Index];
		}
	}

}

//This is the basic framework of a study function.
SCSFExport scsf_MyCL_2ndEntriesStudy(SCStudyGraphRef sc)
{
	SCSubgraphRef LongSignal    = sc.Subgraph[0];
	SCSubgraphRef ShortSignal   = sc.Subgraph[1];
    
    SCSubgraphRef Long1stEntry  = sc.Subgraph[2];
    SCSubgraphRef Short1stEntry = sc.Subgraph[3];
		
    SCSubgraphRef AuxSignal = sc.Subgraph[4];
	//SCSubgraphRef MainMA    = sc.Subgraph[5];
	
	SCSubgraphRef FastFilter = sc.Subgraph[6];
	SCSubgraphRef SlowFilter = sc.Subgraph[7];
    
	int longEntryStateOld, shortEntryStateOld;

	float& entryPrice  = sc.PersistVars->f1;
	float& vwapAtEntry = sc.PersistVars->f2;
	float& vwapStd1    = sc.PersistVars->f3;	
	
	int& longEntryState  = sc.PersistVars->i1;
	int& shortEntryState = sc.PersistVars->i2;
	
	int& lastEntryBar   = sc.PersistVars->i3;
	int& longsEnable  = sc.PersistVars->i4;
	int& shortsEnable = sc.PersistVars->i5;
	
	SCInputRef length      = sc.Input[0];
	SCInputRef enabled     = sc.Input[1];
	SCInputRef studyID     = sc.Input[2];
	SCInputRef targetValue = sc.Input[3];
	SCInputRef stopValue   = sc.Input[4];
    
	float highest, lowest;
	
	// Section 1 - Set the configuration variables and defaults
	if (sc.SetDefaults)
	{
		sc.GraphName = "My CL 2nd Entries Study";
		
		// During development set this flag to 1, so the DLL can be rebuilt without restarting Sierra Chart. When development is completed, set it to 0 to improve performance.
		sc.FreeDLL = 1;

		sc.AutoLoop = 1;  //Auto looping is enabled. 
		sc.GraphRegion = 0;
			
		LongSignal.Name = "Long Signal";
		LongSignal.DrawStyle = DRAWSTYLE_ARROWUP;
		LongSignal.PrimaryColor = COLOR_AQUAMARINE;
		LongSignal.LineWidth = 2;
		LongSignal.DrawZeros = false;
		
		ShortSignal.Name = "Short Signal";
		ShortSignal.DrawStyle = DRAWSTYLE_ARROWDOWN;
		ShortSignal.PrimaryColor = COLOR_MAGENTA;
		ShortSignal.LineWidth = 2;
		ShortSignal.DrawZeros = false;
        
        Long1stEntry.Name = "Long 1st Entry";
		Long1stEntry.DrawStyle = DRAWSTYLE_TRIANGLEUP;
		Long1stEntry.PrimaryColor = COLOR_AQUAMARINE;
		Long1stEntry.LineWidth = 2;
		Long1stEntry.DrawZeros = false;
        
        Short1stEntry.Name = "Short 1st Entry";
		Short1stEntry.DrawStyle = DRAWSTYLE_TRIANGLEDOWN;
		Short1stEntry.PrimaryColor = COLOR_MAGENTA;
		Short1stEntry.LineWidth = 2;
		Short1stEntry.DrawZeros = false;
		
		AuxSignal.Name = "Aux Signal";
		AuxSignal.DrawStyle = DRAWSTYLE_DIAMOND;
		AuxSignal.PrimaryColor = COLOR_YELLOW;
		AuxSignal.LineWidth = 2;
		AuxSignal.DrawZeros = false;
		
		sc.Input[0].Name = "Count Length";
		sc.Input[0].SetInt(55);
		
		enabled.Name = "Enable Trading";
		enabled.SetYesNo(0); 
		
		studyID.Name = "VWAP Study";
		studyID.SetStudyID(8);
		
		targetValue.Name = "Target Value";
		targetValue.SetFloat(.08f);
		
		stopValue.Name = "Stop Value";
		stopValue.SetFloat(.15f);
        		
		longEntryState   = eStateInitial;
		shortEntryState  = eStateInitial;
		
		sc.AllowMultipleEntriesInSameDirection = false; 
		sc.MaximumPositionAllowed = 1;
		sc.SupportReversals = 0;
		
		// ** This false by default. Orders will go to the simulation system always. **
		// @@
		sc.SendOrdersToTradeService = false;
			
		sc.AllowOppositeEntryWithOpposingPositionOrOrders = false;
		
		sc.SupportAttachedOrdersForTrading = false;
		// ** this line will causes to not work
		//sc.TradeWindowConfigFileName = "1Contract.twconfig";
		
		sc.CancelAllOrdersOnEntriesAndReversals= true;
		sc.AllowEntryWithWorkingOrders = false;
		sc.CancelAllWorkingOrdersOnExit = false;
		
		// This is true by default, only 1 trade can be processed per bar.
		sc.AllowOnlyOneTradePerBar = true; 
		sc.MaintainTradeStatisticsAndTradesData = true;
		
		longsEnable  = 1;
		shortsEnable = 1;
		
		return;
	}
	
	// Time filter
	int filterStartTime = HMS_TIME(10, 45, 0);
	int filterEndTime   = HMS_TIME(15, 00, 0);
	int barTime = sc.BaseDateTimeIn.TimeAt(sc.Index);
	
    if(barTime < filterStartTime || barTime > filterEndTime)
    {
       return;
    }
	
	// Section 2 - Do data processing here
	float lastTradePrice = sc.Close[sc.Index];
	
	// Create an s_SCNewOrder object. 
	s_SCNewOrder NewOrder;
	NewOrder.OrderQuantity = 1;
	NewOrder.OrderType = SCT_MARKET;
	int orderResult;
	
	s_SCPositionData InternalPositionData;
	sc.GetTradePosition(InternalPositionData);
	
	float avgP = InternalPositionData.AveragePrice;

	if (InternalPositionData.PositionQuantity > 0
		&& (lastTradePrice <= (avgP-stopValue.GetFloat()) || 
			lastTradePrice >= (avgP+targetValue.GetFloat()))) 
	{
		orderResult = sc.BuyExit(NewOrder);
		if(orderResult>0) 
		{
			if(lastTradePrice >= (avgP+targetValue.GetFloat()))
			{
				GetFormattedDateStr(sc, arTempStr2);
				// save to file
				sprintf(arTempStr, "%s, %.2f, %.2f, %.2f", arTempStr2, entryPrice, vwapAtEntry, vwapStd1);
				AppendToTextFile("c:\\trades\\CL_VWAP_Longs.cvs", arTempStr);
			}

			return;
		}
	}
	else if (InternalPositionData.PositionQuantity < 0 
			 && (lastTradePrice >= (avgP+stopValue.GetFloat()) ||
				 lastTradePrice <= (avgP-targetValue.GetFloat()))) 
	{
		orderResult = sc.SellExit(NewOrder);
		if(orderResult>0)
		{
			if(lastTradePrice <= (avgP-targetValue.GetFloat()))
			{
				GetFormattedDateStr(sc, arTempStr2);
				// save to file
				sprintf(arTempStr, "%s, %.2f, %.2f, %.2f", arTempStr2, entryPrice, vwapAtEntry, vwapStd1);
				AppendToTextFile("c:\\trades\\CL_VWAP_Shorts.cvs", arTempStr);
			}

			return;
		}
	}
	
	longEntryStateOld  = longEntryState; 
	shortEntryStateOld = shortEntryState;

	// Determine current entry state for bars
	BarEntryState(sc, length.GetInt(), longEntryState, shortEntryState);
	
	// Handle long entry states
	if(longEntryStateOld == eStateCount0Retrace && 
	   longEntryState == eStateCount1)
	{
		// 1st entry long
        Long1stEntry[sc.Index] = sc.Low[sc.Index]-sc.TickSize*2;
	}
	else if(longEntryStateOld == eStateCount1Retrace &&
		    longEntryState == eStateCount2)
	{
		// 2nd entry long
		LongSignal[sc.Index] = sc.Low[sc.Index]-sc.TickSize*2;

		if(longsEnable && enabled.GetYesNo())
			EnterLongOrder(sc);
	}

	// Handle short entry states
	if(shortEntryStateOld == eStateCount0Retrace &&
	   shortEntryState == eStateCount1)
	{
		// 1st entry
        Short1stEntry[sc.Index] = sc.High[sc.Index]+sc.TickSize*2;
	}
	else if(shortEntryStateOld == eStateCount1Retrace &&
		    shortEntryState == eStateCount2)
	{
		// 2nd entry
		ShortSignal[sc.Index] = sc.High[sc.Index]+sc.TickSize*2;

		if(shortsEnable && enabled.GetYesNo())
			EnterShortOrder(sc);
	}
	
	/*
	// save study values
	SCFloatArray vwapRef;
	SCFloatArray vwapRefStd;
	//Get the first (0) subgraph from the study the user has selected.
	sc.GetStudyArrayUsingID(sc.Input[2].GetStudyID(), 0, vwapRef);
	sc.GetStudyArrayUsingID(sc.Input[2].GetStudyID(), 1, vwapRefStd);

	vwapAtEntry = vwapRef[sc.Index];
	vwapStd1    = vwapRefStd[sc.Index];
			
	sprintf(arTempStr, "%.2f", vwapAtEntry);		
	DrawSimText1(sc, arTempStr);		
	
	sprintf(arTempStr, "%.2f", vwapStd1);	
	DrawSimText2(sc, arTempStr);	
	*/
	
}

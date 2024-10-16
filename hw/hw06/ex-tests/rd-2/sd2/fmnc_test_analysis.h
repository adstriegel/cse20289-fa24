/*
 * fmnc_test_analysis.h
 *
 *  Created on: Jul 17, 2014
 *      Author: striegel
 */

#ifndef FMNC_TEST_ANALYSIS_H_
#define FMNC_TEST_ANALYSIS_H_

#include <string>
using namespace std;

#include "fmnc_test_sequence.h"
#include "fmnc_measurement_packet.h"
#include "fmnc_measurement_pair.h"

class GraphD3JSLine;
/** An analysis of a FMNC Test Sequence which is a series of sliced packets examined
 * for RTT pairs
 */
class FMNC_Test_Analysis
{
	public:
		FMNC_Test_Analysis ();
		~FMNC_Test_Analysis ();

		/** Set the list of measurement packets
		 * @param pSent The list of sent measurement packets
		 * @param pRcvd The list of received measurement packets
		 */
		void	setMeasurementLists (FMNC_List_Measurement_Packets * pSent,
									 FMNC_List_Measurement_Packets * pRcvd);

		/** Determine if this is a valid set of measurements capable of being
		 * analyzed
		 */
		bool	isValidMeasurement  ();

		bool	isValidMeasurement  (uint16_t nGroup);

		/** Do an analysis of the underlying measurement packets
		 */
		bool	doAnalysis ();

		/** Do an analysis of the underlying measurement packets
		 */
		bool	doAnalysis (uint16_t nGroup);

        //Analysis the flucation
		bool	do_fluctuation_analysis (uint32_t lAnalyzed,double duration,uint16_t * mRTT);
		bool	do_hiccup_analysis (uint32_t lAnalyzed);

		/** Get the minimum RTT pair for the test
		 * @return Minimum RTT value of the various pairs
		 */
		double	getRTT_Min  ();

		/** Get the maximum RTT pair for the test
		 * @return Maximum RTT value of the various pairs
		 */
		double	getRTT_Max  ();

		/** Get the mean RTT pair for the test
		 * @return Mean RTT value of the various pairs
		 */
		double	getRTT_Mean ();


		/** Get the median RTT pair for the test
		 * @return Median RTT value of the various pairs
		 */
		double	getRTT_Median ();

		/** Get the standard deviation of RTT pair for the test
		 * @return Standard deviation of RTT pairs
		 */
		double 	getRTT_StdDev ();

		/** Get the total time across all measurement pairs
		 * @returns Total time in milliseconds for the measurements
		 */
		double	getTest_Time ();

		/** Denotes if the analysis has been conducted on this set of data
		 * @returns True if the values are valid, false otherwise
		 */
		bool	isAnalysisDone ();

        int getFluctuation_Result();
		/** Dump the analysis results to the console
		 */
		void	dumpAnalysisResults ();

		/** Construct a web-based analysis page
		 */
		string	constructWeb_Analysis ();


		void setFlag_CreateDictionary (bool bCreateDict);
		bool getFlag_CreateDictionary ();
        string GetGraphD3JS ();

		ParamDictionary * getDictionary ();
		bool			  allocateDictionary ();

	protected:
		/** Compute the mean value from the measurement pairs
		 * @returns Resulting computation for the mean
		 */
		double	computeMean ();

		/** Compute the standard deviation
		 * @returns Resulting computation for the standard deviation value
		 */
		double	computeStandardDeviation ();

		/** Create a D3JS graph from this particular analysis
		 *
		 */
		bool	createGraphD3JS ();

		/** Construct a table for output to a web page */
		string	constructTable_Web ();


	private:
		FMNC_List_Measurement_Packets 	*	m_pMeasure_Sent;
		FMNC_List_Measurement_Packets 	*	m_pMeasure_Rcvd;

		FMNC_List_Measurement_Pairs			m_Pairs;

		bool			m_bAnalysisDone;

		// Information with regards to the analysis
		double			m_fRTT_Min;
		double			m_fRTT_Max;
		double			m_fRTT_Mean;
		double			m_fRTT_Median;
		double			m_fRTT_StdDev;
        // inter ACK analysis

		double			m_fInterAck_Min;
		double			m_fInterAck_Max;
		double			m_fInterAck_Mean;
		double			m_fInterAck_Median;
		double			m_fInterAck_StdDev;

		double			m_fTest_Time;
        
        int m_Analysis_Result;

		GraphD3JSLine *	m_pGraph;
		// Flag denoting if we should populate the dictionary on the fly
		//   Dictionary population is primarily if we need to live reflect things
		//   back.
		bool				m_bFlag_CreateDictionary;

		// Parameter entries
		ParamDictionary * 	m_pDictionary;
};



#endif /* FMNC_TEST_ANALYSIS_H_ */

/*
 * fmnc_test_analysis.cc
 *
 *  Created on: Jul 17, 2014
 *      Author: striegel
 */

#include <math.h>

#include <iostream>
using namespace std;

#include "fmnc_test_analysis.h"
#include "../viz/d3js.h"
#include "../viz/vizDataSet.h"
#include "../viz/vizDataPoint.h"
#include "../util/ip-utils.h"

FMNC_Test_Analysis::FMNC_Test_Analysis ()
{
	m_pMeasure_Sent = NULL;
	m_pMeasure_Rcvd = NULL;
	m_bAnalysisDone = false;

	m_fRTT_Min = 0.0;
	m_fRTT_Max = 0.0;
	m_fRTT_Mean = 0.0;
	m_fRTT_Median = 0.0;
	m_fRTT_StdDev = 0.0;
	m_fTest_Time = 0.0;
    m_Analysis_Result = -1;

	m_pGraph = NULL;
}

FMNC_Test_Analysis::~FMNC_Test_Analysis ()
{
	if(m_pGraph != NULL)
	{
		delete m_pGraph;
		m_pGraph = NULL;
	}
}

void FMNC_Test_Analysis::setMeasurementLists (FMNC_List_Measurement_Packets * pSent,
 									 	 	  FMNC_List_Measurement_Packets * pRcvd)
{
	m_pMeasure_Sent = pSent;
	m_pMeasure_Rcvd = pRcvd;
}

bool FMNC_Test_Analysis::isValidMeasurement  ()
{
	// Make sure the sent / received buckets are not NULL
    cout<< " Sanity Check the Packets..."<<endl;
	if(m_pMeasure_Sent == NULL || m_pMeasure_Rcvd == NULL)
	{
        cerr<<" ** Error: The packets are NULL !!"<<endl;
		return false;
	}

	// Do we have any data?
	if(m_pMeasure_Rcvd->getCount() < 2 || m_pMeasure_Sent->getCount() < 2)
	{
        cerr<<" ** Error: The packets is too short !!"<<endl;
		return false;
	}

	// Check the number of points
	//
	//  The number of measurement packets sent should be less than or equal to the number that
	//  were received
	if(m_pMeasure_Sent->getCount() > m_pMeasure_Rcvd->getCount())
	{
        cerr<<" ** Error: Received packets "<< m_pMeasure_Rcvd->getCount()<<" are less than sent ones "<<m_pMeasure_Sent->getCount()<<" !!"<<endl;
		//return false;
	}

	// TODO: Group-wise validity check


	return true;
}

bool FMNC_Test_Analysis::isValidMeasurement  (uint16_t nGroup)
{
	if(m_pMeasure_Sent == NULL || m_pMeasure_Rcvd == NULL)
	{
        cerr<<" ** Error: The packets are NULL !!"<<endl;
		return false;
	}

	// Check the number of points
	//
	//  The number of measurement packets sent should be less than or equal to the number that
	//  were received
	if(m_pMeasure_Sent->getCount() > m_pMeasure_Rcvd->getCount())
	{
        cerr<<" ** Error: Received packets are less than sent ones !!"<<endl;
		return false;
	}

	// TODO: Group-wise validity check


	return true;
}

float calc_fluctuation(std::vector<uint16_t> TBS)
{
        uint16_t over_count = 0;
        if(TBS.size()<1) return 0;
        for(int j=0;j<TBS.size()-1;j++){
                int diff = std::abs(TBS[j+1] - TBS[j])*8;
                /* std::cout<<"(DEBUG) TBS_Size_diff "<<TBS[j+1]<<" - "<<TBS[j]<<" = "<<diff<<endl; */
                if(diff > 20000)
                        over_count++;
        }
        return over_count/float(TBS.size());
}

double time_diff(struct timeval * sTime,struct timeval * eTime)
{
        return (double)(1000*(eTime->tv_sec - sTime->tv_sec) + (eTime->tv_usec - sTime->tv_usec)/1000.0); // in millisecond
}

bool FMNC_Test_Analysis::do_hiccup_analysis (uint32_t lAnalyzed)
{
        cout<<"(DEBUG) Start hiccup analysis"<<endl;
        uint32_t	j;

        if(isValidMeasurement())
        {
                uint16_t t_size = 0;
                cout<<"(DEBUG) start index "<<lAnalyzed<<endl;

                FMNC_Measurement_Packet_TCP * sent_pkt_0 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Sent->getPacket(lAnalyzed);
                FMNC_Measurement_Packet_TCP * sent_pkt_1 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Sent->getPacket(lAnalyzed+1);
                FMNC_Measurement_Packet_TCP * rcvd_pkt_0 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Rcvd->getPacket(lAnalyzed);
                FMNC_Measurement_Packet_TCP * rcvd_pkt_1 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Rcvd->getPacket(lAnalyzed+1);

                float G = 0.0;
                if(rcvd_pkt_1->getTs() > rcvd_pkt_0->getTs())
                        G = time_diff(sent_pkt_0->getTime(),sent_pkt_1->getTime())/(rcvd_pkt_1->getTs() - rcvd_pkt_0->getTs());

                cout<<"(DEBUG) Calculate G = "<<G<<" ms/tick."<<endl;
                float the_threshold = max((float)4.0,G);
                float hiccup_acum = 0.0;
                for(j=lAnalyzed+1; j<m_pMeasure_Sent->getCount()-1; j++)
                {
                        sent_pkt_0 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Sent->getPacket(j);
                        sent_pkt_1 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Sent->getPacket(j+1);
                        rcvd_pkt_0 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Rcvd->getPacket(j);
                        rcvd_pkt_1 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Rcvd->getPacket(j+1);
                        if(time_diff(sent_pkt_0->getTime(),sent_pkt_1->getTime()) < the_threshold &&
                                        G*(rcvd_pkt_1->getTs() - rcvd_pkt_0->getTs()) > the_threshold){
                                hiccup_acum += G*(rcvd_pkt_1->getTs() - rcvd_pkt_0->getTs());
                                cout<<"(DEBUG) rcvd_gap =  "<<G*(rcvd_pkt_1->getTs() - rcvd_pkt_0->getTs())<<" ms."<<endl;
                        }
                }

                cout<<"(DEBUG) hiccup_acum =  "<<hiccup_acum<<" ms."<<endl;
                if(hiccup_acum > 150.0 || G == 0.0)
                        return true;
                else
                        return false;


        }
        else{
                cerr<<" **ERROR**: Invalid_Measurement at do_hiccup_analysis!"<<endl;
                return true;
        }
}
bool FMNC_Test_Analysis::do_fluctuation_analysis (uint32_t lAnalyzed, double duration, uint16_t * mRTT)
{
        cout<<"(DEBUG) Start fluctuation analysis"<<endl;
        uint32_t	j;

        vector<uint16_t> TBS_0;
        vector<uint16_t> TBS_1;
        if(isValidMeasurement())
        {
                uint16_t t_size = 0;
                FMNC_Measurement_Packet_TCP * sent_pkt_0 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Sent->getPacket(lAnalyzed);
                cout<<"(DEBUG) start index "<<lAnalyzed<<endl;

                *mRTT = 0;
                for(j=lAnalyzed; j<m_pMeasure_Sent->getCount()-1; j++)
                {
                        FMNC_Measurement_Packet_TCP * sent_pkt = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Sent->getPacket(j);
                        FMNC_Measurement_Packet_TCP * rcvd_pkt_0 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Rcvd->getPacket(j);
                        FMNC_Measurement_Packet_TCP * rcvd_pkt_1 = (FMNC_Measurement_Packet_TCP *)m_pMeasure_Rcvd->getPacket(j+1);
                        t_size += sent_pkt->getLength();
                        if(rcvd_pkt_1->getTs() > rcvd_pkt_0->getTs()){
                                /* cout<<"(DEBUG) time_diff "<<(double)(time_diff(sent_pkt_0->getTime(),sent_pkt->getTime()))<<endl; */
                                if(time_diff(sent_pkt_0->getTime(),sent_pkt->getTime()) >= duration)
                                        TBS_1.push_back(t_size);
                                else
                                        TBS_0.push_back(t_size);
                                t_size = 0;
                        }
                        if(time_diff(sent_pkt->getTime(),rcvd_pkt_0->getTime())> (double)(*mRTT)) 
                                *mRTT = (uint16_t)time_diff(sent_pkt->getTime(),rcvd_pkt_0->getTime());


                }
                /* for(auto t:TBS_0) */
                /*         cout<<"TBS_0 "<<t<<endl; */
                /* for(auto t:TBS_1) */
                /*         cout<<"TBS_1 "<<t<<endl; */
                std::cout<<"(DEBUG) get the fluctuation score "<<calc_fluctuation(TBS_0)<<" "<<calc_fluctuation(TBS_1)<<endl;
                float the_threshold = 0.4;

                if(calc_fluctuation(TBS_0) > the_threshold && calc_fluctuation(TBS_1) > the_threshold)
                        m_Analysis_Result = 0;
                else if (calc_fluctuation(TBS_0) < the_threshold && calc_fluctuation(TBS_1) < the_threshold)
                        m_Analysis_Result = 2;
                else if (calc_fluctuation(TBS_0) < the_threshold && calc_fluctuation(TBS_1) > the_threshold)
                        m_Analysis_Result = 1;
                else
                        m_Analysis_Result = -1;

                return true;
        }
        else
                return false;
}
int FMNC_Test_Analysis::getFluctuation_Result()
{
        return m_Analysis_Result;
}
bool FMNC_Test_Analysis::doAnalysis ()
{
	return doAnalysis(FMNC_MEASUREMENT_GROUP_NONE);
}

bool FMNC_Test_Analysis::doAnalysis (uint16_t nGroup)
{
	int		j;

	if(isValidMeasurement())
	{
		cout << "FMNC_Test_Analysis -> Attempting to create pairs" << endl;

		// Do our pairing
		for(j=0; j<m_pMeasure_Sent->getCount(); j++)
		{
			FMNC_Measurement_Pair *	pPair;

			if(nGroup == FMNC_MEASUREMENT_GROUP_NONE)
			{
				pPair = new FMNC_Measurement_Pair();

				// Create the sent / received pairs
				pPair->setMeasurement_Sent((FMNC_Measurement_Packet_TCP *) m_pMeasure_Sent->getPacket(j));
				pPair->setMeasurement_Rcvd((FMNC_Measurement_Packet_TCP *) m_pMeasure_Rcvd->getPacket(j));

				m_Pairs.addMeasurementPair(pPair);
			}
		}

		cout << "   Attempting to create pairs" << endl;
		m_Pairs.sortByRTT();

		double	fSum;

		cout << "   Setting minimum / maximum values" << endl;
		m_fRTT_Min = m_Pairs.getMeasurementPair(0)->getRTT();
		m_fRTT_Max = m_Pairs.getMeasurementPair(m_Pairs.getCount()-1)->getRTT();

		cout << "   Setting median" << endl;
		// What is the median?
		int		nMedianPos;

		nMedianPos = m_Pairs.getCount() / 2;
		m_fRTT_Median = m_Pairs.getMeasurementPair(nMedianPos)->getRTT();

		fSum = 0;
		cout << "   Setting mean value" << endl;
		computeMean();

		fSum = 0;
		cout << "   Setting standard deviation" << endl;
		computeStandardDeviation();

		dumpAnalysisResults();

		createGraphD3JS ();

//	if(getFlag_CreateDictionary())
//	{
//		char szTemp[60];
//		string sData;
//
//		if(!allocateDictionary())
//		{
//            cerr << " **Error: No dictionary allocated. "<<endl;
//			return false;
//		}
//
//		// OK, we now have a valid dictionary
//
//
//		// The total size of the samples
//		sprintf(szTemp, "%d", m_Pairs.getCount());
//		sData = szTemp;
//		m_pDictionary->addEntry("RTT_Samples_All", sData);
////
////
////		sprintf(szTemp, "%s", m_pGraph->extractScript());
////		sData = szTemp;
////		m_pDictionary->addEntry("D3JS_GRAPH", sData);
//	}

		return true;
	}
	else
	{
		return false;
	}
}

string FMNC_Test_Analysis::GetGraphD3JS ()
{
        string str;
        str = "<html lang = \"en\">";
        str += "<body>";
        str += "<div>" ;
        str += "<svg id=\"visualisation\" width=\"1001\" height=\"500\"></svg>";
        str += "</div>" ;
        str += "</body>";
        str += "</html>";

        return str + m_pGraph->extractScript() ;
}
bool FMNC_Test_Analysis::createGraphD3JS ()
{
	int j;
	VizDataSet	*	pDataset;

	if(m_pGraph != NULL)
	{
		delete m_pGraph;
		m_pGraph = NULL;
	}

	m_pGraph = new GraphD3JSLine();

	pDataset = new VizDataSet();
    m_Pairs.sortByTime();

	for(j=0; j<m_Pairs.getCount(); j++)
	{
		VizDataPoint2D * pPoint;

		pPoint = new VizDataPoint2D();

		pPoint->setX((double) j);
		pPoint->setY(m_Pairs.getMeasurementPair(j)->getRTT());
		pDataset->addDataPoint(pPoint);
	}

	m_pGraph->setVizArea("visualisation");
	m_pGraph->addDataset(pDataset);

	//cout << m_pGraph->extractScript() << endl;

	return true;
}

double FMNC_Test_Analysis::computeStandardDeviation ()
{
	int		j;
	double 	fSum;

	fSum = 0;
	m_fRTT_StdDev = 0;

	double fValue;

	if(m_Pairs.getCount() == 0)
	{
		return 0.0;
	}

	for(j=0; j<m_Pairs.getCount(); j++)
	{
		fValue = m_Pairs.getMeasurementPair(j)->getRTT() - m_fRTT_Mean;

		fValue = fValue * fValue;

		fSum += fValue / (double) m_Pairs.getCount();
	}

	m_fRTT_StdDev = sqrt(fSum);
	return m_fRTT_StdDev;
}

double FMNC_Test_Analysis::computeMean ()
{
	int		j;
	double 	fSum;

	fSum = 0;

	if(m_Pairs.getCount() == 0)
	{
		m_fRTT_Mean = 0.0;
		return m_fRTT_Mean;
	}

	// At this point, we have a fully populated set of measurement pairs
	for(j=0; j<m_Pairs.getCount(); j++)
	{
		fSum += m_Pairs.getMeasurementPair(j)->getRTT();
	}

	m_fRTT_Mean = fSum / (double) (m_Pairs.getCount());
	return m_fRTT_Mean;
}

double	FMNC_Test_Analysis::getRTT_Min  ()
{
	return m_fRTT_Min;
}

double	FMNC_Test_Analysis::getRTT_Max  ()
{
	return m_fRTT_Max;
}

double	FMNC_Test_Analysis::getRTT_Mean ()
{
	return m_fRTT_Mean;
}


double	FMNC_Test_Analysis::getRTT_Median ()
{
	return m_fRTT_Median;
}

double 	FMNC_Test_Analysis::getRTT_StdDev ()
{
	return m_fRTT_StdDev;
}

double	FMNC_Test_Analysis::getTest_Time ()
{
	return 0.0;
}

bool	FMNC_Test_Analysis::isAnalysisDone ()
{
	return m_bAnalysisDone;
}

string	FMNC_Test_Analysis::constructWeb_Analysis ()
{
	return "";
}

string FMNC_Test_Analysis::constructTable_Web ()
{
	string		sWeb;
	char		szTemp[50];

	sWeb = "<table>";

	// Range (Minimum and Maximum Values)
	sWeb += "<tr>";
	sWeb += "<td><b>RTT Range</b></td>";

	sWeb += "<td>";
	sprintf(szTemp, "%f", m_fRTT_Min);
	sWeb += szTemp;
	sWeb += " - ";
	sprintf(szTemp, "%f", m_fRTT_Max);
	sWeb += szTemp;
	sWeb += "ms</td>";
	sWeb += "</tr>";

	// Mean
	sWeb += "<tr>";
	sWeb += "<td><b>RTT Mean</b></td>";

	sWeb += "<td>";
	sprintf(szTemp, "%f", m_fRTT_Mean);
	sWeb += szTemp;
	sWeb += "ms</td>";
	sWeb += "</tr>";

	// Mean
	sWeb += "<tr>";
	sWeb += "<td><b>RTT Median</b></td>";

	sWeb += "<td>";
	sprintf(szTemp, "%f", m_fRTT_Median);
	sWeb += szTemp;
	sWeb += "ms</td>";
	sWeb += "</tr>";

	// Mean
	sWeb += "<tr>";
	sWeb += "<td><b>RTT Standard Deviation</b></td>";

	sWeb += "<td>";
	sprintf(szTemp, "%f", m_fRTT_StdDev);
	sWeb += szTemp;
	sWeb += "ms</td>";
	sWeb += "</tr>";

	sWeb += "</table>";
    // D3J3 viz lsong
//    sWeb += "<div>";
//    sWeb += "<svg id=\"visualisation\" width="500" height="250"></svg";
//    sWeb += "</div>";
//	sWeb += m_pGraph->extractScript();
	return sWeb;
}

void	FMNC_Test_Analysis::dumpAnalysisResults ()
{
	cout << "FMNC_Test_Anaylsis Results of " << m_Pairs.getCount() <<" on slicing size of "<<m_Pairs.getMeasurementPair(1)->getMeasurement_Sent()->getLength()<<endl;
	cout << "   Range:   " << m_fRTT_Min << " - " << m_fRTT_Max << endl;
	cout << "   Mean:    " << m_fRTT_Mean << endl;
	cout << "   Median:  " << m_fRTT_Median << endl;
	cout << "   Std Dev: " << m_fRTT_StdDev << endl;
}


void FMNC_Test_Analysis::setFlag_CreateDictionary (bool bCreateDict)
{
	m_bFlag_CreateDictionary = bCreateDict;
}

bool FMNC_Test_Analysis::getFlag_CreateDictionary ()
{
	return m_bFlag_CreateDictionary;
}

ParamDictionary * FMNC_Test_Analysis::getDictionary ()
{
	return m_pDictionary;
}

bool FMNC_Test_Analysis::allocateDictionary ()
{
	if(m_pDictionary != NULL)
	{
		return false;
	}
	else
	{
		m_pDictionary = new ParamDictionary();
		if(m_pDictionary != NULL)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}


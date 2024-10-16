/*
 * ParamDictionary.cc
 *
 *  Created on: Aug 1, 2014
 *      Author: striegel
 */

#include <vector>
using namespace std;

#include <iostream>
using namespace std;

#include "ParamDictionary.h"

ParamDictionaryEntry::ParamDictionaryEntry ()
{
	m_sName = "";
	m_sValue = "";
	m_pOwner = NULL;
}

ParamDictionaryEntry::ParamDictionaryEntry (string sName, string sValue)
{
	m_sName = sName;
	m_sValue = sValue;
	m_pOwner = NULL;
}


string ParamDictionaryEntry::getName ()
{
	return m_sName;
}

void ParamDictionaryEntry::setName (string sName)
{
	m_sName = sName;
}

string ParamDictionaryEntry::getValue ()
{
	return m_sValue;
}

void ParamDictionaryEntry::setValue (string sValue)
{
	m_sValue = sValue;
}

ParamDictionary * ParamDictionaryEntry::getOwner ()
{
	return m_pOwner;
}

void ParamDictionaryEntry::setOwner (ParamDictionary * pOwner)
{
	m_pOwner = pOwner;
}




ParamDictionary::ParamDictionary ()
{
	m_nMaxLength_Name = PARAM_DICTIONARY_MAXLENGTH_NAME;
	m_sDelimiter_Start =PARAM_DICTIONARY_DEFAULT_START_DELIM;
	m_sDelimiter_End = PARAM_DICTIONARY_DEFAULT_END_DELIM;
}

ParamDictionary::~ParamDictionary ()
{
	int		j;

	for(j=0; j<m_Entries.size(); j++)
	{
		if(m_Entries[j]->getOwner() == this)
		{
			delete m_Entries[j];
			m_Entries[j] = NULL;
		}
	}

	m_Entries.clear();
}

//////////////////////////////////////////////////////////////////////

int	ParamDictionary::fuseEntries (ParamDictionary * pDictionary,
								  bool bOverwrite,
								  bool bRefOnly)
{
	int		nAddCount;

	if(pDictionary == NULL)
	{
		return 0;
	}

	nAddCount = 0;

	for(int j=0; j<pDictionary->getCount(); j++)
	{
		if(getEntry(pDictionary->getEntry(j)->getName()) != NULL)
		{
			if(bOverwrite)
			{
				updateEntry(pDictionary->getEntry(j)->getName(), pDictionary->getEntry(j)->getValue());
			}
			else
			{
				// Do nothing
			}
		}
		else
		{
			// No overlap
			addEntry(pDictionary->getEntry(j)->getName(), pDictionary->getEntry(j)->getValue());
			nAddCount++;
		}
	}

	return nAddCount;
}

void ParamDictionary::dumpConsole ()
{
	for(int j=0; j<m_Entries.size(); j++)
	{
		cout << "  Dict Entry " << j << " -> " << m_Entries[j]->getName() << " = " << m_Entries[j]->getValue() << endl;
	}
}

bool ParamDictionary::isValidParameterName (string sName)
{
	// Check the name length
	if(sName.size() > getMaxLength_Name())
	{
		return false;
	}

	// Check that the name does not contain either of the two
	// delimiters
	size_t	nPos;

	nPos = sName.find(getDelimiter_Start());

	if(nPos != std::string::npos)
	{
		return false;
	}

	nPos = sName.find(getDelimiter_End());

	if(nPos != std::string::npos)
	{
		return false;
	}

	return true;
}


bool ParamDictionary::addEntry (string sName, string sValue)
{
	ParamDictionaryEntry *	pEntry;

	if(!isValidParameterName(sName))
	{
		return false;
	}

	if(getEntry(sName) != NULL)
	{
		return false;
	}

	pEntry = new ParamDictionaryEntry ();

	pEntry->setName(sName);
	pEntry->setValue(sValue);
	pEntry->setOwner(this);

	m_Entries.push_back(pEntry);
	return true;
}

bool ParamDictionary::updateEntry (string sName, string sNewValue)
{
	ParamDictionaryEntry *	pEntry;

	pEntry = getEntry(sName);

	if(pEntry == NULL)
	{
		return false;
	}

	pEntry->setValue(sNewValue);

	return true;
}

bool ParamDictionary::setEntry (string sName, string sValue)
{
	ParamDictionaryEntry *	pEntry;

	pEntry = getEntry(sName);

	if(pEntry != NULL)
	{
		pEntry->setValue(sValue);
		return true;
	}

	if(!isValidParameterName(sName))
	{
		return false;
	}

	return addEntry(sName, sValue);
}

int	ParamDictionary::getCount ()
{
	return m_Entries.size();
}

string	ParamDictionary::replaceText (string sString)
{
	std::size_t foundL;
	std::size_t foundR;
	int		nStartPos;
	string 	sParam;

	nStartPos = 0;

	// Let's iterate through the string

	while(1)
	{

		foundL = sString.find(getDelimiter_Start(), nStartPos);

		if(foundL == std::string::npos)
		{
			// Nope, did not find it, we are done
			break;
		}

		// We found something if we got here, otherwise we broke out

		// Adjust by the delimiter width and where we found it
		foundL += getDelimiter_Start().length();
		nStartPos = foundL;

		foundR = sString.find(getDelimiter_End(), nStartPos);

		if(foundR == std::string::npos)
		{
			// Nope, did not find it, we are done - start delimiter but not an end one
			cerr << "Warning: Found a start delimiter but not an end one" << endl;
			break;
		}

		nStartPos = foundR;

		sParam = sString.substr(foundL, foundR-foundL);
		nStartPos += getDelimiter_End().length();

		cout << " Found a parameter: " << sParam << endl;

		ParamDictionaryEntry	* pEntry;
		string	sValue;

		pEntry = getEntry(sParam);

		if(pEntry == NULL)
		{
			sValue = "";
		}
		else
		{
			sValue = pEntry->getValue();
		}

		sString.replace(sString.begin()+foundL-getDelimiter_Start().length(),
						sString.begin()+foundR+getDelimiter_End().length(),
						sValue);

		nStartPos = 0;
	}


	return sString;
}

int		ParamDictionary::getMaxLength_Name ()
{
	return m_nMaxLength_Name;
}

bool	ParamDictionary::setMaxLength_Name (int nLength)
{
	if(nLength > 0)
	{
		m_nMaxLength_Name = nLength;
		return true;
	}
	else
	{
		return false;
	}
}

string	ParamDictionary::getDelimiter_Start ()
{
	return m_sDelimiter_Start;
}

bool	ParamDictionary::setDelimiter_Start (string sDelim)
{
	if(sDelim.size() > 0)
	{
		m_sDelimiter_Start = sDelim;
		return true;
	}
	else
	{
		return false;
	}
}

string	ParamDictionary::getDelimiter_End ()
{
	return m_sDelimiter_End;
}

bool	ParamDictionary::setDelimiter_End (string sDelim)
{
	if(sDelim.size() > 0)
	{
		m_sDelimiter_End = sDelim;
		return true;
	}
	else
	{
		return false;
	}
}

ParamDictionaryEntry *	ParamDictionary::getEntry (string sName)
{
	int		j;

	for(j=0; j<m_Entries.size(); j++)
	{
		if(m_Entries[j]->getName() == sName)
		{
			return m_Entries[j];
		}
	}

	return NULL;
}

ParamDictionaryEntry *	ParamDictionary::getEntry (int nIndex)
{
	if(nIndex < 0 || nIndex >= getCount())
	{
		return NULL;
	}

	return m_Entries[nIndex];
}





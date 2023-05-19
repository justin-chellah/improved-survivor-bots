//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Class data for an AI Concept, an atom of response-driven dialog.
//
// $NoKeywords: $
//=============================================================================//

#ifndef AI_SPEECHCONCEPT_H
#define AI_SPEECHCONCEPT_H

#if defined( _WIN32 )
#pragma once
#endif

#include "utlsymbol.h"

#define RR_CONCEPTS_ARE_STRINGS 0


typedef CUtlSymbolTable CAI_ConceptSymbolTable;

class CAI_Concept
{
public: // local typedefs
	typedef CUtlSymbol tGenericId; // an int-like type that can be used to refer to all concepts of this type
	tGenericId m_iConcept;

public:
	CAI_Concept() {};
	// construct concept from a string.
	CAI_Concept(const char *fromString);

	// Return as a string
	const char *GetStringConcept() const;
	static const char *GetStringForGenericId(tGenericId genericId);

	operator tGenericId() const { return m_iConcept; }
	operator const char *() const { return GetStringConcept(); }
	inline bool operator==(const CAI_Concept &other) // default is compare by concept ids
	{
		return m_iConcept == other.m_iConcept;
	}
	bool operator==(const char *pszConcept);

protected:

private:
	// dupe a concept
	// CRR_Concept& operator=(CRR_Concept &other);
	CAI_Concept& operator=(const char *fromString);
};

// Used to turn ad-hoc concept from strings into numbers.
static CAI_ConceptSymbolTable *g_pAIConceptTable = NULL;

// Q&D hack to defer initialization of concept table until I can figure out where it
// really needs to come from.
static void InitializeAIConceptTable()
{
	if (g_pAIConceptTable == NULL)
	{
		g_pAIConceptTable = new CAI_ConceptSymbolTable( 64, 64, true );
	}
}

// construct from string
inline CAI_Concept::CAI_Concept(const char *fromString)
{
	InitializeAIConceptTable();
	m_iConcept = g_pAIConceptTable->AddString(fromString);
}

inline CAI_Concept &CAI_Concept::operator=(const char *fromString)
{
	InitializeAIConceptTable();
	m_iConcept = g_pAIConceptTable->AddString(fromString);
	return *this;
}

inline bool CAI_Concept::operator==(const char *pszConcept)
{
	int otherConcept = g_pAIConceptTable->Find(pszConcept);
	return ( otherConcept != UTL_INVAL_SYMBOL && otherConcept == m_iConcept );
}

inline const char *CAI_Concept::GetStringConcept() const
{
	InitializeAIConceptTable();
	AssertMsg( m_iConcept.IsValid(), "AI Concept has invalid string symbol.\n" );
	const char * retval = g_pAIConceptTable->String(m_iConcept);
	AssertMsg( retval, "An AI_Concept couldn't find its string in the symbol table!\n" );
	if (retval == NULL)
	{
		Warning( "An AI_Concept couldn't find its string in the symbol table!\n" );
		retval = "";
	}
	return retval;
}

inline const char *CAI_Concept::GetStringForGenericId(tGenericId genericId)
{
	InitializeAIConceptTable();
	return g_pAIConceptTable->String(genericId);
}

#endif
#include <main.h>
#include <variant.h>
#include <string.h>

void Variant_InitAsString (Variant* pVar, const char* pString)
{
	pVar->m_type = TYPE_STRING;
	pVar->m_strLength = strlen (pString);
	
	if (pVar->m_strLength >= 255) pVar->m_strLength = 255;
	
	memcpy (pVar->m_strContents, pString, pVar->m_strLength + 1);
}

void Variant_InitAsInt(Variant* pVar, int value)
{
	pVar->m_type = TYPE_INT;
	pVar->m_asInt = value;
}

void Variant_InitAsPointer(Variant* pVar, void* value)
{
	pVar->m_type = TYPE_POINTER;
	pVar->m_asPtr = value;
}


void VariantList_Reset(VariantList* pVList)
{
	for (int i = 0; i < C_MAX_ELEMENTS_IN_VARLIST; i++)
	{
		pVList->m_variants[i].m_type = TYPE_NONE;
	}
}

Variant* VariantList_Get(VariantList* pVList, int index)
{
	return  &pVList->m_variants[index];
}


#pragma once
#include <stdafx.h>
#include <string>
#include <EFAL.h>
#include <EFALLIB.h>

class EFALUtilities
{
public:
	EFALUtilities();
	~EFALUtilities();
public:
	static void dumpEFALSession(EFALLIB * efallib, EFALHANDLE hSession);
	static Ellis::MICHARSET str2charset(const wchar_t * str);
	static std::wstring charset2str(Ellis::MICHARSET charset);
	static Ellis::ALLTYPE_TYPE str2alltype_type(const wchar_t * str);
	static std::wstring alltype_type2str(Ellis::ALLTYPE_TYPE alltype_type);

};


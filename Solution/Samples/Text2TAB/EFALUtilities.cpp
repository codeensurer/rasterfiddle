#include <stdafx.h>
#include <EFALUtilities.h>


EFALUtilities::EFALUtilities()
{
}


EFALUtilities::~EFALUtilities()
{
}

void EFALUtilities::dumpEFALSession(EFALLIB * efallib, EFALHANDLE hSession)
{
	if (efallib->HaveErrors(hSession)) {
		wprintf(L"Errors:\n");
		for (size_t i = 0, n = efallib->NumErrors(hSession); i < n; i++) {
			wprintf(L"%ls\n", efallib->GetError(hSession, (int)i));
		}
	}

	wprintf(L"Defined variables:\n");
	for (size_t i = 0, n = efallib->GetVariableCount(hSession); i < n; i++) {
		std::wstring varName = efallib->GetVariableName(hSession, (MI_UINT32)i);
		wprintf(L"      %ls %ls %ls\n", varName.c_str(),
			efallib->GetVariableIsNull(hSession, varName.c_str()) ? L"is" : L"=",
			efallib->GetVariableIsNull(hSession, varName.c_str()) ? L"null" : efallib->GetVariableValueString(hSession, varName.c_str()));
		// todo: output current value
	}
	wprintf(L"Opened tables:\n");
	for (size_t i = 0, n = efallib->GetTableCount(hSession); i < n; i++) {
		wprintf(L"      %ls\n", efallib->GetTableName(hSession, efallib->GetTableHandle(hSession, (MI_UINT32)i)));
		// todo: output schema
	}
}

Ellis::MICHARSET EFALUtilities::str2charset(const wchar_t * str)
{
	if (_wcsicmp(str, L"ISO8859_2") == 0) return Ellis::MICHARSET::CHARSET_ISO8859_2;
	if (_wcsicmp(str, L"ISO8859_3") == 0) return Ellis::MICHARSET::CHARSET_ISO8859_3;
	if (_wcsicmp(str, L"ISO8859_4") == 0) return Ellis::MICHARSET::CHARSET_ISO8859_4;
	if (_wcsicmp(str, L"ISO8859_5") == 0) return Ellis::MICHARSET::CHARSET_ISO8859_5;
	if (_wcsicmp(str, L"ISO8859_6") == 0) return Ellis::MICHARSET::CHARSET_ISO8859_6;
	if (_wcsicmp(str, L"ISO8859_7") == 0) return Ellis::MICHARSET::CHARSET_ISO8859_7;
	if (_wcsicmp(str, L"ISO8859_8") == 0) return Ellis::MICHARSET::CHARSET_ISO8859_8;
	if (_wcsicmp(str, L"ISO8859_9") == 0) return Ellis::MICHARSET::CHARSET_ISO8859_9;
	if (_wcsicmp(str, L"WLATIN1") == 0) return Ellis::MICHARSET::CHARSET_WLATIN1;
	if (_wcsicmp(str, L"WLATIN2") == 0) return Ellis::MICHARSET::CHARSET_WLATIN2;
	if (_wcsicmp(str, L"WARABIC") == 0) return Ellis::MICHARSET::CHARSET_WARABIC;
	if (_wcsicmp(str, L"WCYRILLIC") == 0) return Ellis::MICHARSET::CHARSET_WCYRILLIC;
	if (_wcsicmp(str, L"WGREEK") == 0) return Ellis::MICHARSET::CHARSET_WGREEK;
	if (_wcsicmp(str, L"WHEBREW") == 0) return Ellis::MICHARSET::CHARSET_WHEBREW;
	if (_wcsicmp(str, L"WTURKISH") == 0) return Ellis::MICHARSET::CHARSET_WTURKISH;
	if (_wcsicmp(str, L"WTCHINESE") == 0) return Ellis::MICHARSET::CHARSET_WTCHINESE;
	if (_wcsicmp(str, L"WSCHINESE") == 0) return Ellis::MICHARSET::CHARSET_WSCHINESE;
	if (_wcsicmp(str, L"WJAPANESE") == 0) return Ellis::MICHARSET::CHARSET_WJAPANESE;
	if (_wcsicmp(str, L"WKOREAN") == 0) return Ellis::MICHARSET::CHARSET_WKOREAN;
	if (_wcsicmp(str, L"CP437") == 0) return Ellis::MICHARSET::CHARSET_CP437;
	if (_wcsicmp(str, L"CP850") == 0) return Ellis::MICHARSET::CHARSET_CP850;
	if (_wcsicmp(str, L"CP852") == 0) return Ellis::MICHARSET::CHARSET_CP852;
	if (_wcsicmp(str, L"CP857") == 0) return Ellis::MICHARSET::CHARSET_CP857;
	if (_wcsicmp(str, L"CP860") == 0) return Ellis::MICHARSET::CHARSET_CP860;
	if (_wcsicmp(str, L"CP861") == 0) return Ellis::MICHARSET::CHARSET_CP861;
	if (_wcsicmp(str, L"CP863") == 0) return Ellis::MICHARSET::CHARSET_CP863;
	if (_wcsicmp(str, L"CP865") == 0) return Ellis::MICHARSET::CHARSET_CP865;
	if (_wcsicmp(str, L"CP855") == 0) return Ellis::MICHARSET::CHARSET_CP855;
	if (_wcsicmp(str, L"CP864") == 0) return Ellis::MICHARSET::CHARSET_CP864;
	if (_wcsicmp(str, L"CP869") == 0) return Ellis::MICHARSET::CHARSET_CP869;
	if (_wcsicmp(str, L"WTHAI") == 0) return Ellis::MICHARSET::CHARSET_WTHAI;
	if (_wcsicmp(str, L"WBALTICRIM") == 0) return Ellis::MICHARSET::CHARSET_WBALTICRIM;
	if (_wcsicmp(str, L"WVIETNAMESE") == 0) return Ellis::MICHARSET::CHARSET_WVIETNAMESE;
	if (_wcsicmp(str, L"UTF8") == 0) return Ellis::MICHARSET::CHARSET_UTF8;
	if (_wcsicmp(str, L"UTF16") == 0) return Ellis::MICHARSET::CHARSET_UTF16;
	return Ellis::MICHARSET::CHARSET_NONE;
}
std::wstring EFALUtilities::charset2str(Ellis::MICHARSET charset)
{
	std::wstring str;
	switch (charset)
	{
	case Ellis::MICHARSET::CHARSET_ISO8859_2: str = L"ISO8859_2"; break;
	case Ellis::MICHARSET::CHARSET_ISO8859_3: str = L"ISO8859_3"; break;
	case Ellis::MICHARSET::CHARSET_ISO8859_4: str = L"ISO8859_4"; break;
	case Ellis::MICHARSET::CHARSET_ISO8859_5: str = L"ISO8859_5"; break;
	case Ellis::MICHARSET::CHARSET_ISO8859_6: str = L"ISO8859_6"; break;
	case Ellis::MICHARSET::CHARSET_ISO8859_7: str = L"ISO8859_7"; break;
	case Ellis::MICHARSET::CHARSET_ISO8859_8: str = L"ISO8859_8"; break;
	case Ellis::MICHARSET::CHARSET_ISO8859_9: str = L"ISO8859_9"; break;
	case Ellis::MICHARSET::CHARSET_WLATIN1: str = L"WLATIN1"; break;
	case Ellis::MICHARSET::CHARSET_WLATIN2: str = L"WLATIN2"; break;
	case Ellis::MICHARSET::CHARSET_WARABIC: str = L"WARABIC"; break;
	case Ellis::MICHARSET::CHARSET_WCYRILLIC: str = L"WCYRILLIC"; break;
	case Ellis::MICHARSET::CHARSET_WGREEK: str = L"WGREEK"; break;
	case Ellis::MICHARSET::CHARSET_WHEBREW: str = L"WHEBREW"; break;
	case Ellis::MICHARSET::CHARSET_WTURKISH: str = L"WTURKISH"; break;
	case Ellis::MICHARSET::CHARSET_WTCHINESE: str = L"WTCHINESE"; break;
	case Ellis::MICHARSET::CHARSET_WSCHINESE: str = L"WSCHINESE"; break;
	case Ellis::MICHARSET::CHARSET_WJAPANESE: str = L"WJAPANESE"; break;
	case Ellis::MICHARSET::CHARSET_WKOREAN: str = L"WKOREAN"; break;
	case Ellis::MICHARSET::CHARSET_CP437: str = L"CP437"; break;
	case Ellis::MICHARSET::CHARSET_CP850: str = L"CP850"; break;
	case Ellis::MICHARSET::CHARSET_CP852: str = L"CP852"; break;
	case Ellis::MICHARSET::CHARSET_CP857: str = L"CP857"; break;
	case Ellis::MICHARSET::CHARSET_CP860: str = L"CP860"; break;
	case Ellis::MICHARSET::CHARSET_CP861: str = L"CP861"; break;
	case Ellis::MICHARSET::CHARSET_CP863: str = L"CP863"; break;
	case Ellis::MICHARSET::CHARSET_CP865: str = L"CP865"; break;
	case Ellis::MICHARSET::CHARSET_CP855: str = L"CP855"; break;
	case Ellis::MICHARSET::CHARSET_CP864: str = L"CP864"; break;
	case Ellis::MICHARSET::CHARSET_CP869: str = L"CP869"; break;
	case Ellis::MICHARSET::CHARSET_WTHAI: str = L"WTHAI"; break;
	case Ellis::MICHARSET::CHARSET_WBALTICRIM: str = L"WBALTICRIM"; break;
	case Ellis::MICHARSET::CHARSET_WVIETNAMESE: str = L"WVIETNAMESE"; break;
	case Ellis::MICHARSET::CHARSET_UTF8: str = L"UTF8"; break;
	case Ellis::MICHARSET::CHARSET_UTF16: str = L"UTF16"; break;
	}
	return str;
}
Ellis::ALLTYPE_TYPE EFALUtilities::str2alltype_type(const wchar_t * str)
{
	if (_wcsicmp(str, L"NONE") == 0) return Ellis::ALLTYPE_TYPE::OT_NONE;
	if (_wcsicmp(str, L"CHAR") == 0) return Ellis::ALLTYPE_TYPE::OT_CHAR;
	if (_wcsicmp(str, L"DECIMAL") == 0) return Ellis::ALLTYPE_TYPE::OT_DECIMAL;
	if (_wcsicmp(str, L"INTEGER") == 0) return Ellis::ALLTYPE_TYPE::OT_INTEGER;
	if (_wcsicmp(str, L"SMALLINT") == 0) return Ellis::ALLTYPE_TYPE::OT_SMALLINT;
	if (_wcsicmp(str, L"DATE") == 0) return Ellis::ALLTYPE_TYPE::OT_DATE;
	if (_wcsicmp(str, L"LOGICAL") == 0) return Ellis::ALLTYPE_TYPE::OT_LOGICAL;
	if (_wcsicmp(str, L"FLOAT") == 0) return Ellis::ALLTYPE_TYPE::OT_FLOAT;
	if (_wcsicmp(str, L"OBJECT") == 0) return Ellis::ALLTYPE_TYPE::OT_OBJECT;
	if (_wcsicmp(str, L"STYLE") == 0) return Ellis::ALLTYPE_TYPE::OT_STYLE;
	if (_wcsicmp(str, L"INTEGER64") == 0) return Ellis::ALLTYPE_TYPE::OT_INTEGER64;
	if (_wcsicmp(str, L"TIMESPAN") == 0) return Ellis::ALLTYPE_TYPE::OT_TIMESPAN;
	if (_wcsicmp(str, L"TIME") == 0) return Ellis::ALLTYPE_TYPE::OT_TIME;
	if (_wcsicmp(str, L"DATETIME") == 0) return Ellis::ALLTYPE_TYPE::OT_DATETIME;
	return Ellis::ALLTYPE_TYPE::OT_NONE;
}
std::wstring EFALUtilities::alltype_type2str(Ellis::ALLTYPE_TYPE alltype_type)
{
	std::wstring str = L"NONE";
	switch (alltype_type)
	{
	case Ellis::ALLTYPE_TYPE::OT_NONE:
		str = L"NONE";
		break;
	case Ellis::ALLTYPE_TYPE::OT_CHAR:
		str = L"CHAR";
		break;
	case Ellis::ALLTYPE_TYPE::OT_DECIMAL:
		str = L"DECIMAL";
		break;
	case Ellis::ALLTYPE_TYPE::OT_INTEGER:
		str = L"INTEGER";
		break;
	case Ellis::ALLTYPE_TYPE::OT_SMALLINT:
		str = L"SMALLINT";
		break;
	case Ellis::ALLTYPE_TYPE::OT_DATE:
		str = L"DATE";
		break;
	case Ellis::ALLTYPE_TYPE::OT_LOGICAL:
		str = L"LOGICAL";
		break;
	case Ellis::ALLTYPE_TYPE::OT_FLOAT:
		str = L"FLOAT";
		break;
	case Ellis::ALLTYPE_TYPE::OT_OBJECT:
		str = L"OBJECT";
		break;
	case Ellis::ALLTYPE_TYPE::OT_STYLE:
		str = L"STYLE";
		break;
	case Ellis::ALLTYPE_TYPE::OT_INTEGER64:
		str = L"INTEGER64";
		break;
	case Ellis::ALLTYPE_TYPE::OT_TIMESPAN:
		str = L"TIMESPAN";
		break;
	case Ellis::ALLTYPE_TYPE::OT_TIME:
		str = L"TIME";
		break;
	case Ellis::ALLTYPE_TYPE::OT_DATETIME:
		str = L"DATETIME";
		break;
	}
	return str;
}

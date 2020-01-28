#include <stdafx.h>
#include <MIDefs.h>
#include <EFALAPI.h>
#include <EFAL.h>
#include <EFALLIB.h>
#include <map>
#include <vector>
#include <string>
#include <time.h>
#include <fstream>
#if ELLIS_OS_ISUNIX
#include <iostream>
#endif

EFALLIB * efallib = NULL;
EFALHANDLE hSession = 0;

void Help();

/*
################################################################################################
################################################################################################
#############                                                                      #############
#############                  Utility Functions for this app                      #############
#############                                                                      #############
################################################################################################
################################################################################################
*/
wchar_t isspace(wchar_t ch)
{
	return ((ch)==0x09 || (ch)==0x0A || (ch)==0x0D || (ch)==0x0C || (ch)==0x20);
}
int isprint(wchar_t ch)
{
	if (ch < 0x80) return iswprint(ch); else return true;
}
size_t ltrim(wchar_t* str)
{
	size_t i=0, j, len=wcslen(str);

	while (isspace(str[i]) || !isprint(str[i])) ++i;
	for (j=i; j <= len; j++) {
		str[j-i] = str[j];
	}
	return i;
}
size_t rtrim(wchar_t* str)
{
	__int64 i = static_cast<__int64>(wcslen(str))-1;
	while (i >= 0 && (isspace(str[i]) || !isprint(str[i])))
		str[i--] = 0x00;
	return static_cast<size_t>(i+1);
}

bool MyReportErrors()
{
	if (!efallib->HaveErrors(hSession)) return false;

	wprintf(L"ERRORS:");
	for (int ii = 0, nn = efallib->NumErrors(hSession); ii < nn; ii++)
	{
		wprintf(L"%ls\n", efallib->GetError(hSession, ii));
	}

	efallib->ClearErrors(hSession);
	return true;
}
/*
################################################################################################
################################################################################################
#############                                                                      #############
#############              Table Functions                                         #############
#############                                                                      #############
################################################################################################
################################################################################################
*/
#if ELLIS_OS_ISUNIX
#define _wcsicmp wcscasecmp 
#define _wcsnicmp wcsncasecmp 
#endif

void Open(const wchar_t * txt)
{
	if (_wcsnicmp(txt,L"Table ",6) == 0) txt += 6;

	EFALHANDLE hTable = efallib->OpenTable(hSession, txt);
	if (!MyReportErrors() && (hTable > 0))
	{
		wprintf(L"Opened table as %ls\n", efallib->GetTableName(hSession, hTable));
	}
}
EFALHANDLE TableAlias2Handle(const wchar_t * name)
{
	MI_UINT32 nTables = efallib->GetTableCount(hSession);
	for (MI_UINT32 i = 0; i < nTables; i++)
	{
		EFALHANDLE hTable = efallib->GetTableHandle(hSession, i);
		if (_wcsicmp(efallib->GetTableName(hSession, hTable), name) == 0)
		{
			return hTable;
		}
	}
	return 0;
}
void Close(wchar_t * txt)
{
	if (txt) 
	{
		ltrim(txt);
		rtrim(txt);
	}
	if ((txt == 0) || (_wcsicmp(txt,L"all") == 0))
	{
		while (efallib->GetTableCount(hSession) > 0)
		{
			efallib->CloseTable(hSession, efallib->GetTableHandle(hSession, (MI_UINT32) 0));
		}
	}
	else
	{
		EFALHANDLE hTable = TableAlias2Handle(txt);
		if (hTable > 0)
		{
			efallib->CloseTable(hSession, hTable);
		}
	}
}
void Pack(wchar_t * txt)
{
	if (txt)
	{
		ltrim(txt);
		rtrim(txt);
	}
	EFALHANDLE hTable = TableAlias2Handle(txt);
	if (hTable > 0)
	{
		efallib->Pack(hSession, hTable, Ellis::eTablePackTypeAll);
	}
}
void Tables()
{
	wprintf(L"Tables:\n----------------------------------------\n");
	MI_UINT32 nTables = efallib->GetTableCount(hSession);
	for (MI_UINT32 i = 0; i < nTables; i++)
	{
		EFALHANDLE hTable = efallib->GetTableHandle(hSession, i);
		wprintf(L"%ls", efallib->GetTableName(hSession, hTable));
		wprintf(L"\t");
		wprintf(L"%ls", efallib->GetTablePath(hSession, hTable));
		wprintf(L"\n");
	}
}
void Describe(wchar_t * txt)
{
	ltrim(txt);
	rtrim(txt);
	EFALHANDLE hTable = TableAlias2Handle(txt);

	wprintf(L"Table %ls:\n", efallib->GetTableName(hSession, hTable));
	wprintf(L"   Description   : %ls\n", efallib->GetTableDescription(hSession, hTable));
	wprintf(L"   GUID          : %ls\n", efallib->GetTableGUID(hSession, hTable));
	wprintf(L"   Type          : %ls\n", efallib->GetTableType(hSession, hTable));
	wprintf(L"   IsMappable    : %ls\n", efallib->IsVector(hSession, hTable) ? L"Yes" : L"No");
	wprintf(L"   Columns:\n");
	for (MI_UINT32 i = 0, n = efallib->GetColumnCount(hSession, hTable); i < n; i++)
	{
		wprintf(L"      [%2lu] %-32.32ls ", i, efallib->GetColumnName(hSession, hTable, i));
		switch (efallib->GetColumnType(hSession, hTable, i))
		{
		case Ellis::OT_DECIMAL:
			wprintf(L"Decimal[%2d,%2d]   ", efallib->GetColumnWidth(hSession, hTable, i), efallib->GetColumnDecimals(hSession, hTable, i));
			break;
		case Ellis::OT_CHAR:
			wprintf(L"Char[%3d]        ", efallib->GetColumnWidth(hSession, hTable, i));
			break;
		case Ellis::OT_INTEGER:
			wprintf(L"Integer          ");
			break;
		case Ellis::OT_INTEGER64:
			wprintf(L"Integer64        ");
			break;
		case Ellis::OT_SMALLINT:
			wprintf(L"SmallInt         ");
			break;
		case Ellis::OT_FLOAT:
			wprintf(L"Float            ");
			break;
		case Ellis::OT_LOGICAL:
			wprintf(L"Boolean          ");
			break;
		case Ellis::OT_DATE:
			wprintf(L"Date             ");
			break;
		case Ellis::OT_TIME:
			wprintf(L"Time             ");
			break;
		case Ellis::OT_DATETIME:
			wprintf(L"DateTime         ");
			break;
		case Ellis::OT_OBJECT:
			wprintf(L"Object           ");
			break;
		case Ellis::OT_STYLE:
			wprintf(L"Style            ");
			break;
		}
		wprintf(L"\n");

		if (efallib->GetColumnType(hSession, hTable, i) == Ellis::OT_OBJECT)
		{
			MI_UINT32 nPoints = efallib->GetPointObjectCount(hSession, hTable, i);
			MI_UINT32 nLines = efallib->GetLineObjectCount(hSession, hTable, i);
			MI_UINT32 nPolys = efallib->GetAreaObjectCount(hSession, hTable, i);
			MI_UINT32 nMisc = efallib->GetMiscObjectCount(hSession, hTable, i);
			wprintf(L"                                            LineCount     : %ld\n", nPoints);
			wprintf(L"                                            AreaCount     : %ld\n", nLines);
			wprintf(L"                                            PointCount    : %ld\n", nPolys);
			wprintf(L"                                            MiscCount     : %ld\n", nMisc);
			std::wstring csys = efallib->GetColumnCSys(hSession, hTable, i);
			std::wstring mbcsys = efallib->CoordSys2MBString(hSession, csys.c_str());
			std::wstring prjcsys = efallib->CoordSys2PRJString(hSession, csys.c_str());
			wprintf(L"                                            CoordSys      : %ls\n", csys.c_str());
			wprintf(L"                                            CoordSys (MB) : %ls\n", mbcsys.c_str());
			wprintf(L"                                            CoordSys (PRJ): %ls\n", prjcsys.c_str());
		}
	}

	wprintf(L"   Metadata:\n");
	EFALHANDLE hEnumerator = efallib->EnumerateMetadata(hSession, hTable);
	while (efallib->GetNextEntry(hSession, hEnumerator))
	{
		wprintf(L"   ");
		wprintf(efallib->GetCurrentMetadataKey(hSession, hEnumerator));
		wprintf(L"=");
		wprintf(efallib->GetCurrentMetadataValue(hSession, hEnumerator));
		wprintf(L"\n");
	}
	efallib->DisposeMetadataEnumerator(hSession, hEnumerator);
}
void tokenize(std::wstring str, std::vector<std::wstring> & strings)
{
	strings.clear();
	wchar_t * buf = new wchar_t[str.length() + 1];
	size_t j = 0;
	for (size_t i = 0; i<str.length(); i++){
		wchar_t c = str[i];
		if (c == L' '){
			if (j > 0) { buf[j] = 0;  strings.push_back(buf); }
			j = 0;
		}
		else if (c == L'\"'){
			i++;
			while (i<str.length() && (str[i] != L'\"')){ buf[j++] = str[i]; i++; }
		}
		else{
			buf[j++] = c;
		}
	}
	if (j > 0) { buf[j] = 0;  strings.push_back(buf); }
	delete[] buf;
}
/*
save [table] <<table>> in <<file>>  [charset <<charset>>] [NOINSERT]
save [table] <<table>> as nativex table in <<file>> [UTF16 | UTF8 | charset <<charset>>] [NOINSERT]
save [table] <<table>> as geopackage table <<dbtable>> in <<file>> [coordsys <<codespace:code>>] [UTF16 | UTF8 | charset <<charset>>] [NOINSERT] [CONVERTUNSUPPORTEDOBJECTS]
*/
void SaveAs(wchar_t * txt)
{
	const wchar_t * sourceTableAlias = NULL;
	const wchar_t * pDestinationFileName = NULL;
	const wchar_t * pDestinationCoordSys = NULL;
	const wchar_t * pDestinationTableName = NULL;
	Ellis::MICHARSET destinationCharset = Ellis::MICHARSET::CHARSET_NONE;
	int tableType = 0;  // 0=Native, 1=NativeX, 2=Geopackage
	EFALHANDLE hSourceTable = 0;
	bool saveData = true;
	bool convertUnsupportedObjects = false;

	/* ---------------------------------------------------------------------------
	* Parse and validate the input command
	* ---------------------------------------------------------------------------
	*/
	std::vector<std::wstring> strings;
	tokenize(txt, strings);
	// Keyword Save and optional keyword Table are not part of the input text.
	if (strings.size() < 3)
	{
		wprintf(L"Malformed command\n");
		Help();
		return;
	}
	// first token is the table alias we are saving
	sourceTableAlias = strings[0].c_str();
	if ((hSourceTable = efallib->GetTableHandle(hSession, sourceTableAlias)) == 0)
	{
		wprintf(L"Table %ls not found.\n", sourceTableAlias);
		Help();
		return;
	}
	for (size_t i = 1, n = strings.size(); i < n; i++)
	{
		if (_wcsicmp(strings[i].c_str(), L"in") == 0)
		{
			if (++i >= n)
			{
				wprintf(L"Malformed command\n");
				Help();
				return;
			}
			pDestinationFileName = strings[i].c_str();
		}
		else if (_wcsicmp(strings[i].c_str(), L"as") == 0)
		{
			if (++i >= n)
			{
				wprintf(L"Malformed command\n");
				Help();
				return;
			}
			if (_wcsicmp(strings[i].c_str(), L"nativex") == 0)
			{
				if ((++i >= n) || (_wcsicmp(strings[i].c_str(), L"table") != 0))
				{
					wprintf(L"Malformed command\n");
					Help();
					return;
				}
				tableType = 1;
			}
			else if (_wcsicmp(strings[i].c_str(), L"geopackage") == 0)
			{
				if ((++i >= n) || (_wcsicmp(strings[i].c_str(), L"table") != 0) || (++i >= n))
				{
					wprintf(L"Malformed command\n");
					Help();
					return;
				}
				pDestinationTableName = strings[i].c_str();
				tableType = 2;
				if (destinationCharset == Ellis::MICHARSET::CHARSET_NONE) destinationCharset = Ellis::MICHARSET::CHARSET_UTF8;
			}
		}
		else if (_wcsicmp(strings[i].c_str(), L"UTF8") == 0)
		{
			destinationCharset = Ellis::MICHARSET::CHARSET_UTF8;
		}
		else if (_wcsicmp(strings[i].c_str(), L"UTF16") == 0)
		{
			destinationCharset = Ellis::MICHARSET::CHARSET_UTF16;
		}
		else if (_wcsicmp(strings[i].c_str(), L"charset") == 0)
		{
			if (++i >= n)
			{
				wprintf(L"Malformed command\n");
				Help();
				return;
			}
			if (_wcsicmp(strings[i].c_str(), L"ISO8859_1") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_1;
			else if (_wcsicmp(strings[i].c_str(), L"ISO8859_2") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_2;
			else if (_wcsicmp(strings[i].c_str(), L"ISO8859_3") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_3;
			else if (_wcsicmp(strings[i].c_str(), L"ISO8859_4") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_4;
			else if (_wcsicmp(strings[i].c_str(), L"ISO8859_5") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_5;
			else if (_wcsicmp(strings[i].c_str(), L"ISO8859_6") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_6;
			else if (_wcsicmp(strings[i].c_str(), L"ISO8859_7") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_7;
			else if (_wcsicmp(strings[i].c_str(), L"ISO8859_8") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_8;
			else if (_wcsicmp(strings[i].c_str(), L"ISO8859_9") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_ISO8859_9;
			else if (_wcsicmp(strings[i].c_str(), L"WLATIN1") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WLATIN1;
			else if (_wcsicmp(strings[i].c_str(), L"WLATIN2") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WLATIN2;
			else if (_wcsicmp(strings[i].c_str(), L"WARABIC") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WARABIC;
			else if (_wcsicmp(strings[i].c_str(), L"WCYRILLIC") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WCYRILLIC;
			else if (_wcsicmp(strings[i].c_str(), L"WGREEK") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WGREEK;
			else if (_wcsicmp(strings[i].c_str(), L"WHEBREW") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WHEBREW;
			else if (_wcsicmp(strings[i].c_str(), L"WTURKISH") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WTURKISH;
			else if (_wcsicmp(strings[i].c_str(), L"WTCHINESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WTCHINESE;
			else if (_wcsicmp(strings[i].c_str(), L"WSCHINESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WSCHINESE;
			else if (_wcsicmp(strings[i].c_str(), L"WJAPANESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WJAPANESE;
			else if (_wcsicmp(strings[i].c_str(), L"WKOREAN") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WKOREAN;
#if 0	// Not supported in EFAL
			else if (_wcsicmp(strings[i].c_str(), L"MROMAN") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MROMAN;
			else if (_wcsicmp(strings[i].c_str(), L"MARABIC") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MARABIC;
			else if (_wcsicmp(strings[i].c_str(), L"MGREEK") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MGREEK;
			else if (_wcsicmp(strings[i].c_str(), L"MHEBREW") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MHEBREW;
			else if (_wcsicmp(strings[i].c_str(), L"MCENTEURO") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MCENTEURO;
			else if (_wcsicmp(strings[i].c_str(), L"MCROATIAN") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MCROATIAN;
			else if (_wcsicmp(strings[i].c_str(), L"MCYRILLIC") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MCYRILLIC;
			else if (_wcsicmp(strings[i].c_str(), L"MICELANDIC") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MICELANDIC;
			else if (_wcsicmp(strings[i].c_str(), L"MTHAI") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MTHAI;
			else if (_wcsicmp(strings[i].c_str(), L"MTURKISH") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MTURKISH;
			else if (_wcsicmp(strings[i].c_str(), L"MTCHINESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MTCHINESE;
			else if (_wcsicmp(strings[i].c_str(), L"MJAPANESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MJAPANESE;
			else if (_wcsicmp(strings[i].c_str(), L"MKOREAN") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MKOREAN;
#endif
			else if (_wcsicmp(strings[i].c_str(), L"CP437") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP437;
			else if (_wcsicmp(strings[i].c_str(), L"CP850") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP850;
			else if (_wcsicmp(strings[i].c_str(), L"CP852") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP852;
			else if (_wcsicmp(strings[i].c_str(), L"CP857") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP857;
			else if (_wcsicmp(strings[i].c_str(), L"CP860") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP860;
			else if (_wcsicmp(strings[i].c_str(), L"CP861") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP861;
			else if (_wcsicmp(strings[i].c_str(), L"CP863") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP863;
			else if (_wcsicmp(strings[i].c_str(), L"CP865") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP865;
			else if (_wcsicmp(strings[i].c_str(), L"CP855") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP855;
			else if (_wcsicmp(strings[i].c_str(), L"CP864") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP864;
			else if (_wcsicmp(strings[i].c_str(), L"CP869") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_CP869;
#if 0	// Not supported in EFAL
			else if (_wcsicmp(strings[i].c_str(), L"LICS") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_LICS;
			else if (_wcsicmp(strings[i].c_str(), L"LMBCS") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_LMBCS;
			else if (_wcsicmp(strings[i].c_str(), L"LMBCS1") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_LMBCS1;
			else if (_wcsicmp(strings[i].c_str(), L"LMBCS2") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_LMBCS2;
			else if (_wcsicmp(strings[i].c_str(), L"MSCHINESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_MSCHINESE;
			else if (_wcsicmp(strings[i].c_str(), L"UTCHINESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_UTCHINESE;
			else if (_wcsicmp(strings[i].c_str(), L"USCHINESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_USCHINESE;
			else if (_wcsicmp(strings[i].c_str(), L"UJAPANESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_UJAPANESE;
			else if (_wcsicmp(strings[i].c_str(), L"UKOREAN") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_UKOREAN;
#endif
			else if (_wcsicmp(strings[i].c_str(), L"WTHAI") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WTHAI;
			else if (_wcsicmp(strings[i].c_str(), L"WBALTICRIM") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WBALTICRIM;
			else if (_wcsicmp(strings[i].c_str(), L"WVIETNAMESE") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_WVIETNAMESE;
			else if (_wcsicmp(strings[i].c_str(), L"UTF8") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_UTF8;
			else if (_wcsicmp(strings[i].c_str(), L"UTF16") == 0) destinationCharset = Ellis::MICHARSET::CHARSET_UTF16;
			else
			{
				wprintf(L"Malformed command\n");
				Help();
				return;
			}
		}
		else if (_wcsicmp(strings[i].c_str(), L"CoordSys") == 0)
		{
			if (++i >= n)
			{
				wprintf(L"Malformed command\n");
				Help();
				return;
			}
			pDestinationCoordSys = strings[i].c_str();
		}
		else if (_wcsicmp(strings[i].c_str(), L"noinsert") == 0)
		{
			saveData = false;
		}
		else if (_wcsicmp(strings[i].c_str(), L"ConvertUnsupportedObjects") == 0)
		{
			convertUnsupportedObjects = true;
		}
		else
		{
			wprintf(L"Malformed command\n");
			Help();
			return;
		}
	}
	if (destinationCharset == Ellis::MICHARSET::CHARSET_NONE) destinationCharset = efallib->GetTableCharset(hSession, hSourceTable);

	/* ---------------------------------------------------------------------------
	* Echo back the parsed input values
	* ---------------------------------------------------------------------------
	*/
	wprintf(L"Save Table %ls ", sourceTableAlias);
	if (tableType == 1) wprintf(L"as NativeX table ");
	if (tableType == 2) wprintf(L"as Geopackage table %ls ", pDestinationTableName);
	wprintf(L" in %ls ", pDestinationFileName);
	if (pDestinationCoordSys) wprintf(L"CoordSys %ls ", pDestinationCoordSys);
	switch (destinationCharset)
	{
	case Ellis::MICHARSET::CHARSET_ISO8859_1:
		wprintf(L"Charset ISO8859_1 ");
		break;
	case Ellis::MICHARSET::CHARSET_ISO8859_2:
		wprintf(L"Charset ISO8859_2 ");
		break;
	case Ellis::MICHARSET::CHARSET_ISO8859_3:
		wprintf(L"Charset ISO8859_3 ");
		break;
	case Ellis::MICHARSET::CHARSET_ISO8859_4:
		wprintf(L"Charset ISO8859_4 ");
		break;
	case Ellis::MICHARSET::CHARSET_ISO8859_5:
		wprintf(L"Charset ISO8859_5 ");
		break;
	case Ellis::MICHARSET::CHARSET_ISO8859_6:
		wprintf(L"Charset ISO8859_6 ");
		break;
	case Ellis::MICHARSET::CHARSET_ISO8859_7:
		wprintf(L"Charset ISO8859_7 ");
		break;
	case Ellis::MICHARSET::CHARSET_ISO8859_8:
		wprintf(L"Charset ISO8859_8 ");
		break;
	case Ellis::MICHARSET::CHARSET_ISO8859_9:
		wprintf(L"Charset ISO8859_9 ");
		break;
	case Ellis::MICHARSET::CHARSET_WLATIN1:
		wprintf(L"Charset WLATIN1 ");
		break;
	case Ellis::MICHARSET::CHARSET_WLATIN2:
		wprintf(L"Charset WLATIN2 ");
		break;
	case Ellis::MICHARSET::CHARSET_WARABIC:
		wprintf(L"Charset WARABIC ");
		break;
	case Ellis::MICHARSET::CHARSET_WCYRILLIC:
		wprintf(L"Charset WCYRILLIC ");
		break;
	case Ellis::MICHARSET::CHARSET_WGREEK:
		wprintf(L"Charset WGREEK ");
		break;
	case Ellis::MICHARSET::CHARSET_WHEBREW:
		wprintf(L"Charset WHEBREW ");
		break;
	case Ellis::MICHARSET::CHARSET_WTURKISH:
		wprintf(L"Charset WTURKISH ");
		break;
	case Ellis::MICHARSET::CHARSET_WTCHINESE:
		wprintf(L"Charset WTCHINESE ");
		break;
	case Ellis::MICHARSET::CHARSET_WSCHINESE:
		wprintf(L"Charset WSCHINESE ");
		break;
	case Ellis::MICHARSET::CHARSET_WJAPANESE:
		wprintf(L"Charset WJAPANESE ");
		break;
	case Ellis::MICHARSET::CHARSET_WKOREAN:
		wprintf(L"Charset WKOREAN ");
		break;
#if 0
	case Ellis::MICHARSET::CHARSET_MROMAN:
		wprintf(L"Charset MROMAN ");
		break;
	case Ellis::MICHARSET::CHARSET_MARABIC:
		wprintf(L"Charset MARABIC ");
		break;
	case Ellis::MICHARSET::CHARSET_MGREEK:
		wprintf(L"Charset MGREEK ");
		break;
	case Ellis::MICHARSET::CHARSET_MHEBREW:
		wprintf(L"Charset MHEBREW ");
		break;
	case Ellis::MICHARSET::CHARSET_MCENTEURO:
		wprintf(L"Charset MCENTEURO ");
		break;
	case Ellis::MICHARSET::CHARSET_MCROATIAN:
		wprintf(L"Charset MCROATIAN ");
		break;
	case Ellis::MICHARSET::CHARSET_MCYRILLIC:
		wprintf(L"Charset MCYRILLIC ");
		break;
	case Ellis::MICHARSET::CHARSET_MICELANDIC:
		wprintf(L"Charset MICELANDIC ");
		break;
	case Ellis::MICHARSET::CHARSET_MTHAI:
		wprintf(L"Charset MTHAI ");
		break;
	case Ellis::MICHARSET::CHARSET_MTURKISH:
		wprintf(L"Charset MTURKISH ");
		break;
	case Ellis::MICHARSET::CHARSET_MTCHINESE:
		wprintf(L"Charset MTCHINESE ");
		break;
	case Ellis::MICHARSET::CHARSET_MJAPANESE:
		wprintf(L"Charset MJAPANESE ");
		break;
	case Ellis::MICHARSET::CHARSET_MKOREAN:
		wprintf(L"Charset MKOREAN ");
		break;
#endif
	case Ellis::MICHARSET::CHARSET_CP437:
		wprintf(L"Charset CP437 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP850:
		wprintf(L"Charset CP850 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP852:
		wprintf(L"Charset CP852 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP857:
		wprintf(L"Charset CP857 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP860:
		wprintf(L"Charset CP860 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP861:
		wprintf(L"Charset CP861 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP863:
		wprintf(L"Charset CP863 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP865:
		wprintf(L"Charset CP865 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP855:
		wprintf(L"Charset CP855 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP864:
		wprintf(L"Charset CP864 ");
		break;
	case Ellis::MICHARSET::CHARSET_CP869:
		wprintf(L"Charset CP869 ");
		break;
#if 0	// Not supported on non-Windows
	case Ellis::MICHARSET::CHARSET_LICS:
		wprintf(L"Charset LICS ");
		break;
	case Ellis::MICHARSET::CHARSET_LMBCS:
		wprintf(L"Charset LMBCS ");
		break;
	case Ellis::MICHARSET::CHARSET_LMBCS1:
		wprintf(L"Charset LMBCS1 ");
		break;
	case Ellis::MICHARSET::CHARSET_LMBCS2:
		wprintf(L"Charset LMBCS2 ");
		break;
	case Ellis::MICHARSET::CHARSET_MSCHINESE:
		wprintf(L"Charset MSCHINESE ");
		break;
	case Ellis::MICHARSET::CHARSET_UTCHINESE:
		wprintf(L"Charset UTCHINESE ");
		break;
	case Ellis::MICHARSET::CHARSET_USCHINESE:
		wprintf(L"Charset USCHINESE ");
		break;
	case Ellis::MICHARSET::CHARSET_UJAPANESE:
		wprintf(L"Charset UJAPANESE ");
		break;
	case Ellis::MICHARSET::CHARSET_UKOREAN:
		wprintf(L"Charset UKOREAN ");
		break;
#endif
	case Ellis::MICHARSET::CHARSET_WTHAI:
		wprintf(L"Charset WTHAI ");
		break;
	case Ellis::MICHARSET::CHARSET_WBALTICRIM:
		wprintf(L"Charset WBALTICRIM ");
		break;
	case Ellis::MICHARSET::CHARSET_WVIETNAMESE:
		wprintf(L"Charset WVIETNAMESE ");
		break;
	case Ellis::MICHARSET::CHARSET_UTF8:
		wprintf(L"Charset UTF8 ");
		break;
	case Ellis::MICHARSET::CHARSET_UTF16:
		wprintf(L"Charset UTF16 ");
		break;
	case Ellis::MICHARSET::CHARSET_NONE:
		break;
	default:
		wprintf(L"Charset ??? ");
		break;
	}
	wprintf(L"\n");

	/* ---------------------------------------------------------------------------
	* Create the new table
	* ---------------------------------------------------------------------------
	*/
	EFALHANDLE hMetadata = 0;
	clock_t start = clock();
	if (tableType == 2) { // Geopackage
	#if ELLIS_OS_IS_WINOS
		wchar_t acDir[_MAX_PATH];
		wchar_t acExt[_MAX_EXT];
		wchar_t acAlias[_MAX_PATH];
		wchar_t acDrive[_MAX_DRIVE];
		_wsplitpath_s(pDestinationFileName, acDrive, sizeof(acDrive) / sizeof(wchar_t), acDir, sizeof(acDir) / sizeof(wchar_t), acAlias, sizeof(acAlias) / sizeof(wchar_t), acExt, sizeof(acExt) / sizeof(wchar_t));

		std::wstring strTablePath = acDrive;
		strTablePath += acDir;
		strTablePath += pDestinationTableName;
		strTablePath += L".TAB";
	#else
		std::wstring strTablePath = pDestinationFileName;
		std::size_t sz = strTablePath.find_last_of(L'/');
		if (sz != std::wstring::npos) {
			strTablePath = strTablePath.substr(0,sz);
			strTablePath += pDestinationTableName;
			strTablePath += L".TAB";
		}
	#endif

		hMetadata = efallib->CreateGeopackageTableMetadata(hSession, pDestinationTableName, strTablePath.c_str(), pDestinationFileName, destinationCharset, convertUnsupportedObjects);
	}
	else if (tableType == 1) { // NativeX
	#if ELLIS_OS_IS_WINOS
		wchar_t acDir[_MAX_PATH];
		wchar_t acExt[_MAX_EXT];
		wchar_t acAlias[_MAX_PATH];
		wchar_t acDrive[_MAX_DRIVE];
		_wsplitpath_s(pDestinationFileName, acDrive, sizeof(acDrive) / sizeof(wchar_t), acDir, sizeof(acDir) / sizeof(wchar_t), acAlias, sizeof(acAlias) / sizeof(wchar_t), acExt, sizeof(acExt) / sizeof(wchar_t));
	#else
		wchar_t acAlias[PATH_MAX];
		std::wstring strAlias = pDestinationFileName;
		std::size_t sz = strAlias.find_last_of(L'/');
		if (sz != std::wstring::npos) {
			strAlias = strAlias.substr(1+sz);
		}
		sz = strAlias.find_last_of(L'.');
		if (sz != std::wstring::npos) {
			strAlias = strAlias.substr(0,sz-1);
		}
		wcscpy(acAlias, strAlias.c_str());
	#endif

		hMetadata = efallib->CreateNativeXTableMetadata(hSession, acAlias, pDestinationFileName, destinationCharset);
	}
	else { // Native
	#if ELLIS_OS_IS_WINOS
		wchar_t acDir[_MAX_PATH];
		wchar_t acExt[_MAX_EXT];
		wchar_t acAlias[_MAX_PATH];
		wchar_t acDrive[_MAX_DRIVE];
		_wsplitpath_s(pDestinationFileName, acDrive, sizeof(acDrive) / sizeof(wchar_t), acDir, sizeof(acDir) / sizeof(wchar_t), acAlias, sizeof(acAlias) / sizeof(wchar_t), acExt, sizeof(acExt) / sizeof(wchar_t));
	#else
		wchar_t acAlias[PATH_MAX];
		std::wstring strAlias = pDestinationFileName;
		std::size_t sz = strAlias.find_last_of(L'/');
		if (sz != std::wstring::npos) {
			strAlias = strAlias.substr(1+sz);
		}
		sz = strAlias.find_last_of(L'.');
		if (sz != std::wstring::npos) {
			strAlias = strAlias.substr(0,sz-1);
		}
		wcscpy(acAlias, strAlias.c_str());
	#endif

		hMetadata = efallib->CreateNativeTableMetadata(hSession, acAlias, pDestinationFileName, destinationCharset);
	}
	EFALHANDLE hNewTable = 0;
	if (hMetadata != 0)
	{
		for (int idx = 0, n = efallib->GetColumnCount(hSession, hSourceTable); idx < n; ++idx)  {
			const wchar_t * columnName = efallib->GetColumnName(hSession, hSourceTable, idx);
			Ellis::ALLTYPE_TYPE columnType = efallib->GetColumnType(hSession, hSourceTable, idx);
			bool isIndexed = efallib->IsColumnIndexed(hSession, hSourceTable, idx);
			MI_UINT32 columnWidth = efallib->GetColumnWidth(hSession, hSourceTable, idx);
			MI_UINT32 columnDecimals = efallib->GetColumnDecimals(hSession, hSourceTable, idx);
			const wchar_t * columnCSys = efallib->GetColumnCSys(hSession, hSourceTable, idx);
			if ((columnType == Ellis::ALLTYPE_TYPE::OT_OBJECT) && (pDestinationCoordSys != NULL)) columnCSys = pDestinationCoordSys;
			if ((columnType == Ellis::ALLTYPE_TYPE::OT_INTEGER64) && (tableType != 2 /*Not Geopackage*/)) columnType = Ellis::ALLTYPE_TYPE::OT_INTEGER;
			efallib->AddColumn(hSession, hMetadata, columnName, columnType, isIndexed, columnWidth, columnDecimals, columnCSys);
		}

		hNewTable = efallib->CreateTable(hSession, hMetadata);
		efallib->DestroyTableMetadata(hSession, hMetadata);
	}
	/* ---------------------------------------------------------------------------
	* Insert the records from the source table into the new destination table
	* ---------------------------------------------------------------------------
	*/
	MI_INT32 nrecs = -1;
	if (hNewTable && saveData) {
		std::wstring insert, projection;
		bool first = true;
		insert = L"Insert Into \"";
		insert.append(efallib->GetTableName(hSession, hNewTable));
		insert.append(L"\" (");
		for (int idx(0), cnt(efallib->GetColumnCount(hSession, hNewTable)); idx < cnt; ++idx)  {
			std::wstring newColumnName = efallib->GetColumnName(hSession, hNewTable, idx);
			if (efallib->IsColumnReadOnly(hSession, hNewTable, idx)) continue;

			for (int idx2(0), cnt2(efallib->GetColumnCount(hSession, hSourceTable)); idx2 < cnt2; ++idx2)  {
				std::wstring srcColumnName = efallib->GetColumnName(hSession, hSourceTable, idx2);
				if (_wcsicmp(newColumnName.c_str(), srcColumnName.c_str()) == 0) // geopackage tables use lower case
				{
					if (!first) projection.append(L",");
					projection.append(newColumnName.c_str());
					first = false;
					break;
				}
			}
		}
		insert.append(projection.c_str());
		insert.append(L") Select ");
		insert.append(projection.c_str());
		insert.append(L" From \"");
		insert.append(sourceTableAlias);
		insert.append(L"\"");
		nrecs = -1;
		if (!first)
		{
			nrecs = efallib->Insert(hSession, insert.c_str());
		}
	}

	clock_t finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	wprintf(L"%6.1f seconds to create the table with %ld records\n", duration, nrecs);
}

/*
################################################################################################
################################################################################################
#############                                                                      #############
#############          Geopackage Blob Utility Functions (client side)             #############
#############                                                                      #############
################################################################################################
################################################################################################
*/
#pragma warning( push )
#pragma warning( disable: 4200 )
/*
warning C4200: nonstandard extension used : zero-sized array in struct/union
Cannot generate copy-ctor or copy-assignment operator when UDT contains a zero-sized array
*/

#pragma pack(push, 1)
struct GeoPackageBinaryHeader {
	unsigned char magic[2]; // 0x4750;
	unsigned char version;
	unsigned char flags;
	unsigned      srs_id;
	double envelope[0];
};

union GeoPackageBinaryHeaderReader {
	GeoPackageBinaryHeader header;
	unsigned char data[0];
};
#pragma pack(pop)

#pragma warning( pop )

/*
**  reverse "count" bytes starting at "buf"
*/
static void memrev(char  *buf, size_t count)
{
	char *r;

	for (r = buf + count - 1; buf < r; buf++, r--)
	{
		*buf ^= *r;
		*r ^= *buf;
		*buf ^= *r;
	}
}
static MI_UINT32 read_uint32(const unsigned char * bytes, int littleEndianOrder) {
	// Here I am assuming I as the program am running on Windows, thus littleEndianOrder.
	// If the data is not in littleEndianOrder, it is in BIG Endian order, thus we need to swap
	// the bytes first. Unsigned int32s are simply reversed.
	union {
		MI_INT32 v;
		char c[sizeof(MI_INT32)];
	} d;
	d.v = *((MI_INT32*)bytes);
	if (!littleEndianOrder) {
		memrev((char*)d.c, sizeof(MI_INT32));
	}
	return d.v;
}
static double read_double(const unsigned char * bytes, int littleEndianOrder) {
	// Here I am assuming I as the program am running on Windows, thus littleEndianOrder.
	// If the data is not in littleEndianOrder, it is in BIG Endian order, thus we need to swap
	// the bytes first. Doubles are simply reversed.
	union {
		double v;
		char c[sizeof(double)];
	} d;
	d.v = *((double*)bytes);
	if (!littleEndianOrder) {
		memrev((char*)d.c, sizeof(double));
	}
	return d.v;
}

void PrintGeometry(const unsigned char * bytes, size_t nbytes)
{
	GeoPackageBinaryHeaderReader * headerReader = (GeoPackageBinaryHeaderReader *)bytes;
	if (nbytes < sizeof(GeoPackageBinaryHeader)) {
		wprintf(L"Malformed blob");
	}
	else if (headerReader->header.magic[0] != 'G' || headerReader->header.magic[1] != 'P') {
		wprintf(L"Malformed blob");
	}
	else {
		int isExtendedGeometry, isEmptyGeometry, envelopeType, littleEndianOrder;
		int envelopeBytes = 0;
		isExtendedGeometry = (headerReader->header.flags & 0x20);
		isEmptyGeometry = (headerReader->header.flags & 0x10);
		envelopeType = (headerReader->header.flags & 0x0E) >> 1;
		littleEndianOrder = (headerReader->header.flags & 0x01);

		if (isEmptyGeometry) {
			// Geometry is empty
			wprintf(L"EMPTY");
		}
		else {
			if (isExtendedGeometry) {
				wprintf(L"EXTENDED");
			}
			double minX, maxX, minY, maxY, minZ, maxZ, minM, maxM;
			const unsigned char * pEnvelope = bytes + sizeof(GeoPackageBinaryHeader);

			// Envelope
			if (envelopeType) {
				// 1: envelope is [minx, maxx, miny, maxy], 32 bytes
				// 2: envelope is [minx, maxx, miny, maxy, minz, maxz], 48 bytes
				// 3: envelope is [minx, maxx, miny, maxy, minm, maxm], 48 bytes
				// 4: envelope is [minx, maxx, miny, maxy, minz, maxz, minm, maxm], 64 bytes
				if (envelopeType == 1) {
					envelopeBytes = 32;
					minX = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxX = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					minY = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxY = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
				}
				else if (envelopeType == 2) {
					envelopeBytes = 48;
					minX = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxX = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					minY = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxY = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					minZ = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxZ = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
				}
				else if (envelopeType == 3) {
					envelopeBytes = 48;
					minX = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxX = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					minY = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxY = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					minM = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxM = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
				}
				else if (envelopeType == 4) {
					envelopeBytes = 64;
					minX = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxX = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					minY = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxY = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					minZ = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxZ = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					minM = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
					maxM = read_double(pEnvelope, littleEndianOrder); pEnvelope += sizeof(double);
				}
			}
			const unsigned char * wkb = bytes + sizeof(GeoPackageBinaryHeader)+envelopeBytes;
			littleEndianOrder = wkb[0];
			wkb++;
			MI_UINT32 wkbType = read_uint32(wkb, littleEndianOrder);
			wkb += sizeof(MI_UINT32);
			switch (wkbType)
			{
			case wkbPoint:
				wprintf(L"wkbPoint");
				if (envelopeBytes > 0) wprintf(L"[(%12.6f,%12.6f)]", minX, minY);
				else
				{
					double x = 0.0, y = 0.0;
					x = read_double(wkb, littleEndianOrder);
					wkb += sizeof(double);
					y = read_double(wkb, littleEndianOrder);
					wkb += sizeof(double);
					wprintf(L"[(%12.6f,%12.6f)]", x, y);
				}
				break;

			case wkbPointZ:
				wprintf(L"wkbPointZ");
				if (envelopeBytes > 0) wprintf(L"[(%12.6f,%12.6f)]", minX, minY);
				else
				{
					double x = 0.0, y = 0.0;
					x = read_double(wkb, littleEndianOrder);
					wkb += sizeof(double);
					y = read_double(wkb, littleEndianOrder);
					wkb += sizeof(double);
					wprintf(L"[(%12.6f,%12.6f)]", x, y);
				}
				break;

			case wkbPointM:
				wprintf(L"wkbPointM");
				if (envelopeBytes > 0) wprintf(L"[(%12.6f,%12.6f)]", minX, minY);
				else
				{
					double x = 0.0, y = 0.0;
					x = read_double(wkb, littleEndianOrder);
					wkb += sizeof(double);
					y = read_double(wkb, littleEndianOrder);
					wkb += sizeof(double);
					wprintf(L"[(%12.6f,%12.6f)]", x, y);
				}
				break;

			case wkbPointZM:
				wprintf(L"wkbPointZM");
				if (envelopeBytes > 0) wprintf(L"[(%12.6f,%12.6f)]", minX, minY);
				else
				{
					double x = 0.0, y = 0.0;
					x = read_double(wkb, littleEndianOrder);
					wkb += sizeof(double);
					y = read_double(wkb, littleEndianOrder);
					wkb += sizeof(double);
					wprintf(L"[(%12.6f,%12.6f)]", x, y);
				}
				break;

			case wkbLineString:
				wprintf(L"wkbLineString");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbPolygon:
				wprintf(L"wkbPolygon");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbTriangle:
				wprintf(L"wkbTriangle");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiPoint:
				wprintf(L"wkbMultiPoint");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiLineString:
				wprintf(L"wkbMultiLineString");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiPolygon:
				wprintf(L"wkbMultiPolygon");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbGeometryCollection:
				wprintf(L"wkbGeometryCollection");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbPolyhedralSurface:
				wprintf(L"wkbPolyhedralSurface");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbTIN:
				wprintf(L"wkbTIN");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbLineStringZ:
				wprintf(L"wkbLineStringZ");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbPolygonZ:
				wprintf(L"wkbPolygonZ");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbTrianglez:
				wprintf(L"wkbTrianglez");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiPointZ:
				wprintf(L"wkbMultiPointZ");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiLineStringZ:
				wprintf(L"wkbMultiLineStringZ");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiPolygonZ:
				wprintf(L"wkbMultiPolygonZ");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbGeometryCollectionZ:
				wprintf(L"wkbGeometryCollectionZ");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbPolyhedralSurfaceZ:
				wprintf(L"wkbPolyhedralSurfaceZ");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbTINZ:
				wprintf(L"wkbTINZ");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbLineStringM:
				wprintf(L"wkbLineStringM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbPolygonM:
				wprintf(L"wkbPolygonM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbTriangleM:
				wprintf(L"wkbTriangleM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiPointM:
				wprintf(L"wkbMultiPointM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiLineStringM:
				wprintf(L"wkbMultiLineStringM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiPolygonM:
				wprintf(L"wkbMultiPolygonM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbGeometryCollectionM:
				wprintf(L"wkbGeometryCollectionM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbPolyhedralSurfaceM:
				wprintf(L"wkbPolyhedralSurfaceM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbTINM:
				wprintf(L"wkbTINM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbLineStringZM:
				wprintf(L"wkbLineStringZM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbPolygonZM:
				wprintf(L"wkbPolygonZM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbTriangleZM:
				wprintf(L"wkbTriangleZM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiPointZM:
				wprintf(L"wkbMultiPointZM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiLineStringZM:
				wprintf(L"wkbMultiLineStringZM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbMultiPolygonZM:
				wprintf(L"wkbMultiPolygonZM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbGeometryCollectionZM:
				wprintf(L"wkbGeometryCollectionZM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbPolyhedralSurfaceZM:
				wprintf(L"wkbPolyhedralSurfaceZM");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			case wkbTinZM:
				wprintf(L"wkbTinZM");
				break;
			case ewkbLegacyText:
				wprintf(L"ewkbLegacyText");
				wprintf(L"[(%12.6f,%12.6f)(%12.6f,%12.6f)]", minX, minY, maxX, maxY);
				break;

			}

		}
	}
}
/*
################################################################################################
################################################################################################
#############                                                                      #############
#############          Select, Insert, Update and Delete                           #############
#############                                                                      #############
################################################################################################
################################################################################################
*/
void Select(wchar_t * txt)
{
	clock_t start = clock();
	EFALHANDLE hCursor = efallib->Select(hSession, txt);
	if (!MyReportErrors() && (hCursor > 0))
	{
		wprintf(L"MI_KEY\t");
		for (MI_UINT32 i = 0, n = efallib->GetCursorColumnCount(hSession, hCursor); i < n; i++)
		{
			wprintf(L"%ls\t", efallib->GetCursorColumnName(hSession, hCursor, i));
		}
		wprintf(L"\n");
		MI_UINT32 recordCount = 0;
		MI_UINT32 nbytes;
		char * bytes;
		EFALDATE efalDate;
		EFALTIME efalTime;
		EFALDATETIME efalDateTime;
		while (efallib->FetchNext(hSession, hCursor))
		{
			MyReportErrors();
			recordCount++;
			wprintf(L"%ls\t", efallib->GetCursorCurrentKey(hSession, hCursor));

			for (MI_UINT32 i = 0, n = efallib->GetCursorColumnCount(hSession, hCursor); i < n; i++)
			{
				if (efallib->GetCursorIsNull(hSession, hCursor, i))
				{
					wprintf(L"(null)");
				}
				else 
				{
					switch (efallib->GetCursorColumnType(hSession, hCursor, i))
					{
					case Ellis::OT_CHAR:
						wprintf(L"%ls", efallib->GetCursorValueString(hSession, hCursor, i));
						break;
					case Ellis::OT_LOGICAL:
						wprintf(L"%ls", (efallib->GetCursorValueBoolean(hSession, hCursor, i) ? L"True" : L"False"));
						break;
					case Ellis::OT_FLOAT:
					case Ellis::OT_DECIMAL:
						wprintf(L"%f", efallib->GetCursorValueDouble(hSession, hCursor, i));
						break;
					case Ellis::OT_INTEGER64:
						wprintf(L"%lld", efallib->GetCursorValueInt64(hSession, hCursor, i));
						break;
					case Ellis::OT_INTEGER:
						wprintf(L"%d", efallib->GetCursorValueInt32(hSession, hCursor, i));
						break;
					case Ellis::OT_SMALLINT:
						wprintf(L"%hd", efallib->GetCursorValueInt16(hSession, hCursor, i));
						break;
					case Ellis::OT_BINARY:
						nbytes = efallib->PrepareCursorValueBinary(hSession, hCursor, i);
						wprintf(L"%lu bytes", nbytes);
						// Now I can call efallib->GetData(hSession, bytes, nbytes) BUT I must first make sure bytes
						// has at least nbytes of space allocated!
						break;
					case Ellis::OT_OBJECT:
						nbytes = efallib->PrepareCursorValueGeometry(hSession, hCursor, i);
						bytes = new char[nbytes];
						efallib->GetData(hSession, bytes, nbytes);
						PrintGeometry((const unsigned char *)bytes, nbytes);
						delete[] bytes;
						break;
					case Ellis::OT_STYLE:
						wprintf(L"%ls", efallib->GetCursorValueStyle(hSession, hCursor, i));
						break;
					case Ellis::OT_TIMESPAN:
						wprintf(L"%fs", efallib->GetCursorValueTimespanInMilliseconds(hSession, hCursor, i) / 1000.0);
						break;
					case Ellis::OT_DATE:
						efalDate = efallib->GetCursorValueDate(hSession, hCursor, i);
						wprintf(L"%d-%d-%d", efalDate.year, efalDate.month, efalDate.day);
						break;
					case Ellis::OT_TIME:
						efalTime = efallib->GetCursorValueTime(hSession, hCursor, i);
						wprintf(L"%d:%d:%d.%.03d", efalTime.hour, efalTime.minute, efalTime.second, efalTime.millisecond);
						break;
					case Ellis::OT_DATETIME:
						efalDateTime = efallib->GetCursorValueDateTime(hSession, hCursor, i);
						wprintf(L"%d-%d-%d %d:%d:%d.%.03d", efalDateTime.year, efalDateTime.month, efalDateTime.day, efalDateTime.hour, efalDateTime.minute, efalDateTime.second, efalDateTime.millisecond);
						break;
					}
				}
				wprintf(L"\t");
			}
			wprintf(L"\n");
		}
		wprintf(L"%lu records\n", recordCount);
		efallib->DisposeCursor(hSession, hCursor);
	}
	
	clock_t finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	wprintf(L"%6.1f seconds\n", duration);
}
void Insert(wchar_t * txt)
{
	clock_t start = clock();
	MI_INT32 recordCount = efallib->Insert(hSession, txt);
	if (!MyReportErrors())
	{
		wprintf(L"%ld records affected.\n", recordCount);
	}
	clock_t finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	wprintf(L"%6.1f seconds\n", duration);
}
void Update(wchar_t * txt)
{
	clock_t start = clock();
	MI_INT32 recordCount = efallib->Update(hSession, txt);
	if (!MyReportErrors())
	{
		wprintf(L"%ld records affected.\n", recordCount);
	}
	clock_t finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	wprintf(L"%6.1f seconds\n", duration);
}
void Delete(wchar_t * txt)
{
	clock_t start = clock();
	MI_INT32 recordCount = efallib->Delete(hSession, txt);
	if (!MyReportErrors())
	{
		wprintf(L"%ld records affected.\n", recordCount);
	}
	clock_t finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	wprintf(L"%6.1f seconds\n", duration);
}

/*
################################################################################################
################################################################################################
#############                                                                      #############
#############         Variable Handling                                            #############
#############                                                                      #############
################################################################################################
################################################################################################
*/
void Set(const wchar_t * txt)
{
	clock_t start = clock();
	wchar_t * peq = wcschr((wchar_t *)txt, '=');
	if (peq != 0)
	{
		*peq = 0;
		peq++;
		efallib->ClearErrors(hSession);
		efallib->CreateVariable(hSession, txt);
		efallib->SetVariableValue(hSession, txt, peq);
		if (!MyReportErrors())
		{
			clock_t finish = clock();
			double duration = (double)(finish - start) / CLOCKS_PER_SEC;
			wprintf(L"%6.1f seconds\n", duration);
		}
	}
}
void PrintVariableValue(const wchar_t * name)
{
	if (efallib->GetVariableIsNull(hSession, name))
	{
		wprintf(L"(null)");
		return;
	}
	MI_UINT32 nbytes = 0;
	char * bytes = NULL;
	EFALDATE efalDate;
	EFALTIME efalTime;
	EFALDATETIME efalDateTime;
	switch (efallib->GetVariableType(hSession, name))
	{
		case Ellis::OT_CHAR:
			wprintf(L"%ls", efallib->GetVariableValueString(hSession, name));
			break;
		case Ellis::OT_LOGICAL:
			wprintf(L"%ls", (efallib->GetVariableValueBoolean(hSession, name) ? L"True" : L"False"));
			break;
		case Ellis::OT_FLOAT:
		case Ellis::OT_DECIMAL:
			wprintf(L"%f", efallib->GetVariableValueDouble(hSession, name));
			break;
		case Ellis::OT_INTEGER64:
			wprintf(L"%lld", efallib->GetVariableValueInt64(hSession, name));
			break;
		case Ellis::OT_INTEGER:
			wprintf(L"%d", efallib->GetVariableValueInt32(hSession, name));
			break;
		case Ellis::OT_SMALLINT:
			wprintf(L"%hd", efallib->GetVariableValueInt16(hSession, name));
			break;
		case Ellis::OT_BINARY:
			nbytes = efallib->PrepareVariableValueBinary(hSession, name);
			wprintf(L"%lu bytes", nbytes);
			break;
		case Ellis::OT_OBJECT:
			nbytes = efallib->PrepareVariableValueGeometry(hSession, name);
			bytes = new char[nbytes];
			efallib->GetData(hSession, bytes, nbytes);
			PrintGeometry((const unsigned char *)bytes, nbytes);
			break;
		case Ellis::OT_STYLE:
			wprintf(L"%ls", efallib->GetVariableValueStyle(hSession, name));
			break;
		case Ellis::OT_TIMESPAN:
			wprintf(L"%fs", efallib->GetVariableValueTimespanInMilliseconds(hSession, name) / 1000.0);
			break;
		case Ellis::OT_DATE:
			efalDate = efallib->GetVariableValueDate(hSession, name);
			wprintf(L"%d-%d-%d", efalDate.year, efalDate.month, efalDate.day);
			break;
		case Ellis::OT_TIME:
			efalTime = efallib->GetVariableValueTime(hSession, name);
			wprintf(L"%d:%d:%d.%d", efalTime.hour, efalTime.minute, efalTime.second, efalTime.millisecond);
			break;
		case Ellis::OT_DATETIME:
			efalDateTime = efallib->GetVariableValueDateTime(hSession, name);
			wprintf(L"%d-%d-%d %d:%d:%d.%d", efalDateTime.year, efalDateTime.month, efalDateTime.day, efalDateTime.hour, efalDateTime.minute, efalDateTime.second, efalDateTime.millisecond);
			break;
	}
}
void Show(const wchar_t * txt)
{
	if ((txt != 0) && (wcslen(txt) > 0))
	{
		for (MI_UINT32 i = 0, n = efallib->GetVariableCount(hSession); i < n; i++)
		{
			if (_wcsicmp(efallib->GetVariableName(hSession, i), txt) == 0)
			{
				const wchar_t * name = efallib->GetVariableName(hSession, i);
				wprintf(L"%ls=", name);
				PrintVariableValue(name);
				wprintf(L"\n");
			}
		}
	}
	else
	{
		wprintf(L"\nVariables:\n----------------------------------------\n");
		for (MI_UINT32 i = 0, n = efallib->GetVariableCount(hSession); i < n; i++)
		{
			const wchar_t * name = efallib->GetVariableName(hSession, i);
			wprintf(L"%ls=", name);
			PrintVariableValue(name);
			wprintf(L"\n");
		}
	}
}

void Test()
{
	EFALTIME value;
	value.hour = 3;
	value.minute = 15;
	value.second = 30;
	value.millisecond = 42;

	efallib->CreateVariable(hSession, L"@x2");
	efallib->SetVariableValueTime(hSession, L"@x2", value);
	efallib->Update(hSession, L"update safe_datetime set time = @x2 where row = 1");

	//EFALHANDLE hSeamlessTable = efallib->CreateSeamlessTable(hSession, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\TestSeamless.tab", L"mapinfo:coordsys 12, 62, 7, 0", Ellis::MICHARSET::CHARSET_WLATIN1);
	//Ellis::DRECT mbr;
	//mbr.x1 = mbr.x2 = mbr.y1 = mbr.y2 = 0.0;

	//efallib->AddSeamlessComponentTable(hSession, hSeamlessTable, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\eworld_mnf_africa.tab", mbr);
	//efallib->AddSeamlessComponentTable(hSession, hSeamlessTable, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\eworld_mnf_antarctica.tab", mbr);
	//efallib->AddSeamlessComponentTable(hSession, hSeamlessTable, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\eworld_mnf_asia.tab", mbr);
	//efallib->AddSeamlessComponentTable(hSession, hSeamlessTable, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\eworld_mnf_australia.tab", mbr);
	//efallib->AddSeamlessComponentTable(hSession, hSeamlessTable, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\eworld_mnf_europe.tab", mbr);
	//efallib->AddSeamlessComponentTable(hSession, hSeamlessTable, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\eworld_mnf_northamerica.tab", mbr);
	//efallib->AddSeamlessComponentTable(hSession, hSeamlessTable, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\eworld_mnf_oceana.tab", mbr);
	//efallib->AddSeamlessComponentTable(hSession, hSeamlessTable, L"C:\\Users\\RI008PE\\AppData\\Local\\Temp\\eworld_mnf\\eworld_mnf_southamerica.tab", mbr);
}

/*
################################################################################################
################################################################################################
#############                                                                      #############
#############      Main Processing and Entry Point                                 #############
#############                                                                      #############
################################################################################################
################################################################################################
*/
void Help()
{
	wprintf(L"\n");
	wprintf(L"\n");
	wprintf(L"Lines beginning with ' or # are comment lines and will be ignored.\n");
	wprintf(L"A blank line terminates the execution.\n");
	wprintf(L"\n");
	wprintf(L"Commands:\n");
	wprintf(L"  Open [Table] <<filename>> [as <<alias>>]\n");
	wprintf(L"  Pack <<table>>\n");
	wprintf(L"  Close {<<table>> | ALL}\n");
	wprintf(L"  Set <<var>> = <<expr>>\n");
	wprintf(L"  Show [<<var>>]\n");
	wprintf(L"  Tables\n");
	wprintf(L"  Desc <<table>>\n");
	wprintf(L"  Save [table] <<table>> in <<filename>> [charset << charset >> ][NOINSERT]\n");
	wprintf(L"  Save [table] <<table>> as nativex table in <<filename>> [UTF16 | UTF8 | charset << charset >> ][NOINSERT]\n");
	wprintf(L"  Save [table] <<table>> as geopackage table <<dbtable>> in <<file>>\n");
	wprintf(L"                                     [coordsys <<codespace:code>>]\n");
	wprintf(L"                                     [UTF16 | UTF8 | charset << charset >> ][NOINSERT]\n");
	wprintf(L"  Select ...\n");
	wprintf(L"  Insert ...\n");
	wprintf(L"  Update ...\n");
	wprintf(L"  Delete ...\n");
	wprintf(L"\n");
}
void ProcessCommand(wchar_t * command)
{
	// Process any commands that our command processor doesn't know how to handle...
	if (_wcsnicmp(command, L"'", 1) == 0) return;
	else if (_wcsnicmp(command, L"#", 1) == 0) return;
	else if (_wcsnicmp(command, L"open ", 5) == 0) Open(&(command[5])); 
	else if (_wcsnicmp(command, L"close ", 6) == 0) Close(&(command[6]));
	else if (_wcsnicmp(command, L"pack ", 5) == 0) Pack(&(command[5]));
	else if (_wcsnicmp(command, L"tables", 6) == 0) Tables();
	else if (_wcsnicmp(command, L"desc ", 5) == 0) Describe(&(command[5]));
	else if (_wcsnicmp(command, L"Save Table ", 11) == 0) SaveAs(&(command[11]));
	else if (_wcsnicmp(command, L"Save ", 5) == 0) SaveAs(&(command[5]));
	else if (_wcsnicmp(command, L"select ", 6) == 0) Select(command);
	else if (_wcsnicmp(command, L"insert ", 7) == 0) Insert(command);
	else if (_wcsnicmp(command, L"update ", 7) == 0) Update(command);
	else if (_wcsnicmp(command, L"delete ", 7) == 0) Delete(command);

	else if (_wcsnicmp(command, L"set ", 4) == 0) Set(&(command[4])); 
	else if (_wcsnicmp(command, L"show ", 5) == 0) Show(&(command[5])); 
	else if (_wcsicmp(command, L"show") == 0) Show(0); 

	else if (_wcsnicmp(command, L"help ", 5) == 0) Help(); 
	else if (_wcsicmp(command, L"help") == 0) Help();

	else if (_wcsicmp(command, L"test") == 0) Test();

	MyReportErrors();
}
void Prompt()
{
	bool done = false;
	while (!done)
	{
		wprintf(L"> ");
		wchar_t command[4096];
		#if ELLIS_OS_IS_WINOS
		_getws_s(command, sizeof(command)/sizeof(wchar_t));
		#else
		std::string str;
		std::getline(std::cin, str);
		mbstowcs(command, str.c_str(), sizeof(command) / sizeof(wchar_t));
		#endif
		if (wcslen(command) == 0) return;
		ProcessCommand(command);
	}
}

int wmain(int argc, wchar_t * argv[])
{
	efallib = EFALLIB::Create();
	if (!efallib){
		wprintf(L"Unable to create EFALLIB\n");
		return -1;
	}
	
	hSession = efallib->InitializeSession(nullptr);
	Prompt();

	efallib->DestroySession(hSession);
	return 0;
}
#if ELLIS_OS_ISUNIX
int main(int argc, char** argv)
{
	if (argc == 0) return wmain(0, 0);
	wchar_t ** wargv = new wchar_t *[argc];
	for (int i = 0; i < argc; i++) {
		const char * arg = argv[i];
		size_t len = 1 + strlen(arg);
		wchar_t * warg = new wchar_t[len];
		mbstowcs(warg, arg, len);
		wargv[i] = warg;
	}
	int ret = wmain(argc, wargv);
	for (int i = 0; i < argc; i++) {
		wchar_t * warg = wargv[i];
		delete[] warg;
	}
	delete[] wargv;
	return ret;
}
#endif



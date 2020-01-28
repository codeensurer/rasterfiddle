#include <stdafx.h>
#include <EFAL.h>
#include <time.h>
#include <string>
using namespace std;

EFALHANDLE hSession = 0;
EFALDATE efaldate(int year, int month, int day)
{
	EFALDATE date;
	date.year = year;
	date.month = month;
	date.day = day;
	return date;
}
EFALTIME efaltime(int hour, int minute, int second, int millisecond)
{
	EFALTIME time;
	time.hour = hour;
	time.minute = minute;
	time.second = second;
	time.millisecond = millisecond;
	return time;
}
EFALDATETIME efaldatetime(int year, int month, int day, int hour, int minute, int second, int millisecond)
{
	EFALDATETIME datetime;
	datetime.year = year;
	datetime.month = month;
	datetime.day = day;
	datetime.hour = hour;
	datetime.minute = minute;
	datetime.second = second;
	datetime.millisecond = millisecond;
	return datetime;
}

void CreatePointGeometry(char ** pbytes, unsigned long * pnbytes, double longitude, double latitude);
void CreateMultiPointGeometry(char ** pbytes, unsigned long * pnbytes, double* nodeData, int numPoints);
void CreateMultiCurveGeometry(char ** pbytes, unsigned long * pnbytes, double* nodeData, int* numPoints, int numCurves);
void CreateMultiPolygonGeometry(char ** pbytes, unsigned long * pnbytes, double* nodeData, int* numPoints, int* numRings, int numPolygons);
void CreateTextGeometry(char ** pbytes, unsigned long * pnbytes, Ellis::DRECT mbr, short angle, Ellis::CalloutLineType calloutLineType,
	Ellis::DPNT calloutTarget, const wchar_t* string);

void Sample()
{
	long nrecs, total_recs = 0;
	clock_t start = clock();


	const wchar_t * pathname = L"sample.tab";
	const wchar_t * csys = L"epsg:4326";// mapinfo:coordsys 1,104
	char * bytes;
	unsigned long nbytes;
	double dx = 0.00003, dy = 0.000015;
	double location_1[2] = { 12.084669, 55.646511 }, location_2[2] = { 12.084741, 55.647131 };

	EFALHANDLE hMetadata = 0;
	wchar_t acDir[_MAX_PATH];
	wchar_t acExt[_MAX_EXT];
	wchar_t acAlias[_MAX_PATH];
	wchar_t acDrive[_MAX_DRIVE];
	_wsplitpath_s(pathname, acDrive, sizeof(acDrive) / sizeof(wchar_t), acDir, sizeof(acDir) / sizeof(wchar_t), acAlias, sizeof(acAlias) / sizeof(wchar_t), acExt, sizeof(acExt) / sizeof(wchar_t));
	std::wstring strTablePath = acDrive;
	strTablePath += acDir;
	strTablePath += acAlias;
	strTablePath += L".TAB";

	hMetadata = EFAL::CreateNativeXTableMetadata(hSession, acAlias, strTablePath.c_str(), Ellis::MICHARSET::CHARSET_UTF8);
	EFALHANDLE hNewTable = 0;
	if (hMetadata != 0)
	{
		EFAL::AddColumn(hSession, hMetadata, L"OT_CHAR", Ellis::ALLTYPE_TYPE::OT_CHAR, /*indexed*/false, /*width*/12, /*decimals*/0, /*szCsys*/0);

		EFAL::AddColumn(hSession, hMetadata, L"OT_LOGICAL", Ellis::ALLTYPE_TYPE::OT_LOGICAL, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);

		EFAL::AddColumn(hSession, hMetadata, L"OT_SMALLINT", Ellis::ALLTYPE_TYPE::OT_SMALLINT, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);
		EFAL::AddColumn(hSession, hMetadata, L"OT_INTEGER", Ellis::ALLTYPE_TYPE::OT_INTEGER, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);
		EFAL::AddColumn(hSession, hMetadata, L"OT_INTEGER64", Ellis::ALLTYPE_TYPE::OT_INTEGER64, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);

		EFAL::AddColumn(hSession, hMetadata, L"OT_DECIMAL", Ellis::ALLTYPE_TYPE::OT_DECIMAL, /*indexed*/false, /*width*/10, /*decimals*/2, /*szCsys*/0);
		EFAL::AddColumn(hSession, hMetadata, L"OT_FLOAT", Ellis::ALLTYPE_TYPE::OT_FLOAT, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);

		EFAL::AddColumn(hSession, hMetadata, L"OT_DATE", Ellis::ALLTYPE_TYPE::OT_DATE, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);
		EFAL::AddColumn(hSession, hMetadata, L"OT_TIME", Ellis::ALLTYPE_TYPE::OT_TIME, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);
		EFAL::AddColumn(hSession, hMetadata, L"OT_DATETIME", Ellis::ALLTYPE_TYPE::OT_DATETIME, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);

		EFAL::AddColumn(hSession, hMetadata, L"OBJ", Ellis::ALLTYPE_TYPE::OT_OBJECT, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/csys);
		EFAL::AddColumn(hSession, hMetadata, L"MI_STYLE", Ellis::ALLTYPE_TYPE::OT_STYLE, /*indexed*/false, /*width*/0, /*decimals*/0, /*szCsys*/0);

		hNewTable = EFAL::CreateTable(hSession, hMetadata);
		EFAL::DestroyTableMetadata(hSession, hMetadata);
		if (hNewTable != 0)
		{
			EFAL::CreateVariable(hSession, L"@OT_CHAR");
			EFAL::CreateVariable(hSession, L"@OT_LOGICAL");
			EFAL::CreateVariable(hSession, L"@OT_SMALLINT");
			EFAL::CreateVariable(hSession, L"@OT_INTEGER");
			EFAL::CreateVariable(hSession, L"@OT_INTEGER64");
			EFAL::CreateVariable(hSession, L"@OT_DECIMAL");
			EFAL::CreateVariable(hSession, L"@OT_FLOAT");
			EFAL::CreateVariable(hSession, L"@OT_DATE");
			EFAL::CreateVariable(hSession, L"@OT_TIME");
			EFAL::CreateVariable(hSession, L"@OT_DATETIME");
			EFAL::CreateVariable(hSession, L"@OBJ");
			EFAL::CreateVariable(hSession, L"@MI_STYLE");

			EFAL::BeginWriteAccess(hSession, hNewTable);

			std::wstring insert;
			insert = L"Insert Into \"";
			insert.append(EFAL::GetTableName(hSession, hNewTable));
			insert.append(L"\" (OT_CHAR,OT_LOGICAL,OT_SMALLINT,OT_INTEGER,OT_INTEGER64,OT_DECIMAL,OT_FLOAT,OT_DATE,OT_TIME,OT_DATETIME,OBJ,MI_STYLE) VALUES (@OT_CHAR,@OT_LOGICAL,@OT_SMALLINT,@OT_INTEGER,@OT_INTEGER64,@OT_DECIMAL,@OT_FLOAT,@OT_DATE,@OT_TIME,@OT_DATETIME,@OBJ,@MI_STYLE)");

			// Now insert records by updating the variables
			/*
			* Record: Point geometry
			*/
			EFAL::SetVariableValueString(hSession, L"@OT_CHAR", L"wkbPoint");
			EFAL::SetVariableValueBoolean(hSession, L"@OT_LOGICAL", true);
			EFAL::SetVariableValueDouble(hSession, L"@OT_FLOAT", 1.234);
			EFAL::SetVariableValueDouble(hSession, L"@OT_DECIMAL", 1.234);
			EFAL::SetVariableValueInt64(hSession, L"@OT_INTEGER64", 4294967296);
			EFAL::SetVariableValueInt32(hSession, L"@OT_INTEGER", 2147483647);
			EFAL::SetVariableValueInt16(hSession, L"@OT_SMALLINT", 32767);
			EFAL::SetVariableValueDate(hSession, L"@OT_DATE", efaldate(2018, 9, 28));
			EFAL::SetVariableValueTime(hSession, L"@OT_TIME", efaltime(10, 8, 3, 001));
			EFAL::SetVariableValueDateTime(hSession, L"@OT_DATETIME", efaldatetime(2018, 9, 28, 10, 8, 3, 001));
			CreatePointGeometry(&bytes, &nbytes, location_1[0], location_1[1]);
			EFAL::SetVariableValueGeometry(hSession, L"@OBJ", nbytes, bytes, csys);
			delete[] bytes;
			EFAL::SetVariableValueStyle(hSession, L"@MI_STYLE", L"Symbol (60,255,10,\"MapInfo Cartographic\",0,0)");
			nrecs = EFAL::Insert(hSession, insert.c_str());
			if (nrecs > 0) { total_recs += nrecs; }

			/*
			* Record: MultiPoint geometry
			*/
			EFAL::SetVariableValueString(hSession, L"@OT_CHAR", L"wkbMultiPoint");
			EFAL::SetVariableValueBoolean(hSession, L"@OT_LOGICAL", false);
			EFAL::SetVariableValueDouble(hSession, L"@OT_FLOAT", -1.234);
			EFAL::SetVariableValueDouble(hSession, L"@OT_DECIMAL", -1.234);
			EFAL::SetVariableValueInt64(hSession, L"@OT_INTEGER64", -4294967296);
			EFAL::SetVariableValueInt32(hSession, L"@OT_INTEGER", -2147483647);
			EFAL::SetVariableValueInt16(hSession, L"@OT_SMALLINT", -32767);
			EFAL::SetVariableValueDate(hSession, L"@OT_DATE", efaldate(2018, 2, 28));
			EFAL::SetVariableValueTime(hSession, L"@OT_TIME", efaltime(14, 8, 3, 0));
			EFAL::SetVariableValueDateTime(hSession, L"@OT_DATETIME", efaldatetime(2018, 2, 28, 14, 8, 3, 0));


			double nodes[] = {
				location_2[0], location_2[1],
				location_2[0] + dx,location_2[1] - dy
			};
			CreateMultiPointGeometry(&bytes, &nbytes, nodes, 2);
			EFAL::SetVariableValueGeometry(hSession, L"@OBJ", nbytes, bytes, csys);
			delete[] bytes;
			EFAL::SetVariableValueStyle(hSession, L"@MI_STYLE", L"Symbol(58, 16711680, 12, \"MapInfo Cartographic\", 256, 0)");
			nrecs = EFAL::Insert(hSession, insert.c_str());
			if (nrecs > 0) { total_recs += nrecs; }

			/*
			* Record: LineString geometry
			*/
			EFAL::SetVariableValueString(hSession, L"@OT_CHAR", L"wkbLineString");
			EFAL::SetVariableValueBoolean(hSession, L"@OT_LOGICAL", true);
			EFAL::SetVariableValueDouble(hSession, L"@OT_FLOAT", 12.999);
			EFAL::SetVariableValueDouble(hSession, L"@OT_DECIMAL", 12.999);
			EFAL::SetVariableValueInt64(hSession, L"@OT_INTEGER64", 1);
			EFAL::SetVariableValueInt32(hSession, L"@OT_INTEGER", 1);
			EFAL::SetVariableValueInt16(hSession, L"@OT_SMALLINT", 1);
			EFAL::SetVariableValueDate(hSession, L"@OT_DATE", efaldate(2018, 2, 28));
			EFAL::SetVariableValueTime(hSession, L"@OT_TIME", efaltime(0, 0, 3, 0));
			EFAL::SetVariableValueDateTime(hSession, L"@OT_DATETIME", efaldatetime(2018, 2, 28, 0, 0, 3, 0));
			{
				double nodes[] = {
					location_1[0] + dx * 1,location_1[1] + dy * 1,
					location_1[0] + dx * 2,location_1[1] + dy * 1,
					location_1[0] + dx * 2,location_1[1] + dy * 2,
					location_1[0] + dx * 3,location_1[1] + dy * 2,
					location_1[0] + dx * 3,location_1[1] + dy * 3,
					location_1[0] + dx * 2,location_1[1] + dy * 3,
					location_1[0] + dx * 2,location_1[1] + dy * 4,
					location_1[0] + dx * 1,location_1[1] + dy * 4
				};
				int numPoints[] = { 8 };
				CreateMultiCurveGeometry(&bytes, &nbytes, nodes, numPoints, 1);
			}
			EFAL::SetVariableValueGeometry(hSession, L"@OBJ", nbytes, bytes, csys);
			delete[] bytes;
			EFAL::SetVariableValueStyle(hSession, L"@MI_STYLE", L"Pen (1,2,16711680)");
			nrecs = EFAL::Insert(hSession, insert.c_str());
			if (nrecs > 0) { total_recs += nrecs; }

			/*
			* Record: Multicurve geometry
			*/
			EFAL::SetVariableValueString(hSession, L"@OT_CHAR", L"wkbMultiLineString");
			EFAL::SetVariableValueBoolean(hSession, L"@OT_LOGICAL", true);
			EFAL::SetVariableValueDouble(hSession, L"@OT_FLOAT", 12.999);
			EFAL::SetVariableValueDouble(hSession, L"@OT_DECIMAL", 12.999);
			EFAL::SetVariableValueInt64(hSession, L"@OT_INTEGER64", 1);
			EFAL::SetVariableValueInt32(hSession, L"@OT_INTEGER", 1);
			EFAL::SetVariableValueInt16(hSession, L"@OT_SMALLINT", 1);
			EFAL::SetVariableValueDate(hSession, L"@OT_DATE", efaldate(2018, 2, 28));
			EFAL::SetVariableValueTime(hSession, L"@OT_TIME", efaltime(12, 0, 3, 0));
			EFAL::SetVariableValueDateTime(hSession, L"@OT_DATETIME", efaldatetime(2018, 2, 28, 12, 0, 3, 0));
			{
				double nodes[] = {
					location_2[0] + dx * 1,location_2[1] + dy * 1,
					location_2[0] + dx * 2,location_2[1] + dy * 1,
					location_2[0] + dx * 2,location_2[1] + dy * 2,
					location_2[0] + dx * 3,location_2[1] + dy * 2,
					location_2[0] + dx * 3,location_2[1] + dy * 3,
					location_2[0] + dx * 2,location_2[1] + dy * 3,
					location_2[0] + dx * 2,location_2[1] + dy * 4,
					location_2[0] + dx * 1,location_2[1] + dy * 4
				};
				int numPoints[] = {
					5,
					3
				};
				CreateMultiCurveGeometry(&bytes, &nbytes, nodes, numPoints, 2);
			}
			EFAL::SetVariableValueGeometry(hSession, L"@OBJ", nbytes, bytes, csys);
			delete[] bytes;
			EFAL::SetVariableValueStyle(hSession, L"@MI_STYLE", L"Pen (1,2,16711680)");
			nrecs = EFAL::Insert(hSession, insert.c_str());
			if (nrecs > 0) { total_recs += nrecs; }

			/*
			* Record: Polygon geometry
			*/
			EFAL::SetVariableValueString(hSession, L"@OT_CHAR", L"wkbPolygon");
			EFAL::SetVariableValueBoolean(hSession, L"@OT_LOGICAL", true);
			EFAL::SetVariableValueDouble(hSession, L"@OT_FLOAT", 12.999);
			EFAL::SetVariableValueDouble(hSession, L"@OT_DECIMAL", 12.999);
			EFAL::SetVariableValueInt64(hSession, L"@OT_INTEGER64", 1);
			EFAL::SetVariableValueInt32(hSession, L"@OT_INTEGER", 1);
			EFAL::SetVariableValueInt16(hSession, L"@OT_SMALLINT", 1);
			EFAL::SetVariableValueDate(hSession, L"@OT_DATE", efaldate(2018, 2, 28));
			EFAL::SetVariableValueTime(hSession, L"@OT_TIME", efaltime(12, 0, 3, 0));
			EFAL::SetVariableValueDateTime(hSession, L"@OT_DATETIME", efaldatetime(2018, 2, 28, 12, 0, 3, 0));
			{
				double nodes[] = {
					location_1[0] + dx * -1 * 1,location_1[1] + dy * -1 * 1,
					location_1[0] + dx * -1 * 7,location_1[1] + dy * -1 * 1,
					location_1[0] + dx * -1 * 7,location_1[1] + dy * -1 * 7,
					location_1[0] + dx * -1 * 4,location_1[1] + dy * -1 * 8,
					location_1[0] + dx * -1 * 1,location_1[1] + dy * -1 * 7,
					location_1[0] + dx * -1 * 1,location_1[1] + dy * -1 * 1
				};
				int numRings[] = { 1 };
				int numPoints[] = { 6 };
				CreateMultiPolygonGeometry(&bytes, &nbytes, nodes, numPoints, numRings, 1);
			}
			EFAL::SetVariableValueGeometry(hSession, L"@OBJ", nbytes, bytes, csys);
			delete[] bytes;
			EFAL::SetVariableValueStyle(hSession, L"@MI_STYLE", L"Pen (1,2,0) Brush (2,16774352,16777215)");
			nrecs = EFAL::Insert(hSession, insert.c_str());
			if (nrecs > 0) { total_recs += nrecs; }
			/*
			* Record: Multipolygon geometry
			*/
			EFAL::SetVariableValueString(hSession, L"@OT_CHAR", L"wkbMultiPolygon");
			EFAL::SetVariableIsNull(hSession, L"@OT_LOGICAL");
			EFAL::SetVariableIsNull(hSession, L"@OT_FLOAT");
			EFAL::SetVariableIsNull(hSession, L"@OT_DECIMAL");
			EFAL::SetVariableIsNull(hSession, L"@OT_INTEGER64");
			EFAL::SetVariableIsNull(hSession, L"@OT_INTEGER");
			EFAL::SetVariableIsNull(hSession, L"@OT_SMALLINT");
			EFAL::SetVariableIsNull(hSession, L"@OT_DATE");
			EFAL::SetVariableIsNull(hSession, L"@OT_TIME");
			EFAL::SetVariableIsNull(hSession, L"@OT_DATETIME");
			{
				double nodes[] = {
					location_2[0] + dx * -1 * 1,location_2[1] + dy * -1 * 1,
					location_2[0] + dx * -1 * 7,location_2[1] + dy * -1 * 1,
					location_2[0] + dx * -1 * 7,location_2[1] + dy * -1 * 7,
					location_2[0] + dx * -1 * 4,location_2[1] + dy * -1 * 8,
					location_2[0] + dx * -1 * 1,location_2[1] + dy * -1 * 7,
					location_2[0] + dx * -1 * 1,location_2[1] + dy * -1 * 1,
					location_2[0] + dx * -1 * 2,location_2[1] + dy * -1 * 2,
					location_2[0] + dx * -1 * 6,location_2[1] + dy * -1 * 2,
					location_2[0] + dx * -1 * 6,location_2[1] + dy * -1 * 6,
					location_2[0] + dx * -1 * 2,location_2[1] + dy * -1 * 6,
					location_2[0] + dx * -1 * 2,location_2[1] + dy * -1 * 2,
					location_2[0] + dx * -1 * 3,location_2[1] + dy * -1 * 4,
					location_2[0] + dx * -1 * 4,location_2[1] + dy * -1 * 3,
					location_2[0] + dx * -1 * 5,location_2[1] + dy * -1 * 4,
					location_2[0] + dx * -1 * 4,location_2[1] + dy * -1 * 5,
					location_2[0] + dx * -1 * 3,location_2[1] + dy * -1 * 4,
					location_2[0] + dx * -1 * 8,location_2[1] + dy * -1 * 1,
					location_2[0] + dx * -1 * 9,location_2[1] + dy * -1 * 1,
					location_2[0] + dx * -1 * 9,location_2[1] + dy * -1 * 7,
					location_2[0] + dx * -1 * 8,location_2[1] + dy * -1 * 7,
					location_2[0] + dx * -1 * 8,location_2[1] + dy * -1 * 1
				};
				int numRings[] = { 3, 1 };
				int numPoints[] = { 6,5,5,5 };
				CreateMultiPolygonGeometry(&bytes, &nbytes, nodes, numPoints, numRings, 2);
			}
			EFAL::SetVariableValueGeometry(hSession, L"@OBJ", nbytes, bytes, csys);
			delete[] bytes;
			EFAL::SetVariableValueStyle(hSession, L"@MI_STYLE", L"Pen (1,2,16711680)  Brush (2,16774352,16777215)");
			nrecs = EFAL::Insert(hSession, insert.c_str());
			if (nrecs > 0) { total_recs += nrecs; }

			// Insert text geometry
			EFAL::SetVariableValueString(hSession, L"@OT_CHAR", L"ewkbLegacyText");
			EFAL::SetVariableValueBoolean(hSession, L"@OT_LOGICAL", true);
			EFAL::SetVariableValueDouble(hSession, L"@OT_FLOAT", 1.234);
			EFAL::SetVariableValueDouble(hSession, L"@OT_DECIMAL", 1.234);
			EFAL::SetVariableValueInt64(hSession, L"@OT_INTEGER64", 4294967296);
			EFAL::SetVariableValueInt32(hSession, L"@OT_INTEGER", 2147483647);
			EFAL::SetVariableValueInt16(hSession, L"@OT_SMALLINT", 32767);
			EFAL::SetVariableValueDate(hSession, L"@OT_DATE", efaldate(2018, 9, 28));
			EFAL::SetVariableValueTime(hSession, L"@OT_TIME", efaltime(10, 8, 3, 001));
			EFAL::SetVariableValueDateTime(hSession, L"@OT_DATETIME", efaldatetime(2018, 9, 28, 10, 8, 3, 001));
			Ellis::DRECT dMBR = { 12.10,55.7,12.11,55.71 };
			Ellis::DPNT dPnt = { 0 };
			CreateTextGeometry(&bytes, &nbytes, dMBR, 0, Ellis::CalloutLineType::eNone, dPnt, L"Hello World");
			EFAL::SetVariableValueGeometry(hSession, L"@OBJ", nbytes, bytes, csys);
			delete[] bytes;
			EFAL::SetVariableValueStyle(hSession, L"@MI_STYLE", L"Font ('Arial',34,0,16767152,65280)");
			nrecs = EFAL::Insert(hSession, insert.c_str());
			if (nrecs > 0) { total_recs += nrecs; }

			EFAL::EndAccess(hSession, hNewTable);

			clock_t finish = clock();
			double duration = (double)(finish - start) / CLOCKS_PER_SEC;
			wprintf(L"Duration: %8.1f seconds \n", duration);
		}
	}
}

int wmain(int argc, wchar_t * argv[])
{
	hSession = EFAL::InitializeSession(0);
	Sample();
	EFAL::DestroySession(hSession);
	return 0;
}


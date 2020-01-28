#include <stdafx.h>
#include <EFAL.h>
#include <WktConvert.h>
#include <string>

static void trim(const wchar_t * & txt) {
	while (*txt == (wchar_t)L' ') txt++;
}

WktConvert::WktConvert(const wchar_t * csys) :
	_csys(csys),
	_wkbGeometry(nullptr),
	_wkbGeometryType(),
	_dimensionality(WkbGeometry::DIMENSIONALITY::P),
	_wkt(L""),
	numPoints(0),
	numLineStrings(0),
	numRings(0),
	numPolygons(0),
	numGeometries(0),
	iPoint(0),
	iLineString(0),
	iRing(0),
	iPolygon(0),
	iGeometry(0)
{
}

WktConvert::~WktConvert()
{
	if (_wkbGeometry != nullptr) delete _wkbGeometry;
	_wkbGeometry = nullptr;
}

/*
<point tagged text>              ::= point <point text>
<linestring tagged text>         ::= linestring <linestring text>
<polygon tagged text>            ::= polygon <polygon text>
<polyhedralsurface tagged text>  ::= polyhedralsurface <polyhedralsurface text>
<triangle tagged text>           ::= triangle <polygon text> 
<tin tagged text>                ::= tin <polyhedralsurface text>
<multipoint tagged text>         ::= multipoint <multipoint text>
<multilinestring tagged text>    ::= multilinestring <multilinestring text>
<multipolygon tagged text>       ::= multipolygon <multipolygon text>
<geometrycollection tagged text> ::= geometrycollection <geometrycollection text>
<point>                          ::= <x> <y>
<point text>                     ::= <empty set> | <left paren> <point> <right paren>
<linestring text>                ::= <empty set> | <left paren> <point> {<comma> <point>}* <right paren>
<polygon text>                   ::= <empty set> | <left paren> <linestring text> {<comma> <linestring text>}* <right paren>
<polyhedralsurface text>         ::= <empty set> | <left paren> <polygon text> {<comma> <polygon text>}* <right paren>
<multipoint text>                ::= <empty set> | <left paren> <point text> { <comma> <point text>}* <right paren>
<multilinestring text>           ::= <empty set> | <left paren> <linestring text> { <comma> <linestring text>}* <right paren>
<multipolygon text>              ::= <empty set> | <left paren> <polygon text> { <comma> <polygon text>}* <right paren>
<geometrycollection text>        ::= <empty set> | <left paren> <geometry tagged text> { <comma> <geometry tagged text>}* <right paren>
<geometry tagged text>           ::= <point tagged text>
                                   | <linestring tagged text>
                                   | <polygon tagged text>
                                   | <triangle tagged text>
                                   | <polyhedralsurface tagged text>
                                   | <tin tagged text>
                                   | <multipoint tagged text>
                                   | <multilinestring tagged text>
                                   | <multipolygon tagged text>
                                   | <geometrycollection tagged text>
*/
bool matchComma(const wchar_t * & wkt)
{
	trim(wkt);
	if (_wcsnicmp(wkt, L",", 1) == 0) {
		wkt++;
		return true;
	}
	return false;
}
bool matchOpenParen(const wchar_t * & wkt)
{
	trim(wkt);
	if (_wcsnicmp(wkt, L"(", 1) == 0) {
		wkt++;
		return true;
	}
	return false;
}
bool matchCloseParen(const wchar_t * & wkt)
{
	trim(wkt);
	if (_wcsnicmp(wkt, L")", 1) == 0) {
		wkt++;
		return true;
	}
	return false;
}
bool matchDouble(const wchar_t * & wkt, double & d)
{
	trim(wkt);
	const wchar_t * pDblParts = L"0123456789-+.e";
	if (wcschr(pDblParts, wkt[0]) != nullptr) {
		const wchar_t * pEnd = wkt+1;
		while (wcschr(pDblParts, pEnd[0]) != nullptr) pEnd++;
		std::wstring s;
		size_t len = pEnd - wkt;
		s.append(wkt, len);
		if (swscanf_s(s.c_str(), L"%lf", &d) == 1) {
			wkt = pEnd;
			return true;
		}
	}
	return false;
}
bool parseWKBGeometryType(const wchar_t * & wkt, WKBGeometryType & wkbGeometryType)
{
	bool bOK = false;
	trim(wkt);
	if (_wcsnicmp(wkt, L"point", wcslen(L"point")) == 0) {
		wkt += wcslen(L"point");
		wkbGeometryType = WKBGeometryType::wkbPoint;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"linestring", wcslen(L"linestring")) == 0) {
		wkt += wcslen(L"linestring");
		wkbGeometryType = WKBGeometryType::wkbLineString;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"polygon", wcslen(L"polygon")) == 0) {
		wkt += wcslen(L"polygon");
		wkbGeometryType = WKBGeometryType::wkbPolygon;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"polyhedralsurface", wcslen(L"polyhedralsurface")) == 0) {
		wkt += wcslen(L"polyhedralsurface");
		wkbGeometryType = WKBGeometryType::wkbPolyhedralSurface;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"triangle", wcslen(L"triangle")) == 0) {
		wkt += wcslen(L"triangle");
		wkbGeometryType = WKBGeometryType::wkbTriangle;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"tin", wcslen(L"tin")) == 0) {
		wkt += wcslen(L"tin");
		wkbGeometryType = WKBGeometryType::wkbTIN;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"multipoint", wcslen(L"multipoint")) == 0) {
		wkt += wcslen(L"multipoint");
		wkbGeometryType = WKBGeometryType::wkbMultiPoint;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"multilinestring", wcslen(L"multilinestring")) == 0) {
		wkt += wcslen(L"multilinestring");
		wkbGeometryType = WKBGeometryType::wkbMultiLineString;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"multipolygon", wcslen(L"multipolygon")) == 0) {
		wkt += wcslen(L"multipolygon");
		wkbGeometryType = WKBGeometryType::wkbMultiPolygon;
		bOK = true;
	}
	else if (_wcsnicmp(wkt, L"geometrycollection", wcslen(L"geometrycollection")) == 0) {
		wkt += wcslen(L"geometrycollection");
		wkbGeometryType = WKBGeometryType::wkbGeometryCollection;
		bOK = true;
	}

	if (bOK) {
		trim(wkt);
		if (_wcsnicmp(wkt, L"zm", wcslen(L"zm")) == 0) {
			wkbGeometryType = (WKBGeometryType)((int)wkbGeometryType + 3000);
			wkt += wcslen(L"zm");
		}
		else if (_wcsnicmp(wkt, L"m", wcslen(L"m")) == 0) {
			wkbGeometryType = (WKBGeometryType)((int)wkbGeometryType + 2000);
			wkt += wcslen(L"m");
		}
		else if (_wcsnicmp(wkt, L"z", wcslen(L"z")) == 0) {
			wkbGeometryType = (WKBGeometryType)((int)wkbGeometryType + 1000);
			wkt += wcslen(L"z");
		}
	}
	trim(wkt);
	return bOK;
}
bool WktConvert::convert(const wchar_t * wkt)
{
	return parseGeometry(wkt);
}
bool WktConvert::parseGeometry(const wchar_t * & wkt)
{
	bool bOK = false;
	WKBGeometryType wkbGeometryType = (WKBGeometryType)0;
	if (parseWKBGeometryType(wkt, wkbGeometryType)) {
		switch (wkbGeometryType) {
		case wkbPoint: bOK = parsePoint(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbLineString: bOK = parseLineString(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbPolygon: bOK = parsePolygon(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbTriangle: bOK = parseTriangle(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbMultiPoint: bOK = parseMultiPoint(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbMultiLineString: bOK = parseMultiLineString(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbMultiPolygon: bOK = parseMultiPolygon(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbGeometryCollection: bOK = parseGeometryCollection(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbPolyhedralSurface: bOK = parsePolyhedralSurface(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbTIN: bOK = parseTIN(wkt, WkbGeometry::DIMENSIONALITY::P); break;
		case wkbPointZ: bOK = parsePoint(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbLineStringZ: bOK = parseLineString(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbPolygonZ: bOK = parsePolygon(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbTrianglez: bOK = parseTriangle(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbMultiPointZ: bOK = parseMultiPoint(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbMultiLineStringZ: bOK = parseMultiLineString(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbMultiPolygonZ: bOK = parseMultiPolygon(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbGeometryCollectionZ: bOK = parseGeometryCollection(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbPolyhedralSurfaceZ: bOK = parsePolyhedralSurface(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbTINZ: bOK = parseTIN(wkt, WkbGeometry::DIMENSIONALITY::Z); break;
		case wkbPointM: bOK = parsePoint(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbLineStringM: bOK = parseLineString(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbPolygonM: bOK = parsePolygon(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbTriangleM: bOK = parseTriangle(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbMultiPointM: bOK = parseMultiPoint(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbMultiLineStringM: bOK = parseMultiLineString(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbMultiPolygonM: bOK = parseMultiPolygon(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbGeometryCollectionM: bOK = parseGeometryCollection(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbPolyhedralSurfaceM: bOK = parsePolyhedralSurface(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbTINM: bOK = parseTIN(wkt, WkbGeometry::DIMENSIONALITY::M); break;
		case wkbPointZM: bOK = parsePoint(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbLineStringZM: bOK = parseLineString(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbPolygonZM: bOK = parsePolygon(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbTriangleZM: bOK = parseTriangle(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbMultiPointZM: bOK = parseMultiPoint(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbMultiLineStringZM: bOK = parseMultiLineString(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbMultiPolygonZM: bOK = parseMultiPolygon(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbGeometryCollectionZM: bOK = parseGeometryCollection(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbPolyhedralSurfaceZM: bOK = parsePolyhedralSurface(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		case wkbTinZM: bOK = parseTIN(wkt, WkbGeometry::DIMENSIONALITY::ZM); break;
		}
	}
	return bOK;
}
bool WktConvert::parsePoint(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	double x = 0, y = 0, z = 0, m = 0;
	if (matchOpenParen(wkt) && matchDouble(wkt, x) && matchDouble(wkt, y)) {
		if ((dimensionality == WkbGeometry::Z) && !matchDouble(wkt, z)) return false;
		if ((dimensionality == WkbGeometry::M) && !matchDouble(wkt, m)) return false;
		if ((dimensionality == WkbGeometry::ZM) && !(matchDouble(wkt, z) && matchDouble(wkt, m))) return false;
		if (matchCloseParen(wkt)) {
			_wkbGeometry = WkbGeometry::CreatePointGeometry(_csys.c_str(), dimensionality, x, y, z, m);
			return true;
		}
	}
	return false;
}
bool WktConvert::parsePoints(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality, std::vector<WkbGeometry::PNT> & points)
{
	bool bOK = false;
	WkbGeometry::PNT pnt;
	if (matchOpenParen(wkt))
	{
		bool done = false;
		while (!done)
		{
			done = true;
			if (matchDouble(wkt, pnt.x) && matchDouble(wkt, pnt.y)) {
				if ((dimensionality == WkbGeometry::P)  ||
					((dimensionality == WkbGeometry::Z) && matchDouble(wkt, pnt.z)) ||
					((dimensionality == WkbGeometry::M) && matchDouble(wkt, pnt.m)) ||
					((dimensionality == WkbGeometry::ZM) && matchDouble(wkt, pnt.z) && matchDouble(wkt, pnt.m))) {
					points.push_back(pnt);
					if (matchComma(wkt)) done = false;
				}
			}
		}
		bOK = matchCloseParen(wkt);
	}
	return bOK;
}
bool WktConvert::parsePointText(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality, WkbGeometry::PNT & point)
{
	if (matchOpenParen(wkt) && matchDouble(wkt, point.x) && matchDouble(wkt, point.y)) {
		if ((dimensionality == WkbGeometry::P) ||
			((dimensionality == WkbGeometry::Z) && matchDouble(wkt, point.z)) ||
			((dimensionality == WkbGeometry::M) && matchDouble(wkt, point.m)) ||
			((dimensionality == WkbGeometry::ZM) && matchDouble(wkt, point.z) && matchDouble(wkt, point.m))) {
			if (matchCloseParen(wkt)) {
				return true;
			}
		}
	}
	return false;
}

bool WktConvert::parseMultiPoint(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	/*
	<point>                          ::= <x> <y>
	<point text>                     ::= <empty set> | <left paren> <point> <right paren>
	<linestring text>                ::= <empty set> | <left paren> <point> {<comma> <point>}* <right paren>
	<multipoint text>                ::= <empty set> | <left paren> <point text> { <comma> <point text>}* <right paren>
	*/
	std::vector<WkbGeometry::PNT> points;
	bool bOK = false;
	WkbGeometry::PNT pnt;
	if (matchOpenParen(wkt))
	{
		bool done = false;
		while (!done)
		{
			done = true;
			if (parsePointText(wkt, dimensionality, pnt)) {
				points.push_back(pnt);
				if (matchComma(wkt)) done = false;
			}
		}
		bOK = matchCloseParen(wkt);
	}
	if (bOK) {
		_wkbGeometry = WkbGeometry::CreateMultiPointGeometry(_csys.c_str(), dimensionality, points);
		return true;
	}
	return false;
}
bool WktConvert::parseLineString(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	std::vector<WkbGeometry::PNT> points;
	if (parsePoints(wkt, dimensionality, points)) {
		std::vector<std::vector<WkbGeometry::PNT>> curves;
		curves.push_back(points);
		_wkbGeometry = WkbGeometry::CreateMultiCurveGeometry(_csys.c_str(), dimensionality, curves);
		return true;
	}
	return false;
}
bool WktConvert::parsePointSets(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality, std::vector<std::vector<WkbGeometry::PNT>> & pointSets)
{
	bool bOK = false;
	if (matchOpenParen(wkt))
	{
		bool done = false;
		while (!done)
		{
			done = true;
			std::vector<WkbGeometry::PNT> points;
			if (parsePoints(wkt, dimensionality, points)) {
				pointSets.push_back(points);
				if (matchComma(wkt)) done = false;
			}
		}
		bOK = matchCloseParen(wkt);
	}
	return bOK;
}
bool WktConvert::parseMultiLineString(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	std::vector<std::vector<WkbGeometry::PNT>> linestrings;
	bool bOK = parsePointSets(wkt, dimensionality, linestrings);
	if (bOK) {
		_wkbGeometry = WkbGeometry::CreateMultiCurveGeometry(_csys.c_str(), dimensionality, linestrings);
		return true;
	}
	return false;
}
bool WktConvert::parsePolygon(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	/*
	<polygon tagged text>            ::= polygon <polygon text>
	<polygon text>                   ::= <empty set> | <left paren> <linestring text> {<comma> <linestring text>}* <right paren>
	<linestring text>                ::= <empty set> | <left paren> <point> {<comma> <point>}* <right paren>
	<point>                          ::= <x> <y>

	<multipolygon tagged text>       ::= multipolygon <multipolygon text>
	<multipolygon text>              ::= <empty set> | <left paren> <polygon text> { <comma> <polygon text>}* <right paren>
	*/
	std::vector<std::vector<WkbGeometry::PNT>> linestrings;
	bool bOK = parsePointSets(wkt, dimensionality, linestrings);
	if (bOK) {
		std::vector<std::vector<std::vector<WkbGeometry::PNT>>> polygons;
		polygons.push_back(linestrings);
		_wkbGeometry = WkbGeometry::CreateMultiPolygonGeometry(_csys.c_str(), dimensionality, polygons);
		return true;
	}
	return false;
}
bool WktConvert::parseMultiPolygon(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	bool bOK = false;
	std::vector<std::vector<std::vector<WkbGeometry::PNT>>> polygons;
	if (matchOpenParen(wkt))
	{
		bool done = false;
		while (!done)
		{
			done = true;
			std::vector<std::vector<WkbGeometry::PNT>> linestrings;
			if (parsePointSets(wkt, dimensionality, linestrings)) {
				polygons.push_back(linestrings);
				if (matchComma(wkt)) done = false;
			}

		}
		bOK = matchCloseParen(wkt);
	}
	if (bOK) {
		_wkbGeometry = WkbGeometry::CreateMultiPolygonGeometry(_csys.c_str(), dimensionality, polygons);
		return true;
	}
	return false;
}
bool WktConvert::parseGeometryCollection(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	/*
	<geometrycollection tagged text> ::= geometrycollection <geometrycollection text>

	<geometrycollection text>        ::= <empty set> | <left paren> <geometry tagged text> { <comma> <geometry tagged text>}* <right paren>
	<geometry tagged text>           ::= <point tagged text>
	| <linestring tagged text>
	| <polygon tagged text>
	| <triangle tagged text>
	| <polyhedralsurface tagged text>
	| <tin tagged text>
	| <multipoint tagged text>
	| <multilinestring tagged text>
	| <multipolygon tagged text>
	| <geometrycollection tagged text>
	*/
	bool bOK = false;
	std::vector<WkbGeometry *> geometryCollection;;

	if (matchOpenParen(wkt))
	{
		bool done = false;
		while (!done)
		{
			done = true;
			if (parseGeometry(wkt)) {
				geometryCollection.push_back(_wkbGeometry);
				_wkbGeometry = nullptr;
				if (matchComma(wkt)) done = false;
			}
		}
		bOK = matchCloseParen(wkt);
	}
	if (bOK) {
		_wkbGeometry = WkbGeometry::CreateGeometryCollection(_csys.c_str(), dimensionality, geometryCollection);
		for (size_t i = 0, n = geometryCollection.size(); i < n; i++) {
			delete geometryCollection[i];
		}
		geometryCollection.clear();
		return true;
	}
	return false;
}
bool WktConvert::parseTriangle(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	// Note from OGC: A Triangle is a polygon with 3 distinct, non-collinear vertices and no interior boundary.
	// Here we parse it as a polygon which will create a polygon geometry. This is technically wrong but at the moment
	// EFAL does not directly support Triangle geometries.
	return parsePolygon(wkt, dimensionality);
}
bool WktConvert::parsePolyhedralSurface(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	// Not supported
	return false;
}
bool WktConvert::parseTIN(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality) {
	// Not supported
	return false;
}
bool WktConvert::finish(bool bOK)
{
	if (!bOK)
	{
		_wkt.clear();
	}
	return bOK;
}
bool WktConvert::setWkbType(WKBGeometryType wkbGeometryType)
{
	_wkbGeometryType = WkbGeometry::BaseWKBType(wkbGeometryType, _dimensionality);
	switch (_wkbGeometryType)
	{
	case WKBGeometryType::wkbPoint:
		_wkt += L"POINT ";
		iPoint = 0;
		break;

	case WKBGeometryType::wkbMultiPoint:
		_wkt += L"MULTIPOINT ";
		iPoint = 0;
		break;

	case WKBGeometryType::wkbLineString:
		_wkt += L"LINESTRING ";
		iPoint = 0;
		iLineString = 0;
		break;

	case WKBGeometryType::wkbMultiLineString:
		_wkt += L"MULTILINESTRING ";
		iLineString = 0;
		break;

	case WKBGeometryType::wkbPolygon:
	case WKBGeometryType::wkbTriangle:
		_wkt += L"POLYGON ";
		iRing = 0;
		iPolygon = 0;
		break;

	case WKBGeometryType::wkbMultiPolygon:
		_wkt += L"MULTIPOLYGON ";
		iRing = 0;
		iPolygon = 0;
		break;

	case WKBGeometryType::wkbGeometryCollection:
		_wkt += L"GEOMETRYCOLLECTION ";
		iGeometry = 0;
		break;

	case WKBGeometryType::wkbPolyhedralSurface:
	case WKBGeometryType::wkbTIN:
		// TODO: Unimplemented
		return false;
	}
	if (_dimensionality == WkbGeometry::DIMENSIONALITY::Z)
	{
		_wkt += L"Z ";
	}
	else if (_dimensionality == WkbGeometry::DIMENSIONALITY::M)
	{
		_wkt += L"M ";
	}
	else if (_dimensionality == WkbGeometry::DIMENSIONALITY::ZM)
	{
		_wkt += L"ZM ";
	}
	return true;
}
bool WktConvert::setNumPoints(MI_UINT32 n)
{
	numPoints = n;
	iPoint = 0;
	return true;
}
bool WktConvert::setNumLineStrings(MI_UINT32 n)
{
	numLineStrings = n;
	iLineString = 0;
	return true;
}
bool WktConvert::setNumRings(MI_UINT32 n)
{
	numRings = n;
	iRing = 0;
	return true;
}
bool WktConvert::setNumPolygons(MI_UINT32 n)
{
	numPolygons = n;
	iPolygon = 0;
	return true;
}
bool WktConvert::setNumGeometries(MI_UINT32 n)
{
	numGeometries = n;
	iGeometry = 0;
	return true;
}
bool WktConvert::addPoint(WkbGeometry::PNT point)
{
	if (iPoint > 0) {
		_wkt += L",";
	}
	iPoint++;

	// TODO: Currently this code is written assuming a long/lat coordinate system like WGS84. %.6lf is used to limit the
	// output decimal places. I should make this more generic with maybe a format to be specified by the caller.

	if ((_wkbGeometryType == WKBGeometryType::wkbPoint)|| (_wkbGeometryType == WKBGeometryType::wkbMultiPoint)) _wkt += L"(";
	wchar_t buf[32];
	_swprintf(buf, L"%.6lf", point.x);
	_wkt += buf;
	_wkt += L" ";
	_swprintf(buf, L"%.6lf", point.y);
	_wkt += buf;
	if ((_dimensionality == WkbGeometry::DIMENSIONALITY::Z) || (_dimensionality == WkbGeometry::DIMENSIONALITY::ZM))
	{
		_wkt += L" ";
		_swprintf(buf, L"%.6lf", point.z);
		_wkt += buf;
	}
	if ((_dimensionality == WkbGeometry::DIMENSIONALITY::M) || (_dimensionality == WkbGeometry::DIMENSIONALITY::ZM))
	{
		_wkt += L" ";
		_swprintf(buf, L"%.6lf", point.m);
		_wkt += buf;
	}
	if ((_wkbGeometryType == WKBGeometryType::wkbPoint) || (_wkbGeometryType == WKBGeometryType::wkbMultiPoint)) _wkt += L")";
	return true;
}
bool WktConvert::startMulti()
{
	_wkt += L"(";
	return true;
}
bool WktConvert::endMulti()
{
	_wkt += L")";
	return true;
}
bool WktConvert::startPoints()
{
	_wkt += L"(";
	iPoint = 0;
	return true;
}
bool WktConvert::startLineString()
{
	if (iLineString > 0) {
		_wkt += L",";
	}
	iLineString++;
	_wkt += L"(";
	return true;
}
bool WktConvert::startRing()
{
	if (iRing > 0) {
		_wkt += L",";
	}
	iRing++;
	_wkt += L"(";
	return true;
}
bool WktConvert::startPolygon()
{
	if (iPolygon > 0) {
		_wkt += L",";
	}
	iPolygon++;
	_wkt += L"(";
	return true;
}
bool WktConvert::endPoints()
{
	_wkt += L")";
	return true;
}
bool WktConvert::endLineString()
{
	_wkt += L")";
	return true;
}
bool WktConvert::endRing()
{
	_wkt += L")";
	return true;
}
bool WktConvert::endPolygon()
{
	_wkt += L")";
	return true;
}
bool WktConvert::startGeometry()
{
	if (iGeometry > 0) {
		_wkt += L",";
	}
	iGeometry++;
	//_wkt += L"(";
	return true;
}
bool WktConvert::endGeometry()
{
	//_wkt += L")";
	return true;
}

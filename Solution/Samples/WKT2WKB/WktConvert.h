#pragma once
#include <WkbGeometry.h>
#include <vector>


class WktConvert : public WkbGeometry::WkbParserCallback
{
public:
	WktConvert(const wchar_t * csys);
	~WktConvert();
public:
	bool convert(const wchar_t * wkt);
	const wchar_t * GetCSys() const { return _csys.c_str(); }
	WkbGeometry * GetWkbGeometry() const { return _wkbGeometry; }
	std::wstring AsWkt() const { return _wkt; }

private:
	bool parsePointText(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality, WkbGeometry::PNT & point);
	bool parsePoints(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality, std::vector<WkbGeometry::PNT> & points);
	bool parsePointSets(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality, std::vector<std::vector<WkbGeometry::PNT>> & pointSets);

	bool parseGeometry(const wchar_t * & wkt);
	bool parsePoint(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parseLineString(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parsePolygon(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parseTriangle(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parseMultiPoint(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parseMultiLineString(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parseMultiPolygon(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parseGeometryCollection(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parsePolyhedralSurface(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);
	bool parseTIN(const wchar_t * & wkt, WkbGeometry::DIMENSIONALITY dimensionality);

private:
	bool setWkbType(WKBGeometryType wkbGeometryType);
	bool addPoint(WkbGeometry::PNT point);
	bool setNumPoints(MI_UINT32 n);
	bool setNumLineStrings(MI_UINT32 n);
	bool setNumRings(MI_UINT32 n);
	bool setNumPolygons(MI_UINT32 n);
	bool setNumGeometries(MI_UINT32 n);
	bool startMulti();
	bool startPoints();
	bool startLineString();
	bool startRing();
	bool startPolygon();
	bool startGeometry();
	bool endMulti();
	bool endPoints();
	bool endLineString();
	bool endRing();
	bool endPolygon();
	bool endGeometry();
	bool finish(bool bOK);

private:
	std::wstring  _csys;
	WkbGeometry * _wkbGeometry;

	WKBGeometryType _wkbGeometryType;
	WkbGeometry::DIMENSIONALITY _dimensionality;
	std::wstring _wkt;

	MI_UINT32 numPoints;
	MI_UINT32 numLineStrings;
	MI_UINT32 numRings;
	MI_UINT32 numPolygons;
	MI_UINT32 numGeometries;

	MI_UINT32 iPoint;
	MI_UINT32 iLineString;
	MI_UINT32 iRing;
	MI_UINT32 iPolygon;
	MI_UINT32 iGeometry;
};

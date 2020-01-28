#pragma once
#include <EFAL.h>
#include <vector>

class WkbGeometry
{
public:
	WkbGeometry(const wchar_t * csys);
	~WkbGeometry();

	struct PNT
	{
		double x;
		double y;
		double z;
		double m;
	};

	enum DIMENSIONALITY {
		P,
		Z,
		M,
		ZM
	};

	/*
	* Callback Sequences to expect per Geometry Type
	* -------------------------------------------------------
	* Point
	*		callback->setWkbType(wkbGeometryType);
	*		callback->setEnvelope();
	*		callback->addPoint(pnt);
	*		callback->finish(bOK);
	*
	* MultiPoint
	*		callback->setWkbType(wkbGeometryType);
	*		callback->setEnvelope();
	*		callback->startPoints();
	*		callback->setNumPoints(numPoints);
	*		foreach point {
	*			callback->addPoint(pnt);
	*		}
	*		callback->endPoints();
	*		callback->finish(bOK);
	*
	* LineString
	*		callback->setWkbType(wkbGeometryType);
	*		callback->setEnvelope();
	*		callback->startLineString();
	*		callback->setNumPoints(numPoints);
	*		foreach point {
	*			callback->addPoint(pnt);
	*		}
	*		callback->endLineString();
	*		callback->finish(bOK);
	*
	* MultiLineString
	*		callback->setWkbType(wkbGeometryType);
	*		callback->setEnvelope();
	*		callback->startMulti();
	*		callback->setNumLineStrings(numLineStrings);
	*
	*		foreach linestring {
	*			callback->startLineString();
	*			callback->setNumPoints(numPoints);
	*			foreach point {
	*				callback->addPoint(pnt);
	*			}
	*			callback->endLineString();
	*		}
	*		callback->endMulti();
	*		callback->finish(bOK);
	*
	* Polygon
	*		callback->setWkbType(wkbGeometryType);
	*		callback->setEnvelope();
	*		callback->startPolygon();
	*		callback->setNumRings(numRings);
	*		foreach ring {
	*			callback->startRing();
	*			callback->setNumPoints(numPoints);
	*			foreach point {
	*				callback->addPoint(pnt);
	*			}
	*			callback->endRing();
	*		}
	*		callback->endPolygon();
	*		callback->finish(bOK);
	*
	* MultiPolygon
	*		callback->setWkbType(wkbGeometryType);
	*		callback->setEnvelope();
	*		callback->startMulti();
	*		callback->setNumPolygons(numPolygons);
	*		foreach polygon {
	*			callback->startPolygon();
	*			callback->setNumRings(numRings);
	*			foreach ring {
	*				callback->startRing();
	*				callback->setNumPoints(numPoints);
	*				foreach point {
	*					callback->addPoint(pnt);
	*				}
	*				callback->endRing();
	*			}
	*			callback->endPolygon();
	*		}
	*		callback->endMulti();
	*		callback->finish(bOK);
	*
	* GeometryCollection
	*		callback->setWkbType(wkbGeometryType);
	*		callback->setEnvelope();
	*		callback->startMulti();
	*		callback->setNumGeometries(numGeometries);
	*		foreach geometry {
	*			callback->startGeometry();
	*			callback->setWkbType(wkbGeometryType);
	*			Callbacks as expected for the geometry type
	*			callback->endGeometry();
	*		}
	*		callback->endMulti();
	*		callback->finish(bOK);
	*
	*/
	class WkbParserCallback
	{
	protected:
		WkbParserCallback() {}
	public:
		virtual ~WkbParserCallback() {};

	public:
		virtual bool setWkbType(WKBGeometryType wkbGeometryType) = 0;
		virtual bool addPoint(WkbGeometry::PNT point) = 0;
		virtual bool setNumPoints(MI_UINT32 n) = 0;
		virtual bool setNumLineStrings(MI_UINT32 n) = 0;
		virtual bool setNumRings(MI_UINT32 n) = 0;
		virtual bool setNumPolygons(MI_UINT32 n) = 0;
		virtual bool setNumGeometries(MI_UINT32 n) = 0;
		virtual bool startMulti() = 0;
		virtual bool startPoints() = 0;
		virtual bool startLineString() = 0;
		virtual bool startRing() = 0;
		virtual bool startPolygon() = 0;
		virtual bool startGeometry() = 0;

		virtual bool endMulti() = 0;
		virtual bool endPoints() = 0;
		virtual bool endLineString() = 0;
		virtual bool endRing() = 0;
		virtual bool endPolygon() = 0;
		virtual bool endGeometry() = 0;
		virtual bool finish(bool bOK) = 0; // Supplied bOK indicates if the parsing was successful. Finish is called regardless of success or failure!
	};

public:
	// Accessors
	const wchar_t * GetCSys() const { return _csys.c_str(); }
	size_t GetNbrGPKGWKBBytes() const;
	const unsigned char * AsGPKGWKB() const;
	size_t GetNbrWKBBytes() const;
	const unsigned char * AsWKB() const;
	WKBGeometryType GetGeometryType() const;
	DIMENSIONALITY GetDimensionality() const;
	void GetEnvelope(double & minx, double & maxx, double & miny, double & maxy) const;
	void GetEnvelope(double & minx, double & maxx, double & miny, double & maxy, double & minz, double & maxz, double & minm, double & maxm) const;

	// Creating a WKB
	static WkbGeometry * CreateFromGPKGWKB(const wchar_t * csys, size_t nbytes, const unsigned char * blob);
	static WkbGeometry * CreateFromWKB(const wchar_t * csys, size_t nbytes, const unsigned char * blob);
	static WkbGeometry * CreatePointGeometry(const wchar_t * csys, DIMENSIONALITY dimensionality, double longitude, double latitude, double z, double m);
	static WkbGeometry * CreateMultiPointGeometry(const wchar_t * csys, DIMENSIONALITY dimensionality, std::vector<PNT> & points);
	static WkbGeometry * CreateMultiCurveGeometry(const wchar_t * csys, DIMENSIONALITY dimensionality, std::vector<std::vector<PNT>> & curves);
	static WkbGeometry * CreateMultiPolygonGeometry(const wchar_t * csys, DIMENSIONALITY dimensionality, std::vector<std::vector<std::vector<WkbGeometry::PNT>>> polygons);
	static WkbGeometry * CreateGeometryCollection(const wchar_t * csys, DIMENSIONALITY dimensionality, std::vector<WkbGeometry*> geometryCollection);
	static WkbGeometry * CreateTextGeometry(const wchar_t * csys, Ellis::DRECT mbr, short angle, Ellis::CalloutLineType calloutLineType,
		Ellis::DPNT calloutTarget, const wchar_t* string);
	// Reading a WKB
	static WKBGeometryType BaseWKBType(WKBGeometryType wkbGeometryType, DIMENSIONALITY & dimensionality);
	void GetPointCoordinates(double & x, double & y) const;
	bool Parse(WkbParserCallback * callback);

private:
	bool AdoptGPKGWKB(size_t nbytes, const unsigned char * blob);
	bool AdoptWKB(size_t nbytes, const unsigned char * blob);
	WKBGeometryType Blob2GeopackageDataType(const unsigned char * bytes, size_t nbytes);
	static void DetermineSize(DIMENSIONALITY dimensionality, WKBGeometryType & wkbGeometryType, size_t & nbytesPerPoint);
	static void memrev(char  *buf, size_t count);
	static double read_double(const unsigned char * bytes, int littleEndianOrder);
	static void write_double(unsigned char * bytes, int littleEndianOrder, double dval);
	static unsigned __int32 read_uint32(const unsigned char * bytes, int littleEndianOrder);
	static void write_uint32(unsigned char * bytes, int littleEndianOrder, unsigned __int32 ival);
	static void read_dpoint(const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality, double & x, double & y, double & z, double & m);
	static void read_dpoint(const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality, PNT & pnt);

	static bool ParseInternal(WkbParserCallback * callback, const unsigned char * & wkb); // This method only expects Simple Feature WKB, not with the GPKG header
	static bool ParsePoint(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality);
	static bool ParseMultiPoint(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality);
	static bool ParseLineString(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality);
	static bool ParseMultiLinestring(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality);
	static bool ParsePolygon(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality);
	static bool ParseMultiPolygon(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality);
	static bool ParseGeometryCollection(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality);

private:
	std::wstring _csys;
	WKBGeometryType _wkbGeometryType;
	int _littleEndianOrder;
	DIMENSIONALITY _dimensionality;
	double _minx, _maxx, _miny, _maxy, _minz, _maxz, _minm, _maxm;
	size_t _nbytes;
	const unsigned char * _gpkgwkb;
	const unsigned char * _wkb;
};

#include <stdafx.h>
#include <WkbGeometry.h>

#pragma warning(push)
#pragma warning(disable: 4200)
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
#pragma warning(pop)

/* ======================================================================= */
/* Accessors                                                               */
/* ======================================================================= */
size_t WkbGeometry::GetNbrGPKGWKBBytes() const
{
	return _nbytes;
}
const unsigned char * WkbGeometry::AsGPKGWKB() const
{
	return _gpkgwkb;
}
size_t WkbGeometry::GetNbrWKBBytes() const
{
	return _nbytes - (_wkb - _gpkgwkb);
}
const unsigned char * WkbGeometry::AsWKB() const
{
	return _wkb;
}

WKBGeometryType WkbGeometry::GetGeometryType() const
{
	return _wkbGeometryType;
}
WkbGeometry::DIMENSIONALITY WkbGeometry::GetDimensionality() const
{
	return _dimensionality;
}
void WkbGeometry::GetEnvelope(double & minx, double & maxx, double & miny, double & maxy) const
{
	minx = _minx;
	maxx = _maxx;
	miny = _miny;
	maxy = _maxy;
}
void WkbGeometry::GetEnvelope(double & minx, double & maxx, double & miny, double & maxy, double & minz, double & maxz, double & minm, double & maxm) const
{
	minx = _minx;
	maxx = _maxx;
	miny = _miny;
	maxy = _maxy;
	minz = _minz;
	maxz = _maxz;
	minm = _minm;
	maxm = _maxm;
}

/* ======================================================================= */
/* Blob Memory Functions                                                   */
/* ======================================================================= */
/*
**  reverse "count" bytes starting at "buf"
*/
void WkbGeometry::memrev(char  *buf, size_t count)
{
	char *r;

	for (r = buf + count - 1; buf < r; buf++, r--)
	{
		*buf ^= *r;
		*r ^= *buf;
		*buf ^= *r;
	}
}
double WkbGeometry::read_double(const unsigned char * bytes, int littleEndianOrder)
{
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
void WkbGeometry::write_double(unsigned char * bytes, int littleEndianOrder, double dval) 
{
	union {
		double v;
		char c[sizeof(double)];
	} d;
	if (dval == -0.000000000) d.v = 0.0;
	else d.v = dval;
	if (!littleEndianOrder) {
		memrev((char*)d.c, sizeof(double));
	}
	memcpy(bytes, d.c, sizeof(double));
}

unsigned __int32 WkbGeometry::read_uint32(const unsigned char * bytes, int littleEndianOrder)
{
	// Here I am assuming I as the program am running on Windows, thus littleEndianOrder.
	// If the data is not in littleEndianOrder, it is in BIG Endian order, thus we need to swap
	// the bytes first. Unsigned int32s are simply reversed.
	union {
		__int32 v;
		char c[sizeof(__int32)];
	} d;
	d.v = *((__int32*)bytes);
	if (!littleEndianOrder) {
		memrev((char*)d.c, sizeof(__int32));
	}
	return d.v;
}
void WkbGeometry::write_uint32(unsigned char * bytes, int littleEndianOrder, unsigned __int32 ival) 
{
	union {
		__int32 v;
		char c[sizeof(double)];
	} d;
	d.v = ival;
	if (!littleEndianOrder) {
		memrev((char*)d.c, sizeof(unsigned __int32));
	}
	memcpy(bytes, d.c, sizeof(unsigned __int32));
}
void WkbGeometry::read_dpoint(const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality, double & x, double & y, double & z, double & m)
{
	x = read_double(wkb, littleEndianOrder);
	wkb += sizeof(double);
	y = read_double(wkb, littleEndianOrder);
	wkb += sizeof(double);
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM))
	{
		z = read_double(wkb, littleEndianOrder);
		wkb += sizeof(double);
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM))
	{
		m = read_double(wkb, littleEndianOrder);
		wkb += sizeof(double);
	}
}
void WkbGeometry::read_dpoint(const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality, PNT & pnt)
{
	read_dpoint(wkb, littleEndianOrder, dimensionality, pnt.x, pnt.y, pnt.z, pnt.m);
}
/* ======================================================================= */
/* Calculate Envelope                                                      */
/* ======================================================================= */
class EnvelopeReader : public WkbGeometry::WkbParserCallback
{
public:
	EnvelopeReader() :
		_first(true),
		_minx(0.0),
		_maxx(0.0),
		_miny(0.0),
		_maxy(0.0),
		_minz(0.0),
		_maxz(0.0),
		_minm(0.0),
		_maxm(0.0),
		_dimensionality(WkbGeometry::DIMENSIONALITY::P)
	{

	}
	~EnvelopeReader() {}
public:
	bool setWkbType(WKBGeometryType wkbGeometryType)
	{
		WKBGeometryType baseGeometryType = WkbGeometry::BaseWKBType(wkbGeometryType, _dimensionality);
		return true;
	}
	bool addPoint(WkbGeometry::PNT point)
	{
		if (_first)
		{
			_minx = point.x;
			_maxx = point.x;
			_miny = point.y;
			_maxy = point.y;
			if ((_dimensionality == WkbGeometry::DIMENSIONALITY::Z) || (_dimensionality == WkbGeometry::DIMENSIONALITY::ZM))
			{
				_minz = point.z;
				_maxz = point.z;
			}
			if ((_dimensionality == WkbGeometry::DIMENSIONALITY::M) || (_dimensionality == WkbGeometry::DIMENSIONALITY::ZM))
			{
				_minm = point.m;
				_maxm = point.m;
			}
			_first = false;
		}
		else
		{
			if (point.x < _minx) _minx = point.x;
			if (point.x > _maxx) _maxx = point.x;
			if (point.y < _miny) _miny = point.y;
			if (point.y > _maxy) _maxy = point.y;
			if ((_dimensionality == WkbGeometry::DIMENSIONALITY::Z) || (_dimensionality == WkbGeometry::DIMENSIONALITY::ZM))
			{
				if (point.z < _minz) _minz = point.z;
				if (point.z > _maxz) _maxz = point.z;
			}
			if ((_dimensionality == WkbGeometry::DIMENSIONALITY::M) || (_dimensionality == WkbGeometry::DIMENSIONALITY::ZM))
			{
				if (point.m < _minm) _minm = point.m;
				if (point.m > _maxm) _maxm = point.m;
			}
		}
		return true;
	}
	bool setNumPoints(MI_UINT32 n) { return true; }
	bool setNumLineStrings(MI_UINT32 n) { return true; }
	bool setNumRings(MI_UINT32 n) { return true; }
	bool setNumPolygons(MI_UINT32 n) { return true; }
	bool setNumGeometries(MI_UINT32 n) { return true; }
	bool startMulti() { return true; }
	bool startPoints() { return true; }
	bool startLineString() { return true; }
	bool startRing() { return true; }
	bool startPolygon() { return true; }
	bool startGeometry() { return true; }
	bool endMulti() { return true; }
	bool endPoints() { return true; }
	bool endLineString() { return true; }
	bool endRing() { return true; }
	bool endPolygon() { return true; }
	bool endGeometry() { return true; }
	bool finish(bool bOK) { return true; }
public:
	double MinX() const { return _minx; }
	double MaxX() const { return _maxx; }
	double MinY() const { return _miny; }
	double MaxY() const { return _maxy; }
	double MinZ() const { return _minz; }
	double MaxZ() const { return _maxz; }
	double MinM() const { return _minm; }
	double MaxM() const { return _maxm; }
	WkbGeometry::DIMENSIONALITY Dimensionality() const { return _dimensionality; }
private:
	bool _first;
	double _minx, _maxx, _miny, _maxy, _minz, _maxz, _minm, _maxm;
	WkbGeometry::DIMENSIONALITY _dimensionality;
};

/* ======================================================================= */
/* WKB Utility Functions                                                   */
/* ======================================================================= */
WKBGeometryType WkbGeometry::BaseWKBType(WKBGeometryType wkbGeometryType, DIMENSIONALITY & dimensionality)
{
	int iGeometryType = (int)wkbGeometryType;
	if (iGeometryType >= 3000) {
		dimensionality = DIMENSIONALITY::ZM;
		return (WKBGeometryType)(iGeometryType - 3000);
	}
	else if (iGeometryType >= 2000) {
		dimensionality = DIMENSIONALITY::M;
		return (WKBGeometryType)(iGeometryType - 2000);
	}
	else if (iGeometryType >= 1000) {
		dimensionality = DIMENSIONALITY::Z;
		return (WKBGeometryType)(iGeometryType - 1000);
	}
	else {
		dimensionality = DIMENSIONALITY::P;
		return (WKBGeometryType)(iGeometryType);
	}
}
void WkbGeometry::DetermineSize(DIMENSIONALITY dimensionality, WKBGeometryType & wkbGeometryType, size_t & nbytesPerPoint)
{
	switch (dimensionality)
	{
	case DIMENSIONALITY::P: 
		wkbGeometryType = wkbGeometryType;  
		nbytesPerPoint = sizeof(double) + sizeof(double); 
		break;
	case DIMENSIONALITY::Z: 
		wkbGeometryType = (WKBGeometryType)(1000 + (int)wkbGeometryType); 
		nbytesPerPoint = sizeof(double) + sizeof(double) + sizeof(double); 
		break;
	case DIMENSIONALITY::M: 
		wkbGeometryType = (WKBGeometryType)(2000 + (int)wkbGeometryType); 
		nbytesPerPoint = sizeof(double) + sizeof(double) + sizeof(double); 
		break;
	case DIMENSIONALITY::ZM: 
		wkbGeometryType = (WKBGeometryType)(3000 + (int)wkbGeometryType); 
		nbytesPerPoint = sizeof(double) + sizeof(double) + sizeof(double) + sizeof(double); 
		break;
	}
}

/* ======================================================================= */
/* Creation Functions                                                      */
/* ======================================================================= */
WkbGeometry * WkbGeometry::CreatePointGeometry(const wchar_t * csys, DIMENSIONALITY dimensionality, double longitude, double latitude, double z, double m)
{
	unsigned char * bytes;
	size_t nbytes;
	size_t nbytesPerPoint = 0;
	WKBGeometryType wkbGeometryType = WKBGeometryType::wkbPoint;

	DetermineSize(dimensionality, wkbGeometryType, nbytesPerPoint);

	unsigned char * pWhereCurrentlyAt = 0;

	nbytes = sizeof(GeoPackageBinaryHeader)
		+ 0                        /* No envelope for point */
		+ 1                        /* byteOrder */
		+ sizeof(unsigned int)     /* wkbType */
		+ nbytesPerPoint;          /* Point */

	bytes = new unsigned char[nbytes];
	pWhereCurrentlyAt = bytes;

	int littleEndianOrder = 1;
	GeoPackageBinaryHeader * pHeader = (GeoPackageBinaryHeader *)pWhereCurrentlyAt;
	pHeader->magic[0] = 'G';
	pHeader->magic[1] = 'P';
	pHeader->version = 0;
	int r, x, y, e, b;
	r = 0; // reserved for future use; set to 0
	x = 0; // 0: StandardGeoPackageBinary, 1: ExtendedGeoPackageBinary
	y = 0; // empty geometry flag. 0: non-empty geometry, 1: empty geometry
	e = 0; // No envelope.
	b = 1; // LittleEndianOrder.
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);

	// _wkt points to the WKB portion of the blob which is right after the GPKG header
	unsigned char * wkb = pWhereCurrentlyAt;

	pWhereCurrentlyAt[0] = (char)littleEndianOrder;
	pWhereCurrentlyAt++;
	write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);
	write_double(pWhereCurrentlyAt, littleEndianOrder, longitude); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, latitude); pWhereCurrentlyAt += sizeof(double);
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, z); pWhereCurrentlyAt += sizeof(double);
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, m); pWhereCurrentlyAt += sizeof(double);
	}

	WkbGeometry * wkbGeometry = new WkbGeometry(csys);
	wkbGeometry->_gpkgwkb = bytes;
	wkbGeometry->_nbytes = nbytes;
	wkbGeometry->_wkbGeometryType = wkbGeometryType;
	wkbGeometry->_wkb = wkb;
	wkbGeometry->_minx = wkbGeometry->_maxx = longitude;
	wkbGeometry->_miny = wkbGeometry->_maxy = latitude;
	wkbGeometry->_minz = z;
	wkbGeometry->_maxz = z;
	wkbGeometry->_minm = m;
	wkbGeometry->_maxm = m;
	wkbGeometry->_dimensionality = dimensionality;
	return wkbGeometry;
}
WkbGeometry * WkbGeometry::CreateMultiPointGeometry(const wchar_t * csys, DIMENSIONALITY dimensionality, std::vector<PNT> & points)
{
	unsigned char * bytes;
	size_t nbytes;
	size_t nbytesPerPoint = 0;
	WKBGeometryType wkbGeometryType = WKBGeometryType::wkbMultiPoint;

	DetermineSize(dimensionality, wkbGeometryType, nbytesPerPoint);

	unsigned char * pWhereCurrentlyAt = 0;
	int envelopeCoords = 4;
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		envelopeCoords += 2;
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		envelopeCoords += 2;
	}

	nbytes = sizeof(GeoPackageBinaryHeader)
		+ (envelopeCoords * sizeof(double))          /* Envelope [minx, maxx, miny, maxy] */
		+ 1                                          /* byteOrder */
		+ sizeof(unsigned int)                       /* wkbType */
		+ sizeof(unsigned int)                       /* numPoints */
		+ (unsigned long)points.size() * (
			1                                         /* byteOrder */
			+ sizeof(unsigned int)                    /* wkbType */
			+ nbytesPerPoint                          /* x, y, z, and/or m */
			);

	bytes = new unsigned char[nbytes];
	pWhereCurrentlyAt = bytes;

	int littleEndianOrder = 1;
	GeoPackageBinaryHeader * pHeader = (GeoPackageBinaryHeader *)pWhereCurrentlyAt;
	pHeader->magic[0] = 'G';
	pHeader->magic[1] = 'P';
	pHeader->version = 0;
	int r, x, y, e, b;
	r = 0; // reserved for future use; set to 0
	x = 0; // 0: StandardGeoPackageBinary, 1: ExtendedGeoPackageBinary
	y = 0; // empty geometry flag. 0: non-empty geometry, 1: empty geometry
	e = 1; // envelope is [minx, maxx, miny, maxy], 32 bytes
	b = 1; // LittleEndianOrder.
	if (dimensionality == DIMENSIONALITY::Z) e = 2;
	else if (dimensionality == DIMENSIONALITY::M) e = 3;
	else if (dimensionality == DIMENSIONALITY::ZM) e = 4;
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	double minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0, minm = 0, maxm = 0;
	minx = maxx = points[0].x;
	miny = maxy = points[0].y;
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		minz = maxz = points[0].z;
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		minm = maxm = points[0].m;
	}
	for (size_t i = 0; i < points.size(); i++)
	{
		double x = points[i].x, y = points[i].y;
		if (x < minx) minx = x;
		if (x > maxx) maxx = x;
		if (y < miny) miny = y;
		if (y > maxy) maxy = y;
		if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
			if (points[i].z < minz) minz = points[i].z;
			if (points[i].z > maxz) maxz = points[i].z;
		}
		if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
			if (points[i].m < minm) minm = points[i].m;
			if (points[i].m > maxm) maxm = points[i].m;
		}
	}
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, minz); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, maxz); pWhereCurrentlyAt += sizeof(double);
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, minm); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, maxm); pWhereCurrentlyAt += sizeof(double);
	}

	// _wkt points to the WKB portion of the blob which is right after the GPKG header
	unsigned char * wkb = pWhereCurrentlyAt;

	/* Now write the geometry */
	pWhereCurrentlyAt[0] = (char)littleEndianOrder;
	pWhereCurrentlyAt++;

	write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);
	write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)points.size()); pWhereCurrentlyAt += sizeof(unsigned __int32);
	for (size_t i = 0; i < points.size(); i++)
	{
		pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbPoint); pWhereCurrentlyAt += sizeof(unsigned __int32);
		write_double(pWhereCurrentlyAt, littleEndianOrder, points[i].x); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, points[i].y); pWhereCurrentlyAt += sizeof(double);
		if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
			write_double(pWhereCurrentlyAt, littleEndianOrder, points[i].z); pWhereCurrentlyAt += sizeof(double);
		}
		if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
			write_double(pWhereCurrentlyAt, littleEndianOrder, points[i].m); pWhereCurrentlyAt += sizeof(double);
		}
	}

	WkbGeometry * wkbGeometry = new WkbGeometry(csys);
	wkbGeometry->_gpkgwkb = bytes;
	wkbGeometry->_nbytes = nbytes;
	wkbGeometry->_wkbGeometryType = wkbGeometryType;
	wkbGeometry->_wkb = wkb;
	wkbGeometry->_minx = minx;
	wkbGeometry->_maxx = maxx;
	wkbGeometry->_miny = miny;
	wkbGeometry->_maxy = maxy;
	wkbGeometry->_minz = minz;
	wkbGeometry->_maxz = maxz;
	wkbGeometry->_minm = minm;
	wkbGeometry->_maxm = maxm;
	wkbGeometry->_dimensionality = dimensionality;
	return wkbGeometry;
}

WkbGeometry * WkbGeometry::CreateMultiCurveGeometry(const wchar_t * csys, DIMENSIONALITY dimensionality, std::vector<std::vector<PNT>> & curves)
{
	unsigned char * bytes;
	size_t nbytes;
	size_t nbytesPerPoint = 0;
	size_t numCurves = curves.size();
	WKBGeometryType wkbGeometryType = (numCurves == 1) ? WKBGeometryType::wkbLineString : WKBGeometryType::wkbMultiLineString; 

	DetermineSize(dimensionality, wkbGeometryType, nbytesPerPoint);
	int envelopeCoords = 4;
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		envelopeCoords += 2;
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		envelopeCoords += 2;
	}

	unsigned char * pWhereCurrentlyAt = 0;

	nbytes = sizeof(GeoPackageBinaryHeader)
		+ (envelopeCoords * sizeof(double))          /* Envelope [minx, maxx, miny, maxy] */
		+ 1                                          /* byteOrder */
		+ sizeof(unsigned int);                      /* wkbType */

	if (numCurves > 1)
		nbytes += sizeof(unsigned int);              /* numLineStrings */

	for (size_t i = 0; i < numCurves; i++)
	{
		if (numCurves > 1)
			nbytes +=
			(
				1                                      /* byteOrder */
				+ sizeof(unsigned int)                 /* wkbType */
				);

		nbytes +=
			sizeof(unsigned int) +                    /* numPoints */
			curves[i].size() *                        /* Number of nodes for i-th curve times  */
			nbytesPerPoint;                           /*      size of Point     */
	}

	bytes = new unsigned char[nbytes];
	pWhereCurrentlyAt = bytes;

	int littleEndianOrder = 1;
	GeoPackageBinaryHeader * pHeader = (GeoPackageBinaryHeader *)pWhereCurrentlyAt;
	pHeader->magic[0] = 'G';
	pHeader->magic[1] = 'P';
	pHeader->version = 0;
	int r, x, y, e, b;
	r = 0; // reserved for future use; set to 0
	x = 0; // 0: StandardGeoPackageBinary, 1: ExtendedGeoPackageBinary
	y = 0; // empty geometry flag. 0: non-empty geometry, 1: empty geometry
	e = 1; // envelope is [minx, maxx, miny, maxy], 32 bytes
	b = 1; // LittleEndianOrder.
	if (dimensionality == DIMENSIONALITY::Z) e = 2;
	else if (dimensionality == DIMENSIONALITY::M) e = 3;
	else if (dimensionality == DIMENSIONALITY::ZM) e = 4;
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	double minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0, minm = 0, maxm = 0;
	minx = maxx = curves[0][0].x;
	miny = maxy = curves[0][0].y;
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		minz = maxz = curves[0][0].z;
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		minm = maxm = curves[0][0].m;
	}

	for (size_t j = 0; j < numCurves; j++)
	{
		for (size_t i = 0; i < curves[j].size(); i++)
		{
			double x = curves[j][i].x, y = curves[j][i].y;
			if (x < minx) minx = x;
			if (x > maxx) maxx = x;
			if (y < miny) miny = y;
			if (y > maxy) maxy = y;
			if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
				if (curves[j][i].z < minz) minz = curves[j][i].z;
				if (curves[j][i].z > maxz) maxz = curves[j][i].z;
			}
			if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
				if (curves[j][i].m < minm) minm = curves[j][i].m;
				if (curves[j][i].m > maxm) maxm = curves[j][i].m;
			}
		}
	}
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, minz); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, maxz); pWhereCurrentlyAt += sizeof(double);
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, minm); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, maxm); pWhereCurrentlyAt += sizeof(double);
	}

	// _wkt points to the WKB portion of the blob which is right after the GPKG header
	unsigned char * wkb = pWhereCurrentlyAt;

	/* Now write the geometry */
	pWhereCurrentlyAt[0] = (char)littleEndianOrder;
	pWhereCurrentlyAt++;

	if (numCurves == 1)
	{
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);
	}
	else
	{
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numCurves); pWhereCurrentlyAt += sizeof(unsigned __int32);
	}
	for (size_t j = 0; j < numCurves; j++)
	{
		if (numCurves > 1)
		{
			pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
			write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);
		}
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)curves[j].size()); pWhereCurrentlyAt += sizeof(unsigned __int32);
		for (size_t i = 0; i < curves[j].size(); i++)
		{
			write_double(pWhereCurrentlyAt, littleEndianOrder, curves[j][i].x); pWhereCurrentlyAt += sizeof(double);
			write_double(pWhereCurrentlyAt, littleEndianOrder, curves[j][i].y); pWhereCurrentlyAt += sizeof(double);
			if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
				write_double(pWhereCurrentlyAt, littleEndianOrder, curves[j][i].z); pWhereCurrentlyAt += sizeof(double);
			}
			if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
				write_double(pWhereCurrentlyAt, littleEndianOrder, curves[j][i].m); pWhereCurrentlyAt += sizeof(double);
			}
		}
	}

	WkbGeometry * wkbGeometry = new WkbGeometry(csys);
	wkbGeometry->_gpkgwkb = bytes;
	wkbGeometry->_nbytes = nbytes;
	wkbGeometry->_wkbGeometryType = wkbGeometryType;
	wkbGeometry->_wkb = wkb;
	wkbGeometry->_minx = minx;
	wkbGeometry->_maxx = maxx;
	wkbGeometry->_miny = miny;
	wkbGeometry->_maxy = maxy;
	wkbGeometry->_minz = minz;
	wkbGeometry->_maxz = maxz;
	wkbGeometry->_minm = minm;
	wkbGeometry->_maxm = maxm;
	wkbGeometry->_dimensionality = dimensionality;
	return wkbGeometry;
}
WkbGeometry * WkbGeometry::CreateMultiPolygonGeometry(const wchar_t * csys, DIMENSIONALITY dimensionality, std::vector<std::vector<std::vector<WkbGeometry::PNT>>> polygons)
{
	unsigned char * bytes;
	size_t nbytes;

	size_t nbytesPerPoint = 0;
	size_t numPolygons = polygons.size();
	WKBGeometryType wkbGeometryType = (numPolygons == 1) ? WKBGeometryType::wkbPolygon : WKBGeometryType::wkbMultiPolygon;

	DetermineSize(dimensionality, wkbGeometryType, nbytesPerPoint);
	int envelopeCoords = 4;
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		envelopeCoords += 2;
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		envelopeCoords += 2;
	}

	unsigned char * pWhereCurrentlyAt = 0;

	double minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0, minm = 0, maxm = 0;
	minx = maxx = polygons[0][0][0].x;
	miny = maxy = polygons[0][0][0].y;
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		minz = maxz = polygons[0][0][0].z;
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		minm = maxm = polygons[0][0][0].m;
	}

	if (numPolygons == 1)
	{
		// Simple (single) polygon
		nbytes = sizeof(GeoPackageBinaryHeader)
			+ (envelopeCoords * sizeof(double))          /* Envelope [minx, maxx, miny, maxy] */
			+ 1                                          /* byteOrder */
			+ sizeof(unsigned int)                       /* wkbType */
			+ sizeof(unsigned int);                      /* numRings */

		for (size_t iRing = 0; iRing < polygons[0].size(); iRing++)
		{
			nbytes += sizeof(unsigned int)               /* numPoints */
				+ (polygons[0][iRing].size() * nbytesPerPoint);/* points */
			for (size_t iPoint = 0; iPoint < polygons[0][iRing].size(); iPoint++)
			{
				double x = polygons[0][iRing][iPoint].x, y = polygons[0][iRing][iPoint].y;
				if (x < minx) minx = x;
				if (x > maxx) maxx = x;
				if (y < miny) miny = y;
				if (y > maxy) maxy = y;
				if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
					if (polygons[0][iRing][iPoint].z < minz) minz = polygons[0][iRing][iPoint].z;
					if (polygons[0][iRing][iPoint].z > maxz) maxz = polygons[0][iRing][iPoint].z;
				}
				if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
					if (polygons[0][iRing][iPoint].m < minm) minm = polygons[0][iRing][iPoint].m;
					if (polygons[0][iRing][iPoint].m > maxm) maxm = polygons[0][iRing][iPoint].m;
				}
			}
		}
	}
	else
	{
		// MultiPolygon
		nbytes = sizeof(GeoPackageBinaryHeader)
			+ (envelopeCoords * sizeof(double))          /* Envelope [minx, maxx, miny, maxy] */
			+ 1                                          /* byteOrder */
			+ sizeof(unsigned int)                       /* wkbType */
			+ sizeof(unsigned int);                      /* numPolygons */
		for (size_t iPolygon = 0; iPolygon < numPolygons; iPolygon++)
		{
			nbytes +=
				1                                          /* byteOrder */
				+ sizeof(unsigned int)                       /* wkbType */
				+ sizeof(unsigned int);                      /* numRings */

			for (size_t iRing = 0; iRing < polygons[iPolygon].size(); iRing++)
			{
				nbytes += sizeof(unsigned int)               /* numPoints */
					+ (polygons[iPolygon][iRing].size() * nbytesPerPoint);/* points */
				for (size_t iPoint = 0; iPoint < polygons[iPolygon][iRing].size(); iPoint++)
				{
					double x = polygons[iPolygon][iRing][iPoint].x, y = polygons[iPolygon][iRing][iPoint].y;
					if (x < minx) minx = x;
					if (x > maxx) maxx = x;
					if (y < miny) miny = y;
					if (y > maxy) maxy = y;
					if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
						if (polygons[iPolygon][iRing][iPoint].z < minz) minz = polygons[iPolygon][iRing][iPoint].z;
						if (polygons[iPolygon][iRing][iPoint].z > maxz) maxz = polygons[iPolygon][iRing][iPoint].z;
					}
					if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
						if (polygons[iPolygon][iRing][iPoint].m < minm) minm = polygons[iPolygon][iRing][iPoint].m;
						if (polygons[iPolygon][iRing][iPoint].m > maxm) maxm = polygons[iPolygon][iRing][iPoint].m;
					}
				}
			}
		}
	}

	bytes = new unsigned char[nbytes];
	pWhereCurrentlyAt = bytes;

	int littleEndianOrder = 1;
	GeoPackageBinaryHeader * pHeader = (GeoPackageBinaryHeader *)pWhereCurrentlyAt;
	pHeader->magic[0] = 'G';
	pHeader->magic[1] = 'P';
	pHeader->version = 0;
	int r, x, y, e, b;
	r = 0; // reserved for future use; set to 0
	x = 0; // 0: StandardGeoPackageBinary, 1: ExtendedGeoPackageBinary
	y = 0; // empty geometry flag. 0: non-empty geometry, 1: empty geometry
	e = 1; // envelope is [minx, maxx, miny, maxy], 32 bytes
	b = 1; // LittleEndianOrder.
	if (dimensionality == DIMENSIONALITY::Z) e = 2;
	else if (dimensionality == DIMENSIONALITY::M) e = 3;
	else if (dimensionality == DIMENSIONALITY::ZM) e = 4;
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, minz); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, maxz); pWhereCurrentlyAt += sizeof(double);
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, minm); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, maxm); pWhereCurrentlyAt += sizeof(double);
	}

	// _wkt points to the WKB portion of the blob which is right after the GPKG header
	unsigned char * wkb = pWhereCurrentlyAt;

	/* Now write the geometry */
	pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
	if (numPolygons == 1)
	{
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);

		write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)polygons[0].size()); pWhereCurrentlyAt += sizeof(unsigned __int32);
		for (size_t iRing = 0; iRing < polygons[0].size(); iRing++)
		{
			write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)polygons[0][iRing].size()); pWhereCurrentlyAt += sizeof(unsigned __int32);
			for (size_t iPoint = 0; iPoint < polygons[0][iRing].size(); iPoint++)
			{
				write_double(pWhereCurrentlyAt, littleEndianOrder, polygons[0][iRing][iPoint].x); pWhereCurrentlyAt += sizeof(double);
				write_double(pWhereCurrentlyAt, littleEndianOrder, polygons[0][iRing][iPoint].y); pWhereCurrentlyAt += sizeof(double);
				if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
					write_double(pWhereCurrentlyAt, littleEndianOrder, polygons[0][iRing][iPoint].z); pWhereCurrentlyAt += sizeof(double);
				}
				if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
					write_double(pWhereCurrentlyAt, littleEndianOrder, polygons[0][iRing][iPoint].m); pWhereCurrentlyAt += sizeof(double);
				}
			}
		}
	}
	else
	{
		// MultiPolygon
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numPolygons); pWhereCurrentlyAt += sizeof(unsigned __int32);

		for (size_t iPolygon = 0; iPolygon < numPolygons; iPolygon++)
		{
			pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
			write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);
			write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)polygons[iPolygon].size()); pWhereCurrentlyAt += sizeof(unsigned __int32);

			for (size_t iRing = 0; iRing < polygons[iPolygon].size(); iRing++)
			{
				write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)polygons[iPolygon][iRing].size()); pWhereCurrentlyAt += sizeof(unsigned __int32);
				for (size_t iPoint = 0; iPoint < polygons[iPolygon][iRing].size(); iPoint++)
				{
					double x = polygons[iPolygon][iRing][iPoint].x, y = polygons[iPolygon][iRing][iPoint].y;
					write_double(pWhereCurrentlyAt, littleEndianOrder, polygons[iPolygon][iRing][iPoint].x); pWhereCurrentlyAt += sizeof(double);
					write_double(pWhereCurrentlyAt, littleEndianOrder, polygons[iPolygon][iRing][iPoint].y); pWhereCurrentlyAt += sizeof(double);
					if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
						write_double(pWhereCurrentlyAt, littleEndianOrder, polygons[iPolygon][iRing][iPoint].z); pWhereCurrentlyAt += sizeof(double);
					}
					if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
						write_double(pWhereCurrentlyAt, littleEndianOrder, polygons[iPolygon][iRing][iPoint].m); pWhereCurrentlyAt += sizeof(double);
					}
				}
			}
		}
	}

	WkbGeometry * wkbGeometry = new WkbGeometry(csys);
	wkbGeometry->_gpkgwkb = bytes;
	wkbGeometry->_nbytes = nbytes;
	wkbGeometry->_wkbGeometryType = wkbGeometryType;
	wkbGeometry->_wkb = wkb;
	wkbGeometry->_minx = minx;
	wkbGeometry->_maxx = maxx;
	wkbGeometry->_miny = miny;
	wkbGeometry->_maxy = maxy;
	wkbGeometry->_minz = minz;
	wkbGeometry->_maxz = maxz;
	wkbGeometry->_minm = minm;
	wkbGeometry->_maxm = maxm;
	wkbGeometry->_dimensionality = dimensionality;
	return wkbGeometry;
}

WkbGeometry * WkbGeometry::CreateTextGeometry(const wchar_t * csys, Ellis::DRECT mbr, short angle, Ellis::CalloutLineType calloutLineType,
	Ellis::DPNT calloutTarget, const wchar_t* string)
{
	/*
	WKBText {
		byte byteOrder;
		uint32 wkbType = 206;	// New value (chosen from MFAL.h)
		double textBounds[4];	// The rectangular bounds of the rotated text.
								// The un-rotated text fills this rectangular space.
								// Note: although the Geopackage WKB has a header that also
								// includes the bounds, which *could* be used and not
								// duplicated here, this struct will include the bounds.
								// This allows for WKB to possibly someday be used without
								// the Geopackage header and remain well defined and
								// also there is a possibility in the Geopackage header for
								// the MBR to be missing (E flag set to zero for
								// space saving option, slower indexing)
		double angle;  // The angle of rotation of the text caption.
		byte justify; // Text justification
						// 0 = Left
						// 1 = Center
						// 2 = Right
						// 3 = Full
		byte spacing; // Character spacing
						// 0 = 1
						// 1 = 1.5
						// 2 = 2
		byte calloutLineType; // The type of the callout line to use.
								// 0 = No callout line
								// 1 = Simple, Indicates that the text uses a simple callout
								//             line (no pointer to its reference geometry).
								// 2 = Arrow, Indicates that the text uses an arrow that
								//            points to its reference geometry.
		double target[2]; // Point representing the location of the end of the callout line.
							// [x, y], 16 bytes - Only present if callOutLineType is 1 or 2
		uint32 numBytes;  // Number of bytes in the Caption.
		char[] Caption;   // A string representing the text as UTF-8 characters.
	}
*/

	unsigned char * bytes;

	WKBGeometryType wkbGeometryType = WKBGeometryType::ewkbLegacyText;
	int envelopeCoords = 4;
	
	size_t nbytes = sizeof(GeoPackageBinaryHeader)
		+ (4 * sizeof(double))                       /* Envelope [minx, maxx, miny, maxy] */
		+ 1                                          /* byteOrder */
		+ sizeof(unsigned int)                       /* wkbType */
		+ (sizeof(double) * 4)                       /* Bounds */
		+ sizeof(double)                             /* angle */
		+ sizeof(UCHAR)                              /* justify */
		+ sizeof(UCHAR)                              /* spacing */
		+ sizeof(UCHAR)                              /* calloutLineType */
		+ (calloutLineType != Ellis::CalloutLineType::eNone ? 2 : 0) /* target*/
		+ sizeof(MI_UINT32);                         /* numBytes */

	std::string converted;
	MI_UINT32 numBytes = 0;
	if (string != nullptr) {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
		converted = utf8_conv.to_bytes(string);
		numBytes = (MI_UINT32)converted.length();
	}
	nbytes += numBytes;

	bytes = new unsigned char[nbytes];
	unsigned char * pWhereCurrentlyAt = bytes;

	int littleEndianOrder = 1;
	GeoPackageBinaryHeader * pHeader = (GeoPackageBinaryHeader *)pWhereCurrentlyAt;
	pHeader->magic[0] = 'G';
	pHeader->magic[1] = 'P';
	pHeader->version = 0;
	int r, x, y, e, b;
	r = 0; // reserved for future use; set to 0
	x = 1; // 0: StandardGeoPackageBinary, 1: ExtendedGeoPackageBinary
	y = 0; // empty geometry flag. 0: non-empty geometry, 1: empty geometry
	e = 1; // envelope is [minx, maxx, miny, maxy], 32 bytes
	b = 1; // LittleEndianOrder.
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;
	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	double minx = mbr.x1, maxx = mbr.x2, miny = mbr.y1, maxy = mbr.y2;
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);

	// _wkt points to the WKB portion of the blob which is right after the GPKG header
	unsigned char * wkb = pWhereCurrentlyAt;

	pWhereCurrentlyAt[0] = (char)littleEndianOrder;
	pWhereCurrentlyAt++;
	write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::ewkbLegacyText); pWhereCurrentlyAt += sizeof(unsigned __int32);

	// write the un-rotated text MBR
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);

	write_double(pWhereCurrentlyAt, littleEndianOrder, (double)angle); pWhereCurrentlyAt += sizeof(double);
	pWhereCurrentlyAt[0] = (char)0; // justify
	pWhereCurrentlyAt++;
	pWhereCurrentlyAt[0] = (char)0; // spacing
	pWhereCurrentlyAt++;
	pWhereCurrentlyAt[0] = (char)calloutLineType; // calloutLineType
	pWhereCurrentlyAt++;

	if (calloutLineType != Ellis::CalloutLineType::eNone) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, calloutTarget.x); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, calloutTarget.y); pWhereCurrentlyAt += sizeof(double);
	}

	write_uint32(pWhereCurrentlyAt, littleEndianOrder, numBytes); pWhereCurrentlyAt += sizeof(MI_UINT32);
	if (numBytes > 0) {
		memcpy(pWhereCurrentlyAt, converted.c_str(), numBytes);
		pWhereCurrentlyAt += numBytes;
	}

	WkbGeometry * wkbGeometry = new WkbGeometry(csys);
	wkbGeometry->_gpkgwkb = bytes;
	wkbGeometry->_nbytes = nbytes;
	wkbGeometry->_wkbGeometryType = wkbGeometryType;
	wkbGeometry->_wkb = wkb;
	wkbGeometry->_minx = minx;
	wkbGeometry->_maxx = maxx;
	wkbGeometry->_miny = miny;
	wkbGeometry->_maxy = maxy;
	wkbGeometry->_minz = 0;
	wkbGeometry->_maxz = 0;
	wkbGeometry->_minm = 0;
	wkbGeometry->_maxm = 0;
	return wkbGeometry;
}

WkbGeometry * WkbGeometry::CreateGeometryCollection(const wchar_t * csys, DIMENSIONALITY dimensionality, std::vector<WkbGeometry*> geometryCollection)
{
	unsigned char * bytes;
	size_t nbytes;

	WKBGeometryType wkbGeometryType = WKBGeometryType::wkbGeometryCollection;

	size_t nbytesPerPointDummy = 0;
	DetermineSize(dimensionality, wkbGeometryType, nbytesPerPointDummy);
	int envelopeCoords = 4;
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		envelopeCoords += 2;
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		envelopeCoords += 2;
	}

	unsigned char * pWhereCurrentlyAt = 0;

	nbytes = sizeof(GeoPackageBinaryHeader)
		+ (envelopeCoords * sizeof(double))          /* Envelope [minx, maxx, miny, maxy] */
		+ 1                                          /* byteOrder */
		+ sizeof(unsigned int)                       /* wkbType */
		+ sizeof(unsigned int);                      /* numGeometries */

	double minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0, minm = 0, maxm = 0;
	minx = geometryCollection[0]->_minx;
	maxx = geometryCollection[0]->_maxx;
	miny = geometryCollection[0]->_miny;
	maxy = geometryCollection[0]->_maxy;
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		minz = geometryCollection[0]->_minz;
		maxz = geometryCollection[0]->_maxz;
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		minm = geometryCollection[0]->_minm;
		maxm = geometryCollection[0]->_maxm;
	}

	for (size_t i = 0, n = geometryCollection.size(); i < n; i++)
	{
		nbytes += geometryCollection[i]->GetNbrWKBBytes();
		if (geometryCollection[i]->_minx < minx) minx = geometryCollection[i]->_minx;
		if (geometryCollection[i]->_maxx > maxx) maxx = geometryCollection[i]->_maxx;
		if (geometryCollection[i]->_miny < miny) miny = geometryCollection[i]->_miny;
		if (geometryCollection[i]->_maxy > maxy) maxy = geometryCollection[i]->_maxy;
		if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
			if (geometryCollection[i]->_minz < minz) minz = geometryCollection[i]->_minz;
			if (geometryCollection[i]->_maxz > maxz) maxz = geometryCollection[i]->_maxz;
		}
		if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
			if (geometryCollection[i]->_minm < minm) minm = geometryCollection[i]->_minm;
			if (geometryCollection[i]->_maxm > maxm) maxm = geometryCollection[i]->_maxm;
		}
	}

	bytes = new unsigned char[nbytes];
	pWhereCurrentlyAt = bytes;

	int littleEndianOrder = 1;
	GeoPackageBinaryHeader * pHeader = (GeoPackageBinaryHeader *)pWhereCurrentlyAt;
	pHeader->magic[0] = 'G';
	pHeader->magic[1] = 'P';
	pHeader->version = 0;
	int r, x, y, e, b;
	r = 0; // reserved for future use; set to 0
	x = 0; // 0: StandardGeoPackageBinary, 1: ExtendedGeoPackageBinary
	y = 0; // empty geometry flag. 0: non-empty geometry, 1: empty geometry
	e = 1; // envelope is [minx, maxx, miny, maxy], 32 bytes
	b = 1; // LittleEndianOrder.
	if (dimensionality == DIMENSIONALITY::Z) e = 2;
	else if (dimensionality == DIMENSIONALITY::M) e = 3;
	else if (dimensionality == DIMENSIONALITY::ZM) e = 4;
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);
	if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, minz); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, maxz); pWhereCurrentlyAt += sizeof(double);
	}
	if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
		write_double(pWhereCurrentlyAt, littleEndianOrder, minm); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, maxm); pWhereCurrentlyAt += sizeof(double);
	}

	// _wkt points to the WKB portion of the blob which is right after the GPKG header
	unsigned char * wkb = pWhereCurrentlyAt;

	/* Now write the geometry */
	pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
	write_uint32(pWhereCurrentlyAt, littleEndianOrder, wkbGeometryType); pWhereCurrentlyAt += sizeof(unsigned __int32);
	write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)geometryCollection.size()); pWhereCurrentlyAt += sizeof(unsigned __int32);

	for (size_t i = 0, n = geometryCollection.size(); i < n; i++)
	{
		memcpy(pWhereCurrentlyAt, geometryCollection[i]->AsWKB(), geometryCollection[i]->GetNbrWKBBytes());
		pWhereCurrentlyAt += geometryCollection[i]->GetNbrWKBBytes();
	}

	WkbGeometry * wkbGeometry = new WkbGeometry(csys);
	wkbGeometry->_gpkgwkb = bytes;
	wkbGeometry->_nbytes = nbytes;
	wkbGeometry->_wkbGeometryType = wkbGeometryType;
	wkbGeometry->_wkb = wkb;
	wkbGeometry->_minx = minx;
	wkbGeometry->_maxx = maxx;
	wkbGeometry->_miny = miny;
	wkbGeometry->_maxy = maxy;
	wkbGeometry->_minz = minz;
	wkbGeometry->_maxz = maxz;
	wkbGeometry->_minm = minm;
	wkbGeometry->_maxm = maxm;
	wkbGeometry->_dimensionality = dimensionality;
	return wkbGeometry;
}

WkbGeometry::WkbGeometry(const wchar_t * csys) :
	_csys(csys),
	_wkbGeometryType((WKBGeometryType)-1),
	_littleEndianOrder(1),
	_dimensionality(DIMENSIONALITY::P),
	_minx(0.0),
	_maxx(0.0), 
	_miny(0.0), 
	_maxy(0.0),
	_minz(0.0),
	_maxz(0.0), 
	_minm(0.0), 
	_maxm(0.0),
	_nbytes(0),
	_gpkgwkb(nullptr),
	_wkb(nullptr)
{
}

WkbGeometry::~WkbGeometry()
{
	if (_gpkgwkb != nullptr) delete[] _gpkgwkb;
	_gpkgwkb = nullptr;
}

WKBGeometryType WkbGeometry::Blob2GeopackageDataType(const unsigned char * bytes, size_t nbytes)
{
	GeoPackageBinaryHeaderReader * headerReader = (GeoPackageBinaryHeaderReader *)bytes;
	if (nbytes < sizeof(GeoPackageBinaryHeader)) {
		return (WKBGeometryType)-1; // Illformed blob if we can't even get enough bytes to tell us the type...
	}
	else if (headerReader->header.magic[0] != 'G' || headerReader->header.magic[1] != 'P') {
		return (WKBGeometryType)-1; // Illformed blob if we can't even get enough bytes to tell us the type...
	}
	else {
		int isExtendedGeometry, isEmptyGeometry, envelopeType;
		isExtendedGeometry = (headerReader->header.flags & 0x20);
		isEmptyGeometry = (headerReader->header.flags & 0x10);
		envelopeType = (headerReader->header.flags & 0x0E) >> 1;
		_littleEndianOrder = (headerReader->header.flags & 0x01);

		if (isEmptyGeometry) {
			// Geometry is empty
			return (WKBGeometryType)-1;
		}
		else if (isExtendedGeometry) {
			// Geometry is an extended geometry - Not Supported for now
			return (WKBGeometryType)-1;
		}
		else {
			int envelopeBytes = 0;
			const unsigned char * pEnvelope = bytes + sizeof(GeoPackageBinaryHeader);

			// Envelope
			if (envelopeType) {
				// 1: envelope is [minx, maxx, miny, maxy], 32 bytes
				// 2: envelope is [minx, maxx, miny, maxy, minz, maxz], 48 bytes
				// 3: envelope is [minx, maxx, miny, maxy, minm, maxm], 48 bytes
				// 4: envelope is [minx, maxx, miny, maxy, minz, maxz, minm, maxm], 64 bytes
				if (envelopeType == 1) {
					envelopeBytes = 32;
					_minx = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxx = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_miny = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxy = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
				}
				else if (envelopeType == 2) {
					envelopeBytes = 48;
					_minx = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxx = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_miny = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxy = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_minz = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxz = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
				}
				else if (envelopeType == 3) {
					envelopeBytes = 48;
					_minx = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxx = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_miny = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxy = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_minm = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxm = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
				}
				else if (envelopeType == 4) {
					envelopeBytes = 64;
					_minx = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxx = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_miny = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxy = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_minz = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxz = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_minm = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
					_maxm = read_double(pEnvelope, _littleEndianOrder); pEnvelope += sizeof(double);
				}
			}
			const unsigned char * wkb = bytes + sizeof(GeoPackageBinaryHeader) + envelopeBytes;
			// _wkt points to the WKB portion of the blob which is right after the GPKG header
			_wkb = wkb;
			_littleEndianOrder = wkb[0];
			wkb++;
			WKBGeometryType wkbType = (WKBGeometryType)read_uint32(wkb, _littleEndianOrder);
			wkb += sizeof(MI_UINT32);
			return wkbType;
		}
	}
	return (WKBGeometryType)-1;
}
bool WkbGeometry::AdoptGPKGWKB(size_t nbytes, const unsigned char * blob)
{
	_wkbGeometryType = Blob2GeopackageDataType(blob, nbytes);
	if (_wkbGeometryType == (WKBGeometryType)-1) {
		return false;
	}
	switch (_wkbGeometryType)
	{
	case WKBGeometryType::wkbPoint:
		_dimensionality = WkbGeometry::DIMENSIONALITY::P;
		break;
	case WKBGeometryType::wkbPointZ:
		_dimensionality = WkbGeometry::DIMENSIONALITY::Z;
		break;
	case WKBGeometryType::wkbPointM:
		_dimensionality = WkbGeometry::DIMENSIONALITY::M;
		break;
	case WKBGeometryType::wkbPointZM:
		_dimensionality = WkbGeometry::DIMENSIONALITY::ZM;
		break;
	}

	// Adopt
	_nbytes = nbytes;
	_gpkgwkb = blob;
	// Note that Blob2GeopackageDataType also set _wkb, _littleEndianOrder, _minx, _maxx, etc.

	return true;
}
bool WkbGeometry::AdoptWKB(size_t nbytes, const unsigned char * blob)
{
	const unsigned char * wkb = blob;

	int littleEndianOrder = wkb[0];
	wkb++;
	WKBGeometryType wkbGeometryType = (WKBGeometryType)read_uint32(wkb, littleEndianOrder);
	wkb += sizeof(MI_UINT32);

	if (wkbGeometryType == (WKBGeometryType)-1) {
		return false;
	}
	DIMENSIONALITY dimensionality = WkbGeometry::DIMENSIONALITY::P;
	WKBGeometryType wkbBaseGeometryType = BaseWKBType(wkbGeometryType, dimensionality);

	// Since I didn't have the header, I need to read the blob to determine the envelope
	EnvelopeReader * reader = new EnvelopeReader();
	wkb = blob;
	WkbGeometry::ParseInternal(reader, wkb);

	int envelopeCoords = 0;
	if (wkbBaseGeometryType != WKBGeometryType::wkbPoint)
	{
		envelopeCoords = 4;
		if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
			envelopeCoords += 2;
		}
		if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
			envelopeCoords += 2;
		}
	}

	// Now allocate a GPKG BLOB 
	unsigned char * gpkgbytes;
	size_t ngpkgbytes;
	ngpkgbytes = sizeof(GeoPackageBinaryHeader)
		+ (envelopeCoords * sizeof(double))  /* Envelope [minx, maxx, miny, maxy] */ 
		+ nbytes;

	gpkgbytes = new unsigned char[ngpkgbytes];
	unsigned char * pWhereCurrentlyAt = gpkgbytes;

	GeoPackageBinaryHeader * pHeader = (GeoPackageBinaryHeader *)pWhereCurrentlyAt;
	pHeader->magic[0] = 'G';
	pHeader->magic[1] = 'P';
	pHeader->version = 0;
	int r, x, y, e, b;
	r = 0; // reserved for future use; set to 0
	x = 0; // 0: StandardGeoPackageBinary, 1: ExtendedGeoPackageBinary
	y = 0; // empty geometry flag. 0: non-empty geometry, 1: empty geometry
	e = 1; // envelope is [minx, maxx, miny, maxy], 32 bytes
	b = 1; // LittleEndianOrder.
	if (wkbBaseGeometryType == WKBGeometryType::wkbPoint) e = 0; // no header
	else if (dimensionality == DIMENSIONALITY::Z) e = 2;
	else if (dimensionality == DIMENSIONALITY::M) e = 3;
	else if (dimensionality == DIMENSIONALITY::ZM) e = 4;
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	if (wkbBaseGeometryType != WKBGeometryType::wkbPoint)
	{
		write_double(pWhereCurrentlyAt, littleEndianOrder, reader->MinX()); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, reader->MaxX()); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, reader->MinY()); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, reader->MaxY()); pWhereCurrentlyAt += sizeof(double);
		if ((dimensionality == DIMENSIONALITY::Z) || (dimensionality == DIMENSIONALITY::ZM)) {
			write_double(pWhereCurrentlyAt, littleEndianOrder, reader->MinZ()); pWhereCurrentlyAt += sizeof(double);
			write_double(pWhereCurrentlyAt, littleEndianOrder, reader->MaxZ()); pWhereCurrentlyAt += sizeof(double);
		}
		if ((dimensionality == DIMENSIONALITY::M) || (dimensionality == DIMENSIONALITY::ZM)) {
			write_double(pWhereCurrentlyAt, littleEndianOrder, reader->MinM()); pWhereCurrentlyAt += sizeof(double);
			write_double(pWhereCurrentlyAt, littleEndianOrder, reader->MaxM()); pWhereCurrentlyAt += sizeof(double);
		}
	}
	memcpy(pWhereCurrentlyAt, blob, nbytes);

	// Note that Blob2GeopackageDataType also set _wkb, _littleEndianOrder, _minx, _maxx, etc.
	_wkbGeometryType = Blob2GeopackageDataType(gpkgbytes, ngpkgbytes);
	_dimensionality = dimensionality;
	_nbytes = ngpkgbytes;
	_gpkgwkb = gpkgbytes;
	delete[] blob;
	return true;
}

WkbGeometry * WkbGeometry::CreateFromGPKGWKB(const wchar_t * csys, size_t nbytes, const unsigned char * blob)
{
	WkbGeometry * wkbGeometry = new WkbGeometry(csys);
	if (!wkbGeometry->AdoptGPKGWKB(nbytes, blob)) {
		delete wkbGeometry;
		wkbGeometry = nullptr;
	}
	return wkbGeometry;
}
WkbGeometry * WkbGeometry::CreateFromWKB(const wchar_t * csys, size_t nbytes, const unsigned char * blob)
{
	WkbGeometry * wkbGeometry = new WkbGeometry(csys);
	if (!wkbGeometry->AdoptWKB(nbytes, blob)) {
		delete wkbGeometry;
		wkbGeometry = nullptr;
	}
	return wkbGeometry;
}

void WkbGeometry::GetPointCoordinates(double & x, double & y) const
{
	double z = 0, m = 0;
	const unsigned char * wkb = _wkb + /*endian order*/1 + /*wkbType*/sizeof(MI_UINT32);
	read_dpoint(wkb, _littleEndianOrder, _dimensionality, x, y, z, m);
}
/* ======================================================================= */
/* WKB Parse Methods                                                       */
/* ======================================================================= */
bool WkbGeometry::Parse(WkbParserCallback * callback)
{
	const unsigned char * wkb = _wkb;
	return ParseInternal(callback, wkb);
}
/* private static */
bool WkbGeometry::ParseInternal(WkbParserCallback * callback, const unsigned char * & wkb)
{
	int littleEndianOrder = wkb[0];
	wkb++;
	WKBGeometryType wkbGeometryType = (WKBGeometryType)read_uint32(wkb, littleEndianOrder);
	wkb += sizeof(MI_UINT32);

	bool bOK = callback->setWkbType(wkbGeometryType);

	if (bOK)
	{
		DIMENSIONALITY dimensionality = DIMENSIONALITY::P;
		switch (BaseWKBType(wkbGeometryType, dimensionality))
		{
		case WKBGeometryType::wkbPoint:
			bOK = ParsePoint(callback, wkb, littleEndianOrder, dimensionality);
			break;

		case WKBGeometryType::wkbMultiPoint:
			bOK = ParseMultiPoint(callback, wkb, littleEndianOrder, dimensionality);
			break;

		case WKBGeometryType::wkbLineString:
			bOK = ParseLineString(callback, wkb, littleEndianOrder, dimensionality);
			break;

		case WKBGeometryType::wkbMultiLineString:
			bOK = ParseMultiLinestring(callback, wkb, littleEndianOrder, dimensionality);
			break;

		case WKBGeometryType::wkbPolygon:
		case WKBGeometryType::wkbTriangle:
			bOK = ParsePolygon(callback, wkb, littleEndianOrder, dimensionality);
			break;

		case WKBGeometryType::wkbMultiPolygon:
			bOK = ParseMultiPolygon(callback, wkb, littleEndianOrder, dimensionality);
			break;

		case WKBGeometryType::wkbGeometryCollection:
			bOK = ParseGeometryCollection(callback, wkb, littleEndianOrder, dimensionality);
			break;

		case WKBGeometryType::wkbPolyhedralSurface:
		case WKBGeometryType::wkbTIN:
			// TODO: Unimplemented
			bOK = false;
			break;
		}
	}

	if (bOK) {
		bOK = callback->finish(bOK);
	}
	else {
		callback->finish(bOK);
	}
	return bOK;
}
bool WkbGeometry::ParsePoint(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality)
{
	WkbGeometry::PNT pnt;
	pnt.x = 0;
	pnt.y = 0;
	pnt.z = 0;
	pnt.m = 0;

	read_dpoint(wkb, littleEndianOrder, dimensionality, pnt);
	return callback->addPoint(pnt);
}
bool WkbGeometry::ParseMultiPoint(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality)
{
	bool bOK = callback->startPoints();
	if (bOK)
	{
		MI_UINT32 numPoints = read_uint32(wkb, littleEndianOrder);
		wkb += sizeof(MI_UINT32);
		bOK = callback->setNumPoints(numPoints);

		for (MI_UINT32 i = 0; bOK && i < numPoints; i++) {
			wkb++; // skip the local point's byte order - should be the same as the MPoint
			wkb += sizeof(MI_UINT32); // skip the local point's wkbType - known based on the type of MPoint (Z &/or M)
			WkbGeometry::PNT pnt;
			pnt.x = 0;
			pnt.y = 0;
			pnt.z = 0;
			pnt.m = 0;
			read_dpoint(wkb, littleEndianOrder, dimensionality, pnt);
			bOK = callback->addPoint(pnt);
		}
		bOK = bOK && callback->endPoints();
	}
	return bOK;
}
bool WkbGeometry::ParseLineString(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality)
{
	bool bOK = callback->startLineString();
	if (bOK)
	{
		MI_UINT32 numPoints = read_uint32(wkb, littleEndianOrder);
		wkb += sizeof(MI_UINT32);
		bOK = callback->setNumPoints(numPoints);

		for (MI_UINT32 i = 0; bOK && i < numPoints; i++) {
			WkbGeometry::PNT pnt;
			pnt.x = 0;
			pnt.y = 0;
			pnt.z = 0;
			pnt.m = 0;
			read_dpoint(wkb, littleEndianOrder, dimensionality, pnt);
			bOK = callback->addPoint(pnt);
		}
		bOK = bOK && callback->endLineString();
	}
	return bOK;
}
bool WkbGeometry::ParseMultiLinestring(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality)
{
	bool bOK = callback->startMulti();
	if (bOK)
	{
		MI_UINT32 numLineStrings = read_uint32(wkb, littleEndianOrder);
		wkb += sizeof(MI_UINT32);
		bOK = callback->setNumLineStrings(numLineStrings);

		for (MI_UINT32 j = 0; bOK && j < numLineStrings; j++) {
			bOK = callback->startLineString();
			if (bOK)
			{
				wkb++; // skip the local linestring's byte order - should be the same as the multi
				wkb += sizeof(MI_UINT32); // skip the local linestring's wkbType - known based on the type of multi (Z &/or M)

				MI_UINT32 numPoints = read_uint32(wkb, littleEndianOrder);
				wkb += sizeof(MI_UINT32);
				bOK = callback->setNumPoints(numPoints);
				for (MI_UINT32 i = 0; bOK && i < numPoints; i++) {
					WkbGeometry::PNT pnt;
					pnt.x = 0;
					pnt.y = 0;
					pnt.z = 0;
					pnt.m = 0;
					read_dpoint(wkb, littleEndianOrder, dimensionality, pnt);
					bOK = callback->addPoint(pnt);
				}
			}
			bOK = bOK && callback->endLineString();
		}
		bOK = bOK && callback->endMulti();
	}
	return bOK;
}
bool WkbGeometry::ParsePolygon(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality)
{
	bool bOK = callback->startPolygon();
	if (bOK)
	{
		MI_UINT32 numRings = read_uint32(wkb, littleEndianOrder);
		wkb += sizeof(MI_UINT32);
		bOK = callback->setNumRings(numRings);

		for (MI_UINT32 j = 0; bOK && j < numRings; j++) {
			bOK = callback->startRing();
			if (bOK)
			{
				MI_UINT32 numPoints = read_uint32(wkb, littleEndianOrder);
				wkb += sizeof(MI_UINT32);
				bOK = callback->setNumPoints(numPoints);
				for (MI_UINT32 i = 0; bOK && i < numPoints; i++) {
					WkbGeometry::PNT pnt;
					pnt.x = 0;
					pnt.y = 0;
					pnt.z = 0;
					pnt.m = 0;
					read_dpoint(wkb, littleEndianOrder, dimensionality, pnt);
					bOK = callback->addPoint(pnt);
				}
			}
			bOK = bOK && callback->endRing();
		}
		bOK = bOK && callback->endPolygon();
	}
	return bOK;
}
bool WkbGeometry::ParseMultiPolygon(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality)
{
	bool bOK = callback->startMulti();
	if (bOK)
	{
		MI_UINT32 numPolygons = read_uint32(wkb, littleEndianOrder);
		wkb += sizeof(MI_UINT32);
		bOK = callback->setNumPolygons(numPolygons);

		for (MI_UINT32 k = 0; bOK && k < numPolygons; k++) {
			bOK = callback->startPolygon();
			if (bOK)
			{
				wkb++; // skip the local polygon's byte order - should be the same as the multi
				wkb += sizeof(MI_UINT32); // skip the local polygon's wkbType - known based on the type of multi (Z &/or M)
				MI_UINT32 numRings = read_uint32(wkb, littleEndianOrder);
				wkb += sizeof(MI_UINT32);
				bOK = callback->setNumRings(numRings);

				for (MI_UINT32 j = 0; bOK && j < numRings; j++) {
					bOK = callback->startRing();
					if (bOK)
					{
						MI_UINT32 numPoints = read_uint32(wkb, littleEndianOrder);
						wkb += sizeof(MI_UINT32);
						bOK = callback->setNumPoints(numPoints);

						for (MI_UINT32 i = 0; bOK && i < numPoints; i++) {
							WkbGeometry::PNT pnt;
							pnt.x = 0;
							pnt.y = 0;
							pnt.z = 0;
							pnt.m = 0;
							read_dpoint(wkb, littleEndianOrder, dimensionality, pnt);
							bOK = callback->addPoint(pnt);
						}
					}
					bOK = bOK && callback->endRing();
				}
			}
			bOK = bOK && callback->endPolygon();
		}
		bOK = bOK && callback->endMulti();
	}
	return bOK;
}
bool WkbGeometry::ParseGeometryCollection(WkbParserCallback * callback, const unsigned char * & wkb, int littleEndianOrder, DIMENSIONALITY dimensionality)
{
	bool bOK = callback->startMulti();
	if (bOK)
	{
		MI_UINT32 numGeometries = read_uint32(wkb, littleEndianOrder);
		wkb += sizeof(MI_UINT32);
		bOK = callback->setNumGeometries(numGeometries);

		for (MI_UINT32 k = 0; bOK && k < numGeometries; k++) {
			bOK = callback->startGeometry();
			bOK = bOK && ParseInternal(callback, wkb);
			bOK = bOK && callback->endGeometry();
		}
		bOK = bOK && callback->endMulti();
	}
	return bOK;
}

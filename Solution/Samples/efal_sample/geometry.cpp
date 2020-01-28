#include <stdafx.h>
#include <EFAL.h>

using namespace std;

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
static void write_double(char * bytes, int littleEndianOrder, double dval) {
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

static unsigned __int32 read_uint32(const unsigned char * bytes, int littleEndianOrder) {
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
static void write_uint32(char * bytes, int littleEndianOrder, unsigned __int32 ival) {
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
static __int64 read_dpoint(const unsigned char * bytes, int littleEndianOrder, double * pxVal, double * pyVal, double * pzVal, double *pmVal) {
	const unsigned char * wkb = bytes;
	*pxVal = read_double(wkb, littleEndianOrder);
	wkb += sizeof(double);
	*pyVal = read_double(wkb, littleEndianOrder);
	wkb += sizeof(double);
	if (pzVal != NULL) {
		*pzVal = read_double(wkb, littleEndianOrder);
		wkb += sizeof(double);
	}
	if (pmVal != NULL) {
		*pmVal = read_double(wkb, littleEndianOrder);
		wkb += sizeof(double);
	}
	return (wkb - bytes);
}

void CreatePointGeometry(char ** pbytes, unsigned long * pnbytes, double longitude, double latitude)
{
	char * bytes;
	unsigned long nbytes;

	char * pWhereCurrentlyAt = 0;

	nbytes = sizeof(GeoPackageBinaryHeader)
		+ 0                                          /* No envelope for point */
		+ 1                                          /* byteOrder */
		+ sizeof(unsigned int)                       /* wkbType */
		+ sizeof(double) + sizeof(double);           /* Point */

	bytes = new char[nbytes];
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

	pWhereCurrentlyAt[0] = (char)littleEndianOrder;
	pWhereCurrentlyAt++;
	write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbPoint); pWhereCurrentlyAt += sizeof(unsigned __int32);
	write_double(pWhereCurrentlyAt, littleEndianOrder, longitude); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, latitude); pWhereCurrentlyAt += sizeof(double);

	*pbytes = bytes;
	*pnbytes = nbytes;
}
void CreateMultiPointGeometry(char ** pbytes, unsigned long * pnbytes, double* nodeData, int numPoints)
{
	char * bytes;
	unsigned long nbytes;

	char * pWhereCurrentlyAt = 0;
	/*

	Point {
		double x;
		double y;
	};
	WKBPoint {
		byte byteOrder;
		uint32 wkbType; // 1
		Point point;
	}
	WKBMultiPoint {
		byte byteOrder;
		uint32 wkbType; // 4
		uint32 num_wkbPoints;
		WKBPoint WKBPoints[num_wkbPoints];
	}
	*/

	nbytes = sizeof(GeoPackageBinaryHeader)
		+ (4 * sizeof(double))                       /* Envelope [minx, maxx, miny, maxy] */
		+ 1                                          /* byteOrder */
		+ sizeof(unsigned int)                       /* wkbType */
		+ sizeof(unsigned int)                       /* numPoints */
		+ numPoints * (
			1                                         /* byteOrder */
			+ sizeof(unsigned int)                    /* wkbType */
			+ sizeof(double)                          /* x */
			+ sizeof(double)                          /* y */
			);

	bytes = new char[nbytes];
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
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	double minx, maxx, miny, maxy;
	minx = maxx = nodeData[0];
	miny = maxy = nodeData[1];
	int idxPoint = 0;
	for (int i = 0; i < numPoints; i++)
	{
		double x = nodeData[idxPoint], y = nodeData[idxPoint + 1];
		if (x < minx) minx = x;
		if (x > maxx) maxx = x;
		if (y < miny) miny = y;
		if (y > maxy) maxy = y;
		idxPoint += 2;
	}
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);

	/* Now write the geometry */
	pWhereCurrentlyAt[0] = (char)littleEndianOrder;
	pWhereCurrentlyAt++;

	write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbMultiPoint); pWhereCurrentlyAt += sizeof(unsigned __int32);
	idxPoint = 0;
	write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numPoints); pWhereCurrentlyAt += sizeof(unsigned __int32);
	for (int i = 0; i < numPoints; i++)
	{
		double x = nodeData[idxPoint], y = nodeData[idxPoint + 1];
		pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbPoint); pWhereCurrentlyAt += sizeof(unsigned __int32);
		write_double(pWhereCurrentlyAt, littleEndianOrder, x); pWhereCurrentlyAt += sizeof(double);
		write_double(pWhereCurrentlyAt, littleEndianOrder, y); pWhereCurrentlyAt += sizeof(double);
		idxPoint += 2;
	}

	*pbytes = bytes;
	*pnbytes = nbytes;
}
void CreateMultiCurveGeometry(char ** pbytes, unsigned long * pnbytes, double* nodeData, int* numPoints, int numCurves)
{
	char * bytes;
	unsigned long nbytes;

	char * pWhereCurrentlyAt = 0;


	nbytes = sizeof(GeoPackageBinaryHeader)
		+ (4 * sizeof(double))                       /* Envelope [minx, maxx, miny, maxy] */
		+ 1                                          /* byteOrder */
		+ sizeof(unsigned int);                      /* wkbType */

	if (numCurves > 1)
		nbytes += sizeof(unsigned int);              /* numLineStrings */

	for (int i = 0; i < numCurves; i++)
	{
		if (numCurves > 1)
			nbytes +=
			(
				1                                      /* byteOrder */
				+ sizeof(unsigned int)                 /* wkbType */
				+ sizeof(unsigned int)                 /* numPoints */
				);

		nbytes +=
			sizeof(unsigned int)                      /* numPoints */
			+ numPoints[i] *                          /* Number of nodes times  */
			(sizeof(double) + sizeof(double));        /*      size of Point     */
	}

	bytes = new char[nbytes];
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
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	double minx, maxx, miny, maxy;
	minx = maxx = nodeData[0];
	miny = maxy = nodeData[1];
	int idxPoint = 0;
	for (int j = 0; j < numCurves; j++)
	{
		for (int i = 0; i < numPoints[j]; i++)
		{
			double x = nodeData[idxPoint], y = nodeData[idxPoint + 1];
			if (x < minx) minx = x;
			if (x > maxx) maxx = x;
			if (y < miny) miny = y;
			if (y > maxy) maxy = y;
			idxPoint += 2;
		}
	}
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);

	/* Now write the geometry */
	pWhereCurrentlyAt[0] = (char)littleEndianOrder;
	pWhereCurrentlyAt++;

	if (numCurves == 1)
	{
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbLineString); pWhereCurrentlyAt += sizeof(unsigned __int32);
	}
	else
	{
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbMultiLineString); pWhereCurrentlyAt += sizeof(unsigned __int32);
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numCurves); pWhereCurrentlyAt += sizeof(unsigned __int32);
	}
	idxPoint = 0;
	for (int j = 0; j < numCurves; j++)
	{
		if (numCurves > 1)
		{
			pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
			write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbLineString); pWhereCurrentlyAt += sizeof(unsigned __int32);
		}
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numPoints[j]); pWhereCurrentlyAt += sizeof(unsigned __int32);
		for (int i = 0; i < numPoints[j]; i++)
		{
			double x = nodeData[idxPoint], y = nodeData[idxPoint + 1];
			write_double(pWhereCurrentlyAt, littleEndianOrder, x); pWhereCurrentlyAt += sizeof(double);
			write_double(pWhereCurrentlyAt, littleEndianOrder, y); pWhereCurrentlyAt += sizeof(double);
			idxPoint += 2;
		}
	}

	*pbytes = bytes;
	*pnbytes = nbytes;
}

void CreateMultiPolygonGeometry(char ** pbytes, unsigned long * pnbytes, double* nodeData, int* numPoints, int* numRings, int numPolygons)
{
	char * bytes;
	unsigned long nbytes;

	char * pWhereCurrentlyAt = 0;

	double minx, maxx, miny, maxy;
	minx = maxx = nodeData[0];
	miny = maxy = nodeData[1];

	int idxPoint = 0;

	if (numPolygons == 1)
	{
		// Simple (single) polygon
		nbytes = sizeof(GeoPackageBinaryHeader)
			+ (4 * sizeof(double))                       /* Envelope [minx, maxx, miny, maxy] */
			+ 1                                          /* byteOrder */
			+ sizeof(unsigned int)                       /* wkbType */
			+ sizeof(unsigned int);                      /* numRings */

		for (int iRing = 0; iRing < numRings[/*iPolygon=*/0]; iRing++)
		{
			nbytes += sizeof(unsigned int)               /* numPoints */
				+ (numPoints[iRing] * sizeof(double) * 2);/* points */
			for (int iPoint = 0; iPoint < numPoints[iRing]; iPoint++)
			{
				double x = nodeData[idxPoint], y = nodeData[idxPoint + 1];
				if (x < minx) minx = x;
				if (x > maxx) maxx = x;
				if (y < miny) miny = y;
				if (y > maxy) maxy = y;
				idxPoint += 2;
			}
		}
	}
	else
	{
		// MultiPolygon
		nbytes = sizeof(GeoPackageBinaryHeader)
			+ (4 * sizeof(double))                       /* Envelope [minx, maxx, miny, maxy] */
			+ 1                                          /* byteOrder */
			+ sizeof(unsigned int)                       /* wkbType */
			+ sizeof(unsigned int);                      /* numPolygons */
		for (int iPolygon = 0; iPolygon < numPolygons; iPolygon++)
		{
			nbytes +=
				1                                          /* byteOrder */
				+ sizeof(unsigned int)                       /* wkbType */
				+ sizeof(unsigned int);                      /* numRings */

			for (int iRing = 0; iRing < numRings[iPolygon]; iRing++)
			{
				nbytes += sizeof(unsigned int)               /* numPoints */
					+ (numPoints[iRing] * sizeof(double) * 2);/* points */
				for (int iPoint = 0; iPoint < numPoints[iRing]; iPoint++)
				{
					double x = nodeData[idxPoint], y = nodeData[idxPoint + 1];
					if (x < minx) minx = x;
					if (x > maxx) maxx = x;
					if (y < miny) miny = y;
					if (y > maxy) maxy = y;
					idxPoint += 2;
				}
			}
		}
	}

	bytes = new char[nbytes];
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
	pHeader->flags = (unsigned char)((r << 6) | (x << 5) | (y << 4) | (e << 1) | b);
	pHeader->srs_id = 0;

	pWhereCurrentlyAt += sizeof(GeoPackageBinaryHeader);
	/* Write the envelope */
	write_double(pWhereCurrentlyAt, littleEndianOrder, minx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxx); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, miny); pWhereCurrentlyAt += sizeof(double);
	write_double(pWhereCurrentlyAt, littleEndianOrder, maxy); pWhereCurrentlyAt += sizeof(double);

	/* Now write the geometry */
	idxPoint = 0;
	pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
	if (numPolygons == 1)
	{
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbPolygon); pWhereCurrentlyAt += sizeof(unsigned __int32);

		write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numRings[/*iPolygon=*/0]); pWhereCurrentlyAt += sizeof(unsigned __int32);
		for (int iRing = 0; iRing < numRings[/*iPolygon=*/0]; iRing++)
		{
			write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numPoints[iRing]); pWhereCurrentlyAt += sizeof(unsigned __int32);
			for (int iPoint = 0; iPoint < numPoints[iRing]; iPoint++)
			{
				double x = nodeData[idxPoint], y = nodeData[idxPoint + 1];
				write_double(pWhereCurrentlyAt, littleEndianOrder, x); pWhereCurrentlyAt += sizeof(double);
				write_double(pWhereCurrentlyAt, littleEndianOrder, y); pWhereCurrentlyAt += sizeof(double);
				idxPoint += 2;
			}
		}
	}
	else
	{
		// MultiPolygon
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbMultiPolygon); pWhereCurrentlyAt += sizeof(unsigned __int32);
		write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numPolygons); pWhereCurrentlyAt += sizeof(unsigned __int32);

		for (int iPolygon = 0; iPolygon < numPolygons; iPolygon++)
		{
			pWhereCurrentlyAt[0] = (char)littleEndianOrder; pWhereCurrentlyAt++;
			write_uint32(pWhereCurrentlyAt, littleEndianOrder, WKBGeometryType::wkbPolygon); pWhereCurrentlyAt += sizeof(unsigned __int32);
			write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numRings[iPolygon]); pWhereCurrentlyAt += sizeof(unsigned __int32);

			for (int iRing = 0; iRing < numRings[iPolygon]; iRing++)
			{
				write_uint32(pWhereCurrentlyAt, littleEndianOrder, (unsigned __int32)numPoints[iRing]); pWhereCurrentlyAt += sizeof(unsigned __int32);
				for (int iPoint = 0; iPoint < numPoints[iRing]; iPoint++)
				{
					double x = nodeData[idxPoint], y = nodeData[idxPoint + 1];
					write_double(pWhereCurrentlyAt, littleEndianOrder, x); pWhereCurrentlyAt += sizeof(double);
					write_double(pWhereCurrentlyAt, littleEndianOrder, y); pWhereCurrentlyAt += sizeof(double);
					idxPoint += 2;
				}
			}
		}
	}

	*pbytes = bytes;
	*pnbytes = nbytes;
}

void CreateTextGeometry(char ** pbytes, unsigned long * pnbytes, Ellis::DRECT mbr, short angle, Ellis::CalloutLineType calloutLineType,
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

	char * bytes;
	char * pWhereCurrentlyAt = 0;
	unsigned long nbytes = sizeof(GeoPackageBinaryHeader)
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

	bytes = new char[nbytes];
	pWhereCurrentlyAt = bytes;

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

	*pbytes = bytes;
	*pnbytes = nbytes;
}
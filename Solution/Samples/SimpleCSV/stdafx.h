// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <EFALLIB.h>

#pragma warning(push)
#pragma warning(disable: 4200)
#pragma pack(push, 1)
struct GeoPackageBinaryHeader {
	byte magic[2]; // 0x4750;
	byte version;
	byte flags;
	__int32 srs_id;
	//double[] envelope; - we will on build ponts and do not need the envelope
};
struct Point {
	double x;
	double y;
};
struct WKBPoint {
	byte byteOrder;// = 1 LittleEndian
	__int32 wkbType;// = 1;
	Point point;
};

struct StandardGeoPackageBinary {
	GeoPackageBinaryHeader header;
	WKBPoint geometry; // NOTE: We are only supporting Points here in this sample!
};
union Blob
{
	StandardGeoPackageBinary wkb;
	char data[1];
};
#pragma pack(pop)
#pragma warning(pop)

std::vector<std::wstring> parse(const wchar_t * line);
void CreatePointWKB(double x, double y, Blob *& pBlob, size_t& nbytes);

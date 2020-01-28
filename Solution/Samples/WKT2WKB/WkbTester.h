#pragma once
class WkbTester
{
public:
	WkbTester();
	~WkbTester();

public:
	bool runTests();

private:
	bool test_POINT();
	bool test_POINT_Z();
	bool test_POINT_M();
	bool test_POINT_ZM();
	bool test_LINESTRING();
	bool test_LINESTRING_Z();
	bool test_LINESTRING_M();
	bool test_LINESTRING_ZM();
	bool test_MULTILINESTRING();
	bool test_MULTILINESTRING_Z();
	bool test_MULTILINESTRING_M();
	bool test_MULTILINESTRING_ZM();
	bool test_POLYGON1();
	bool test_POLYGON1_Z();
	bool test_POLYGON1_M();
	bool test_POLYGON1_ZM();
	bool test_POLYGON2();
	bool test_POLYGON2_Z();
	bool test_POLYGON2_M();
	bool test_POLYGON2_ZM();
	bool test_MULTIPOINT();
	bool test_MULTIPOINT_Z();
	bool test_MULTIPOINT_M();
	bool test_MULTIPOINT_ZM();
	bool test_MULTIPOLYGON1();
	bool test_MULTIPOLYGON1_Z();
	bool test_MULTIPOLYGON1_M();
	bool test_MULTIPOLYGON1_ZM();
	bool test_MULTIPOLYGON2();
	bool test_MULTIPOLYGON2_Z();
	bool test_MULTIPOLYGON2_M();
	bool test_MULTIPOLYGON2_ZM();
	bool test_GEOMETRYCOLLECTION();
	bool test_GEOMETRYCOLLECTION_Z();
	bool test_GEOMETRYCOLLECTION_M();
	bool test_GEOMETRYCOLLECTION_ZM();

private:
	bool testWKT2GPKGWKB(const wchar_t * wkt, const wchar_t * strgpkgwkb);
	bool testGPKGWKB2WKT(const wchar_t * strgpkgwkb, const wchar_t * wkt);
	bool testWKB2WKT(const wchar_t * strwkb, const wchar_t * wkt);
	bool testWKB2GPKGWKB(const wchar_t * strwkb, const wchar_t * strgpkgwkb);
	unsigned char tobyte(const wchar_t * pstrbyte);
	void str2blob(const wchar_t * str, size_t & sz, unsigned char * & blob);
};


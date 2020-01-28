#pragma once
#include <EFALAPI.h>
#include <EFAL.h>
#include <EFALLIB.h>
#include <string>
#include <vector>
#include <time.h>

class FieldSpec
{
public:
	std::wstring        _fieldName;
	Ellis::ALLTYPE_TYPE _efalType;
	bool                _couldBeInt;
	bool                _couldBeDouble;
	bool                _indexed = false;
	int                 _widthTAB;
	int                 _maxWidth;
	std::wstring        _csys;
	bool                _includeInTAB;
	bool                _isWktString;
	bool                _isStyleJoinColumn;
	std::wstring        _variableName;
	FieldSpec *         _objField;
	FieldSpec *         _latField;
	FieldSpec *         _lonField;
	FieldSpec *         _wktField;

public:
	FieldSpec() :
		_fieldName(),
		_efalType(Ellis::ALLTYPE_TYPE::OT_CHAR),
		_couldBeInt(true),
		_couldBeDouble(true),
		_indexed(false),
		_widthTAB(10),
		_maxWidth(-1),
		_csys(),
		_includeInTAB(true),
		_isWktString(false),
		_isStyleJoinColumn(false),
		_variableName(),
		_objField(nullptr),
		_latField(nullptr),
		_lonField(nullptr),
		_wktField(nullptr)
	{}
	~FieldSpec() {
	}
};

class ProcessingOptions
{
public:
	EFALLIB *               _efallib;
	EFALHANDLE              _hSession;

	bool                    _report_only;
	bool                    _linecount_only;
	bool                    _isExport;

	std::wstring            _input_file;
	std::wstring            _input_filename_no_path;
	bool                    _has_header;
	bool                    _has_quotes;
	int                     _sample_size;
	std::wstring            _delimiter;
	std::wstring            _delimiter_char;
	std::wstring            _filter;
	bool                    _filter_include;

	std::vector<FieldSpec*> _fields;
	size_t                  _nbrFieldsFromData;

	std::wstring            _export_file;
	EFALHANDLE              _hExportTable;

	std::wstring            _output_path;
	std::wstring            _output_format;
	std::wstring            _output_alias;
	std::wstring            _output_gpkg_dbname;
	Ellis::MICHARSET        _output_charset;
	std::wstring            _split_on;
	std::wstring            _default_style;

	EFALHANDLE              _hStyleTable;
	std::wstring            _style_table;
	std::wstring            _style_table_join_column;
	std::wstring            _style_join_column;
	std::wstring            _style_table_style_column;

	bool                    _silent;
	bool                    _verbose;
	int                     _start_at;
	int                     _max_records;
	bool                    _echo_error_lines;
	std::wstring            _error_file;
	int                     _batch_size;
	bool                    _append;
	bool                    _no_insert;
	int                     _start_debug_at;
	int                     _debug_records;

	std::vector<std::wstring> _sampleRecords;

public:
	ProcessingOptions() :
		_efallib(nullptr),
		_hSession(0),

		_report_only(true),
		_linecount_only(false),
		_isExport(false),

		_input_file(),
		_input_filename_no_path(),
		_has_header(false),
		_has_quotes(false),
		_sample_size(200),
		_delimiter(L"DETECT"),
		_delimiter_char(L","),
		_filter(),
		_filter_include(true),

		_fields(),
		_nbrFieldsFromData(0),

		_export_file(),
		_hExportTable(0),

		_output_path(),
		_output_format(L"NATIVEX"),
		_output_alias(),
		_output_gpkg_dbname(),
		_output_charset(Ellis::MICHARSET::CHARSET_UTF8),
		_split_on(),
		_default_style(),

		_hStyleTable(0),
		_style_table(),
		_style_table_join_column(),
		_style_join_column(),
		_style_table_style_column(),

		_silent(false),
		_verbose(false),
		_start_at(1),
		_max_records(-1),
		_echo_error_lines(false),
		_error_file(L"error.txt"),
		_batch_size(1000),
		_append(false),
		_no_insert(false),
		_start_debug_at(-1),
		_debug_records(10),

		_sampleRecords()
	{}
	~ProcessingOptions() {
		for (size_t i = 0, n = _fields.size(); i < n; i++) {
			delete _fields[i];
		}
		_fields.clear();
		if (_hSession > 0) {
			_efallib->DestroySession(_hSession);
			_hSession = 0;
		}
	}
};

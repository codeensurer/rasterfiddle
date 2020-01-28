#include <stdafx.h>
#include <Transaction.h>
#include <WktConvert.h>
#include <EFALUtilities.h>

/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*= */
/*                             SESSION and VARIABLE LEVEL FUNCTIONS                                 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*= */
void Transaction::createVariables(ProcessingOptions * options, EFALHANDLE hSession)
{

	if (options->_filter.length() > 0) {
		options->_efallib->CreateVariable(hSession, L"@@FILTER@@");
	}
	for (size_t i = 0, n = options->_fields.size(); i < n; i++)
	{
		FieldSpec * field = options->_fields[i];
		options->_efallib->CreateVariable(hSession, field->_variableName.c_str());
		if ((field->_efalType == Ellis::ALLTYPE_TYPE::OT_STYLE) && (wcslen(options->_default_style.c_str()) > 0)) {
			options->_efallib->SetVariableValueStyle(hSession, field->_variableName.c_str(), options->_default_style.c_str());
		}
	}
	options->_efallib->CreateVariable(hSession, L"@MI_STYLE");
	options->_efallib->SetVariableValueStyle(hSession, L"@MI_STYLE", L"Symbol(35, 0, 10) Pen(1, 2, 0)  Brush(2, 16777215, 16777215)");
}
bool Transaction::filterExcludeThisRecord()
{
	if (_options->_filter.length() > 0) {
		_options->_efallib->SetVariableValue(_hSession, L"@@FILTER@@", _options->_filter.c_str());
		if (_options->_efallib->GetVariableIsNull(_hSession, L"@@FILTER@@")) {
			// Whether we are filtering records out of the result or into the result, we don't
			// know what to do for NULLs so let's just always leave NULLs out.
			return true;
		}
		int b = _options->_efallib->GetVariableValueInt32(_hSession, L"@@FILTER@@");
		if (_options->_filter_include) {
			return (b == 0);
		}
		else { // exclude
			return (b != 0);
		}
	}
	return false;
}

bool Transaction::setVariable(FieldSpec * field, const wchar_t * varValue)
{
	ProcessingOptions * options = _options;
	EFALHANDLE hSession = _hSession;

	bool bOK = false;
	const wchar_t *varName = field->_variableName.c_str();
	Ellis::ALLTYPE_TYPE type = field->_efalType;

	if (type == Ellis::ALLTYPE_TYPE::OT_STYLE) {
		if (wcslen(varValue) == 0) {
			varValue = L"Symbol(35, 0, 10) Pen(1, 2, 0)  Brush(2, 16777215, 16777215)";
		}
		//wprintf(L"Setting %ls to %ls\n", varName, varValue);
		bOK = options->_efallib->SetVariableValueStyle(hSession, varName, varValue);
	}
	else if (wcslen(varValue) == 0) {
		bOK = options->_efallib->SetVariableIsNull(hSession, varName);
	}
	else if (type == Ellis::ALLTYPE_TYPE::OT_FLOAT) {
		double d = 0.0;
		if (1 == swscanf_s(varValue, L"%lf", &d)) {
			bOK = options->_efallib->SetVariableValueDouble(hSession, varName, d);
		}
	}
	else if (type == Ellis::ALLTYPE_TYPE::OT_INTEGER) {
		int i = 0;
		if (1 == swscanf_s(varValue, L"%d", &i)) {
			bOK = options->_efallib->SetVariableValueInt32(hSession, varName, i);
		}
	}
	else if (field->_isWktString) {
		// No need to set this string as a variable, it's too long for a TAB file. 
		// Convert it to a WKB and bind it to @OBJ
		FieldSpec * objField = nullptr;
		for (size_t i = 0, n = options->_fields.size(); i < n; i++) {
			if (options->_fields[i]->_wktField == field) {
				objField = options->_fields[i];
				break;
			}
		}
		if (nullptr != objField) {
			WktConvert * wktConvert = new WktConvert(L"EPSG:4326");
			if (wktConvert->convert(varValue)) {
				bOK = options->_efallib->SetVariableValueGeometry(hSession, objField->_variableName.c_str(), 
					(MI_UINT32)wktConvert->GetWkbGeometry()->GetNbrGPKGWKBBytes(), (const char *) wktConvert->GetWkbGeometry()->AsGPKGWKB(),
					objField->_csys.c_str());
			}
			else {
				bOK = false;
			}
			delete wktConvert;
		}
	}
	else {
		bOK = options->_efallib->SetVariableValueString(hSession, varName, varValue);
	}
	return bOK;
}
EFALHANDLE Transaction::createSession(ProcessingOptions * options)
{
	EFALHANDLE hSession = options->_hSession;
	if (hSession == 0) {
		hSession = options->_efallib->InitializeSession(nullptr);
		if (hSession == 0) {
			if (options->_verbose) wprintf(L"Failed to create session.\n");
			return 0;
		}
		else if (options->_verbose) wprintf(L"New EFAL session created successfully.\n");
	}
	return hSession;
}
void Transaction::closeSession(ProcessingOptions * options, EFALHANDLE hSession)
{
	if (options->_verbose) wprintf(L"Closing EFAL session.\n");
	options->_efallib->DestroySession(hSession);
}
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*= */
/*                                TABLE and STATEMENT LEVEL FUNCTIONS                               */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*= */
EFALHANDLE Transaction::openTable(ProcessingOptions * options, EFALHANDLE hSession)
{
	EFALHANDLE hTable = 0;
	hTable = options->_efallib->GetTableHandle(hSession, options->_output_alias.c_str());
	if (hTable == 0) {
		std::wstring table_alias = options->_output_alias.c_str();
		std::wstring table_filename = options->_output_path.c_str();
		table_filename += L"\\";
		table_filename += table_alias;
		table_filename += L".TAB";

		hTable = options->_efallib->OpenTable(hSession, table_filename.c_str());
		if (options->_verbose) wprintf(L"Table %ls successfully opened.\n", table_filename.c_str());
	}
	else if (options->_verbose) wprintf(L"Table %ls already open.\n", options->_output_alias.c_str());
	return hTable;
}

EFALHANDLE Transaction::createTable(ProcessingOptions * options, EFALHANDLE hSession, EFALHANDLE hBaseTable, const wchar_t * output_alias, bool append)
{
	EFALHANDLE hTable = 0;
	hTable = options->_efallib->GetTableHandle(hSession, output_alias);
	if (hTable == 0) {
		std::wstring table_alias = output_alias;
		std::wstring table_filename = options->_output_path.c_str();
		table_filename += L"\\";
		table_filename += table_alias;
		table_filename += L".TAB";

		if (append) {
			hTable = options->_efallib->OpenTable(hSession, table_filename.c_str());
		}
		if (hTable == 0) {
			EFALHANDLE hMetadataNewTable = 0;
			if (_wcsicmp(options->_output_format.c_str(), L"NATIVE") == 0) {
				hMetadataNewTable = options->_efallib->CreateNativeTableMetadata(hSession, table_alias.c_str(), table_filename.c_str(), options->_output_charset);
			}
			else if (_wcsicmp(options->_output_format.c_str(), L"GPKG") == 0) {
				std::wstring databasePath = options->_output_path.c_str();
				databasePath += L"\\";
				if (wcslen(options->_output_gpkg_dbname.c_str()) == 0)
					databasePath += table_alias;
				else
					databasePath += options->_output_gpkg_dbname.c_str();
				databasePath += L".gpkg";

				hMetadataNewTable = options->_efallib->CreateGeopackageTableMetadata(hSession, table_alias.c_str(),
					table_filename.c_str(), databasePath.c_str(), options->_output_charset, /*convertUnsupportedObjects*/true);
			}
			else { // NATIVEX is the default
				hMetadataNewTable = options->_efallib->CreateNativeXTableMetadata(hSession, table_alias.c_str(), table_filename.c_str(), options->_output_charset);
			}
			if (hMetadataNewTable != 0) {
				if (hBaseTable != 0) {
					for (int idx = 0, n = options->_efallib->GetColumnCount(hSession, hBaseTable); idx < n; ++idx) {
						const wchar_t * columnName = options->_efallib->GetColumnName(hSession, hBaseTable, idx);
						Ellis::ALLTYPE_TYPE columnType = options->_efallib->GetColumnType(hSession, hBaseTable, idx);
						bool isIndexed = options->_efallib->IsColumnIndexed(hSession, hBaseTable, idx);
						MI_UINT32 columnWidth = options->_efallib->GetColumnWidth(hSession, hBaseTable, idx);
						MI_UINT32 columnDecimals = options->_efallib->GetColumnDecimals(hSession, hBaseTable, idx);
						const wchar_t * columnCSys = options->_efallib->GetColumnCSys(hSession, hBaseTable, idx);
						options->_efallib->AddColumn(hSession, hMetadataNewTable, columnName, columnType, isIndexed, columnWidth, columnDecimals, columnCSys);
					}
				}
				else {
					for (size_t i = 0, n = options->_fields.size(); i < n; i++)
					{
						if (!options->_fields[i]->_includeInTAB) continue;
						options->_efallib->AddColumn(hSession, hMetadataNewTable,
							options->_fields[i]->_fieldName.c_str(),
							options->_fields[i]->_efalType,
							options->_fields[i]->_indexed,
							options->_fields[i]->_widthTAB,
							/*decimals*/0,
							(options->_fields[i]->_csys.length() > 0) ? options->_fields[i]->_csys.c_str() : nullptr);
					}
				}
				hTable = options->_efallib->CreateTable(hSession, hMetadataNewTable);
				if (options->_verbose) {
					if (hTable == 0) wprintf(L"Table %ls failed to create!\n", table_alias.c_str());
					else wprintf(L"Table %ls created successfully.\n", table_alias.c_str());
				}

				options->_efallib->DestroyTableMetadata(hSession, hMetadataNewTable);
			}
		}
		else if (options->_verbose) wprintf(L"Table %ls opened for appending.\n", table_filename.c_str());
	}
	else if (options->_verbose) wprintf(L"Table %ls not created since already open.\n", output_alias);
	return hTable;
}
void Transaction::closeTable(ProcessingOptions * options, EFALHANDLE hSession, EFALHANDLE hTable)
{
	if (options->_verbose) wprintf(L"Closing table %ls\n", options->_efallib->GetTableName(hSession, hTable));
	options->_efallib->CloseTable(hSession, hTable);
}

void Transaction::dumpSession(ProcessingOptions * options)
{
	EFALHANDLE hSession = options->_hSession;
	EFALLIB * efallib = options->_efallib;

	EFALUtilities::dumpEFALSession(efallib, hSession);
}

EFALHANDLE Transaction::createStatement(ProcessingOptions * options, EFALHANDLE hSession, EFALHANDLE hTable)
{
	EFALHANDLE hStatement = 0;
	std::wstring table_alias = options->_efallib->GetTableName(hSession, hTable);
	std::wstring insert_text = L"INSERT INTO \"";
	insert_text.append(table_alias);
	insert_text.append(L"\" (");

	bool first = true;
	for (size_t i = 0, n = options->_fields.size(); i < n; i++) {
		if (!options->_fields[i]->_includeInTAB) continue;
		if (!first) insert_text.append(L",");
		insert_text.append(options->_fields[i]->_fieldName.c_str());
		first = false;
	}
	insert_text.append(L") VALUES (");
	first = true;
	for (size_t i = 0, n = options->_fields.size(); i < n; i++) {
		if (!options->_fields[i]->_includeInTAB) continue;
		if (!first) insert_text.append(L",");
		if (options->_fields[i]->_efalType == Ellis::ALLTYPE_TYPE::OT_OBJECT) {
			if ((nullptr != options->_fields[i]->_lonField) && (nullptr != options->_fields[i]->_latField)) {
				insert_text.append(L"MI_Point(");
				insert_text.append(options->_fields[i]->_lonField->_variableName.c_str());
				insert_text.append(L",");
				insert_text.append(options->_fields[i]->_latField->_variableName.c_str());
				insert_text.append(L",'EPSG:4326')");
			}
			else if (nullptr != options->_fields[i]->_wktField) {
				insert_text.append(options->_fields[i]->_variableName.c_str()); // @OBJ
			}
		}
		else {
			insert_text.append(options->_fields[i]->_variableName.c_str());
		}
		first = false;
	}
	insert_text.append(L")");

	hStatement = options->_efallib->Prepare(hSession, insert_text.c_str());
	if (options->_verbose) {
		if (hStatement == 0) wprintf(L"Failure to prepare statement [%ls]\n", insert_text.c_str());
		else wprintf(L"Statement [%ls] prepared successfully\n", insert_text.c_str());
	}

	return hStatement;
}
EFALHANDLE Transaction::getSplitStatement(const wchar_t * value)
{
	std::wstring table_alias = _options->_output_alias.c_str();
	table_alias.append(L"_");
	table_alias.append(value);
	for (size_t i = 0, n = table_alias.length(); i < n; i++) {
		if (table_alias[i] == L' ') table_alias[i] = L'_';
		if (table_alias[i] == L'.') table_alias[i] = L'_';
		if (table_alias[i] == L'/') table_alias[i] = L'_';
		if (table_alias[i] == L'\\') table_alias[i] = L'_';
	}
	EFALHANDLE hTable = _options->_efallib->GetTableHandle(_hSession, table_alias.c_str());
	if (hTable != 0) {
		for (size_t i = 0, n = _vctSplitTables.size(); i < n; i++) {
			if (_vctSplitTables[i] == hTable) {
				return _vctSplitStmtInserts[i];
			}
		}
	}
	else {
		EFALHANDLE hTable = createTable(_options, _hSession, this->_hTable, table_alias.c_str(), true);
		if (hTable == 0) {
			wprintf(L"Unable to create table.\n");
		}
		else {
			EFALHANDLE hStmtInsert = createStatement(_options, _hSession, hTable);
			if (hStmtInsert == 0) {
				wprintf(L"Unable to create insert statement.\n");
				if (_options->_verbose) {
					dumpSession(_options);
				}
				_options->_efallib->CloseTable(_hSession, hTable);
				hTable = 0;
			}
			else {
				_options->_efallib->BeginWriteAccess(_hSession, hTable);
				_vctSplitTables.push_back(hTable);
				_vctSplitStmtInserts.push_back(hStmtInsert);
				return hStmtInsert;
			}
		}
	}
	return _hStmtInsert; // if all else fails, insert into the unsplit table
}

static void trim(const wchar_t * & txt) {
	while (*txt == (wchar_t)L' ') txt++;
}

/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*= */
/*                              TRANSACTION LEVEL FUNCTIONS                                         */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*= */
long Transaction::ExecuteInsert()
{
	long nrecs = 0;
	if (nullptr != _split_on) {
		if (_options->_efallib->GetVariableIsNull(_hSession, _split_on->_variableName.c_str())) {
			nrecs = _options->_efallib->ExecuteInsert(_hSession, _hStmtInsert);
		}
		else {
			std::wstring str = _options->_efallib->GetVariableValueString(_hSession, _split_on->_variableName.c_str());
			const wchar_t * value = str.c_str();
			trim(value);
			if (wcslen(value) == 0) {
				nrecs = _options->_efallib->ExecuteInsert(_hSession, _hStmtInsert);
			}
			else {
				EFALHANDLE hStmtInsert = getSplitStatement(value);
				nrecs = _options->_efallib->ExecuteInsert(_hSession, hStmtInsert);
			}
		}
	}
	else {
		nrecs = _options->_efallib->ExecuteInsert(_hSession, _hStmtInsert);
	}
	if (nrecs < 0) {
		wprintf(L"Insert returned %d\n", nrecs);
		if (_options->_verbose) dumpSession(_options);
	}
	return nrecs;
}
bool Transaction::Commit()
{
	if (_options->_verbose) wprintf(L"Committing...\n");
	for (size_t i = 0, n = _vctSplitStmtInserts.size(); i < n; i++) {
		_options->_efallib->DisposeStmt(_hSession, _vctSplitStmtInserts[i]);
	}
	_vctSplitStmtInserts.clear();

	for (size_t i = 0, n = _vctSplitTables.size(); i < n; i++) {
		_options->_efallib->EndAccess(_hSession, _vctSplitTables[i]);
		_options->_efallib->CloseTable(_hSession, _vctSplitTables[i]);
	}
	_vctSplitTables.clear();

	_options->_efallib->DisposeStmt(_hSession, _hStmtInsert);
	_hStmtInsert = 0;
	_options->_efallib->EndAccess(_hSession, _hTable);
	_options->_efallib->CloseTable(_hSession, _hTable);
	_hTable = 0;

	EFALHANDLE hTable = openTable(_options, _hSession);
	if (hTable == 0) {
		if (_ownSession) {
			closeSession(_options, _hSession);
		}
		_hSession = 0;
	}
	else {
		_hTable = hTable;
		EFALHANDLE hStmtInsert = createStatement(_options, _hSession, _hTable);
		if (hStmtInsert == 0) {
			_options->_efallib->CloseTable(_hSession, _hTable);
			if (_ownSession) {
				closeSession(_options, _hSession);
			}
			_hTable = 0;
			_hSession = 0;
		}
		else {
			_hStmtInsert = hStmtInsert;
			_options->_efallib->BeginWriteAccess(_hSession, _hTable);
			return true;
		}
	}
	return false;
}

#include <Windows.h>
void Transaction::DeleteSplitFiles(ProcessingOptions * options)
{
	if ((wcslen(options->_split_on.c_str()) > 0) && (!options->_append))
	{
		std::wstring file_pattern = options->_output_path.c_str();
		file_pattern += L"\\";
		file_pattern += options->_output_alias.c_str();
		file_pattern += L"_*.*";

		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;

		hFind = FindFirstFile(file_pattern.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do {
				if (
					(_wcsicmp(FindFileData.cFileName + (wcslen(FindFileData.cFileName) - 4), L".TAB") == 0) ||
					(_wcsicmp(FindFileData.cFileName + (wcslen(FindFileData.cFileName) - 4), L".MAP") == 0) ||
					(_wcsicmp(FindFileData.cFileName + (wcslen(FindFileData.cFileName) - 4), L".DAT") == 0) ||
					(_wcsicmp(FindFileData.cFileName + (wcslen(FindFileData.cFileName) - 4), L".IND") == 0) ||
					(_wcsicmp(FindFileData.cFileName + (wcslen(FindFileData.cFileName) - 3), L".ID") == 0) ||
					(_wcsicmp(FindFileData.cFileName + (wcslen(FindFileData.cFileName) - 5), L".GPKG") == 0)
					) 
				{
					if (options->_verbose) wprintf(L"Deleting file %ls\n", FindFileData.cFileName);
					DeleteFile(FindFileData.cFileName);
				}
			} while (FindNextFile(hFind, &FindFileData));
			FindClose(hFind);
		}
	}
}
Transaction * Transaction::Begin(ProcessingOptions * options)
{
	if (options->_verbose) wprintf(L"Begin Transaction\n");
	DeleteSplitFiles(options);
	bool ownSession = false;
	EFALHANDLE hSession = options->_hSession;
	if (hSession == 0) {
		hSession = createSession(options);
		options->_hSession = hSession;
		ownSession = true;
	}
	if (hSession == 0) {
		wprintf(L"Unable to create EFAL session.\n");
	}
	else {
		createVariables(options, hSession);
		EFALHANDLE hTable = createTable(options, hSession, 0, options->_output_alias.c_str(), options->_append);
		if (hTable == 0) {
			wprintf(L"Unable to create table.\n");
			if (ownSession) {
				closeSession(options, hSession);
			}
			hSession = 0;
		}
		else {
			EFALHANDLE hStmtInsert = createStatement(options, hSession, hTable);
			if (hStmtInsert == 0) {
				wprintf(L"Unable to create insert statement.\n");
				if (options->_verbose) {
					dumpSession(options);
				}
				options->_efallib->CloseTable(hSession, hTable);
				if (ownSession) {
					closeSession(options, hSession);
				}
				hTable = 0;
				hSession = 0;
			}
			else {
				options->_efallib->BeginWriteAccess(hSession, hTable);
				Transaction * t = new Transaction(options, hSession, hTable, hStmtInsert);
				return t;
			}
		}
	}
	return nullptr;
}
void Transaction::End(Transaction * transaction)
{
	if (transaction->_options->_verbose) wprintf(L"Begin Transaction\n");

	for (size_t i = 0, n = transaction->_vctSplitStmtInserts.size(); i < n; i++) {
		transaction->_options->_efallib->DisposeStmt(transaction->_hSession, transaction->_vctSplitStmtInserts[i]);
	}
	transaction->_vctSplitStmtInserts.clear();

	for (size_t i = 0, n = transaction->_vctSplitTables.size(); i < n; i++) {
		transaction->_options->_efallib->EndAccess(transaction->_hSession, transaction->_vctSplitTables[i]);
		transaction->_options->_efallib->CloseTable(transaction->_hSession, transaction->_vctSplitTables[i]);
	}
	transaction->_vctSplitTables.clear();

	transaction->_options->_efallib->DisposeStmt(transaction->_hSession, transaction->_hStmtInsert);
	transaction->_hStmtInsert = 0;
	transaction->_options->_efallib->EndAccess(transaction->_hSession, transaction->_hTable);
	transaction->_options->_efallib->CloseTable(transaction->_hSession, transaction->_hTable);
	transaction->_hTable = 0;
	if (transaction->_ownSession) {
		closeSession(transaction->_options, transaction->_hSession);
	}
	delete transaction;
}
Transaction::Transaction(ProcessingOptions * options, EFALHANDLE hSession, EFALHANDLE hTable, EFALHANDLE hStmtInsert) :
	_options(options),
	_split_on(nullptr),
	_ownSession(options->_hSession != hSession),
	_hSession(hSession),
	_hTable(hTable),
	_hStmtInsert(hStmtInsert),
	_vctSplitTables(),
	_vctSplitStmtInserts()
{
	if (wcslen(_options->_split_on.c_str()) > 0) {
		for (size_t i = 0, n = _options->_fields.size(); i < n; i++) {
			if (_wcsicmp(_options->_split_on.c_str(), _options->_fields[i]->_fieldName.c_str()) == 0) {
				_split_on = _options->_fields[i];
				break;
			}
		}
	}
}
Transaction::~Transaction() 
{
}

#include <stdafx.h>
#include <Text2TAB.h>
#include <Transaction.h>
#include <CommandLine.h>
#include <TextProcessing.h>
#include <WkbGeometry.h>
#include <WktConvert.h>

void  countlines(ProcessingOptions * options)
{
	TextReader * reader = TextReader::Open(options->_input_file.c_str());
	if (nullptr != reader) {
		unsigned long recno = 0;
		unsigned long longest_line = 0;

		std::wstring buf;
		while (reader->read_line(buf))
		{
			recno++;
			unsigned long l = (unsigned long)buf.length();
			if (l > longest_line) longest_line = l;
		}
		wprintf(L"%lu lines. Longest line is %lu characters.\n", recno, longest_line);

		delete reader;
	}
}

EFALHANDLE prepare_styleLookup(ProcessingOptions * options)
{
	if (options->_hStyleTable != 0)
	{
		std::wstring sql = L"SELECT \"";
		sql += options->_style_table_style_column.c_str();
		sql += L"\" FROM \"";
		sql += options->_efallib->GetTableName(options->_hSession, options->_hStyleTable);
		sql += L"\" WHERE \"";
		sql += options->_style_table_join_column.c_str();
		sql += L"\" = @styleLookupKey";
		options->_efallib->CreateVariable(options->_hSession, L"@styleLookupKey");
		EFALHANDLE hLookupStatement = options->_efallib->Prepare(options->_hSession, sql.c_str());
		if (hLookupStatement == 0) {
			wprintf(L"Unable to parse style lookup query: %ls\n", sql.c_str());
			if (options->_verbose) {
				Transaction::dumpSession(options);
			}
		}
		else if (options->_verbose) {
			wprintf(L"Style lookup query: %ls\n", sql.c_str());
		}
		return hLookupStatement;
	}
	return 0;
}
bool lookupStyleValue(ProcessingOptions * options, EFALHANDLE hLookupStatement, const wchar_t * lookupKey)
{
	bool bOK = false;
	if (wcslen(lookupKey) == 0) {
		if (options->_default_style.length() > 0) {
			options->_efallib->SetVariableValueStyle(options->_hSession, L"@MI_STYLE", options->_default_style.c_str());
		}
		else {
			options->_efallib->SetVariableValueStyle(options->_hSession, L"@MI_STYLE", L"Symbol(35, 0, 10) Pen(1, 2, 0)  Brush(2, 16777215, 16777215)");
			//options->_efallib->SetVariableIsNull(options->_hSession, L"@MI_STYLE");
		}
		bOK = true;
	}
	else {
		options->_efallib->SetVariableValueString(options->_hSession, L"@styleLookupKey", lookupKey);
		EFALHANDLE hCursor = options->_efallib->ExecuteSelect(options->_hSession, hLookupStatement);
		if (hCursor != 0) {
			if (options->_efallib->FetchNext(options->_hSession, hCursor)) {
				std::wstring style;
				switch (options->_efallib->GetCursorColumnType(options->_hSession, hCursor, 0))
				{
				case Ellis::ALLTYPE_TYPE::OT_STYLE:
					style = options->_efallib->GetCursorValueStyle(options->_hSession, hCursor, 0);
					break;
				default:
					style = options->_efallib->GetCursorValueString(options->_hSession, hCursor, 0);
					break;
				}
				bOK = options->_efallib->SetVariableValueStyle(options->_hSession, L"@MI_STYLE", style.c_str());
			}
			else {
				// key is not found 
				if (options->_default_style.length() > 0) {
					options->_efallib->SetVariableValueStyle(options->_hSession, L"@MI_STYLE", options->_default_style.c_str());
				}
				else {
					options->_efallib->SetVariableValueStyle(options->_hSession, L"@MI_STYLE", L"Symbol(35, 0, 10) Pen(1, 2, 0)  Brush(2, 16777215, 16777215)");
					//options->_efallib->SetVariableIsNull(options->_hSession, L"@MI_STYLE");
				}
				bOK = true;
			}
			options->_efallib->DisposeCursor(options->_hSession, hCursor);
		}
	}
	return bOK;
}
bool  process(ProcessingOptions * options)
{
	bool bOK = true;
	bool verbose = options->_verbose;

	FILE * we = nullptr;
	if (options->_echo_error_lines) {
		_wfopen_s(&we, options->_error_file.c_str(), L"w, ccs=UTF-8");
		if (nullptr == we) return false;
	}

	TextReader * reader = TextReader::Open(options->_input_file.c_str());
	if (nullptr != reader) {
		Transaction * transaction = Transaction::Begin(options);
		if (nullptr == transaction) {
			wprintf(L"Unable to begin transaction.\n");
		}
		else {
			EFALHANDLE hLookupStatement = prepare_styleLookup(options);

			clock_t start = clock();
			bool first = true;
			long nrecs = 0, recno = 0, commitRecs = 0;

			std::wstring buf;
			while (reader->read_line(buf))
			{
				recno++;
				if (options->_has_header && first) { 
					first = false; 
					if (options->_echo_error_lines && (nullptr == we)) {
						fwprintf(we, L"Line%ls%ls\n", options->_delimiter_char.c_str(), buf.c_str());
					}
					continue; 
				} // header line...
				if (recno < options->_start_at) continue;

				if (options->_verbose  && (recno % 5000 == 0)) {
					wprintf(L"%ld\n", (long)recno);
				}

				std::vector<std::wstring> parsed_row = parse(buf.c_str(), options->_delimiter_char.c_str());

				if ((options->_start_debug_at > 0) &&
					(recno >= options->_start_debug_at) && (recno < (options->_start_debug_at + options->_debug_records))
					) {
					wprintf(L"Debug[%ld]: '%ls' parsed %ld fields\n", (long)recno, buf.c_str(), (long)parsed_row.size());
					for (size_t j = 0, m = parsed_row.size(); j < m; j++) {
						wprintf(L"   '%ls'\n", parsed_row[j].c_str());
					}
				}
				if (parsed_row.size() == options->_nbrFieldsFromData)
				{
					// There is an expectation that the list of fields in options aligns to the vector of values with 
					// the exception that there may be some fields added such as MI_Style and OBJ that are not in the values.
					// Also, some fields might not be in the destination TAB file, but we will at least bind them to variables
					// in case they are used in functions within the insert statement e.g. MI_Point(@LON,@LAT) with LON and LAT excluded
					// as standalone columns.
					for (size_t i = 0, n = parsed_row.size(); i < n; i++) {
						bool setOK = false;
						if (options->_fields[i]->_isStyleJoinColumn) {
							setOK = lookupStyleValue(options, hLookupStatement, parsed_row[i].c_str());
						}
						else {
							setOK = transaction->setVariable(options->_fields[i], parsed_row[i].c_str());
						}
						if (!setOK && options->_verbose) {
							fwprintf(stderr, L"Line %ld: Error binding value for field %ld, text: %ls\n",
								(long)recno, (long)i, parsed_row[i].c_str());
						}
					}
					if (!transaction->filterExcludeThisRecord()) {
						if (options->_no_insert || (transaction->ExecuteInsert() == 1))
						{
							nrecs++;
							commitRecs++;
						}
						if ((options->_batch_size > 0) && (commitRecs > options->_batch_size)) {
							transaction->Commit();
							commitRecs = 0;
						}
					}
				}
				else if (options->_echo_error_lines) {
					if (nullptr != we) {
						fwprintf(we, L"%ld%ls%ls\n",(long)recno, options->_delimiter_char.c_str(), buf.c_str());
					}
					if (options->_verbose || (nullptr != we)) {
						fwprintf(stderr, L"Line %ld: %ls only parsed %ld fields\n",
							(long)recno, buf.c_str(), (long)parsed_row.size());
					}
				}
				if ((options->_max_records > 0) && (options->_max_records <= nrecs)) break;

				if ((options->_start_debug_at > 0) &&
					(recno > (options->_start_debug_at + options->_debug_records)) &&
					options->_no_insert) {
					// If we're debugging and not inserting, after we print the last debug record there's no reason to continue so let's just stop.
					break;
				}
			}

			clock_t finish = clock();
			double duration = (double)(finish - start) / CLOCKS_PER_SEC;
			wprintf(L"%ls...%d records in %6.1f seconds (%d records/second)\n", options->_input_filename_no_path.c_str(), nrecs, duration, (int)(nrecs / duration));

			Transaction::End(transaction);

			bOK = (nrecs > 0);
		}
		delete reader;
	}
	else {
		bOK = false;
	}
	if (nullptr != we) {
		fclose(we);
		we = nullptr;
	}
	return bOK;
}

void quote_and_print(FILE * we, bool has_quotes, const wchar_t * s)
{
	if (!has_quotes) {
		fwprintf(we, L"%ls", s);
	}
	else {
		fwprintf(we, L"\"");
		for (size_t i = 0, n = wcslen(s); i < n; i++)
		{
			if (s[i] == L'\"')
				fwprintf(we, L"\"");
			fwprintf(we, L"%lc", s[i]);
		}
		fwprintf(we, L"\"");
	}
}

MI_UINT32 fieldName2CursorIndex(EFALLIB * efallib, const wchar_t * fieldName, EFALHANDLE hSession, EFALHANDLE hCursor)
{
	for (MI_UINT32 i = 0, n = efallib->GetCursorColumnCount(hSession, hCursor); i < n; i++)
	{
		if (_wcsicmp(fieldName, efallib->GetCursorColumnName(hSession, hCursor, i)) == 0)
			return i;
	}
	return (MI_UINT32)-1;
}
void do_export(ProcessingOptions * options)
{
	// we have already opened the table.
	// -delimiter, -has_quotes, -excludeFields or -includeFields all pertain to this
	// -output_path is the output file name (full path)
	// -start_at / -max_records
	// -has_header : should we write a header
	// -wkt, -x, and -y
	FILE * we = nullptr;
	_wfopen_s(&we, options->_output_path.c_str(), L"w, ccs=UTF-8");
	if (nullptr == we) {
		wprintf(L"Unable to open output file %ls\n", options->_output_path.c_str());
		return;
	}
	else {
		EFALLIB * efallib = options->_efallib;
		EFALHANDLE hSession = options->_hSession;
		EFALHANDLE hExportTable = options->_hExportTable;

		bool first = true;

		// If there's an OBJECT column and no wkt or x/y columns have been specified,
		// let's add them in.
		for (size_t i = 0, n = options->_fields.size(); i < n; i++)
		{
			FieldSpec * field = options->_fields[i];
			if (field->_efalType == Ellis::ALLTYPE_TYPE::OT_OBJECT) {
				field->_includeInTAB = false; // no matter what!

				if ((nullptr == field->_wktField) && (nullptr == field->_lonField) && (nullptr == field->_latField)) {
					// Get the obj column
					for (size_t j = 0, m = efallib->GetColumnCount(hSession, hExportTable); j < m; j++)
					{
						if (efallib->GetColumnType(hSession, hExportTable, (MI_UINT32)j) == Ellis::ALLTYPE_TYPE::OT_OBJECT)
						{
							size_t nPoints = efallib->GetPointObjectCount(hSession, hExportTable, (MI_UINT32)j);
							size_t nOthers = efallib->GetLineObjectCount(hSession, hExportTable, (MI_UINT32)j) +
								efallib->GetAreaObjectCount(hSession, hExportTable, (MI_UINT32)j) +
								efallib->GetMiscObjectCount(hSession, hExportTable, (MI_UINT32)j);
							if ((nOthers > 0) || (nPoints == 0)) {
								// can only do WKT
								FieldSpec * newField = new FieldSpec();
								newField->_fieldName = L"WKT";
								field->_wktField = newField;
								options->_fields.push_back(newField);
							}
							else {
								// lat/lon
								FieldSpec * xField = new FieldSpec();
								xField->_fieldName = L"X";
								field->_lonField = xField;
								options->_fields.push_back(xField);

								FieldSpec * yField = new FieldSpec();
								yField->_fieldName = L"Y";
								field->_latField = yField;
								options->_fields.push_back(yField);
							}
						}
					}
				}
			}
		}

		// Create a cursor to fetch records from the data
		std::wstring sql = L"SELECT ";
		sql += L"*";
		sql += L" FROM \"";
		sql += efallib->GetTableName(hSession, hExportTable);
		sql += L"\"";
		EFALHANDLE hCursor = efallib->Select(hSession, sql.c_str());
		if (hCursor != 0)
		{
			MI_UINT32 idxObjColumn = (MI_UINT32)-1;
			for (MI_UINT32 i = 0, n = efallib->GetCursorColumnCount(hSession, hCursor); i < n; i++)
			{
				if (efallib->GetCursorColumnType(hSession, hCursor, i) == Ellis::ALLTYPE_TYPE::OT_OBJECT)
					idxObjColumn = i;
			}

			if (options->_has_header)
			{
				// write header
				first = true;
				for (size_t i = 0, n = options->_fields.size(); i < n; i++)
				{
					FieldSpec * field = options->_fields[i];
					if (field->_includeInTAB)
					{
						if (!first) fwprintf(we, L"%ls", options->_delimiter_char.c_str());
						quote_and_print(we, options->_has_quotes, field->_fieldName.c_str());
						first = false;
					}
				}
				fwprintf(we, L"\n");
			}
			while (efallib->FetchNext(hSession, hCursor))
			{
				first = true;
				for (size_t i = 0, n = options->_fields.size(); i < n; i++)
				{
					FieldSpec * field = options->_fields[i];
					if (field->_includeInTAB)
					{
						if (!first) fwprintf(we, L"%ls", options->_delimiter_char.c_str());
						first = false;
						// Now we need to find out which field in the cursor has the right value for this field in the output.
						// The cursor may include the OBJ column which might be output as WKT or X & Y, some columns are
						// excluded, etc...
						MI_UINT32 idxCursor = fieldName2CursorIndex(efallib, field->_fieldName.c_str(), hSession, hCursor);
						if (idxCursor != (MI_UINT32)-1) {
							if (efallib->GetCursorIsNull(hSession, hCursor, idxCursor))
							{
								// output nothing
							}
							else
							{
								switch (efallib->GetCursorColumnType(hSession, hCursor, idxCursor))
								{
								case Ellis::ALLTYPE_TYPE::OT_FLOAT:
								case Ellis::ALLTYPE_TYPE::OT_DECIMAL:
								{
									double d = efallib->GetCursorValueDouble(hSession, hCursor, idxCursor);
									fwprintf(we, L"%lf", d);
								}
								break;

								case Ellis::ALLTYPE_TYPE::OT_LOGICAL:
								{
									bool bValue = efallib->GetCursorValueBoolean(hSession, hCursor, idxCursor);
									fwprintf(we, L"%ls", bValue ? L"true" : L"false");
								}
								break;

								case Ellis::ALLTYPE_TYPE::OT_SMALLINT:
								{
									int iValue = efallib->GetCursorValueInt16(hSession, hCursor, idxCursor);
									fwprintf(we, L"%d", iValue);
								}
								break;

								case Ellis::ALLTYPE_TYPE::OT_INTEGER:
								{
									int iValue = efallib->GetCursorValueInt32(hSession, hCursor, idxCursor);
									fwprintf(we, L"%d", iValue);
								}
								break;

								case Ellis::ALLTYPE_TYPE::OT_INTEGER64:
								{
									long long int iValue = efallib->GetCursorValueInt64(hSession, hCursor, idxCursor);
									fwprintf(we, L"%lld", iValue);
								}
								break;

								case Ellis::ALLTYPE_TYPE::OT_STYLE:
									quote_and_print(we, options->_has_quotes, efallib->GetCursorValueStyle(hSession, hCursor, idxCursor));
									break;

								case Ellis::ALLTYPE_TYPE::OT_DATE:
								case Ellis::ALLTYPE_TYPE::OT_DATETIME:
								case Ellis::ALLTYPE_TYPE::OT_TIME:
								case Ellis::ALLTYPE_TYPE::OT_TIMESPAN:
									// TODO: not supported
									// break; does string work?

								case Ellis::ALLTYPE_TYPE::OT_CHAR:
								default:
									quote_and_print(we, options->_has_quotes, efallib->GetCursorValueString(hSession, hCursor, idxCursor));
									break;
								}
							}
						}
						else if (field->_objField != nullptr) {
							// Must be a virtual field like WKT, X, or Y
							// For these field->_objField must be non-null and we can go get the value of the OBJ column
							// to determine what to output here.
							
							MI_UINT32 nbytes = efallib->PrepareCursorValueGeometry(hSession, hCursor, idxObjColumn);
							unsigned char * blob = new unsigned char[nbytes];
							efallib->GetData(hSession, (char *) blob, nbytes);
							WkbGeometry * wkbGeometry = WkbGeometry::CreateFromGPKGWKB(efallib->GetCursorColumnCSys(hSession, hCursor, idxObjColumn), nbytes, blob); 
							if (field->_objField->_lonField == field) {
								double x, y;
								wkbGeometry->GetPointCoordinates(x, y);
								fwprintf(we, L"%.6lf", x);
							}
							else if (field->_objField->_latField == field) {
								double x, y;
								wkbGeometry->GetPointCoordinates(x, y);
								fwprintf(we, L"%.6lf", y);
							}
							else {
								WktConvert * wktConvert = new WktConvert(efallib->GetCursorColumnCSys(hSession, hCursor, idxObjColumn));
								wkbGeometry->Parse(wktConvert);
								std::wstring wkt = wktConvert->AsWkt();
								quote_and_print(we, options->_has_quotes, wkt.c_str());
							}
						}
						first = false;
					}
				}
				fwprintf(we, L"\n");
			}
			efallib->DisposeCursor(hSession, hCursor);
		}
		else
		{
			wprintf(L"Unable to execute statement %ls.\n", sql.c_str());
		}

		fclose(we);
		we = nullptr;
	}
}


int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	EFALLIB * efallib = EFALLIB::Create();
	if ((efallib == NULL) || !efallib->HasPrepareProc()) {
		// EFAL is not present or is an older version that does not have the newer methods we depend upon
		return -1;
	}

	ProcessingOptions * options = readCommandLine(efallib, argc, argv);
	if (nullptr != options) {

		if (options->_isExport) {
			do_export(options);
		}
		else if (options->_linecount_only) {
			countlines(options);
		}
		else if (!options->_report_only) {
			process(options);
		}

	}
	return 0;
}

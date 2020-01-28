#include <stdafx.h>
#include <CommandLine.h>
#include <TextProcessing.h>
#include <EFALUtilities.h>

bool sample_text(ProcessingOptions * options)
{
	bool bOK = false;
	long nrecs = 0;

	TextReader * reader = TextReader::Open(options->_input_file.c_str());
	if (nullptr != reader) {
		std::wstring buf;
		while (reader->read_line(buf))
		{
			options->_sampleRecords.push_back(buf.c_str());
			if (options->_sampleRecords.size() > options->_sample_size) break;
		}
		delete reader;
		bOK = true;
	}
	if (bOK) {
		// ---------------------------------------------
		// Now detect the type of delimiter
		// ---------------------------------------------
		if (wcscmp(options->_delimiter.c_str(), L"COMMA") == 0) {
			options->_delimiter_char = L",";
		}
		else if (wcscmp(options->_delimiter.c_str(), L"TAB") == 0) {
			options->_delimiter_char = L"\t";
		}
		else if (wcscmp(options->_delimiter.c_str(), L"PIPE") == 0) {
			options->_delimiter_char = L"|";
		}
		else { // DETECT
			int nbrTabs = 0, nbrPipes = 0, nbrCommas = 0, nbrQuotes = 0;
			for (size_t i = 0, n = options->_sampleRecords.size(); i < n; i++)
			{
				int nbrTabsThisLine = 0, nbrPipesThisLine = 0, nbrCommasThisLine = 0, nbrQuotesThisLine = 0;
				bool in_quote = false, last_char_was_quote = false;

				for (size_t j = 0, m = options->_sampleRecords[i].length(); j < m; j++) {
					wchar_t ch = options->_sampleRecords[i][j];
					if (ch == L'"') {
						nbrQuotesThisLine++;
						// skip the contents within this quote (e.g. it MAY contain a PIPE or COMMA and we don't want to count it as a delimiter)
						for (j++; j < m; j++) {
							wchar_t ch2 = options->_sampleRecords[i][j];
							if (ch2 != L'"') continue;
							if (((j + 1) < m) && (options->_sampleRecords[i][j + 1] == L'"'))
								j++; // "we something like ""this"
							else
								break;
						}
					}
					else {
						if (ch == L',') nbrCommasThisLine++;
						if (ch == L'|') nbrPipesThisLine++;
						if (ch == L'\t') nbrTabsThisLine++;
					}
				}
				if (i == 0) {
					nbrTabs = nbrTabsThisLine;
					nbrPipes = nbrPipesThisLine;
					nbrCommas = nbrCommasThisLine;
					// Ignore quotes on first line, it might be a header line
				}
				else {
					if (nbrTabsThisLine != nbrTabs) nbrTabs = 0;
					if (nbrPipesThisLine != nbrPipes) nbrPipes = 0;
					if (nbrCommasThisLine != nbrCommas) nbrCommas = 0;
					if (nbrQuotesThisLine > 0) nbrQuotes = nbrQuotesThisLine;
				}
			}
			if ((nbrPipes > 0) && (nbrPipes > nbrCommas) && (nbrPipes > nbrTabs)) { options->_delimiter = L"PIPE"; options->_delimiter_char = L"|"; }
			else if ((nbrCommas > 0) && (nbrCommas > nbrPipes) && (nbrCommas > nbrTabs)) { options->_delimiter = L"COMMA"; options->_delimiter_char = L","; }
			else if ((nbrTabs > 0) && (nbrTabs > nbrCommas) && (nbrTabs > nbrPipes)) { options->_delimiter = L"TAB"; options->_delimiter_char = L"\t"; }
			else {
				wprintf(L"Unable to determine the delimiter.\n");
				bOK = false;
			}
			if (!options->_has_quotes && (nbrQuotes > 0)) options->_has_quotes = true;
		}
	}
	if (bOK && (options->_fields.size() == 0)) {
		// ---------------------------------------------
		// Create the fields
		// ---------------------------------------------
		std::vector<std::wstring> parsed_first_row = parse(options->_sampleRecords[0].c_str(), options->_delimiter_char.c_str());
		size_t nFields = parsed_first_row.size();
		for (size_t i = 0; i < nFields; i++)
		{
			FieldSpec * field = new FieldSpec();
			field->_efalType = Ellis::ALLTYPE_TYPE::OT_CHAR;
			options->_fields.push_back(field);
		}
		options->_nbrFieldsFromData = options->_fields.size();

		if (options->_has_header) {
			for (size_t i = 0; i < options->_fields.size(); i++)
			{
				std::wstring & fieldName = parsed_first_row[i];
				if (fieldName.length() > 31) {
					fieldName = fieldName.substr(0, 31).c_str();
				}
				options->_fields[i]->_fieldName = fieldName.c_str();
			}
		}
		else {
			for (size_t i = 0; i < options->_fields.size(); i++)
			{
				wchar_t fname[32];
				swprintf_s(fname, sizeof(fname) / sizeof(wchar_t), L"COL%d", (int)(i + 1));
				options->_fields[i]->_fieldName = fname;
			}
		}
		// ---------------------------------------------
		// Inspect the sample rows to detect the data type and widths
		// ---------------------------------------------
		for (size_t i = 1, n = options->_sampleRecords.size(); i < n; i++)
		{
			std::vector<std::wstring> parsed = parse(options->_sampleRecords[i].c_str(), options->_delimiter_char.c_str());
			for (size_t j = 0, m = parsed.size(); j < m; j++)
			{
				FieldSpec * field = options->_fields[j];
				if ((int)parsed[j].length() > field->_maxWidth) field->_maxWidth = (int)parsed[j].length();
				if (field->_couldBeDouble || field->_couldBeInt) {
					for (size_t k = 0, l = parsed[j].length(); k < l; k++)
					{
						if (nullptr == wcschr(L"0123456789-", parsed[j][k])) field->_couldBeInt = false;
						if (nullptr == wcschr(L"0123456789-+.eE", parsed[j][k])) field->_couldBeDouble = false;
					}
				}
			}
		}
		for (size_t j = 0, m = options->_fields.size(); j < m; j++)
		{
			FieldSpec * field = options->_fields[j];
			if (field->_couldBeInt) field->_efalType = Ellis::ALLTYPE_TYPE::OT_INTEGER;
			else if (field->_couldBeDouble) field->_efalType = Ellis::ALLTYPE_TYPE::OT_FLOAT;
			else {
				field->_widthTAB = field->_maxWidth;
				if (field->_widthTAB > 254) field->_widthTAB = 254;
			}
		}
	}
	return bOK;
}

void print_args(int argc, wchar_t *argv[])
{
	if (argc > 1) {
		wprintf(L"%ls\n", argv[0]);
		for (int i = 1; i < argc; i++) {
			wprintf(L"   %ls\n", argv[i]);
		}
	}
}
void print_options(ProcessingOptions * options)
{
	wprintf(L"\n");
	wprintf(L"Environment determined from command line and sampling of input text:\n");
	wprintf(L"Action: %ls\n",
		(options->_isExport ? L"Export" : 
			(options->_linecount_only ? L"Line Count" : 
				(options->_report_only ? L"Report Only" : L"Process"))));
	wprintf(L"   \n");
	wprintf(L"Input:\n");
	wprintf(L"   -input_file:%ls\n", options->_input_file.c_str());
	wprintf(L"   -has_header:%ls\n", (options->_has_header ? L"true" : L"false"));
	wprintf(L"   -has_quotes:%ls\n", (options->_has_quotes ? L"true" : L"false"));
	wprintf(L"   -delimiter:%ls\n", options->_delimiter.c_str());
	wprintf(L"   -sample_size:%d\n", options->_sample_size);
	if (options->_filter.length() > 0) {
	wprintf(L"   -filter%ls:%ls\n", options->_filter_include ? L"_include" : L"_exclude", options->_filter.c_str());
	}
	else {
		wprintf(L"   No filter applied\n");
	}
	wprintf(L"   \n");
	wprintf(L"Fields:   \n");
	if (options->_fields.size() == 0) {
		wprintf(L"   NONE\n");
	}
	else {
		int wName = 0;
		for (size_t i = 0, n = options->_fields.size(); i < n; i++) {
			if (wcslen(options->_fields[i]->_fieldName.c_str()) > wName) wName = (int)wcslen(options->_fields[i]->_fieldName.c_str());
		}
		wprintf(L"      %-*.*ls %-9.9ls %-7.7ls %9ls %9ls %-12.12ls %-31.31ls\n",
			wName, wName, L"Name", L"TYPE", L"Indexed", L"TAB Width", L"MAX Width", L"IncludeInTAB", L"Variable Name");
		wprintf(L"      %-*.*ls %-9.9ls %-7.7ls %9ls %9ls %-12.12ls %-31.31ls\n",
			wName, wName, L"----------------", L"---------", L"-------", L"---------", L"---------", L"------------", L"----------------");
		for (size_t i = 0, n = options->_fields.size(); i < n; i++) {
			std::wstring str_alltype_type = EFALUtilities::alltype_type2str(options->_fields[i]->_efalType);
			wprintf(L"      %-*.*ls %-9.9ls %-7.7ls %9d %9d %-12.12ls %-31.31ls\n",
				wName, wName, options->_fields[i]->_fieldName.c_str(),
				str_alltype_type.c_str(),
				(options->_fields[i]->_indexed ? L"true" : L"false"),
				options->_fields[i]->_widthTAB,
				options->_fields[i]->_maxWidth,
				(options->_fields[i]->_includeInTAB ? L"true" : L"false"),
				options->_fields[i]->_variableName.c_str());
		}
	}
	wprintf(L"   \n");
	wprintf(L"Output file: \n");
	wprintf(L"   -output_path:%ls\n", options->_output_path.c_str());
	wprintf(L"   -output_format:%ls\n", options->_output_format.c_str());
	wprintf(L"   -output_alias:%ls\n", options->_output_alias.c_str());
	wprintf(L"   -output_gpkg_dbname:%ls\n", options->_output_gpkg_dbname.c_str());
	wprintf(L"   -output_charset:%ls\n", EFALUtilities::charset2str(options->_output_charset).c_str());
	wprintf(L"   -split_on:%ls\n", options->_split_on.c_str());
	wprintf(L"  \n");
	wprintf(L"Processing options:\n");
	wprintf(L"   -verbose:%ls\n", (options->_verbose ? L"true" : L"false"));
	wprintf(L"   -start_at:%d\n", options->_start_at);
	wprintf(L"   -max_records:%d\n", options->_max_records);
	wprintf(L"   -echo_error_lines:%ls\n", (options->_echo_error_lines ? L"true" : L"false"));
	wprintf(L"   -error_file:%ls\n", options->_error_file.c_str());
	wprintf(L"   -batch_size:%d\n", options->_batch_size);
	wprintf(L"   -append:%ls\n", (options->_append ? L"true" : L"false"));
	wprintf(L"   -no_insert:%ls\n", (options->_no_insert ? L"true" : L"false"));
	wprintf(L"   -start_debug_at:%d\n", options->_start_debug_at);
	wprintf(L"   -debug_records:%d\n", options->_debug_records);
}
void help(int argc, wchar_t *argv[], bool long_help)
{
	wprintf(L"\n");
	wprintf(L"This utility is designed to import delimited text files into TAB or GeoPackage format using the EFAL SDK. \n");
	wprintf(L"\n");
	wprintf(L"Command line parameters\n");
	wprintf(L"Input file parameters:\n");
	wprintf(L"   -input_file:<input filename> - this is a REQUIRED input parameter\n");
	wprintf(L"   -has_header : default is false and columns will be C1, C2, etc\n");
	wprintf(L"   -has_quotes : defaults to false but will be auto-detected\n");
	wprintf(L"   -delimiter:TAB,COMMA,PIPE,DETECT : defaults to DETECT which will use the one that's most steady looking at\n");
	wprintf(L"                                     -sample_size rows\n");
	wprintf(L"   -sample_size:<nbr> : default is 200\n");
	wprintf(L"   -filter_include:<expression> : only include records from the source that satisfy the specified expression.\n");
	wprintf(L"   -filter_exclude:<expression> : include all records from the source except those that satisfy the specified\n");
	wprintf(L"                                  expression.\n");
	wprintf(L"   -filter_include and -filter_exclude are mutually exclusive. Use -long-help for more detailed information.\n");
	wprintf(L"   \n");
	wprintf(L"Field determination parameters:   \n");
	wprintf(L"   -includeFields:C1,C2,C3 - only these fields will be included in the output TAB file. \n");
	wprintf(L"                             Do not use with -excludeFields\n");
	wprintf(L"   -excludeFields:C1,C2,C3 - these fields will be excluded in the output TAB file.\n");
	wprintf(L"                             Do not use with -includeFields\n");
	wprintf(L"   -forceChar:C1,C2,C3 - force the specified fields to be char fields even if they look like INTs or DOUBLEs\n");
	wprintf(L"   -index:C1,C2,C3 - index the specified fields\n");
	wprintf(L"   -wkt:<column name> - this column will be treated as Well Known Text strings\n");
	wprintf(L"   -x:<column name> - this column will be treated as a Longitude or X column, intended to be used with -Y\n");
	wprintf(L"   -y:<column name> - this column will be treated as a Latitude or Y column, intended to be used with -X\n");
	wprintf(L"   -style:<column name> - this column will be treated as the style column. The content of this column\n");
	wprintf(L"                          are expected to contain MapBasic strings.\n");
	wprintf(L"   \n");
	wprintf(L"Output file parameters: \n");
	wprintf(L"   -output_path:<output path with no trailing slash> (same as input file if not specified)\n");
	wprintf(L"   -output_format:NATIVE|NATIVEX|GPKG (default is NATIVEX)\n");
	wprintf(L"   -output_alias:alias (derived from input file if not specified)\n");
	wprintf(L"   -output_gpkg_dbname:<file name> - name of the gpkg file, if not specified will be same as .TAB file\n");
	wprintf(L"   -output_charset:UTF8|WLATIN1|... (default is UTF8 for NATIVEX and GPKG, WLATIN1 for NATIVE)\n");
	wprintf(L"   -default_style:<MapBasic style string>\n");
	wprintf(L"   -split_on:fieldName : Using this option will create multiple output tables using output_alias\n");
	wprintf(L"           with \"_<value>\" in the name. Use -long-help for more details about this option.\n");
	wprintf(L"  \n");
	wprintf(L"   -style_table:<path to TAB file> : specifies the pathname to a .TAB file that contains style \n");
	wprintf(L"                                     information to look up.\n");
	wprintf(L"   -style_table_join_column:<column name> : specifies the name of the column in the style table \n");
	wprintf(L"                                     that will be joined to.\n");
	wprintf(L"   -style_join_column:<column name> : specifies the name of the column in the input data used \n");
	wprintf(L"                                     to look up the style.\n");
	wprintf(L"   -style_table_style_column:<column name> : specifies the column in the lookup table from which the \n");
	wprintf(L"                                     style will be taken.\n");
	wprintf(L"  \n");
	wprintf(L"Action parameters:\n");
	wprintf(L"   -help : displays this information\n");
	wprintf(L"   -long-help : displays more detailed help\n");
	wprintf(L"   -process : This switch initiates processing. By default no actual processing will be done, \n");
	wprintf(L"              only a report will be generated.\n");
	wprintf(L"   -linecount_only : only count the input lines, do not create any tables or parse the fields.\n");
	wprintf(L"   \n");
	wprintf(L"Processing parameters:\n");
	wprintf(L"   -silent : default is false, if true the details of fields and other information detected from \n");
	wprintf(L"             the input will not be displayed.\n");
	wprintf(L"   -verbose : default is false, if true the interpreted field information and other processing \n");
	wprintf(L"              status information will be output.\n");
	wprintf(L"   -start_at:<nbr> : default is 1. Number greater than 1 will still read the first record if \n");
	wprintf(L"                     -has_headers is set\n");
	wprintf(L"   -max_records:<nbr> : default is -1 meaning process all records, otherwise process only this many records\n");
	wprintf(L"   -echo_error_lines : default is false, true will echo records that fail to parse to an error file\n");
	wprintf(L"   -error_file:<file name> - default is error.txt. If the input file has a header record, that header \n");
	wprintf(L"                             record will also be written out.\n");
	wprintf(L"   -batch_size:<nbr> : default is 1000 - number of records to write before closing and re-opening. \n");
	wprintf(L"                       0 means single batch. MapInfo TAB files do not support transactions, this option is \n");
	wprintf(L"                       mainly helpful to drive updates to file sizes in Windows explorer for monitoring \n");
	wprintf(L"                       the progress.\n");
	wprintf(L"   -no_insert : when debugging, it may be useful to do all the processing but not actually insert the records. \n");
	wprintf(L"                This option is also useful for creating an empty TAB file. The schema may then be modified \n");
	wprintf(L"                using MapInfo Pro and reprocessed by this application using the -append option.\n");
	wprintf(L"   -append : append to the destination TAB if it exists? Default is false, if the destination \n");
	wprintf(L"             TAB exists it will be overwritten.\n");
	wprintf(L"   -start_debug_at:<nbr> : default is -1. Beginning at this record, the processed text \n");
	wprintf(L"                           will be written out to the console.\n");
	wprintf(L"   -debug_records:<nbr> : default is 10, controls the number of records written to the console \n");
	wprintf(L"                          starting with -start_debug\n");
	if (long_help)
	{
		wprintf(L"   \n");
		wprintf(L"Geometry (OBJ) Column:\n");
		wprintf(L"   A geometry column will be added to the output table based on the following criteria.\n");
		wprintf(L"   1. If a column is designated on the command line to contain well known text values, that column \n");
		wprintf(L"      will be used to populate a geometry column. The WKT text column itself will be excluded from the \n");
		wprintf(L"      TAB file. The WKT is assumed to be in WGS-84 reference system.\n");
		wprintf(L"   \n");
		wprintf(L"   2. If two columns are designated on the command line to contain X and Y values, those columns will be \n");
		wprintf(L"      used to populate a geometry column with POINT objects. The X and Y columns will be excluded from the \n");
		wprintf(L"      TAB file. The X and Y values are assumed to be in WGS-84 reference system.\n");
		wprintf(L"   \n");
		wprintf(L"   3. If no WKT field as been specifically identified from the command line, a geometry column will be \n");
		wprintf(L"      auto-detected as follows.\n");
		wprintf(L"   \n");
		wprintf(L"      a. If the input contains two columns that are identified (using the -sample_size records) as containing \n");
		wprintf(L"         floating point numbers and whose names are \"X\", \"LON\", or \"LONGITUDE\" and \"Y\", \"LAT\", or \n");
		wprintf(L"         \"LATITUDE\" (case insensitive) then an OBJ column will be added and these values will be used to \n");
		wprintf(L"         construct point geometries. The values are assumed to be in WGS-84 reference system.\n");
		wprintf(L"   \n");
		wprintf(L"      b. If the input contains a column that contains string values and is named either \"WKT\" or \"WKT_GEOM\" \n");
		wprintf(L"         (case insensitive) then an OBJ column will be added and these values will be used to construct geometries \n");
		wprintf(L"         from the WKT strings. The values are assumed to be in WGS-84 reference system. \n");
		wprintf(L"   \n");
		wprintf(L"      The source longitude, latitude, and WKT input fields will be excluded from the output TAB file.\n");
		wprintf(L" \n");
		wprintf(L"   4. Exceptions to the above processing apply when a column for WKT, X, or Y is specified as NONE. \n");
		wprintf(L"      For example, -wkt:NONE on the command line will disable detection and/or inclusion of Well Known Text. \n");
		wprintf(L"      Similarly, -x:NONE and -y:NONE will disable auto-detection of X and Y columns to be treated as \n");
		wprintf(L"      POINT geometries. \n");
		wprintf(L"\n");
		wprintf(L"Style Column:\n");
		wprintf(L"   A style column will be added to the output table and populated based on the following criteria. Note \n");
		wprintf(L"   that if no style column is added, one will still exist in the table with global default values. Also \n");
		wprintf(L"   note that the output table MUST have a geometry column for any of these options to work.\n");
		wprintf(L"   1. If a column exists in the data containing MapBasic strings, that column can be interpreted as the \n");
		wprintf(L"      style column using the -style:<column name> option. The \"column name\" specified is the column name \n");
		wprintf(L"      determined from the data. If the input file has no headers, it would be something like COLn where n \n");
		wprintf(L"      is the 1-based column number. If the input file has headers, it would be the name assigned from the \n");
		wprintf(L"      header. Regardless of the name used to specify the column, the column in the output table will (must) \n");
		wprintf(L"      be named MI_STYLE.\n");
		wprintf(L"		\n");
		wprintf(L"   2. If no column is specified in the input as containing style values (-style), style values may be\n");
		wprintf(L"      looked up through another table using the -style_table and related options. A style table \n");
		wprintf(L"      is any TAB file that contains styles either associated to geometries or in a column with \n");
		wprintf(L"      MapBasic strings. The four command line parameters of -style_table, -style_table_join_column, \n");
		wprintf(L"      -style_join_column, and -style_table_style_column MUST all be specified for this option. The\n");
		wprintf(L"      style table MUST be a TAB file and must contain columns with the names specified in \n");
		wprintf(L"      style_table_join_column and style_table_style_column. Additionally, there must be a column\n");
		wprintf(L"      in the input file with a name matching style_join_column. \n");
		wprintf(L"      \n");
		wprintf(L"      Each record in the input file will take the value from the style_join_column and select the\n");
		wprintf(L"      value from style_table_style_colun in the style_table where the input value equals the \n");
		wprintf(L"      style_table_join_column value.\n");
		wprintf(L"      \n");
		wprintf(L"      Note: if there are values in the input text that are not found in the lookup table, the insert will \n");
		wprintf(L"      fail unless a default style is also specified using the -default_style parameter.\n");
		wprintf(L"		 \n");
		wprintf(L"   3. If no column is specified in the input as containing style values (-style), the command line parameter\n");
		wprintf(L"      -default_style will be used. If this option is specified, a column named MI_STYLE will be added \n");
		wprintf(L"      (if there is a geometry column) and the Map Basic string supplied as part of the -default_style will \n");
		wprintf(L"      be bound to that column in every insert.\n");
		wprintf(L"\n");
		wprintf(L"Splitting input data:\n");
		wprintf(L"   The -split-on command line parameter will cause the input data to split into multiple TAB files based on \n");
		wprintf(L"   a column value. Each record will be directed to the file whose name matches the value in the specified \n");
		wprintf(L"   field. This option is useful for splitting input files like World_Premium_Display by the field NAME_LEVEL. \n");
		wprintf(L"   For example, calling this utility for WORLD_PREMIUM_D.txt with the option -split_on:NAME_LEVEL will \n");
		wprintf(L"   create 8 TAB files:\n");
		wprintf(L"            WORLD_PREMIUM_D_LEVEL1.TAB - contains records where the NAME_LEVEL column has the value 'LEVEL1'\n");
		wprintf(L"            WORLD_PREMIUM_D_LEVEL2.TAB - contains records where the NAME_LEVEL column has the value 'LEVEL2'\n");
		wprintf(L"            WORLD_PREMIUM_D_LEVEL3.TAB - contains records where the NAME_LEVEL column has the value 'LEVEL3'\n");
		wprintf(L"            WORLD_PREMIUM_D_LEVEL4.TAB - contains records where the NAME_LEVEL column has the value 'LEVEL4'\n");
		wprintf(L"            WORLD_PREMIUM_D_LEVEL5.TAB - contains records where the NAME_LEVEL column has the value 'LEVEL5'\n");
		wprintf(L"            WORLD_PREMIUM_D_LEVEL6.TAB - contains records where the NAME_LEVEL column has the value 'LEVEL6'\n");
		wprintf(L"            WORLD_PREMIUM_D_SEA_OCEAN.TAB - contains records where the NAME_LEVEL column contains 'SEA/OCEAN'\n");
		wprintf(L"            WORLD_PREMIUM_D.TAB - contains records where the NAME_LEVEL column has no value (or empty string)\n");
		wprintf(L"Filtering:\n");
		wprintf(L"   Input records are parsed into fields with names that are either generated (e.g. COL1, COL2, etc) or \n");
		wprintf(L"   derived from the header row of the input file. The values for each record read in are bound to variables \n");
		wprintf(L"   whose name is the column name preceeded by the \"@\" sign. The field information that is displayed will \n");
		wprintf(L"   also display the variable names. The expressions specified for the filtering options must use the variable \n");
		wprintf(L"   names. For example, given the following sample input data (which is fixed width for display purposes only)\n");
		wprintf(L"\n");
		wprintf(L"      Name                                Longitude   Latitude    Type                            MiCode\n");
		wprintf(L"      CHURCH OF THE ASSUMPTION            -9.295314   52.385845   CHURCH                          10220100\n");
		wprintf(L"      CHURCH OF THE IMMACULATE CONCEPTION -9.811517   54.312611   CHURCH                          10220100\n");
		wprintf(L"      ADAMSTOWN                           -6.4698944  53.3361529  (SUB) URBAN RAILWAY STATION     10320204\n");
		wprintf(L"      GALTYMORE ROAD                      -6.31949    53.3345     LOCAL POST OFFICE               10240502\n");
		wprintf(L"      ROBSWALLS COASTAL CAR PARK          -6.1263918  53.4436279  AUTOMOBILE PARKING              10330200\n");
		wprintf(L"      CLONEA STRAND                       -7.5420831  52.096349   BEACH                           10110402\n");
		wprintf(L"      VICTORIA'S WAY                      -6.219773   53.08605    ARBORETA AND BOTANICAL GARDENS  10130212\n");
		wprintf(L"      STAR OF THE SEA CHURCH              -9.731926   53.243029   CHURCH                          10220100\n");
		wprintf(L"      MANORHAMILTON CHURCH OF IRELAND     -8.172227   54.305173   CHURCH                          10220100\n");
		wprintf(L"      SAINT PAUL'S CHURCH OF IRELAND      -7.326156   53.116217   CHURCH                          10220100\n");
		wprintf(L"\n");
		wprintf(L"   This data will be parsed into variables named @Name, @Longitude, @Latitude, @Type, and @MiCode. If only \n");
		wprintf(L"   the church records are desired in the TAB file, then they can be filtered and included using the parameter\n");
		wprintf(L"      \"-filter_include:@Type = 'CHURCH'\"\n");
		wprintf(L"   The presence of -filter_include means that all other records will be excluded from the output table except \n");
		wprintf(L"   those for which the expression evaluates to true. Similarly, if all records EXCEPT for church records were \n");
		wprintf(L"   desired, the paramter -filter_exclude would be used instead. There should only be one filter_include or \n");
		wprintf(L"   filter_exclude option. The logic may consist of AND, OR, and NOT operators as well. Note that the expression \n");
		wprintf(L"   can only refer to variable names since it is evaluated before the record is inserted into the table.\n");
		wprintf(L"\n");
		//////////0123456789-0123456789-0123456789-0123456789-0123456789-0123456789-0123456789-0123456789-0123456789-0123456789-
	}
}

ProcessingOptions * readCommandLine(EFALLIB * efallib, int argc, wchar_t *argv[])
{
	bool bOK = true;
	bool printHelp = false;
	bool long_help = false;

	ProcessingOptions * options = new ProcessingOptions();
	options->_efallib = efallib;

	/*
	* We will process basic command line options first including the input file
	* name so we can read the input file, sample the rows, and pre-determine
	* the fields. Then we'll re-process the the command line options that set properties
	* on the fields.
	*/
	for (int i = 1; bOK && (i < argc); i++)
	{
		if ((wcscmp(argv[i], L"-help") == 0) || (wcscmp(argv[i], L"-?") == 0)) {
			printHelp = true;
		}
		else if (wcscmp(argv[i], L"-long-help") == 0) {
			printHelp = true;
			long_help = true;
		}
		else if (wcscmp(argv[i], L"-silent") == 0) {
			options->_silent = true;
			options->_verbose = false;
		}
		else if (wcscmp(argv[i], L"-process") == 0) {
			options->_report_only = false;
		}
		else if (wcscmp(argv[i], L"-linecount_only") == 0) {
			options->_linecount_only = true;
		}
		else if (wcscmp(argv[i], L"-echo_error_lines") == 0) {
			options->_echo_error_lines = true;
		}
		else if (wcscmp(argv[i], L"-verbose") == 0) {
			options->_verbose = true;
			options->_silent = false;
		}
		else if (wcscmp(argv[i], L"-append") == 0) {
			options->_append = true;
		}
		else if (wcscmp(argv[i], L"-has_header") == 0) {
			options->_has_header = true;
		}
		else if (wcscmp(argv[i], L"-has_quotes") == 0) {
			options->_has_quotes = true;
		}
		else if (wcsncmp(argv[i], L"-export:", wcslen(L"-export:")) == 0) {
			options->_export_file = argv[i] + wcslen(L"-export:");
			options->_isExport = true;
		}
		else if (wcsncmp(argv[i], L"-input_file:", wcslen(L"-input_file:")) == 0) {
			options->_input_file = argv[i] + wcslen(L"-input_file:");
		}
		else if (wcsncmp(argv[i], L"-filter_include:", wcslen(L"-filter_include:")) == 0) {
			options->_filter = argv[i] + wcslen(L"-filter_include:");
			options->_filter_include = true;
		}
		else if (wcsncmp(argv[i], L"-filter_exclude:", wcslen(L"-filter_exclude:")) == 0) {
			options->_filter = argv[i] + wcslen(L"-filter_exclude:");
			options->_filter_include = false;
		}
		else if (wcsncmp(argv[i], L"-output_path:", wcslen(L"-output_path:")) == 0) {
			options->_output_path = argv[i] + wcslen(L"-output_path:");
		}
		else if (wcsncmp(argv[i], L"-output_alias:", wcslen(L"-output_alias:")) == 0) {
			options->_output_alias = argv[i] + wcslen(L"-output_alias:");
		}
		else if (wcsncmp(argv[i], L"-output_gpkg_dbname:", wcslen(L"-output_gpkg_dbname:")) == 0) {
			options->_output_gpkg_dbname = argv[i] + wcslen(L"-output_gpkg_dbname:");
		}
		else if (wcsncmp(argv[i], L"-split_on:", wcslen(L"-split_on:")) == 0) {
			options->_split_on = argv[i] + wcslen(L"-split_on:");
		}
		else if (wcsncmp(argv[i], L"-default_style:", wcslen(L"-default_style:")) == 0) {
			options->_default_style = argv[i] + wcslen(L"-default_style:");
		}
		else if (wcsncmp(argv[i], L"-style_table:", wcslen(L"-style_table:")) == 0) {
			options->_style_table = argv[i] + wcslen(L"-style_table:");
		}
		else if (wcsncmp(argv[i], L"-style_table_join_column:", wcslen(L"-style_table_join_column:")) == 0) {
			options->_style_table_join_column = argv[i] + wcslen(L"-style_table_join_column:");
		}
		else if (wcsncmp(argv[i], L"-style_join_column:", wcslen(L"-style_join_column:")) == 0) {
			options->_style_join_column = argv[i] + wcslen(L"-style_join_column:");
		}
		else if (wcsncmp(argv[i], L"-style_table_style_column:", wcslen(L"-style_table_style_column:")) == 0) {
			options->_style_table_style_column = argv[i] + wcslen(L"-style_table_style_column:");
		}
		else if (wcsncmp(argv[i], L"-error_file:", wcslen(L"-error_file:")) == 0) {
			options->_error_file = argv[i] + wcslen(L"-error_file:");
		}
		else if (wcsncmp(argv[i], L"-output_format:", wcslen(L"-output_format:")) == 0) {
			options->_output_format = argv[i] + wcslen(L"-output_format:");
		}
		else if (wcsncmp(argv[i], L"-output_charset:", wcslen(L"-output_charset:")) == 0) {
			options->_output_charset = EFALUtilities::str2charset(argv[i] + wcslen(L"-output_charset:"));
		}
		else if (wcsncmp(argv[i], L"-delimiter:", wcslen(L"-delimiter:")) == 0) {
			options->_delimiter = argv[i] + wcslen(L"-delimiter:");
		}
		else if (wcsncmp(argv[i], L"-sample_size:", wcslen(L"-sample_size:")) == 0) {
			swscanf_s(argv[i] + wcslen(L"-sample_size:"), L"%d", &(options->_sample_size));
		}
		else if (wcsncmp(argv[i], L"-batch_size:", wcslen(L"-batch_size:")) == 0) {
			swscanf_s(argv[i] + wcslen(L"-batch_size:"), L"%d", &(options->_batch_size));
		}
		else if (wcsncmp(argv[i], L"-max_records:", wcslen(L"-max_records:")) == 0) {
			swscanf_s(argv[i] + wcslen(L"-max_records:"), L"%d", &(options->_max_records));
		}
		else if (wcsncmp(argv[i], L"-start_at:", wcslen(L"-start_at:")) == 0) {
			swscanf_s(argv[i] + wcslen(L"-start_at:"), L"%d", &(options->_start_at));
		}
		else if (wcsncmp(argv[i], L"-start_debug_at:", wcslen(L"-start_debug_at:")) == 0) {
			swscanf_s(argv[i] + wcslen(L"-start_debug_at:"), L"%d", &(options->_start_debug_at));
		}
		else if (wcsncmp(argv[i], L"-debug_records:", wcslen(L"-debug_records:")) == 0) {
			swscanf_s(argv[i] + wcslen(L"-debug_records:"), L"%d", &(options->_debug_records));
		}
		else if (wcscmp(argv[i], L"-no_insert") == 0) {
			options->_no_insert = true;
		}
		// skip these for now, but validate that it' a valid argument
		else if (wcsncmp(argv[i], L"-includeFields:", wcslen(L"-includeFields:")) == 0) {}
		else if (wcsncmp(argv[i], L"-excludeFields:", wcslen(L"-excludeFields:")) == 0) {}
		else if (wcsncmp(argv[i], L"-forceChar:", wcslen(L"-forceChar:")) == 0) { }
		else if (wcsncmp(argv[i], L"-index:", wcslen(L"-index:")) == 0) {}
		else if (wcsncmp(argv[i], L"-wkt:", wcslen(L"-wkt:")) == 0) {}
		else if (wcsncmp(argv[i], L"-x:", wcslen(L"-x:")) == 0) {}
		else if (wcsncmp(argv[i], L"-y:", wcslen(L"-y:")) == 0) {}
		else if (wcsncmp(argv[i], L"-style:", wcslen(L"-style:")) == 0) {}
		else {
			wprintf(L"Invalid option %ls\n", argv[i]);
			bOK = false;
		}
	}
	if (printHelp) {
		help(argc, argv, long_help);
		delete options;
		return nullptr;
	}
	// ---------------------------------------------
	// If we're creating NATIVE tables, then we cannot use UTF.
	// ---------------------------------------------
	if ((_wcsicmp(options->_output_format.c_str(), L"NATIVE") == 0) && (
		(options->_output_charset == Ellis::MICHARSET::CHARSET_UTF8) ||
		(options->_output_charset == Ellis::MICHARSET::CHARSET_UTF16))) {
		options->_output_charset = Ellis::MICHARSET::CHARSET_WLATIN1;
	}
	// ---------------------------------------------
	// Determine output file and alias if not specified
	// ---------------------------------------------
	bool autoDetectFields = true;
	if (bOK && !options->_isExport)
	{
		wchar_t path[4096];
		wchar_t * filePart = nullptr;
		if ((0 == GetFullPathName(options->_input_file.c_str(), sizeof(path) / sizeof(wchar_t), path, &filePart)) || (filePart == nullptr))
		{
			wprintf(L"Invalid input filename (%ls)\n", options->_input_file.c_str());
			bOK = false;
		}
		else
		{
			options->_input_file = path;
			if (filePart > path) {
				*(--filePart) = (wchar_t)0;
				if (options->_output_path.length() == 0) options->_output_path = path;
				filePart++;
			}
			options->_input_filename_no_path = filePart;
			if (options->_output_alias.length() == 0) {
				wchar_t * dot = wcschr(filePart, L'.');
				if (dot != nullptr) *dot = (wchar_t)0;
				options->_output_alias = filePart;
			}
		}
	}
	// ---------------------------------------------
	// Read in a sampling of the records
	// ---------------------------------------------
	if (bOK && !options->_linecount_only && !options->_isExport)
	{
		bOK = sample_text(options);
	}
	else if (bOK && options->_isExport) {
		if (wcscmp(options->_delimiter.c_str(), L"COMMA") == 0) {
			options->_delimiter_char = L",";
		}
		else if (wcscmp(options->_delimiter.c_str(), L"TAB") == 0) {
			options->_delimiter_char = L"\t";
		}
		else if (wcscmp(options->_delimiter.c_str(), L"PIPE") == 0) {
			options->_delimiter_char = L"|";
		}
		else { // DETECT - which here means use what ever we want!
			options->_delimiter_char = L",";
		}

		// open the TAB file and create the fields
		EFALHANDLE hSession = options->_hSession;
		if (hSession == 0) {
			hSession = options->_efallib->InitializeSession(nullptr);
			options->_hSession = hSession;
		}
		if (hSession != 0) {
			EFALHANDLE hTable = options->_efallib->OpenTable(hSession, options->_export_file.c_str());
			if (hTable != 0) {
				options->_hExportTable = hTable;

				for (int idx = 0, n = options->_efallib->GetColumnCount(hSession, hTable); idx < n; ++idx) {
					const wchar_t * fname = options->_efallib->GetColumnName(hSession, hTable, idx);
					FieldSpec * field = new FieldSpec();
					field->_fieldName = fname;
					field->_includeInTAB = true; // "TAB" means "TXT" in this case...
					field->_efalType = options->_efallib->GetColumnType(hSession, hTable, idx);
					field->_widthTAB = options->_efallib->GetColumnWidth(hSession, hTable, idx);
					options->_fields.push_back(field);
				}
			}
			else {
				wprintf(L"ERROR unable to open table to export %ls\n", options->_export_file.c_str());
				bOK = false;
			}
		}
	}
	// ---------------------------------------------
	// Process the command line options that effect
	// the fields such as type override, indexed, etc.
	// ---------------------------------------------
	std::wstring wktFieldName, xFieldName, yFieldName;
	bool specificWktField = false, specificXField = false, specificYField = false;
	bool disableWkt = false, disableX = false, disableY = false;
	size_t idxStyleColumn = (size_t)-1;

	for (int i = 1; bOK && (i < argc); i++)
	{
		if (wcsncmp(argv[i], L"-includeFields:", wcslen(L"-includeFields:")) == 0) {
			// We will only be activating certain fields so first let's deactivate them all
			for (size_t j = 0, m = options->_fields.size(); j < m; j++) options->_fields[j]->_includeInTAB = false;

			std::vector<std::wstring> vctFieldNames = parse(argv[i] + wcslen(L"-includeFields:"), L",");
			for (size_t k = 0, l = vctFieldNames.size(); bOK && k < l; k++)
			{
				const wchar_t * fieldName = vctFieldNames[k].c_str();
				bool found = false;
				for (size_t j = 0, m = options->_fields.size(); bOK && !found && j < m; j++)
				{
					if (wcscmp(options->_fields[j]->_fieldName.c_str(), fieldName) == 0) {
						found = true;
						options->_fields[j]->_includeInTAB = true;
					}
				}
				if (!found)
				{
					wprintf(L"ERROR: Unknown field name '%ls' from option '%ls'\n", fieldName, argv[i]);
					bOK = false;
				}
			}

		}
		else if (wcsncmp(argv[i], L"-excludeFields:", wcslen(L"-excludeFields:")) == 0) {
			std::vector<std::wstring> vctFieldNames = parse(argv[i] + wcslen(L"-excludeFields:"), L",");
			for (size_t k = 0, l = vctFieldNames.size(); bOK && k < l; k++)
			{
				const wchar_t * fieldName = vctFieldNames[k].c_str();
				bool found = false;
				for (size_t j = 0, m = options->_fields.size(); bOK && !found && j < m; j++)
				{
					if (wcscmp(options->_fields[j]->_fieldName.c_str(), fieldName) == 0) {
						found = true;
						options->_fields[j]->_includeInTAB = false;
					}
				}
				if (!found)
				{
					wprintf(L"ERROR: Unknown field name '%ls' from option '%ls'\n", fieldName, argv[i]);
					bOK = false;
				}
			}
		}
		else if (wcsncmp(argv[i], L"-forceChar:", wcslen(L"-forceChar:")) == 0) {
			std::vector<std::wstring> vctFieldNames = parse(argv[i] + wcslen(L"-forceChar:"), L",");
			for (size_t k = 0, l = vctFieldNames.size(); bOK && k < l; k++)
			{
				const wchar_t * fieldName = vctFieldNames[k].c_str();
				bool found = false;
				for (size_t j = 0, m = options->_fields.size(); bOK && !found && j < m; j++)
				{
					if (wcscmp(options->_fields[j]->_fieldName.c_str(), fieldName) == 0) {
						found = true;
						options->_fields[j]->_efalType = Ellis::ALLTYPE_TYPE::OT_CHAR;
					}
				}
				if (!found)
				{
					wprintf(L"ERROR: Unknown field name '%ls' from option '%ls'\n", fieldName, argv[i]);
					bOK = false;
				}
			}
		}
		else if (wcsncmp(argv[i], L"-index:", wcslen(L"-index:")) == 0) {
			std::vector<std::wstring> vctFieldNames = parse(argv[i] + wcslen(L"-index:"), L",");
			for (size_t k = 0, l = vctFieldNames.size(); bOK && k < l; k++)
			{
				const wchar_t * fieldName = vctFieldNames[k].c_str();
				bool found = false;
				for (size_t j = 0, m = options->_fields.size(); bOK && !found && j < m; j++)
				{
					if (wcscmp(options->_fields[j]->_fieldName.c_str(), fieldName) == 0) {
						found = true;
						options->_fields[j]->_indexed = true;
					}
				}
				if (!found)
				{
					wprintf(L"ERROR: Unknown field name '%ls' from option '%ls'\n", fieldName, argv[i]);
					bOK = false;
				}
			}
		}
		else if (wcsncmp(argv[i], L"-wkt:", wcslen(L"-wkt:")) == 0) {
			wktFieldName = argv[i] + wcslen(L"-wkt:");
			if (wcscmp(L"NONE", wktFieldName.c_str()) != 0) {
				if (options->_isExport) {
					for (size_t j = 0, m = options->_fields.size(); bOK && j < m; j++)
					{
						if (options->_fields[j]->_efalType == Ellis::ALLTYPE_TYPE::OT_OBJECT) {
							FieldSpec * field = new FieldSpec();
							field->_fieldName = wktFieldName;
							field->_objField = options->_fields[j];
							options->_fields[j]->_wktField = field;
							options->_fields.push_back(field);
						}
					}
				}
				else {
					bool found = false;
					for (size_t j = 0, m = options->_fields.size(); bOK && !found && j < m; j++)
					{
						if (wcscmp(options->_fields[j]->_fieldName.c_str(), wktFieldName.c_str()) == 0) {
							found = true;
							specificWktField = true;
						}
					}
					if (!found)
					{
						wprintf(L"ERROR: Unknown field name '%ls' from option '%ls'\n", wktFieldName.c_str(), argv[i]);
						bOK = false;
					}
				}
			}
			else {
				disableWkt = true;
			}
		}
		else if (wcsncmp(argv[i], L"-x:", wcslen(L"-x:")) == 0) {
			xFieldName = argv[i] + wcslen(L"-x:");
			if (wcscmp(L"NONE", xFieldName.c_str()) != 0) {
				if (options->_isExport) {
					for (size_t j = 0, m = options->_fields.size(); bOK && j < m; j++)
					{
						if (options->_fields[j]->_efalType == Ellis::ALLTYPE_TYPE::OT_OBJECT) {
							FieldSpec * field = new FieldSpec();
							field->_fieldName = xFieldName;
							field->_objField = options->_fields[j];
							options->_fields[j]->_lonField = field;
							options->_fields.push_back(field);
						}
					}
				}
				else
				{
					bool found = false;
					for (size_t j = 0, m = options->_fields.size(); bOK && !found && j < m; j++)
					{
						if (wcscmp(options->_fields[j]->_fieldName.c_str(), xFieldName.c_str()) == 0) {
							found = true;
							specificXField = true;
						}
					}
					if (!found)
					{
						wprintf(L"ERROR: Unknown field name '%ls' from option '%ls'\n", xFieldName.c_str(), argv[i]);
						bOK = false;
					}
				}
			}
			else {
				disableX = true;
			}
		}
		else if (wcsncmp(argv[i], L"-y:", wcslen(L"-y:")) == 0) {
			yFieldName = argv[i] + wcslen(L"-y:");
			if (wcscmp(L"NONE", yFieldName.c_str()) != 0) {
				if (options->_isExport) {
					for (size_t j = 0, m = options->_fields.size(); bOK && j < m; j++)
					{
						if (options->_fields[j]->_efalType == Ellis::ALLTYPE_TYPE::OT_OBJECT) {
							FieldSpec * field = new FieldSpec();
							field->_fieldName = yFieldName;
							field->_objField = options->_fields[j];
							options->_fields[j]->_latField = field;
							options->_fields.push_back(field);
						}
					}
				}
				else
				{
					bool found = false;
					for (size_t j = 0, m = options->_fields.size(); bOK && !found && j < m; j++)
					{
						if (wcscmp(options->_fields[j]->_fieldName.c_str(), yFieldName.c_str()) == 0) {
							found = true;
							specificYField = true;
						}
					}
					if (!found)
					{
						wprintf(L"ERROR: Unknown field name '%ls' from option '%ls'\n", yFieldName.c_str(), argv[i]);
						bOK = false;
					}
				}
			}
			else {
				disableY = true;
			}
		}
		else if (wcsncmp(argv[i], L"-style:", wcslen(L"-style:")) == 0) {
			std::wstring styleFieldName = argv[i] + wcslen(L"-style:");
			bool found = false;
			for (size_t j = 0, m = options->_fields.size(); bOK && !found && j < m; j++)
			{
				if (wcscmp(options->_fields[j]->_fieldName.c_str(), styleFieldName.c_str()) == 0) {
					found = true;
					idxStyleColumn = j;
					options->_fields[j]->_fieldName = L"MI_STYLE";
					options->_fields[j]->_efalType = Ellis::ALLTYPE_TYPE::OT_STYLE;
				}
			}
			if (!found)
			{
				wprintf(L"ERROR: Unknown field name '%ls' from option '%ls'\n", styleFieldName.c_str(), argv[i]);
				bOK = false;
			}
		}

	}
	// ---------------------------------------------
	// If a style table is specified, validate that
	// we can find it and it has the columns specified.
	// ---------------------------------------------
	if (bOK && (idxStyleColumn == (size_t)-1) && (wcslen(options->_style_table.c_str()) > 0)) {
		if (wcslen(options->_style_table_join_column.c_str()) == 0) {
			wprintf(L"Style table specified REQUIRES -style_table_join_column.\n");
			bOK = false;
		}
		if (wcslen(options->_style_join_column.c_str()) == 0) {
			wprintf(L"Style table specified REQUIRES -_style_join_column.\n");
			bOK = false;
		}
		if (wcslen(options->_style_table_style_column.c_str()) == 0) {
			options->_style_table_style_column = L"MI_STYLE";
		}
		if (_waccess(options->_style_table.c_str(), 0) == -1) {
			// does not exist - see if we add the path of the input file if that helps
			wchar_t path[4096];
			wchar_t * filePart = nullptr;
			if ((0 != GetFullPathName(options->_input_file.c_str(), sizeof(path) / sizeof(wchar_t), path, &filePart)) || (filePart == nullptr))
			{
				if (filePart > path) {
					*filePart = (wchar_t)0;
					std::wstring f = path;
					f += options->_style_table.c_str();
					if (_waccess(options->_style_table.c_str(), 0) == -1) {
						wprintf(L"Unable to locate style table %ls\n", options->_style_table.c_str());
						bOK = false;
					}
					else {
						options->_style_table = f.c_str();
					}
				}
			}
		}
		// Now let's try to open the style TAB file so we can validate the join and style column names
		EFALHANDLE hSession = options->_hSession;
		if (hSession == 0) {
			hSession = options->_efallib->InitializeSession(nullptr);
			options->_hSession = hSession;
		}
		if (hSession != 0) {
			EFALHANDLE hTable = options->_efallib->OpenTable(hSession, options->_style_table.c_str());
			if (hTable != 0) {
				bool found_style_table_join_column = false;
				bool found_style_table_style_column = false;

				for (int idx = 0, n = options->_efallib->GetColumnCount(hSession, hTable); idx < n; ++idx) {
					const wchar_t * fname = options->_efallib->GetColumnName(hSession, hTable, idx);
					if (_wcsicmp(options->_style_table_join_column.c_str(), fname) == 0)
						found_style_table_join_column = true;
					if (_wcsicmp(options->_style_table_style_column.c_str(), fname) == 0)
						found_style_table_style_column = true;
				}
				if (!found_style_table_join_column) {
					wprintf(L"Column %ls not found in the style table.\n", options->_style_table_join_column.c_str());
					bOK = false;
				}
				if (!found_style_table_style_column) {
					wprintf(L"Column %ls not found in the style table.\n", options->_style_table_style_column.c_str());
					bOK = false;
				}
				if (!bOK) {
					options->_efallib->CloseTable(hSession, hTable);
				}
				else {
					options->_hStyleTable = hTable;
				}
			}
			else {
				wprintf(L"Unable to open style table %ls\n", options->_style_table.c_str());
				bOK = false;
			}
		}

	}

	// ---------------------------------------------
	// If we're using a style lookup table, validate 
	// that the join column in the input exists.
	// ---------------------------------------------
	if (bOK && (options->_hStyleTable != 0))
	{
		bool found = false;
		for (size_t i = 0, n = options->_fields.size(); i < n; i++)
		{
			if (_wcsicmp(options->_fields[i]->_fieldName.c_str(), options->_style_join_column.c_str()) == 0)
			{
				found = true;
				options->_fields[i]->_isStyleJoinColumn = true;
				break;
			}
		}
		if (!found) {
			wprintf(L"Style lookup column '%ls' in source data is not found.\n", options->_style_join_column.c_str());
			bOK = false;
		}
	}

	// ---------------------------------------------
	// Detect if there should be an OBJ column
	// ---------------------------------------------
	if (bOK && !options->_isExport)
	{
		FieldSpec * lat = nullptr, *lon = nullptr, *wkt = nullptr; 
		if (!disableWkt && specificWktField) {
			for (size_t i = 0, n = options->_fields.size(); i < n; i++)
			{
				if (_wcsicmp(options->_fields[i]->_fieldName.c_str(), wktFieldName.c_str()) == 0)
				{
					wkt = options->_fields[i];
				}
			}
		}
		else if (!disableX && !disableY && specificXField && specificYField) {
			for (size_t i = 0, n = options->_fields.size(); i < n; i++)
			{
				if (_wcsicmp(options->_fields[i]->_fieldName.c_str(), xFieldName.c_str()) == 0)
				{
					lon = options->_fields[i];
				}
				else if (_wcsicmp(options->_fields[i]->_fieldName.c_str(), yFieldName.c_str()) == 0)
				{
					lat = options->_fields[i];
				}
			}
		}
		else {
			for (size_t i = 0, n = options->_fields.size(); i < n; i++)
			{
				if (options->_fields[i]->_efalType == Ellis::ALLTYPE_TYPE::OT_FLOAT) {
					if (
						(_wcsicmp(options->_fields[i]->_fieldName.c_str(), L"X") == 0) ||
						(_wcsicmp(options->_fields[i]->_fieldName.c_str(), L"LON") == 0) ||
						(_wcsicmp(options->_fields[i]->_fieldName.c_str(), L"LONGITUDE") == 0)
						) {
						lon = options->_fields[i];
					}
					else if (
						(_wcsicmp(options->_fields[i]->_fieldName.c_str(), L"Y") == 0) ||
						(_wcsicmp(options->_fields[i]->_fieldName.c_str(), L"LAT") == 0) ||
						(_wcsicmp(options->_fields[i]->_fieldName.c_str(), L"LATITUDE") == 0)
						) {
						lat = options->_fields[i];
					}
				}
				else if (
					(_wcsicmp(options->_fields[i]->_fieldName.c_str(), L"WKT_GEOM") == 0) ||
					(_wcsicmp(options->_fields[i]->_fieldName.c_str(), L"WKT") == 0)
					) {
					wkt = options->_fields[i];
				}
			}
		}

		bool bAddStyleColumn = false;
		if (!disableWkt && (nullptr != wkt)) {
			FieldSpec * field = new FieldSpec();
			field->_efalType = Ellis::ALLTYPE_TYPE::OT_OBJECT;
			field->_fieldName = L"OBJ";
			field->_wktField = wkt;
			field->_csys = L"EPSG:4326";
			field->_includeInTAB = true;
			options->_fields.push_back(field);
			wkt->_includeInTAB = false;
			wkt->_isWktString = true;
			bAddStyleColumn = true;
		}
		else if ((!disableX && !disableY) && ((lon != nullptr) && (lat != nullptr))) {
			FieldSpec * field = new FieldSpec();
			field->_efalType = Ellis::ALLTYPE_TYPE::OT_OBJECT;
			field->_fieldName = L"OBJ";
			field->_latField = lat;
			field->_lonField = lon;
			field->_csys = L"EPSG:4326";
			field->_includeInTAB = true;
			options->_fields.push_back(field);
			lat->_includeInTAB = false;
			lon->_includeInTAB = false;
			bAddStyleColumn = true;
		}
		if (bAddStyleColumn) {
			// If there is a style column in the input text, then we don't need to do anything
			if (idxStyleColumn == (size_t)-1) {
				// We have either specified a default style string or we will be getting the value from a lookup step
				// Either way, add a style column to receive the values.
				FieldSpec * field = new FieldSpec();
				field->_efalType = Ellis::ALLTYPE_TYPE::OT_STYLE;
				field->_fieldName = L"MI_STYLE";
				field->_includeInTAB = true;
				options->_fields.push_back(field);
			}
		}
	}
	if (_wcsicmp(options->_output_format.c_str(), L"GPKG") == 0) {
		for (size_t i = 0, n = options->_fields.size(); i < n; i++)
		{
			if (options->_fields[i]->_efalType == Ellis::ALLTYPE_TYPE::OT_STYLE) {
				options->_fields[i]->_includeInTAB = false;
			}
		}
	}
	// ---------------------------------------------
	// Create the variable names
	// ---------------------------------------------
	if (bOK)
	{
		for (size_t i = 0, n = options->_fields.size(); i < n; i++)
		{
			FieldSpec * field = options->_fields[i];
			field->_variableName = L"@";
			field->_variableName += field->_fieldName.c_str();
		}
	}

	// ---------------------------------------------
	// If we are appending and the output table exists,
	// then repopulate the fields with the existing fields.
	// ---------------------------------------------
	if (bOK && options->_append) {
		/*
		std::wstring table_alias = options->_output_alias;
		std::wstring table_filename = options->_output_path.c_str();
		table_filename += L"\\";
		table_filename += table_alias;
		table_filename += L".TAB";

		// hmmm I don't have a session yet...
		EFALHANDLE hSession = options->_efallib->InitializeSession(nullptr);
		if (hSession != 0) {
			options->_hSession = hSession;
			EFALHANDLE hTable = options->_efallib->OpenTable(hSession, table_filename.c_str());
			if (hTable != 0) {
				for (int idx = 0, n = options->_efallib->GetColumnCount(hSession, hTable); idx < n; ++idx) {
					FieldSpec * field = new FieldSpec();
					field->_fieldName = options->_efallib->GetColumnName(hSession, hTable, idx);
					field->_efalType = options->_efallib->GetColumnType(hSession, hTable, idx);
					field->_indexed = options->_efallib->IsColumnIndexed(hSession, hTable, idx);
					field->_widthTAB = options->_efallib->GetColumnWidth(hSession, hTable, idx);
					MI_UINT32 columnDecimals = options->_efallib->GetColumnDecimals(hSession, hTable, idx);
					const wchar_t * csys = options->_efallib->GetColumnCSys(hSession, hTable, idx);
					if (nullptr != csys) {
						field->_csys = csys;
					}
					options->_fields.push_back(field);
				}

				autoDetectFields = false;
			}
		}
		*/
	}
	if (!bOK) {
		print_args(argc, argv);
		help(argc, argv, long_help);
		if (options != nullptr) {
			print_options(options);
		}
		delete options;
		options = nullptr;
	}

	if (options && !options->_silent && (options->_report_only || options->_verbose)) {
		print_options(options);
	}

	return options;
}

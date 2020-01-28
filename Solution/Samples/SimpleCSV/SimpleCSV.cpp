/* 
 * This application demonstrates how to read an input text file of addresses and creates a MapInfo TAB file.
 * The input file for this sample is hardcoded, has 4 columns containing the first and last line of an address
 * and the latitude and logitude of their locations. The data is comma delimeted as follows:
 * 	 StreetAddress,MainAddress,X,Y
 * 	 3630 SPINNAKER DR,"ANCHORAGE, AK  99516-3429",-149.829305,61.10065
 * 	 9177 JAMES BLVD,"JUNEAU, AK  99801-9617",-134.585619,58.37361
 * 	 10821 SUSHANA CIR,"EAGLE RIVER, AK  99577-8394",-149.528306,61.318491
 * 	 1624 HILTON AVE,"FAIRBANKS, AK  99701-4018",-147.753998,64.838329
 * 	 824 BROOKS AVE,"SOLDOTNA, AK  99669-8204",-151.125283,60.475465
 * 	 520 PINE AVE,"KENAI, AK  99611-7557",-151.274853,60.569625
 * 	 342 HOLLAND CIR,"KODIAK, AK  99615-7039",-152.362316,57.814804
 * 	 2750 US HIGHWAY 278 E,"HOKES BLUFF, AL  35903-7608",-85.916208,33.993934
 * 	 531 E LAKESIDE DR,"FLORENCE, AL  35630-4118",-87.618445,34.813344
 * 	 5612 11TH AVE S,"BIRMINGHAM, AL  35222-4138",-86.745331,33.527431
 * 	 2811 MONROE ST,"SELMA, AL  36701-7843",-87.016163,32.365909
 * 	 621 LANIER PL,"TUSCALOOSA, AL  35406-3049",-87.561172,33.225632
 * 	 12694 BEAR CREEK RD,"DUNCANVILLE, AL  35456-2536",-87.491348,33.117867
 *
 * This sample looks for the input file named "InputText.csv" which is part of this project.
 * The file is opened from the current directory so if debugging it is best to set the current
 * directory to $(ProjectDir). Also, EFAL.DLL must be found on the system path. If this is not 
 * already the case, the path can be modified in the debug tab of the project properties.
*/

#include "stdafx.h"

int main()
{
	/* ******************************************************************************
	* Open the input CSV text file
	* ******************************************************************************
	*/
	FILE * csv_file = nullptr;
	_wfopen_s(&csv_file, L"InputText.csv", L"r");
	if (!csv_file) {
		wprintf(L"Unable to open InputText.csv\n");
		return -1;
	}

	/* ******************************************************************************
	* Instantiate the EFAL Session. This sample application uses the dynamic binding
	* interface allowing for the EFAL API to be located on the system path and
	* dynamically loaded. 
	* ******************************************************************************
	*/
	EFALLIB * efallib = EFALLIB::Create();
	if ((efallib == NULL) || !efallib->HasPrepareProc())  {
		// EFAL is not present (or is an older version that does not have the newer methods we depend upon so don't register the driver.
		return -1;
	}
	EFALHANDLE hSession = efallib->InitializeSession(nullptr);
	if (hSession == 0) {
		return -1;
	}

	/* ******************************************************************************
	* Create the new (empty) TAB file. This sample application can do this before
	* examining the CSV data since the schema for the CSV file is already known.
	* ******************************************************************************
	*/
	const wchar_t * table_alias = L"addresses";
	const wchar_t * table_filename = L"addresses.tab";
	EFALHANDLE hMetadata = efallib->CreateNativeTableMetadata(hSession, table_alias, table_filename, Ellis::MICHARSET::CHARSET_WLATIN1);
	efallib->AddColumn(hSession, hMetadata, L"StreetAddress", Ellis::ALLTYPE_TYPE::OT_CHAR, false, 254, 0, nullptr);
	efallib->AddColumn(hSession, hMetadata, L"MainAddress", Ellis::ALLTYPE_TYPE::OT_CHAR, false, 254, 0, nullptr);
	efallib->AddColumn(hSession, hMetadata, L"X", Ellis::ALLTYPE_TYPE::OT_FLOAT, false, 0, 0, nullptr);
	efallib->AddColumn(hSession, hMetadata, L"Y", Ellis::ALLTYPE_TYPE::OT_FLOAT, false, 0, 0, nullptr);
	efallib->AddColumn(hSession, hMetadata, L"OBJ", Ellis::ALLTYPE_TYPE::OT_OBJECT, false, 0, 0, L"EPSG:4326");
	EFALHANDLE hTable = efallib->CreateTable(hSession, hMetadata);
	efallib->DestroyTableMetadata(hSession, hMetadata);

	/* ******************************************************************************
	* Before reading the input file records, we will want to call insert repeatedly.
	* TO do this efficiently, rather than building a new INSERT command with the
	* record values specified in the string, this sample will use variables that
	* contain values for each of the columns and it will create an insert statement
	* that references those variables. Then, when reading the text file, the current
	* record values will be placed in the variables and the insert statement will
	* be re-executed. This saves on parsing of the input statement for each record if input.
	* ******************************************************************************
	*/
	const wchar_t * strInsertStatement = 
		L"INSERT INTO addresses (StreetAddress, MainAddress, X, Y, OBJ) "
		L" VALUES (@StreetAddress, @MainAddress, @X, @Y, @OBJ)";
	efallib->CreateVariable(hSession, L"@StreetAddress");
	efallib->CreateVariable(hSession, L"@MainAddress");
	efallib->CreateVariable(hSession, L"@X");
	efallib->CreateVariable(hSession, L"@Y");
	efallib->CreateVariable(hSession, L"@OBJ");
	EFALHANDLE hInsertStatement = efallib->Prepare(hSession, strInsertStatement);

	/* ******************************************************************************
	* Now read the CSV file a record at a time, parse the values and update the
	* relevant variables. Then execute the prepared statement by calling ExecuteInsert.
	* ******************************************************************************
	*/
	size_t recno = 0;
	wchar_t line[1024];
	while (fgetws(line, sizeof(line) / sizeof(wchar_t), csv_file) != nullptr)
	{
		recno++;
		if (recno == 1) continue; // skip the header line

		// Parse the line into 4 fields - this is not a tutorial on parsing so this is very basic code.
		std::vector<std::wstring> data = parse(line);
		if (data.size() == 4) {
			if (data[0].length() == 0) {
				efallib->SetVariableIsNull(hSession, L"@StreetAddress");
			}
			else {
				efallib->SetVariableValueString(hSession, L"@StreetAddress", data[0].c_str());
			}
			if (data[1].length() == 0) {
				efallib->SetVariableIsNull(hSession, L"@MainAddress");
			}
			else {
				efallib->SetVariableValueString(hSession, L"@MainAddress", data[1].c_str());
			}
			if ((data[2].length() == 0) || (data[3].length() == 0)) {
				efallib->SetVariableIsNull(hSession, L"@X");
				efallib->SetVariableIsNull(hSession, L"@Y");
				efallib->SetVariableIsNull(hSession, L"@OBJ");
			}
			else {
				double x = 0.0, y = 0.0;
				swscanf_s(data[2].c_str(), L"%lf", &x);
				swscanf_s(data[3].c_str(), L"%lf", &y);
				efallib->SetVariableValueDouble(hSession, L"@X", x);
				efallib->SetVariableValueDouble(hSession, L"@Y", y);
				/*
				* Here we create a Geopackage WKB binary that represents the point.
				* Note this is overly simplistic for points only for this sample code.
				*/
				Blob * blob = nullptr;
				size_t nbytes = 0;
				CreatePointWKB(x, y, blob, nbytes);
				if (blob != nullptr) {
					efallib->SetVariableValueGeometry(hSession, L"@OBJ", (unsigned long) nbytes, blob->data, L"EPSG:4326");
					delete blob;
				}
				else {
					efallib->SetVariableIsNull(hSession, L"@OBJ");
				}
			}
			/*
			 * The variables have all been set (to values or null as appropriate), now execute the insert statement.
			*/
			efallib->ExecuteInsert(hSession, hInsertStatement);
		}
		else {
			wprintf(L"Malformed record #%lu.\n", (unsigned long)recno);
		}
	}

	/* ******************************************************************************
	* Clean up: close the file, dispose the statement and close down the session.
	* ******************************************************************************
	*/
	fclose(csv_file);
	efallib->DisposeStmt(hSession, hInsertStatement);
	efallib->CloseTable(hSession, hTable);
	efallib->DestroySession(hSession);

	return 0;
}


void CreatePointWKB(double x, double y, Blob *& pBlob, size_t& nbytes)
{
	Blob * blob = new Blob();
	blob->wkb.header.magic[0] = 0x47;
	blob->wkb.header.magic[1] = 0x50;
	blob->wkb.header.version = 0;
	blob->wkb.header.flags = 1;
	blob->wkb.header.srs_id = 0; // undefined geographic - the srs is specified separately on the variable
	blob->wkb.geometry.byteOrder = 1;
	blob->wkb.geometry.wkbType = 1;
	blob->wkb.geometry.point.x = x;
	blob->wkb.geometry.point.y = y;
	pBlob = blob;
	nbytes = sizeof(blob); // note we use a fixed size since we're only handling points in this sample code!
}

std::vector<std::wstring> parse(const wchar_t * line)
{
	std::vector<std::wstring> data;
	const wchar_t * start = line;
	bool done = false;
	while (!done) {
		done = true;
		if (*start && *start != L'\n') {
			done = false;
			const wchar_t * end = start;
			while (*end && *end != L',' && *end != L'\n') {
				if (*end == L'"') {
					// forward to next quote
					end++;
					while (*end && *end != L'"') end++;
					if (*end == L'"') end++;
				}
				else end++;
			}
			if (*start == L'"') {
				start++;
				std::wstring s(start, end - start - 1);
				data.push_back(s);
			}
			else {
				std::wstring s(start, end - start);
				data.push_back(s);
			}
			start = end;
			if (*end == L',') start++;
		}
	}
	return data;
}

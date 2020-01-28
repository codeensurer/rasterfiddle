#pragma once
#include <stdafx.h>
#include <Text2TAB.h>
#include <EFAL.h>

class Transaction
{
public:
	static Transaction * Begin(ProcessingOptions * options);
	static void End(Transaction * transaction);
	bool Commit();
	bool setVariable(FieldSpec * field, const wchar_t * varValue);
	bool filterExcludeThisRecord();
	long ExecuteInsert();
	static void dumpSession(ProcessingOptions * options);

private:
	Transaction(ProcessingOptions * options, EFALHANDLE hSession, EFALHANDLE hTable, EFALHANDLE hStmtInsert);
	~Transaction();

private:
	// Session & Variable level
	static void createVariables(ProcessingOptions * options, EFALHANDLE hSession);
	static EFALHANDLE createSession(ProcessingOptions * options);
	static void closeSession(ProcessingOptions * options, EFALHANDLE hSession);

	// Table & Statement level
	static EFALHANDLE openTable(ProcessingOptions * options, EFALHANDLE hSession);
	static EFALHANDLE createTable(ProcessingOptions * options, EFALHANDLE hSession, EFALHANDLE hBaseTable, const wchar_t * output_alias, bool append);
	static void closeTable(ProcessingOptions * options, EFALHANDLE hSession, EFALHANDLE hTable);
	static EFALHANDLE createStatement(ProcessingOptions * options, EFALHANDLE hSession, EFALHANDLE hTable);
	// Split specific
	EFALHANDLE getSplitStatement(const wchar_t * value);
	static void DeleteSplitFiles(ProcessingOptions * options);

private:
	ProcessingOptions * _options;
	FieldSpec * _split_on;
	bool _ownSession;
	EFALHANDLE _hSession;
	EFALHANDLE _hTable;
	EFALHANDLE _hStmtInsert;
	std::vector<EFALHANDLE> _vctSplitTables;
	std::vector<EFALHANDLE> _vctSplitStmtInserts;
};

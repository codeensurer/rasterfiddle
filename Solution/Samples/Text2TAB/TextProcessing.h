#pragma once
#include <Text2TAB.h>
#include <string>

class TextFile;

class TextReader
{
public:
	~TextReader();
	static TextReader * Open(std::wstring path);
	bool read_line(std::wstring& str);

private:
	TextReader(std::wstring path, TextFile * tf);

private:
	std::wstring _path;
	TextFile *   _tf;
	TCHAR *      _line_buf;
};

std::vector<std::wstring> parse(const wchar_t * str, const wchar_t * delimiter);

#include <stdafx.h>
#include <TextProcessing.h>
#include <hpsutils.h>
#include <textfile.h>

std::vector<std::wstring> parse(const wchar_t * str, const wchar_t * delimiter)
{
	std::vector<std::wstring> parsed;

	size_t sz = 1 + wcslen(str);
	wchar_t * buf = new wchar_t[sz];
	wcscpy_s(buf, sz, str);
	wchar_t * pBeg = buf, *pEnd = buf;

	for (size_t j = 0, m = wcslen(buf); j < m; j++) {
		wchar_t ch = buf[j];
		if (ch == L'"') {
			pBeg = buf + j + 1; // skip quote
			for (j++; j < m; j++) {
				wchar_t ch2 = buf[j];
				if (ch2 != L'"') continue;
				if (((j + 1) < m) && (buf[j + 1] == L'"'))
					j++; // "we something like ""this"
				else
					break;
			}
			// I should point to a quote and *should* be followed by a delimiter so let that handle it. Just null out the quote.
			buf[j] = (wchar_t)0;
		}
		else if (ch == *delimiter) {
			buf[j] = (wchar_t)0;
			std::wstring s = pBeg;
			pBeg = buf + (j + 1);

			std::size_t found = 0;
			while ((found = s.find(L"\"\"", found)) != std::wstring::npos) {
				s.replace(found, 2, L"\"");
			}

			parsed.push_back(s);
		}
	}
	std::wstring s = pBeg;
	std::size_t found = 0;
	while ((found = s.find(L"\"\"", found)) != std::wstring::npos) {
		s.replace(found, 2, L"\"");
	}
	parsed.push_back(s);
	return parsed;
}

TextReader * TextReader::Open(std::wstring path)
{
	TextFile * tf = new TextFile();
	int result = tf->Open(path.c_str(), TF_READ);
	if (result == 0) {
		return new TextReader(path, tf);
	}
	delete tf;
	return nullptr;
}

TextReader::TextReader(std::wstring path, TextFile * tf) :
	_path(path),
	_tf(tf),
	_line_buf(nullptr)
{
}
TextReader::~TextReader()
{
	if (nullptr != _tf) {
		delete _tf;
	}
	if (nullptr != _line_buf) {
		free_block(_line_buf);
	}
}
bool TextReader::read_line(std::wstring& str)
{
	bool bOK = false;

	str = L"";
	if (_tf->ReadLine(NULL, &_line_buf) >= 0) {
		str = _line_buf;
		bOK = true;
	}

	return bOK;
}

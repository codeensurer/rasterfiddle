#pragma once
#include <Text2TAB.h>

ProcessingOptions * readCommandLine(EFALLIB * efallib, int argc, wchar_t *argv[]);
void help(int argc, wchar_t *argv[], ProcessingOptions * options);


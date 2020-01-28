%{
#include "wchar.h"

int wstrlen(const wchar_t *s)
{
	int cnt = 0;
	while(*s++)
		cnt++;
	return cnt;
}
%}

%typemap(jni) wchar_t * "jstring"
%typemap(jtype) wchar_t * "String"
%typemap(jstype) wchar_t * "String"
%typemap(javadirectorin) wchar_t * "$jniinput"
%typemap(javadirectorout) wchar_t * "$javacall"


%typemap(in) wchar_t *
%{if(!$input) {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null wchar_t*");
    return $null;
  }
  const jchar *$1_pstr = jenv->GetStringChars($input, 0);
  if (!$1_pstr) return $null;
  jsize $1_len = jenv->GetStringLength($input);
  if ($1_len) {
    $1 = new wchar_t[$1_len + 1];
    for (jsize i = 0; i < $1_len; ++i) {
      $1[i] = ((wchar_t)$1_pstr[i]);
    }
    $1[$1_len] = (wchar_t)L'\0';
  }
  jenv->ReleaseStringChars($input, $1_pstr);
 %}

%typemap(directorout) wchar_t *
%{if(!$input) {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null wchar_t*");
    return $null;
  }
  const jchar *$1_pstr = jenv->GetStringChars($input, 0);
  if (!$1_pstr) return $null;
  jsize $1_len = jenv->GetStringLength($input);
  if ($1_len) {
    $result = new wchar_t[$1_len + 1];
    for (jsize i = 0; i < $1_len; ++i) {
      $result[i] = ((wchar_t)$1_pstr[i]);
    }
    $result[$1_len] = (wchar_t)L'\0';
  }
  jenv->ReleaseStringChars($input, $1_pstr);
 %}

%typemap(directorin,descriptor="Ljava/lang/String;") wchar_t * 
{
  jsize $1_len = 0;
  jchar *conv_buf = 0;
  if($1) {
   $1_len = wstrlen($1);
   conv_buf = new jchar[$1_len + 1];
   for (jsize i = 0; i < $1_len; ++i) {
     conv_buf[i] = (jchar)$1[i];
   }
   conv_buf[$1_len] = (jchar)'\0';
  }
  else {
   jsize $1_len = 1;
   jchar *conv_buf = new jchar[$1_len + 1];
   conv_buf[0] = (jchar)'\0';
  }
  $input = jenv->NewString(conv_buf, $1_len);
  delete [] conv_buf;
}

%typemap(out) wchar_t *
%{
  jsize $1_len = 0;
  jchar *conv_buf = 0;
  if($1) {
   $1_len = wstrlen($1);
   conv_buf = new jchar[$1_len + 1];
   for (jsize i = 0; i < $1_len; ++i) {
     conv_buf[i] = (jchar)$1[i];
   }
   conv_buf[$1_len] = (jchar)'\0';
  }
  else {
   jsize $1_len = 1;
   conv_buf = new jchar[$1_len];
   conv_buf[0] = (jchar)'\0';
  }
  $result = jenv->NewString(conv_buf, $1_len);
  delete [] conv_buf;
%}

%typemap(javain) wchar_t * "$javainput"

%typemap(javaout) wchar_t * {
    return $jnicall;
  }

%typemap(freearg) wchar_t * { if ($1) delete[] $1; }

%typemap(throws) wchar_t *
%{ 
   SWIG_JavaThrowException(jenv, SWIG_JavaRuntimeException, $1);
   return $null; 
%}
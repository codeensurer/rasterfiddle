/* File : JFAL.i */
%module JFAL
%include "enums.swg"
%include "wchar.i"
%include "carrays.i"
%include "cdata.i"
%include "various.i"

%pragma(java) jniclasscode=%{
  static {
    try {
         System.loadLibrary("JFAL");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }
%}


// --------------------------------------------------------------------------------
// Used for mapping the SetVariableValueBinary and SetVariableValueGeometry calls
// --------------------------------------------------------------------------------
%typemap(jtype) (MI_UINT32 nbytes, const char * value) "byte[]"
%typemap(jstype) (MI_UINT32 nbytes, const char * value) "byte[]"
%typemap(jni) (MI_UINT32 nbytes, const char * value) "jbyteArray"
%typemap(javain) (MI_UINT32 nbytes, const char * value) "$javainput"
%typemap(in,numinputs=1) (MI_UINT32 nbytes, const char * value) {
  $1 = JCALL1(GetArrayLength, jenv, $input);
  $2 = (char *)JCALL2(GetByteArrayElements, jenv, $input, NULL);
}
%typemap(freearg) (MI_UINT32 nbytes, const char * value) {
  JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte*)$2, JNI_ABORT); 
}

// --------------------------------------------------------------------------------
// Used for mapping the GetData call
//    C++ : GetData(EFALHANDLE hSession, char bytes[], size_t nBytes);
//    JNI : Java_JFALJNI_GetData(JNIEnv *jenv, jclass jcls, jlong jarg1, jbyteArray jarg2)
//    Java: GetData(long hSession, byte[] bytes)
// --------------------------------------------------------------------------------
%typemap(jni) (char bytes[], size_t nBytes) "jbyteArray"
%typemap(jstype) (char bytes[], size_t nBytes) "byte[]"
%typemap(javain) (char bytes[], size_t nBytes) "$javainput"
%typemap(jtype) (char bytes[], size_t nBytes) "byte[]"
%typemap(in,numinputs=1) (char bytes[], size_t nBytes) {
  $1 = (char*) JCALL2(GetByteArrayElements, jenv, $input, NULL);
  $2 = JCALL1(GetArrayLength, jenv, $input);
}
%typemap(freearg) (char bytes[], size_t nBytes) {
  JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte*)$1, 0); 
}

%{

#include <wchar.h>
#include <EFALAPI.h>
#include <EFAL.h>
using namespace EFAL;

%}

// --------------------------------------------------------------------------------
// Note we redefine EFALHANDLE to remove the "unsigned" which is not supported in 
// Java. Since handles in EFAL are opaque and used only to pass to the client and 
// then back to other function calls, interpretation as signed or unsigned does not 
// matter as long as the binary representation remains unchanged.
// --------------------------------------------------------------------------------
typedef long long EFALHANDLE;
typedef long long MI_UINT32;
typedef long long MI_UINT64;
typedef int MI_INT32;
typedef long long MI_INT64;
typedef short MI_INT16;
typedef int MI_UINT16;
typedef const wchar_t* (ResourceStringCallback)(const wchar_t* resourceStringName);
typedef const wchar_t* (CoordSys_PRJ2OGCWKT_Callback)(const wchar_t * szPRJString);
typedef const wchar_t* (CoordSys_OGCWKT2PRJ_Callback)(const wchar_t * szWKTString);
#define EFALFUNCTION
#define EFAL_STRUCTS_ONLY
%include "EFALAPI.h"
%include "EFAL.h"
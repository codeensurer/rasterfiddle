// This is the main DLL file.

#include <stdafx.h>

#include < vcclr.h >
#include <NFAL.h>
#include <wchar.h>


namespace MapInfo {
	namespace NFAL {
		/*************************************************************************
		* Character set stuff
		*
		* These values define what character set the system is running.
		* They are used for the system_charset and os_charset variables.
		*
		* NOTE WELL: Begining in MapInfo v3.0, we store these values into index
		* files, among other things.  DO NOT CHANGE ANY OF THE EXISTING VALUES
		* UNDER ANY CIRCUMSTANCE!  To add new charsets, append them to the end.
		*
		* The value CHARSET_NEUTRAL is used to identify a character set that we
		* do not want to perform conversions on.  This is useful if we know we
		* have portable 7-bit ASCII characters (blank through tilde), or if we
		* encounter a (single byte) character set that we don't know what else to
		* do with.  Replaces previous concept of CHARSET_UNKNOWN - not knowing
		* what the character set is is only one reason to treat it NEUTRALly.
		*************************************************************************/
		public enum class MICHARSET
		{
			CHARSET_NONE = -1,
			CHARSET_NEUTRAL = 0,         // Treat as if ASCII-7: never convert, etc.

												  // Unicode, vol.I, p. 467ff: Unicode Encoding to ISO 8859 Mappings
			CHARSET_ISO8859_1 = 1,         // ISO 8859-1 (Latin-1)
			CHARSET_ISO8859_2 = 2,         // ISO 8859-2 (Latin-2)
			CHARSET_ISO8859_3 = 3,         // ISO 8859-3 (Latin-3)
			CHARSET_ISO8859_4 = 4,         // ISO 8859-4 (Latin-4)
			CHARSET_ISO8859_5 = 5,         // ISO 8859-5 (English and Cyrillic-based)
			CHARSET_ISO8859_6 = 6,         // ISO 8859-6 (English and Arabic)
			CHARSET_ISO8859_7 = 7,         // ISO 8859-7 (English and Greek)
			CHARSET_ISO8859_8 = 8,         // ISO 8859-8 (English and Hebrew)
			CHARSET_ISO8859_9 = 9,         // ISO 8859-9 (Latin-5: Western Europe and Turkish)

													 // Unicode, vol.I, p. 519ff: Microsoft Windows Character Sets
			CHARSET_WLATIN1 = 10,         // Windows Latin-1 (Code Page 1252, a.k.a. "ANSI")
			CHARSET_WLATIN2 = 11,         // Windows Latin-2 (CP 1250)
			CHARSET_WARABIC = 12,         // Windows Arabic (CP 1256)
			CHARSET_WCYRILLIC = 13,         // Windows Cyrillic (CP 1251)
			CHARSET_WGREEK = 14,         // Windows Greek (CP 1253)
			CHARSET_WHEBREW = 15,         // Windows Hebrew (CP 1255)
			CHARSET_WTURKISH = 16,         // Windows Turkish (CP 1254)

													 // Windows Far Eastern character sets
			CHARSET_WTCHINESE = 17,         // Windows Big 5 ("Traditional": Taiwan, Hong Kong)
			CHARSET_WSCHINESE = 18,         // Windows ?? ("Simplified": China)
			CHARSET_WJAPANESE = 19,         // Windows Shift JIS X0208 (Japan)
			CHARSET_WKOREAN = 20,         // Windows KS C5601 (Korea)

#if 0	// Not supported in EFAL
													// Unicode, vol.I, p. 509ff: Unicode Encoding to Macintosh
			CHARSET_MROMAN = 21,         // Mac Standard Roman
			CHARSET_MARABIC = 22,         // Mac Arabic
			CHARSET_MGREEK = 23,         // Mac Greek: ISO 8859-7
			CHARSET_MHEBREW = 24,         // Mac Hebrew: extension of ISO 8859-8

													// Other Macintosh character sets, including Far Eastern
			CHARSET_MCENTEURO = 25,         // Mac Central European
			CHARSET_MCROATIAN = 26,         // Mac Croatian
			CHARSET_MCYRILLIC = 27,         // Mac Cyrillic
			CHARSET_MICELANDIC = 28,         // Mac Icelandic
			CHARSET_MTHAI = 29,         // Mac Thai: TIS 620-2529
			CHARSET_MTURKISH = 30,         // Mac Turkish
			CHARSET_MTCHINESE = 31,         // Mac Big 5 ("Traditional": Taiwan, Hong Kong)
			CHARSET_MJAPANESE = 32,         // Mac Shift JIS X0208 (Japan)
			CHARSET_MKOREAN = 33,         // Mac KS C5601 (Korea)
#endif

													// Unicode, vol.I, p. 536ff: Unicode to PC Code Page Mappings (Latin)
			CHARSET_CP437 = 34,          // IBM Code Page 437 ("extended ASCII")
			CHARSET_CP850 = 35,          // IBM CP 850 (Multilingual)
			CHARSET_CP852 = 36,          // IBM CP 852 (Eastern Europe)
			CHARSET_CP857 = 37,          // IBM CP 857 (Turkish)
			CHARSET_CP860 = 38,          // IBM CP 860 (Portugeuse)
			CHARSET_CP861 = 39,          // IBM CP 861 (Icelandic)
			CHARSET_CP863 = 40,          // IBM CP 863 (French Canada)
			CHARSET_CP865 = 41,          // IBM CP 865 (Norway)

												  // Unicode, vol.I, p. 546ff: Unicode to PC Code Page Mappings (Greek,Cyrillic,Arabic)
			CHARSET_CP855 = 42,          // IBM CP 855 (Cyrillic)
			CHARSET_CP864 = 43,          // IBM CP 864 (Arabic)
			CHARSET_CP869 = 44,          // IBM CP 869 (Modern Greek)

#if 0	// Not supported in EFAL
												  // Lotus proprietary character sets
			CHARSET_LICS = 45,          // Lotus International: for older Lotus files
			CHARSET_LMBCS = 46,          // Lotus MultiByte: for newer Lotus files
			CHARSET_LMBCS1 = 47,          // Lotus MultiByte group 1: newer Lotus files
			CHARSET_LMBCS2 = 48,          // Lotus MultiByte group 2: newer Lotus files

													// Another Macintosh Far Eastern character set
			CHARSET_MSCHINESE = 49,          // Macintosh ?? ("Simplified": China)

														// UNIX Far Eastern character sets
			CHARSET_UTCHINESE = 50,          // UNIX ?? ("Traditional": Taiwan, Hong Kong)
			CHARSET_USCHINESE = 51,          // UNIX ?? ("Simplified": China)
			CHARSET_UJAPANESE = 52,          // UNIX Packed EUC/JIS (Japan)
			CHARSET_UKOREAN = 53,          // UNIX ?? (Korea)
#endif
													 // More Windows code pages (introduced by Windows 95)

			CHARSET_WTHAI = 54,          // Windows Thai (CP 874)
			CHARSET_WBALTICRIM = 55,          // Windows Baltic Rim (CP 1257)
			CHARSET_WVIETNAMESE = 56,          // Windows Vietnamese (CP 1258)

			CHARSET_UTF8 = 57,          // standard UTF-8
			CHARSET_UTF16 = 58,          // standard UTF-16
		};
		/*************************************************************************
		* Data types
		*************************************************************************/
		public enum class ALLTYPE_TYPE
		{
			OT_NONE = 0,
			OT_CHAR = 1,
			OT_DECIMAL = 2,
			OT_INTEGER = 3,
			OT_SMALLINT = 4,
			OT_DATE = 5,
			OT_LOGICAL = 6,
			/*OT_GRAPHIC      = 7, this does not exist anymore as oif 11/23/99 -REF */
			OT_FLOAT = 8,
			OT_MEMO = 9,
			OT_PCHAR = 10,
			OT_PEN = 11,
			OT_BRUSH = 12,
			OT_OBJECT = 13,
			OT_SYMBOL = 14,
			OT_FONT = 15,
			OT_FLSTRING = 16,
			OT_NULL = 17,
			OT_KEY = 18, // Ellis replacement for ROWID - Holds onto an MIDataKey (not reusing ROWID in case it needs to be used for backward support)
							 //*** Alltypes that are used internally ***
			OT_KEYWORD = 20,
			OT_IDENT = 21,
			OT_SUBEDIDENT = 22, // subscripted ident - e.g. states(2)
			OT_PIDENT = 24,
			//	OT_COMPUTEDCOL	= 25, // computed column (used in expression packets)
			//	OT_BASECOLUMN	  = 26, // base table column (used in expression packets)
			OT_BINARY = 27, // used as an index type
			OT_ARRAY = 28, // array variables
			OT_STRUCT = 29, // type (struct) variables
			OT_PARAMETER = 30, // parameter referenced in subroutine
			OT_SUBSELECT = 31,
			OT_DEPSUBSELECT = 32, // dependent sub-select
			OT_VARBYVAL = 33, // flag var refs in pkts; use value
			OT_VARBYREF = 34, // flag var refs in pkts; use as parm
									//	OT_ROWID        = 35, // constant type in pkts - cur row id
			OT_STYLE = 36, //for style column which is a type of ALLSTYLE
			OT_RASTER = 37,
			OT_GRID = 38,
			OT_WMS = 39,
			OT_INTEGER64 = 40,
			OT_TIMESPAN = 41,
			OT_TIME = 42,
			OT_DATETIME = 43,
		};
		/*************************************************************************
		* Pack table operations
		*************************************************************************/
		public enum class ETablePackType {
			eTablePackTypePackGraphics = 0x01,
			eTablePackTypeRebuildGraphics = eTablePackTypePackGraphics << 1,
			eTablePackTypePackIndex = eTablePackTypePackGraphics << 2,
			eTablePackTypeRebuildIndex = eTablePackTypePackGraphics << 3,
			eTablePackTypeRemoveDeletedRecords = eTablePackTypePackGraphics << 4,
			eTablePackTypeCompactDB = eTablePackTypePackGraphics << 5,
			eTablePackTypeAll = eTablePackTypePackGraphics | eTablePackTypePackIndex | eTablePackTypeRemoveDeletedRecords, // NOTE: Does not include eTablePackTypeCompactDB
		};
		public value struct EFALDATE
		{
		public:
			int year;
			int month;
			int day;
		};

		public value struct EFALTIME
		{
		public:
			int hour;
			int minute;
			int second;
			int millisecond;
		};

		public value struct EFALDATETIME
		{
		public:
			int year;
			int month;
			int day;
			int hour;
			int minute;
			int second;
			int millisecond;
		};


		public delegate String^ ResourceStringDelegate(String^ str);
		public ref class ResourceStringDelegateCallback 
		{
		private:
			ResourceStringDelegate ^ _mResourceStringDelegate;
			wchar_t * _szBuffer = nullptr;
		public:
			ResourceStringDelegateCallback(ResourceStringDelegate ^ mResourceStringDelegate)
			{
				_mResourceStringDelegate = mResourceStringDelegate;
			}
		public:
			const wchar_t * UmResourceStringCallback(const wchar_t * sz)
			{
				if (sz == nullptr) return nullptr;
				String ^ str = _mResourceStringDelegate(gcnew String(sz));
				if (str == nullptr) return sz;
				pin_ptr<const wchar_t> wchstr = PtrToStringChars(str);
				if (_szBuffer != nullptr) { delete[] _szBuffer; _szBuffer = 0; }
				_szBuffer = new wchar_t[str->Length + 1];
				wcscpy_s(_szBuffer, str->Length + 1, wchstr);
				return wchstr;
			}
		};
		private delegate const wchar_t * UmResourceStringDelegate(const wchar_t * sz);

		public ref class NFAL
		{
		public:
			/* ***********************************************************
			* Session
			* ***********************************************************
			*/
			/* ***********************************************************
			* InitializeSession : Initializes the EFAL session and returns
			* EFALHANDLE to use in other APIs. User can pass in optional
			* ResourceStringCallback to allow client application to return
			* custom EFAL string resources. If passed as nullptr, default
			* EFAL string resources will be used.
			* ***********************************************************
			*/
			static EFALHANDLE InitializeSession(ResourceStringDelegate ^ resourceStringCallback)
			{
				EFALHANDLE hSession = 0;
				if (nullptr != resourceStringCallback)
				{
					// EFAL takes a pointer to a function that accepts a resource string identifier as a wchar_t string
					// and should return a string message for that identifier (returned as a wchar_t string). Our clients
					// are .NET CLR clients and should expect to pass a function that takes a CLR String and returns a CLR String.
					// Here we accept and argument named resourceStringCallback which is the outward facing delegate of type
					// ResourceStringDelegate which works with String arguments. We have a utility class ResourceStringDelegateCallback
					// which will hold that reference and has a native function callback (UmResourceStringCallback) which 
					// works with wchar_t arrays. Here we allocate an instance of ResourceStringDelegateCallback to hold
					// the public callback and then pass an unmanaged pointer to it's member method UmResourceStringCallback
					// to the EFAL session initialization.
					GCHandle gch = GCHandle::Alloc(resourceStringCallback);
					ResourceStringDelegateCallback ^ a = gcnew ResourceStringDelegateCallback(resourceStringCallback);
					UmResourceStringDelegate ^d = gcnew UmResourceStringDelegate(a, &ResourceStringDelegateCallback::UmResourceStringCallback);
					IntPtr ip = Marshal::GetFunctionPointerForDelegate(d);
					::ResourceStringCallback * cb = static_cast<::ResourceStringCallback *>(ip.ToPointer());
					hSession = EFAL::InitializeSession(cb);
					gch.Free();
				}
				else {
					hSession = EFAL::InitializeSession(nullptr);
				}
				return hSession;
			}
			static void DestroySession(EFALHANDLE hSession)
			{
				EFAL::DestroySession(hSession);
			}


			/* ***********************************************************
			* Variable length data retrieval (for use after calls to
			* PrepareCursorValueBinary, PrepareCursorValueGeometry,
			* PrepareVariableValueBinary, and PrepareVariableValueGeometry,
			* ***********************************************************
			*/
			static void GetData(EFALHANDLE hSession, array<Byte> ^ arr_bytes)
			{
				if (arr_bytes != nullptr)
				{
					pin_ptr<Byte> p = &arr_bytes[0];
					unsigned char * np = p;
					size_t nBytes = arr_bytes->Length;
					EFAL::GetData(hSession, (char *) np, nBytes);
				}
			}

			/* ***********************************************************
			* Error Handling
			* ***********************************************************
			*/
			static bool HaveErrors(EFALHANDLE hSession)
			{
				return EFAL::HaveErrors(hSession);
			}
			static void ClearErrors(EFALHANDLE hSession)
			{
				EFAL::ClearErrors(hSession);
			}
			static int NumErrors(EFALHANDLE hSession)
			{
				return EFAL::NumErrors(hSession);
			}
			static String ^ GetError(EFALHANDLE hSession, int ierror)
			{
				const wchar_t * szError = EFAL::GetError(hSession, ierror);
				String ^ strError = gcnew String(szError);
				return strError;
			}

			/* ***********************************************************
			* Table Catalog methods
			* ***********************************************************
			*/
			static void CloseAll(EFALHANDLE hSession)
			{
				EFAL::CloseAll(hSession);
			}
			static EFALHANDLE OpenTable(EFALHANDLE hSession, String ^ path)
			{
				if (path == nullptr) return 0;
				pin_ptr<const wchar_t> wchpath = PtrToStringChars(path);
				return EFAL::OpenTable(hSession, wchpath);
			}
			static void CloseTable(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				EFAL::CloseTable(hSession, hTable);
			}
			static bool BeginReadAccess(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				return EFAL::BeginReadAccess(hSession, hTable);
			}
			static bool BeginWriteAccess(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				return EFAL::BeginWriteAccess(hSession, hTable);
			}
			static void EndAccess(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				EFAL::EndAccess(hSession, hTable);
			}
			static unsigned long GetTableCount(EFALHANDLE hSession)
			{
				return EFAL::GetTableCount(hSession);
			}
			static EFALHANDLE GetTableHandle(EFALHANDLE hSession, unsigned long idx)
			{
				return EFAL::GetTableHandle(hSession, idx);
			}
			static EFALHANDLE GetTableHandle(EFALHANDLE hSession, String ^ alias)
			{
				if (alias == nullptr) return 0;
				pin_ptr<const wchar_t> wchalias = PtrToStringChars(alias);
				return EFAL::GetTableHandle(hSession, wchalias);
			}
			static bool SupportsPack(EFALHANDLE hSession, EFALHANDLE hTable, ETablePackType ePackType)
			{
				return EFAL::SupportsPack(hSession, hTable, Ellis::ETablePackType(ePackType));
			}
			static bool Pack(EFALHANDLE hSession, EFALHANDLE hTable, ETablePackType ePackType)
			{
				return EFAL::Pack(hSession, hTable, Ellis::ETablePackType(ePackType));
			}

			/* ***********************************************************
			* Table Metadata methods
			* ***********************************************************
			*/
			static String ^ GetTableName(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				const wchar_t * sz = EFAL::GetTableName(hSession, hTable);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static String ^ GetTableDescription(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				const wchar_t * sz = EFAL::GetTableDescription(hSession, hTable);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static String ^ GetTablePath(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				const wchar_t * sz = EFAL::GetTablePath(hSession, hTable);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static String ^ GetTableGUID(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				const wchar_t * sz = EFAL::GetTableGUID(hSession, hTable);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static MICHARSET GetTableCharset(EFALHANDLE hSession, EFALHANDLE hTable)
			{
				return (MICHARSET)EFAL::GetTableCharset(hSession, hTable);
			}
			static bool HasRaster(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::HasRaster(hSession, hTable); }
			static bool HasGrid(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::HasGrid(hSession, hTable); }
			static bool IsSeamless(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::IsSeamless(hSession, hTable); }
			static bool IsVector(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::IsVector(hSession, hTable); }
			static bool SupportsInsert(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::SupportsInsert(hSession, hTable); }
			static bool SupportsUpdate(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::SupportsUpdate(hSession, hTable); }
			static bool SupportsDelete(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::SupportsDelete(hSession, hTable); }
			static bool SupportsBeginAccess(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::SupportsBeginAccess(hSession, hTable); }
			static long GetReadVersion(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::GetReadVersion(hSession, hTable); }
			static long GetEditVersion(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::GetEditVersion(hSession, hTable); }
			static unsigned long GetColumnCount(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::GetColumnCount(hSession, hTable); }
			static String ^ GetColumnName(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr)
			{
				const wchar_t * sz = EFAL::GetColumnName(hSession, hTable, columnNbr);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static ALLTYPE_TYPE GetColumnType(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return (ALLTYPE_TYPE)EFAL::GetColumnType(hSession, hTable, columnNbr); }
			static unsigned long GetColumnWidth(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetColumnWidth(hSession, hTable, columnNbr); }
			static unsigned long GetColumnDecimals(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetColumnDecimals(hSession, hTable, columnNbr); }
			static bool IsColumnIndexed(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::IsColumnIndexed(hSession, hTable, columnNbr); }
			static bool IsColumnReadOnly(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::IsColumnReadOnly(hSession, hTable, columnNbr); }
			static String ^ GetColumnCSys(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) 
			{ 
				const wchar_t * sz = EFAL::GetColumnCSys(hSession, hTable, columnNbr); 
				return gcnew String(sz);
			}
			static Ellis::DRECT GetEntireBounds(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetEntireBounds(hSession, hTable, columnNbr); }
			static Ellis::DRECT GetDefaultView(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetDefaultView(hSession, hTable, columnNbr); }
			static unsigned long GetPointObjectCount(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetPointObjectCount(hSession, hTable, columnNbr); }
			static unsigned long GetLineObjectCount(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetLineObjectCount(hSession, hTable, columnNbr); }
			static unsigned long GetAreaObjectCount(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetAreaObjectCount(hSession, hTable, columnNbr); }
			static unsigned long GetMiscObjectCount(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetMiscObjectCount(hSession, hTable, columnNbr); }
			static bool HasZ(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::HasZ(hSession, hTable, columnNbr); }
			static bool IsZRangeKnown(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::IsZRangeKnown(hSession, hTable, columnNbr); }
			static Ellis::DRANGE GetZRange(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetZRange(hSession, hTable, columnNbr); }
			static bool HasM(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::HasM(hSession, hTable, columnNbr); }
			static bool IsMRangeKnown(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::IsMRangeKnown(hSession, hTable, columnNbr); }
			static Ellis::DRANGE GetMRange(EFALHANDLE hSession, EFALHANDLE hTable, unsigned long columnNbr) { return EFAL::GetMRange(hSession, hTable, columnNbr); }

			/* ***********************************************************
			* TAB file Metadata methods
			* ***********************************************************
			*/
			static String ^ GetMetadata(EFALHANDLE hSession, EFALHANDLE hTable, String ^ key)
			{
				if (key == nullptr) return nullptr;
				pin_ptr<const wchar_t> szKey = PtrToStringChars(key);
				const wchar_t * sz = EFAL::GetMetadata(hSession, hTable, szKey);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static EFALHANDLE EnumerateMetadata(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::EnumerateMetadata(hSession, hTable); }
			static void DisposeMetadataEnumerator(EFALHANDLE hSession, EFALHANDLE hEnumerator) { EFAL::DisposeMetadataEnumerator(hSession, hEnumerator); }
			static bool GetNextEntry(EFALHANDLE hSession, EFALHANDLE hEnumerator) { return EFAL::GetNextEntry(hSession, hEnumerator); }
			static String ^ GetCurrentMetadataKey(EFALHANDLE hSession, EFALHANDLE hEnumerator)
			{
				const wchar_t * sz = EFAL::GetCurrentMetadataKey(hSession, hEnumerator);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static String ^ GetCurrentMetadataValue(EFALHANDLE hSession, EFALHANDLE hEnumerator)
			{
				const wchar_t * sz = EFAL::GetCurrentMetadataValue(hSession, hEnumerator);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static void SetMetadata(EFALHANDLE hSession, EFALHANDLE hTable, String ^ key, String ^ value)
			{
				if (key == nullptr) return;
				pin_ptr<const wchar_t> szKey = PtrToStringChars(key);
				if (value == nullptr)
				{
					EFAL::SetMetadata(hSession, hTable, szKey, nullptr);
				}
				else
				{
					pin_ptr<const wchar_t> szValue = PtrToStringChars(value);
					EFAL::SetMetadata(hSession, hTable, szKey, szValue);
				}
			}
			static void DeleteMetadata(EFALHANDLE hSession, EFALHANDLE hTable, String ^ key)
			{
				if (key == nullptr) return;
				pin_ptr<const wchar_t> szKey = PtrToStringChars(key);
				EFAL::DeleteMetadata(hSession, hTable, szKey);
			}
			static bool WriteMetadata(EFALHANDLE hSession, EFALHANDLE hTable) { return EFAL::WriteMetadata(hSession, hTable); }

			/* ***********************************************************
			* Create Table methods
			* ***********************************************************
			*/
			// Should data source be an ENUM?
			static EFALHANDLE CreateNativeTableMetadata(EFALHANDLE hSession, String ^ tableName, String ^ tablePath, MICHARSET charset)
			{
				if (tableName == nullptr) return 0;
				if (tablePath == nullptr) return 0;
				pin_ptr<const wchar_t> szTableName = PtrToStringChars(tableName);
				pin_ptr<const wchar_t> szTablePath = PtrToStringChars(tablePath);
				return EFAL::CreateNativeTableMetadata(hSession, szTableName, szTablePath, Ellis::MICHARSET(charset));
			}
			static EFALHANDLE CreateNativeXTableMetadata(EFALHANDLE hSession, String ^ tableName, String ^ tablePath, MICHARSET charset)
			{
				if (tableName == nullptr) return 0;
				if (tablePath == nullptr) return 0;
				pin_ptr<const wchar_t> szTableName = PtrToStringChars(tableName);
				pin_ptr<const wchar_t> szTablePath = PtrToStringChars(tablePath);
				return EFAL::CreateNativeXTableMetadata(hSession, szTableName, szTablePath, Ellis::MICHARSET(charset));
			}
			static EFALHANDLE CreateGeopackageTableMetadata(EFALHANDLE hSession, String ^ tableName, String ^ tablePath, String ^ databasePath, MICHARSET charset, bool convertUnsupportedObjects)
			{
				if (tableName == nullptr) return 0;
				if (tablePath == nullptr) return 0;
				if (databasePath == nullptr) return 0;
				pin_ptr<const wchar_t> szTableName = PtrToStringChars(tableName);
				pin_ptr<const wchar_t> szTablePath = PtrToStringChars(tablePath);
				pin_ptr<const wchar_t> szDatabasePath = PtrToStringChars(databasePath);
				return EFAL::CreateGeopackageTableMetadata(hSession, szTableName, szTablePath, szDatabasePath, Ellis::MICHARSET(charset), convertUnsupportedObjects);
			}
			static void AddColumn(EFALHANDLE hSession, EFALHANDLE hTableMetadata, String ^ columnName, ALLTYPE_TYPE dataType, bool indexed, unsigned long width, unsigned long decimals, String ^ strCsys)
			{
				if (columnName == nullptr) return;
				if (strCsys == nullptr) return;
				pin_ptr<const wchar_t> szColumnName = PtrToStringChars(columnName);
				pin_ptr<const wchar_t> szCsys = PtrToStringChars(strCsys);
				EFAL::AddColumn(hSession, hTableMetadata, szColumnName, Ellis::ALLTYPE_TYPE(dataType), indexed, width, decimals, szCsys);
			}
			static EFALHANDLE CreateTable(EFALHANDLE hSession, EFALHANDLE hTableMetadata)
			{
				return EFAL::CreateTable(hSession, hTableMetadata);
			}
			static void DestroyTableMetadata(EFALHANDLE hSession, EFALHANDLE hTableMetadata)
			{
				EFAL::DestroyTableMetadata(hSession, hTableMetadata);
			}

			/* ***********************************************************
			* Create Seamless Table methods
			* ***********************************************************
			* A seamless table is a MapInfo TAB file that represents a spatial partitioning of feature
			* records across multiple component TAB file tables. Each component table must have the same
			* schema and same coordinate system. This API exposes two functions for creating a seamless
			* table. CreateSeamlessTable will create an empty seamless TAB file located in the supplied
			* tablePath. AddSeamlessComponentTable will register the specified component TAB file into
			* the seamless table. The registration entry will use the supplied bounds (mbr) unless the
			* mbr values are all zero in which case the component table will be opened and the MBR of the
			* component table data will be used.
			*/
			static EFALHANDLE CreateSeamlessTable(EFALHANDLE hSession, String ^ tablePath, String ^ csys, MICHARSET charset)
			{
				if (csys == nullptr) return 0;
				if (tablePath == nullptr) return 0;
				pin_ptr<const wchar_t> szTablePath = PtrToStringChars(tablePath);
				pin_ptr<const wchar_t> szCSys = PtrToStringChars(csys);
				return EFAL::CreateSeamlessTable(hSession, szTablePath, szCSys, Ellis::MICHARSET(charset));
			}
			static bool AddSeamlessComponentTable(EFALHANDLE hSession, EFALHANDLE hSeamlessTable, String ^ componentTablePath, Ellis::DRECT mbr)
			{
				if (componentTablePath == nullptr) return 0;
				pin_ptr<const wchar_t> szComponentTablePath = PtrToStringChars(componentTablePath);
				return EFAL::AddSeamlessComponentTable(hSession, hSeamlessTable, szComponentTablePath, mbr);
			}

			/* ***********************************************************
			* SQL and Expression methods
			* ***********************************************************
			*/
			static EFALHANDLE Select(EFALHANDLE hSession, String ^ txt)
			{
				if (txt == nullptr) return 0;
				pin_ptr<const wchar_t> sz = PtrToStringChars(txt);
				return EFAL::Select(hSession, sz);
			}
			static bool FetchNext(EFALHANDLE hSession, EFALHANDLE hCursor)
			{
				return EFAL::FetchNext(hSession, hCursor);
			}
			static void DisposeCursor(EFALHANDLE hSession, EFALHANDLE hCursor)
			{
				EFAL::DisposeCursor(hSession, hCursor);
			}
			static long Insert(EFALHANDLE hSession, String ^ txt)
			{
				if (txt == nullptr) return -1;
				pin_ptr<const wchar_t> sz = PtrToStringChars(txt);
				return EFAL::Insert(hSession, sz);
			}
			static long Update(EFALHANDLE hSession, String ^ txt)
			{
				if (txt == nullptr) return -1;
				pin_ptr<const wchar_t> sz = PtrToStringChars(txt);
				return EFAL::Update(hSession, sz);
			}
			static long Delete(EFALHANDLE hSession, String ^ txt)
			{
				if (txt == nullptr) return -1;
				pin_ptr<const wchar_t> sz = PtrToStringChars(txt);
				return EFAL::Delete(hSession, sz);
			}

			/* ***********************************************************
			* Cursor Record Methods
			* ***********************************************************
			*/
			static unsigned long GetCursorColumnCount(EFALHANDLE hSession, EFALHANDLE hCursor) { return EFAL::GetCursorColumnCount(hSession, hCursor); }
			static String ^ GetCursorColumnName(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) 
			{ 
				const wchar_t * sz = EFAL::GetCursorColumnName(hSession, hCursor, columnNbr); 
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static ALLTYPE_TYPE GetCursorColumnType(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return (ALLTYPE_TYPE)EFAL::GetCursorColumnType(hSession, hCursor, columnNbr); }
			static String ^ GetCursorColumnCSys(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) 
			{ 
				const wchar_t * sz = EFAL::GetCursorColumnCSys(hSession, hCursor, columnNbr);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static String ^ GetCursorCurrentKey(EFALHANDLE hSession, EFALHANDLE hCursor)
			{
				const wchar_t * sz = EFAL::GetCursorCurrentKey(hSession, hCursor);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static bool GetCursorIsNull(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::GetCursorIsNull(hSession, hCursor, columnNbr); }
			static String ^  GetCursorValueString(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) 
			{ 
				const wchar_t * sz = EFAL::GetCursorValueString(hSession, hCursor, columnNbr);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static bool GetCursorValueBoolean(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::GetCursorValueBoolean(hSession, hCursor, columnNbr); }
			static double GetCursorValueDouble(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::GetCursorValueDouble(hSession, hCursor, columnNbr); }
			static long long GetCursorValueInt64(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::GetCursorValueInt64(hSession, hCursor, columnNbr); }
			static long int GetCursorValueInt32(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::GetCursorValueInt32(hSession, hCursor, columnNbr); }
			static short int GetCursorValueInt16(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::GetCursorValueInt16(hSession, hCursor, columnNbr); }
			static String ^  GetCursorValueStyle(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) 
			{ 
				const wchar_t * sz = EFAL::GetCursorValueStyle(hSession, hCursor, columnNbr);
				if (sz == nullptr) return nullptr;
				return gcnew String(sz);
			}
			static unsigned long PrepareCursorValueBinary(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::PrepareCursorValueBinary(hSession, hCursor, columnNbr); }
			static unsigned long PrepareCursorValueGeometry(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::PrepareCursorValueGeometry(hSession, hCursor, columnNbr); }
			static double GetCursorValueTimespanInMilliseconds(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) { return EFAL::GetCursorValueTimespanInMilliseconds(hSession, hCursor, columnNbr); }
			static EFALDATE GetCursorValueDate(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) 
			{ 
				::EFALDATE g = EFAL::GetCursorValueDate(hSession, hCursor, columnNbr);
				EFALDATE v;
				v.year = g.year;
				v.month = g.month;
				v.day = g.day;
				return v;
			}
			static EFALTIME GetCursorValueTime(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) 
			{ 
				::EFALTIME g = EFAL::GetCursorValueTime(hSession, hCursor, columnNbr);
				EFALTIME v;
				v.hour = g.hour;
				v.minute = g.minute;
				v.second = g.second;
				v.millisecond = g.millisecond;
				return v;
			}
			static EFALDATETIME GetCursorValueDateTime(EFALHANDLE hSession, EFALHANDLE hCursor, unsigned long columnNbr) 
			{ 
				::EFALDATETIME g = EFAL::GetCursorValueDateTime(hSession, hCursor, columnNbr);
				EFALDATETIME v;
				v.year = g.year;
				v.month = g.month;
				v.day = g.day;
				v.hour = g.hour;
				v.minute = g.minute;
				v.second = g.second;
				v.millisecond = g.millisecond;
				return v;
			}

			/* ***********************************************************
			* Variable Methods
			* ***********************************************************
			*/
			static bool CreateVariable(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::CreateVariable(hSession, sz);
			}
			static void DropVariable(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				EFAL::DropVariable(hSession, sz);
			}
			static unsigned long GetVariableCount(EFALHANDLE hSession) { return EFAL::GetVariableCount(hSession); }
			static String ^ GetVariableName(EFALHANDLE hSession, unsigned long index)
			{
				const wchar_t * sz = EFAL::GetVariableName(hSession, index);
				return gcnew String(sz);
			}
			static ALLTYPE_TYPE GetVariableType(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return ALLTYPE_TYPE::OT_NONE;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return (ALLTYPE_TYPE)EFAL::GetVariableType(hSession, sz);
			}
			static ALLTYPE_TYPE SetVariableValue(EFALHANDLE hSession, String ^ name, String ^ expression)
			{
				if (name == nullptr) return ALLTYPE_TYPE::OT_NONE;
				pin_ptr<const wchar_t> szName = PtrToStringChars(name);
				if (expression == nullptr)
				{
					return (ALLTYPE_TYPE)EFAL::SetVariableValue(hSession, szName, nullptr);
				}
				else
				{
					pin_ptr<const wchar_t> szExpression = PtrToStringChars(expression);
					return (ALLTYPE_TYPE)EFAL::SetVariableValue(hSession, szName, szExpression);
				}
			}
			static bool GetVariableIsNull(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::GetVariableIsNull(hSession, sz);
			}
			static String ^  GetVariableValueString(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return nullptr;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				const wchar_t * str = EFAL::GetVariableValueString(hSession, sz);
				return gcnew String(str);
			}
			static bool GetVariableValueBoolean(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::GetVariableValueBoolean(hSession, sz);
			}
			static double GetVariableValueDouble(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return 0;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::GetVariableValueDouble(hSession, sz);
			}
			static long long GetVariableValueInt64(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return 0;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::GetVariableValueInt64(hSession, sz);
			}
			static long int GetVariableValueInt32(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return 0;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::GetVariableValueInt32(hSession, sz);
			}
			static short int GetVariableValueInt16(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return 0;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::GetVariableValueInt16(hSession, sz);
			}
			static String ^  GetVariableValueStyle(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return nullptr;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				const wchar_t * str =EFAL::GetVariableValueStyle(hSession, sz);
				return gcnew String(str);
			}
			static unsigned long PrepareVariableValueBinary(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return 0;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::PrepareVariableValueBinary(hSession, sz);
			}
			static unsigned long PrepareVariableValueGeometry(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return 0;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::PrepareVariableValueGeometry(hSession, sz);
			}
			static String ^ GetVariableColumnCSys(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return nullptr;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				const wchar_t * str = EFAL::GetVariableColumnCSys(hSession, sz);
				return gcnew String(str);
			}
			static double GetVariableValueTimespanInMilliseconds(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return 0;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::GetVariableValueTimespanInMilliseconds(hSession, sz);
			}
			static EFALDATE GetVariableValueDate(EFALHANDLE hSession, String ^ name)
			{
				EFALDATE v; v.year = v.month = v.day = 0;
				if (name == nullptr) { return v; }
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				::EFALDATE g = EFAL::GetVariableValueDate(hSession, sz);
				v.year = g.year;
				v.month = g.month;
				v.day = g.day;
				return v;
			}
			static EFALTIME GetVariableValueTime(EFALHANDLE hSession, String ^ name)
			{
				EFALTIME v;  v.hour = v.minute = v.second = v.millisecond = 0; 
				if (name == nullptr) { return v; }
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				::EFALTIME g = EFAL::GetVariableValueTime(hSession, sz);
				v.hour = g.hour;
				v.minute = g.minute;
				v.second = g.second;
				v.millisecond = g.millisecond;
				return v;
			}
			static EFALDATETIME GetVariableValueDateTime(EFALHANDLE hSession, String ^ name)
			{
				EFALDATETIME v; v.year = v.month = v.day = v.hour = v.minute = v.second = v.millisecond = 0;
				if (name == nullptr) { return v; }
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				::EFALDATETIME g = EFAL::GetVariableValueDateTime(hSession, sz);
				v.year = g.year;
				v.month = g.month;
				v.day = g.day;
				v.hour = g.hour;
				v.minute = g.minute;
				v.second = g.second;
				v.millisecond = g.millisecond;
				return v;
			}

			static bool SetVariableIsNull(EFALHANDLE hSession, String ^ name)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::SetVariableIsNull(hSession, sz);
			}
			static bool SetVariableValueString(EFALHANDLE hSession, String ^ name, String ^ value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> szName = PtrToStringChars(name);
				pin_ptr<const wchar_t> szValue = PtrToStringChars(value);
				return EFAL::SetVariableValueString(hSession, szName, szValue);
			}
			static bool SetVariableValueBoolean(EFALHANDLE hSession, String ^ name, bool value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::SetVariableValueBoolean(hSession, sz, value);
			}
			static bool SetVariableValueDouble(EFALHANDLE hSession, String ^ name, double value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::SetVariableValueDouble(hSession, sz, value);
			}
			static bool SetVariableValueInt64(EFALHANDLE hSession, String ^ name, long long value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::SetVariableValueInt64(hSession, sz, value);
			}
			static bool SetVariableValueInt32(EFALHANDLE hSession, String ^ name, long int value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::SetVariableValueInt32(hSession, sz, value);
			}
			static bool SetVariableValueInt16(EFALHANDLE hSession, String ^ name, short int value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::SetVariableValueInt16(hSession, sz, value);
			}
			static bool SetVariableValueStyle(EFALHANDLE hSession, String ^ name, String ^ value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> szName = PtrToStringChars(name);
				if (value == nullptr)
				{
					return EFAL::SetVariableIsNull(hSession, szName);
				}
				else
				{
					pin_ptr<const wchar_t> szValue = PtrToStringChars(value);
					return EFAL::SetVariableValueStyle(hSession, szName, szValue);
				}
			}
			static bool SetVariableValueBinary(EFALHANDLE hSession, String ^ name, array<Byte> ^ arr_bytes)
			{
				if ((name != nullptr) && (arr_bytes != nullptr))
				{
					pin_ptr<const wchar_t> sz = PtrToStringChars(name);
					pin_ptr<Byte> p = &arr_bytes[0];
					const unsigned char * np = p;
					unsigned long nBytes = arr_bytes->Length;
					return EFAL::SetVariableValueBinary(hSession, sz, nBytes, (const char *)np);
				}
				return false;
			}
			static bool SetVariableValueGeometry(EFALHANDLE hSession, String ^ name, array<Byte> ^ arr_bytes, String ^ strCSys)
			{
				if ((name != nullptr) && (strCSys != nullptr) && (arr_bytes != nullptr))
				{
					pin_ptr<const wchar_t> sz = PtrToStringChars(name);
					pin_ptr<const wchar_t> szCsys = PtrToStringChars(strCSys);
					pin_ptr<Byte> p = &arr_bytes[0];
					const unsigned char * np = p;
					unsigned long nBytes = arr_bytes->Length;
					return EFAL::SetVariableValueGeometry(hSession, sz, nBytes, (const char *)np, szCsys);
				}
				return false;
			}
			static bool SetVariableValueTimespanInMilliseconds(EFALHANDLE hSession, String ^ name, double value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				return EFAL::SetVariableValueTimespanInMilliseconds(hSession, sz, value);
			}
			static bool SetVariableValueDate(EFALHANDLE hSession, String ^ name, EFALDATE value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				::EFALDATE g;
				g.year = value.year;
				g.month = value.month;
				g.day = value.day;
				return EFAL::SetVariableValueDate(hSession, sz, g);
			}
			static bool SetVariableValueTime(EFALHANDLE hSession, String ^ name, EFALTIME value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				::EFALTIME g;
				g.hour = value.hour;
				g.minute = value.minute;
				g.second = value.second;
				g.millisecond = value.millisecond;
				return EFAL::SetVariableValueTime(hSession, sz, g);
			}
			static bool SetVariableValueDateTime(EFALHANDLE hSession, String ^ name, EFALDATETIME value)
			{
				if (name == nullptr) return false;
				pin_ptr<const wchar_t> sz = PtrToStringChars(name);
				::EFALDATETIME g;
				g.year = value.year;
				g.month = value.month;
				g.day = value.day;
				g.hour = value.hour;
				g.minute = value.minute;
				g.second = value.second;
				g.millisecond = value.millisecond;
				return EFAL::SetVariableValueDateTime(hSession, sz, g);
			}

		};
	}
}


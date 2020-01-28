using MapInfo.NFAL;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;

namespace NFALShell
{
	enum WKBGeometryType
	{
		wkbPoint = 1,
		wkbLineString = 2,
		wkbPolygon = 3,
		wkbTriangle = 17,
		wkbMultiPoint = 4,
		wkbMultiLineString = 5,
		wkbMultiPolygon = 6,
		wkbGeometryCollection = 7,
		wkbPolyhedralSurface = 15,
		wkbTIN = 16,
		ewkbLegacyText = 206,
		wkbPointZ = 1001,
		wkbLineStringZ = 1002,
		wkbPolygonZ = 1003,
		wkbTrianglez = 1017,
		wkbMultiPointZ = 1004,
		wkbMultiLineStringZ = 1005,
		wkbMultiPolygonZ = 1006,
		wkbGeometryCollectionZ = 1007,
		wkbPolyhedralSurfaceZ = 1015,
		wkbTINZ = 1016,
		wkbPointM = 2001,
		wkbLineStringM = 2002,
		wkbPolygonM = 2003,
		wkbTriangleM = 2017,
		wkbMultiPointM = 2004,
		wkbMultiLineStringM = 2005,
		wkbMultiPolygonM = 2006,
		wkbGeometryCollectionM = 2007,
		wkbPolyhedralSurfaceM = 2015,
		wkbTINM = 2016,
		wkbPointZM = 3001,
		wkbLineStringZM = 3002,
		wkbPolygonZM = 3003,
		wkbTriangleZM = 3017,
		wkbMultiPointZM = 3004,
		wkbMultiLineStringZM = 3005,
		wkbMultiPolygonZM = 3006,
		wkbGeometryCollectionZM = 3007,
		wkbPolyhedralSurfaceZM = 3015,
		wkbTinZM = 3016,
	};
	class NFALShell
	{
		private ulong hSession = 0;
		public NFALShell(ulong sesid)
		{
			hSession = sesid;
		}

		public static void Main(String[] args)
		{
			ulong hSession = NFAL.InitializeSession(null);
			NFALShell app = new NFALShell(hSession);
			app.Prompt();
			NFAL.DestroySession(hSession);
		}
		/*
		################################################################################################
		################################################################################################
		#############                                                                      #############
		#############                  Utility Functions for this app                      #############
		#############                                                                      #############
		################################################################################################
		################################################################################################
		*/
		private bool MyReportErrors()
		{
			if (!NFAL.HaveErrors(hSession)) return false;
			Console.Out.Write("ERRORS: ");
			for (int ii = 0, nn = NFAL.NumErrors(hSession); ii < nn; ii++)
			{
				Console.Out.Write("{0}\n", NFAL.GetError(hSession, ii));
			}
			NFAL.ClearErrors(hSession);
			return true;
		}
		/*
		################################################################################################
		################################################################################################
		#############                                                                      #############
		#############              Table Functions                                         #############
		#############                                                                      #############
		################################################################################################
		################################################################################################
		*/
		private void Open(String txt)
		{
			if (txt.ToLower().StartsWith("table ")) txt = txt.Substring(6);
			ulong hTable = NFAL.OpenTable(hSession, txt);
			if (!MyReportErrors() && (hTable > 0))
			{
				Console.Out.Write("Opened table as ");
				Console.Out.Write(NFAL.GetTableName(hSession, hTable));
				Console.Out.Write("\n");
			}
		}
		private ulong TableAlias2Handle(String name)
		{
			uint nTables = NFAL.GetTableCount(hSession);
			for (uint i = 0; i < nTables; i++)
			{
				ulong hTable = NFAL.GetTableHandle(hSession, i);
				if (NFAL.GetTableName(hSession, hTable).Equals(name, StringComparison.InvariantCultureIgnoreCase))
				{
					return hTable;
				}
			}
			return 0;
		}
		private void Close(String txt)
		{
			if (txt != null)
			{
				txt = txt.Trim();
			}
			if (String.IsNullOrEmpty(txt) || (txt.Equals("al", StringComparison.InvariantCultureIgnoreCase)))
			{
				while (NFAL.GetTableCount(hSession) > 0)
				{
					NFAL.CloseTable(hSession, NFAL.GetTableHandle(hSession, (uint)0));
				}
			}
			else
			{
				ulong hTable = TableAlias2Handle(txt);
				if (hTable > 0)
				{
					NFAL.CloseTable(hSession, hTable);
				}
			}
		}
		private void Pack(String txt)
		{
			if (txt != null)
			{
				txt = txt.Trim();
			}
			ulong hTable = TableAlias2Handle(txt);
			if (hTable > 0)
			{
				NFAL.Pack(hSession, hTable, ETablePackType.eTablePackTypeAll);
			}
		}
		private void Tables()
		{
			Console.Out.Write("Tables:\n----------------------------------------\n");
			uint nTables = NFAL.GetTableCount(hSession);
			for (uint i = 0; i < nTables; i++)
			{
				ulong hTable = NFAL.GetTableHandle(hSession, i);
				Console.Out.Write(NFAL.GetTableName(hSession, hTable));
				Console.Out.Write("\t");
				Console.Out.Write(NFAL.GetTablePath(hSession, hTable));
				Console.Out.Write("\n");
			}
		}
		private void Describe(String txt)
		{
			txt = txt.Trim();
			ulong hTable = TableAlias2Handle(txt);

			Console.Out.Write("Table {0}:\n", NFAL.GetTableName(hSession, hTable));
			Console.Out.Write("   Description   : {0}\n", NFAL.GetTableDescription(hSession, hTable));
			Console.Out.Write("   GUID          : {0}\n", NFAL.GetTableGUID(hSession, hTable));
			Console.Out.Write("   IsMappable    : {0}\n", NFAL.IsVector(hSession, hTable) ? "Yes" : "No");
			Console.Out.Write("   Columns:\n");
			for (uint i = 0, n = NFAL.GetColumnCount(hSession, hTable); i < n; i++)
			{
				Console.Out.Write("      [{0}] {1} ", i, NFAL.GetColumnName(hSession, hTable, i));
				switch (NFAL.GetColumnType(hSession, hTable, i))
				{
					case ALLTYPE_TYPE.OT_DECIMAL:
						Console.Out.Write("Decimal[{0},{1}]   ", NFAL.GetColumnWidth(hSession, hTable, i), NFAL.GetColumnDecimals(hSession, hTable, i));
						break;
					case ALLTYPE_TYPE.OT_CHAR:
					case ALLTYPE_TYPE.OT_FLSTRING:
						Console.Out.Write("Char[{0}]        ", NFAL.GetColumnWidth(hSession, hTable, i));
						break;
					case ALLTYPE_TYPE.OT_INTEGER:
						Console.Out.Write("Integer          ");
						break;
					case ALLTYPE_TYPE.OT_INTEGER64:
						Console.Out.Write("Integer64        ");
						break;
					case ALLTYPE_TYPE.OT_SMALLINT:
						Console.Out.Write("SmallInt         ");
						break;
					case ALLTYPE_TYPE.OT_FLOAT:
						Console.Out.Write("Float            ");
						break;
					case ALLTYPE_TYPE.OT_LOGICAL:
						Console.Out.Write("Boolean          ");
						break;
					case ALLTYPE_TYPE.OT_DATE:
						Console.Out.Write("Date             ");
						break;
					case ALLTYPE_TYPE.OT_TIME:
						Console.Out.Write("Time             ");
						break;
					case ALLTYPE_TYPE.OT_DATETIME:
						Console.Out.Write("DateTime         ");
						break;
					case ALLTYPE_TYPE.OT_OBJECT:
						Console.Out.Write("Object           ");
						break;
					case ALLTYPE_TYPE.OT_KEY:
						Console.Out.Write("Key              ");
						break;
					case ALLTYPE_TYPE.OT_STYLE:
						Console.Out.Write("Style            ");
						break;
				}
				Console.Out.Write("\n");

				if (NFAL.GetColumnType(hSession, hTable, i) == ALLTYPE_TYPE.OT_OBJECT)
				{
					uint nPoints = NFAL.GetPointObjectCount(hSession, hTable, i);
					uint nLines = NFAL.GetLineObjectCount(hSession, hTable, i);
					uint nPolys = NFAL.GetAreaObjectCount(hSession, hTable, i);
					uint nMisc = NFAL.GetMiscObjectCount(hSession, hTable, i);
					Console.Out.Write("                                            LineCount     : {0}\n", nPoints);
					Console.Out.Write("                                            AreaCount     : {0}\n", nLines);
					Console.Out.Write("                                            PointCount    : {0}\n", nPolys);
					Console.Out.Write("                                            MiscCount     : {0}\n", nMisc);
					Console.Out.Write("                                            CoordSys      : {0}\n", NFAL.GetColumnCSys(hSession, hTable, i));
				}
			}
			Console.Out.Write("   Metadata:\n");
			ulong hEnumerator = NFAL.EnumerateMetadata(hSession, hTable);
			while (NFAL.GetNextEntry(hSession, hEnumerator))
			{
				Console.Out.Write("   ");
				Console.Out.Write(NFAL.GetCurrentMetadataKey(hSession, hEnumerator));
				Console.Out.Write("=");
				Console.Out.Write(NFAL.GetCurrentMetadataValue(hSession, hEnumerator));
				Console.Out.Write("\n");
			}
			NFAL.DisposeMetadataEnumerator(hSession, hEnumerator);
		}

		private void dbg_tokenize(String str)
		{
			List<String> strings = tokenize(str);
			Console.Out.WriteLine("Strings:\n----------------");
			foreach (String s in strings) Console.WriteLine("'{0}'", s);
			Console.Out.WriteLine();
		}
		private List<String> tokenize(String str)
		{
			List<String> list = new List<String>();
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < str.Length; i++)
			{
				char c = str[i];
				if (c == ' ')
				{
					if (sb.Length > 0) { list.Add(sb.ToString()); }
					sb.Clear();
				}
				else if (c == '\"')
				{
					i++;
					while (i < str.Length && (str[i] != '\"')) { sb.Append(str[i]); i++; }
				}
				else {
					sb.Append(c);
				}
			}
			if (sb.Length > 0) { list.Add(sb.ToString()); }
			return list;
		}

		/*
		save [table] <<table>> in <<file>> [charset <<charset>>] [NOINSERT]
		save [table] <<table>> as nativex table in <<file>> [UTF16 | UTF8 | charset <<charset>>] [NOINSERT]
		save [table] <<table>> as geopackage table <<dbtable>> in <<file>> [coordsys <<codespace:code>>] [UTF16 | UTF8 | charset <<charset>>] [NOINSERT] [CONVERTUNSUPPORTEDOBJECTS]
		*/
		private void SaveAs(String txt)
		{
			String sourceTableAlias = null;
			String pDestinationFileName = null;
			String pDestinationCoordSys = null;
			String pDestinationTableName = null;
			MICHARSET destinationCharset = MICHARSET.CHARSET_NONE;
			int tableType = 0;  // 0=Native, 1=NativeX, 2=Geopackage
			ulong hSourceTable = 0;
			bool saveData = true;
			bool convertUnsupportedObjects = false;

			/* ---------------------------------------------------------------------------
			* Parse and validate the input command
			* ---------------------------------------------------------------------------
			*/
			List<String> strings = tokenize(txt);

			// Keyword Save and optional keyword Table are not part of the input text.
			if (strings.Count < 3)
			{
				Console.Out.Write("Malformed command\n");
				Help();
				return;
			}
			// first token is the table alias we are saving
			sourceTableAlias = strings[0];
			if ((hSourceTable = NFAL.GetTableHandle(hSession, sourceTableAlias)) == 0)
			{
				Console.Out.Write("Table {0} not found.\n", sourceTableAlias);
				Help();
				return;
			}
			for (int i = 1, n = strings.Count; i < n; i++)
			{
				if (strings[i].Equals("in", StringComparison.InvariantCultureIgnoreCase))
				{
					if (++i >= n)
					{
						Console.Out.Write("Malformed command\n");
						Help();
						return;
					}
					pDestinationFileName = strings[i];
				}
				else if (strings[i].Equals("as", StringComparison.InvariantCultureIgnoreCase))
				{
					if (++i >= n)
					{
						Console.Out.Write("Malformed command\n");
						Help();
						return;
					}
					if (strings[i].Equals("nativex", StringComparison.InvariantCultureIgnoreCase))
					{
						if ((++i >= n) || (!strings[i].Equals("table", StringComparison.InvariantCultureIgnoreCase)))
						{
							Console.Out.Write("Malformed command\n");
							Help();
							return;
						}
						tableType = 1;
					}
					else if (strings[i].Equals("geopackage", StringComparison.InvariantCultureIgnoreCase))
					{
						if ((++i >= n) || (!strings[i].Equals("table", StringComparison.InvariantCultureIgnoreCase) || (++i >= n)))
						{
							Console.Out.Write("Malformed command\n");
							Help();
							return;
						}
						pDestinationTableName = strings[i];
						tableType = 2;
						if (destinationCharset == MICHARSET.CHARSET_NONE) destinationCharset = MICHARSET.CHARSET_UTF8;
					}
				}
				else if (strings[i].Equals("UTF8", StringComparison.InvariantCultureIgnoreCase))
				{
					destinationCharset = MICHARSET.CHARSET_UTF8;
				}
				else if (strings[i].Equals("UTF16", StringComparison.InvariantCultureIgnoreCase))
				{
					destinationCharset = MICHARSET.CHARSET_UTF16;
				}
				else if (strings[i].Equals("charset", StringComparison.InvariantCultureIgnoreCase))
				{
					if (++i >= n)
					{
						Console.Out.Write("Malformed command\n");
						Help();
						return;
					}
					if (strings[i].Equals("ISO8859_1", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_1;
					else if (strings[i].Equals("ISO8859_2", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_2;
					else if (strings[i].Equals("ISO8859_3", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_3;
					else if (strings[i].Equals("ISO8859_4", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_4;
					else if (strings[i].Equals("ISO8859_5", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_5;
					else if (strings[i].Equals("ISO8859_6", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_6;
					else if (strings[i].Equals("ISO8859_7", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_7;
					else if (strings[i].Equals("ISO8859_8", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_8;
					else if (strings[i].Equals("ISO8859_9", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_ISO8859_9;
					else if (strings[i].Equals("WLATIN1", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WLATIN1;
					else if (strings[i].Equals("WLATIN2", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WLATIN2;
					else if (strings[i].Equals("WARABIC", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WARABIC;
					else if (strings[i].Equals("WCYRILLIC", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WCYRILLIC;
					else if (strings[i].Equals("WGREEK", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WGREEK;
					else if (strings[i].Equals("WHEBREW", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WHEBREW;
					else if (strings[i].Equals("WTURKISH", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WTURKISH;
					else if (strings[i].Equals("WTCHINESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WTCHINESE;
					else if (strings[i].Equals("WSCHINESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WSCHINESE;
					else if (strings[i].Equals("WJAPANESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WJAPANESE;
					else if (strings[i].Equals("WKOREAN", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WKOREAN;
					//else if (strings[i].Equals("MROMAN", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MROMAN;
					//else if (strings[i].Equals("MARABIC", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MARABIC;
					//else if (strings[i].Equals("MGREEK", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MGREEK;
					//else if (strings[i].Equals("MHEBREW", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MHEBREW;
					//else if (strings[i].Equals("MCENTEURO", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MCENTEURO;
					//else if (strings[i].Equals("MCROATIAN", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MCROATIAN;
					//else if (strings[i].Equals("MCYRILLIC", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MCYRILLIC;
					//else if (strings[i].Equals("MICELANDIC", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MICELANDIC;
					//else if (strings[i].Equals("MTHAI", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MTHAI;
					//else if (strings[i].Equals("MTURKISH", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MTURKISH;
					//else if (strings[i].Equals("MTCHINESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MTCHINESE;
					//else if (strings[i].Equals("MJAPANESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MJAPANESE;
					//else if (strings[i].Equals("MKOREAN", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MKOREAN;
					else if (strings[i].Equals("CP437", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP437;
					else if (strings[i].Equals("CP850", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP850;
					else if (strings[i].Equals("CP852", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP852;
					else if (strings[i].Equals("CP857", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP857;
					else if (strings[i].Equals("CP860", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP860;
					else if (strings[i].Equals("CP861", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP861;
					else if (strings[i].Equals("CP863", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP863;
					else if (strings[i].Equals("CP865", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP865;
					else if (strings[i].Equals("CP855", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP855;
					else if (strings[i].Equals("CP864", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP864;
					else if (strings[i].Equals("CP869", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_CP869;
					//else if (strings[i].Equals("LICS", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_LICS;
					//else if (strings[i].Equals("LMBCS", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_LMBCS;
					//else if (strings[i].Equals("LMBCS1", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_LMBCS1;
					//else if (strings[i].Equals("LMBCS2", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_LMBCS2;
					//else if (strings[i].Equals("MSCHINESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_MSCHINESE;
					//else if (strings[i].Equals("UTCHINESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_UTCHINESE;
					//else if (strings[i].Equals("USCHINESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_USCHINESE;
					//else if (strings[i].Equals("UJAPANESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_UJAPANESE;
					//else if (strings[i].Equals("UKOREAN", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_UKOREAN;
					else if (strings[i].Equals("WTHAI", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WTHAI;
					else if (strings[i].Equals("WBALTICRIM", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WBALTICRIM;
					else if (strings[i].Equals("WVIETNAMESE", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_WVIETNAMESE;
					else if (strings[i].Equals("UTF8", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_UTF8;
					else if (strings[i].Equals("UTF16", StringComparison.InvariantCultureIgnoreCase)) destinationCharset = MICHARSET.CHARSET_UTF16;
					else
					{
						Console.Out.Write("Malformed command\n");
						Help();
						return;
					}
				}
				else if (strings[i].Equals("CoordSys", StringComparison.InvariantCultureIgnoreCase))
				{
					if (++i >= n)
					{
						Console.Out.Write("Malformed command\n");
						Help();
						return;
					}
					pDestinationCoordSys = strings[i];
				}
				else if (strings[i].Equals("noinsert", StringComparison.InvariantCultureIgnoreCase))
				{
					saveData = false;
				}
				else if (strings[i].Equals("ConvertUnsupportedObjects", StringComparison.InvariantCultureIgnoreCase))
				{
					convertUnsupportedObjects = true;
				}
				else
				{
					Console.Out.Write("Malformed command\n");
					Help();
					return;
				}
			}

			if (destinationCharset == MICHARSET.CHARSET_NONE) destinationCharset = NFAL.GetTableCharset(hSession, hSourceTable);

			/* ---------------------------------------------------------------------------
			* Echo back the parsed input values
			* ---------------------------------------------------------------------------
			*/
			Console.Out.Write("Save Table {0} ", sourceTableAlias);
			if (tableType == 1) Console.Out.Write("as NativeX table ");
			if (tableType == 2) Console.Out.Write("as Geopackage table {0} ", pDestinationTableName);
			Console.Out.Write(" in {0} ", pDestinationFileName);
			if (pDestinationCoordSys != null) Console.Out.Write("CoordSys {0} ", pDestinationCoordSys);
			switch (destinationCharset)
			{
				case MICHARSET.CHARSET_ISO8859_1:
					Console.Out.Write("Charset ISO8859_1 ");
					break;
				case MICHARSET.CHARSET_ISO8859_2:
					Console.Out.Write("Charset ISO8859_2 ");
					break;
				case MICHARSET.CHARSET_ISO8859_3:
					Console.Out.Write("Charset ISO8859_3 ");
					break;
				case MICHARSET.CHARSET_ISO8859_4:
					Console.Out.Write("Charset ISO8859_4 ");
					break;
				case MICHARSET.CHARSET_ISO8859_5:
					Console.Out.Write("Charset ISO8859_5 ");
					break;
				case MICHARSET.CHARSET_ISO8859_6:
					Console.Out.Write("Charset ISO8859_6 ");
					break;
				case MICHARSET.CHARSET_ISO8859_7:
					Console.Out.Write("Charset ISO8859_7 ");
					break;
				case MICHARSET.CHARSET_ISO8859_8:
					Console.Out.Write("Charset ISO8859_8 ");
					break;
				case MICHARSET.CHARSET_ISO8859_9:
					Console.Out.Write("Charset ISO8859_9 ");
					break;
				case MICHARSET.CHARSET_WLATIN1:
					Console.Out.Write("Charset WLATIN1 ");
					break;
				case MICHARSET.CHARSET_WLATIN2:
					Console.Out.Write("Charset WLATIN2 ");
					break;
				case MICHARSET.CHARSET_WARABIC:
					Console.Out.Write("Charset WARABIC ");
					break;
				case MICHARSET.CHARSET_WCYRILLIC:
					Console.Out.Write("Charset WCYRILLIC ");
					break;
				case MICHARSET.CHARSET_WGREEK:
					Console.Out.Write("Charset WGREEK ");
					break;
				case MICHARSET.CHARSET_WHEBREW:
					Console.Out.Write("Charset WHEBREW ");
					break;
				case MICHARSET.CHARSET_WTURKISH:
					Console.Out.Write("Charset WTURKISH ");
					break;
				case MICHARSET.CHARSET_WTCHINESE:
					Console.Out.Write("Charset WTCHINESE ");
					break;
				case MICHARSET.CHARSET_WSCHINESE:
					Console.Out.Write("Charset WSCHINESE ");
					break;
				case MICHARSET.CHARSET_WJAPANESE:
					Console.Out.Write("Charset WJAPANESE ");
					break;
				case MICHARSET.CHARSET_WKOREAN:
					Console.Out.Write("Charset WKOREAN ");
					break;
				//case MICHARSET.CHARSET_MROMAN:
				//	Console.Out.Write("Charset MROMAN ");
				//	break;
				//case MICHARSET.CHARSET_MARABIC:
				//	Console.Out.Write("Charset MARABIC ");
				//	break;
				//case MICHARSET.CHARSET_MGREEK:
				//	Console.Out.Write("Charset MGREEK ");
				//	break;
				//case MICHARSET.CHARSET_MHEBREW:
				//	Console.Out.Write("Charset MHEBREW ");
				//	break;
				//case MICHARSET.CHARSET_MCENTEURO:
				//	Console.Out.Write("Charset MCENTEURO ");
				//	break;
				//case MICHARSET.CHARSET_MCROATIAN:
				//	Console.Out.Write("Charset MCROATIAN ");
				//	break;
				//case MICHARSET.CHARSET_MCYRILLIC:
				//	Console.Out.Write("Charset MCYRILLIC ");
				//	break;
				//case MICHARSET.CHARSET_MICELANDIC:
				//	Console.Out.Write("Charset MICELANDIC ");
				//	break;
				//case MICHARSET.CHARSET_MTHAI:
				//	Console.Out.Write("Charset MTHAI ");
				//	break;
				//case MICHARSET.CHARSET_MTURKISH:
				//	Console.Out.Write("Charset MTURKISH ");
				//	break;
				//case MICHARSET.CHARSET_MTCHINESE:
				//	Console.Out.Write("Charset MTCHINESE ");
				//	break;
				//case MICHARSET.CHARSET_MJAPANESE:
				//	Console.Out.Write("Charset MJAPANESE ");
				//	break;
				//case MICHARSET.CHARSET_MKOREAN:
				//	Console.Out.Write("Charset MKOREAN ");
				//	break;
				case MICHARSET.CHARSET_CP437:
					Console.Out.Write("Charset CP437 ");
					break;
				case MICHARSET.CHARSET_CP850:
					Console.Out.Write("Charset CP850 ");
					break;
				case MICHARSET.CHARSET_CP852:
					Console.Out.Write("Charset CP852 ");
					break;
				case MICHARSET.CHARSET_CP857:
					Console.Out.Write("Charset CP857 ");
					break;
				case MICHARSET.CHARSET_CP860:
					Console.Out.Write("Charset CP860 ");
					break;
				case MICHARSET.CHARSET_CP861:
					Console.Out.Write("Charset CP861 ");
					break;
				case MICHARSET.CHARSET_CP863:
					Console.Out.Write("Charset CP863 ");
					break;
				case MICHARSET.CHARSET_CP865:
					Console.Out.Write("Charset CP865 ");
					break;
				case MICHARSET.CHARSET_CP855:
					Console.Out.Write("Charset CP855 ");
					break;
				case MICHARSET.CHARSET_CP864:
					Console.Out.Write("Charset CP864 ");
					break;
				case MICHARSET.CHARSET_CP869:
					Console.Out.Write("Charset CP869 ");
					break;
				//case MICHARSET.CHARSET_LICS:
				//	Console.Out.Write("Charset LICS ");
				//	break;
				//case MICHARSET.CHARSET_LMBCS:
				//	Console.Out.Write("Charset LMBCS ");
				//	break;
				//case MICHARSET.CHARSET_LMBCS1:
				//	Console.Out.Write("Charset LMBCS1 ");
				//	break;
				//case MICHARSET.CHARSET_LMBCS2:
				//	Console.Out.Write("Charset LMBCS2 ");
				//	break;
				//case MICHARSET.CHARSET_MSCHINESE:
				//	Console.Out.Write("Charset MSCHINESE ");
				//	break;
				//case MICHARSET.CHARSET_UTCHINESE:
				//	Console.Out.Write("Charset UTCHINESE ");
				//	break;
				//case MICHARSET.CHARSET_USCHINESE:
				//	Console.Out.Write("Charset USCHINESE ");
				//	break;
				//case MICHARSET.CHARSET_UJAPANESE:
				//	Console.Out.Write("Charset UJAPANESE ");
				//	break;
				//case MICHARSET.CHARSET_UKOREAN:
				//	Console.Out.Write("Charset UKOREAN ");
				//	break;
				case MICHARSET.CHARSET_WTHAI:
					Console.Out.Write("Charset WTHAI ");
					break;
				case MICHARSET.CHARSET_WBALTICRIM:
					Console.Out.Write("Charset WBALTICRIM ");
					break;
				case MICHARSET.CHARSET_WVIETNAMESE:
					Console.Out.Write("Charset WVIETNAMESE ");
					break;
				case MICHARSET.CHARSET_UTF8:
					Console.Out.Write("Charset UTF8 ");
					break;
				case MICHARSET.CHARSET_UTF16:
					Console.Out.Write("Charset UTF16 ");
					break;
				case MICHARSET.CHARSET_NONE:
					break;
				default:
					Console.Out.Write("Charset ??? ");
					break;
			}
			Console.Out.Write("\n");

			/* ---------------------------------------------------------------------------
			* Create the new table
			* ---------------------------------------------------------------------------
			*/
			ulong hMetadata = 0;
			Stopwatch stopwatch = Stopwatch.StartNew(); //creates and start the instance of Stopwatch

			if (tableType == 2)
			{ // Geopackage
				
				String fTableFile = Path.Combine(Path.GetDirectoryName(pDestinationFileName), Path.GetFileNameWithoutExtension(pDestinationTableName) + ".TAB");

				Console.Out.Write("fTableFile:'");
				Console.Out.Write(fTableFile);
				Console.Out.Write("'\n");

				hMetadata = NFAL.CreateGeopackageTableMetadata(hSession, pDestinationTableName, fTableFile, pDestinationFileName, destinationCharset, convertUnsupportedObjects);
			}
			else if (tableType == 1)
			{ // NativeX
				String acAlias = Path.GetFileNameWithoutExtension(pDestinationFileName);
				int pos = acAlias.LastIndexOf(".");
				if (pos > 0)
				{
					acAlias = acAlias.Substring(0, pos);
				}
				hMetadata = NFAL.CreateNativeXTableMetadata(hSession, acAlias, pDestinationFileName, destinationCharset);
			}
			else { // Native
				String acAlias = Path.GetFileNameWithoutExtension(pDestinationFileName);
				int pos = acAlias.LastIndexOf(".");
				if (pos > 0)
				{
					acAlias = acAlias.Substring(0, pos);
				}
				hMetadata = NFAL.CreateNativeTableMetadata(hSession, acAlias, pDestinationFileName, destinationCharset);
			}
			ulong hNewTable = 0;
			if (hMetadata != 0)
			{
				for (uint idx = 0, n = NFAL.GetColumnCount(hSession, hSourceTable); idx < n; ++idx)
				{
					String columnName = NFAL.GetColumnName(hSession, hSourceTable, idx);
					ALLTYPE_TYPE columnType = NFAL.GetColumnType(hSession, hSourceTable, idx);
					bool isIndexed = NFAL.IsColumnIndexed(hSession, hSourceTable, idx);
					uint columnWidth = NFAL.GetColumnWidth(hSession, hSourceTable, idx);
					uint columnDecimals = NFAL.GetColumnDecimals(hSession, hSourceTable, idx);
					String columnCSys = NFAL.GetColumnCSys(hSession, hSourceTable, idx);
					if ((columnType == ALLTYPE_TYPE.OT_OBJECT) && (pDestinationCoordSys != null)) columnCSys = pDestinationCoordSys;
					if ((columnType == ALLTYPE_TYPE.OT_INTEGER64) && (tableType != 2 /*Not Geopackage*/)) columnType = ALLTYPE_TYPE.OT_INTEGER;
					NFAL.AddColumn(hSession, hMetadata, columnName, columnType, isIndexed, columnWidth, columnDecimals, columnCSys);
				}
				hNewTable = NFAL.CreateTable(hSession, hMetadata);
				NFAL.DestroyTableMetadata(hSession, hMetadata);
			}
			/* ---------------------------------------------------------------------------
			* Insert the records from the source table into the new destination table
			* ---------------------------------------------------------------------------
			*/
			long nrecs = -1;
			if ((hNewTable != 0) && saveData)
			{
				StringBuilder insert = new StringBuilder(), projection = new StringBuilder();
				bool first = true;
				insert.Append("Insert Into \"");
				insert.Append(NFAL.GetTableName(hSession, hNewTable));
				insert.Append("\" (");
				for (uint idx = 0, cnt = NFAL.GetColumnCount(hSession, hNewTable); idx < cnt; ++idx)
				{
					String newColumnName = NFAL.GetColumnName(hSession, hNewTable, idx);
					if (NFAL.IsColumnReadOnly(hSession, hNewTable, idx)) continue;

					for (uint idx2 = 0, cnt2 = NFAL.GetColumnCount(hSession, hSourceTable); idx2 < cnt2; ++idx2)
					{
						String srcColumnName = NFAL.GetColumnName(hSession, hSourceTable, idx2);
						if (newColumnName.Equals(srcColumnName, StringComparison.InvariantCultureIgnoreCase)) // Geopackage tables use lower case
						{
							if (!first) projection.Append(",");
							projection.Append(newColumnName);
							first = false;
							break;
						}
					}
				}
				insert.Append(projection);
				insert.Append(") Select ");
				insert.Append(projection);
				insert.Append(" From \"");
				insert.Append(sourceTableAlias);
				insert.Append("\"");
				nrecs = -1;
				if (!first)
				{
					nrecs = NFAL.Insert(hSession, insert.ToString());
				}
			}

			stopwatch.Stop();
			Console.WriteLine("{0} seconds to create the table with {1} records", (int)(stopwatch.ElapsedMilliseconds / 1000), nrecs);
		}

		/*
		################################################################################################
		################################################################################################
		#############                                                                      #############
		#############          Geopackage Blob Utility Functions (client side)             #############
		#############                                                                      #############
		################################################################################################
		################################################################################################
		*/
		private static char[] hexArray = "0123456789ABCDEF".ToCharArray();
		private static String bytesToHex(byte[] bytes)
		{
			char[] hexChars = new char[bytes.Length * 2];
			for (int j = 0; j < bytes.Length; j++)
			{
				int v = bytes[j] & 0xFF;
				hexChars[j * 2] = hexArray[v >> 4];
				hexChars[j * 2 + 1] = hexArray[v & 0x0F];
			}
			return new String(hexChars);
		}
		private static byte ReadByte(byte[] data, ref int idx)
		{
			return data[idx++];
		}
		private static Int32 ReadInt32(byte[] data, ref int idx, int littleEndian)
		{
			if (littleEndian != 0)
			{
				Int32 i = BitConverter.ToInt32(data, idx);
				idx += 4;
				return i;
			}
			else
			{
				byte[] t = new byte[4];
				Array.Copy(data, idx, t, 0, 4);
				Array.Reverse(t);
				Int32 i = BitConverter.ToInt32(t, 0);
				idx += 4;
				return i;
			}
		}
		private static double ReadDouble(byte[] data, ref int idx, int littleEndian)
		{
			if (littleEndian != 0)
			{
				double d = BitConverter.ToDouble(data, idx);
				idx += 8;
				return d;
			}
			else
			{
				int fieldSize = 8;
				byte[] bytes = new byte[fieldSize];
				for (int i = fieldSize - 1; i > -1; i--)
					bytes[i] = data[idx + i];
				double d = BitConverter.ToDouble(bytes, 0);
				idx += 8;
				return d;
			}
		}
		private static void PrintGeometry(byte[] data, long nbytes)
		{
			//Console.Out.Write("'0x");
			//Console.Out.Write(bytesToHex(data));
			//Console.Out.Write("'");
			int idx = 0;

			if (nbytes < 7 /* sizeof(GeoPackageBinaryHeader)*/)
			{
				Console.Out.Write("Malformed blob");
			}
			else if (ReadByte(data,ref idx) != (byte)'G' || ReadByte(data,ref idx) != (byte)'P')
			{
				Console.Out.Write("Malformed blob");
			}
			else {
				byte version = ReadByte(data,ref idx);
				byte flags = ReadByte(data,ref idx);
				int isExtendedGeometry, isEmptyGeometry, envelopeType, littleEndianOrder;
				int envelopeBytes = 0;
				isExtendedGeometry = (flags & 0x20);
				isEmptyGeometry = (flags & 0x10);
				envelopeType = (flags & 0x0E) >> 1;
				littleEndianOrder = (flags & 0x01);

				int srs_id = ReadInt32(data,ref idx, littleEndianOrder);

				if (isEmptyGeometry != 0)
				{
					// Geometry is empty
					Console.Out.Write("EMPTY");
				}
				else {

					if (isExtendedGeometry != 0)
					{
						Console.Out.Write("EXTENDED");
					}

					double minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0, minM = 0, maxM = 0;
					//int idxData = 7; /* sizeof(GeoPackageBinaryHeader)*/
										  // Envelope
					if (envelopeType != 0)
					{
						// 1: envelope is [minx, maxx, miny, maxy], 32 bytes
						// 2: envelope is [minx, maxx, miny, maxy, minz, maxz], 48 bytes
						// 3: envelope is [minx, maxx, miny, maxy, minm, maxm], 48 bytes
						// 4: envelope is [minx, maxx, miny, maxy, minz, maxz, minm, maxm], 64 bytes
						if (envelopeType == 1)
						{
							envelopeBytes = 32;
							minX = ReadDouble(data,ref idx, littleEndianOrder);
							maxX = ReadDouble(data,ref idx, littleEndianOrder);
							minY = ReadDouble(data,ref idx, littleEndianOrder);
							maxY = ReadDouble(data,ref idx, littleEndianOrder);
						}
						else if (envelopeType == 2)
						{
							envelopeBytes = 48;
							minX = ReadDouble(data,ref idx, littleEndianOrder);
							maxX = ReadDouble(data,ref idx, littleEndianOrder);
							minY = ReadDouble(data,ref idx, littleEndianOrder);
							maxY = ReadDouble(data,ref idx, littleEndianOrder);
							minZ = ReadDouble(data,ref idx, littleEndianOrder);
							maxZ = ReadDouble(data,ref idx, littleEndianOrder);
						}
						else if (envelopeType == 3)
						{
							envelopeBytes = 48;
							minX = ReadDouble(data,ref idx, littleEndianOrder);
							maxX = ReadDouble(data,ref idx, littleEndianOrder);
							minY = ReadDouble(data,ref idx, littleEndianOrder);
							maxY = ReadDouble(data,ref idx, littleEndianOrder);
							minM = ReadDouble(data,ref idx, littleEndianOrder);
							maxM = ReadDouble(data,ref idx, littleEndianOrder);
						}
						else if (envelopeType == 4)
						{
							envelopeBytes = 64;
							minX = ReadDouble(data, ref idx, littleEndianOrder);
							maxX = ReadDouble(data,ref idx, littleEndianOrder);
							minY = ReadDouble(data,ref idx, littleEndianOrder);
							maxY = ReadDouble(data,ref idx, littleEndianOrder);
							minZ = ReadDouble(data,ref idx, littleEndianOrder);
							maxZ = ReadDouble(data,ref idx, littleEndianOrder);
							minM = ReadDouble(data,ref idx, littleEndianOrder);
							maxM = ReadDouble(data,ref idx, littleEndianOrder);
						}
					}
					littleEndianOrder = (int)ReadByte(data,ref idx);
					int wkbType = (int)ReadInt32(data,ref idx, littleEndianOrder);
					if (wkbType == (int)WKBGeometryType.wkbPoint)
					{
						Console.Out.Write("wkbPoint");
						if (envelopeBytes > 0) Console.Out.Write("[({0},{1})]", minX.ToString("F6"), minY.ToString("F6"));
					else
					{
							double x = 0.0, y = 0.0;
							x = ReadDouble(data,ref idx, littleEndianOrder);
							y = ReadDouble(data,ref idx, littleEndianOrder);
							Console.Out.Write("[({0},{1})]", x.ToString("F6"), y.ToString("F6"));
						}
					}
					else if (wkbType == (int)WKBGeometryType.wkbPointZ)
					{
						Console.Out.Write("wkbPointZ");
						if (envelopeBytes > 0) Console.Out.Write("[({0},{1})]", minX.ToString("F6"), minY.ToString("F6"));
					else
					{
							double x = 0.0, y = 0.0;
							x = ReadDouble(data,ref idx, littleEndianOrder);
							y = ReadDouble(data,ref idx, littleEndianOrder);
							Console.Out.Write("[({0},{1})]", x.ToString("F6"), y.ToString("F6"));
						}
					}
					else if (wkbType == (int)WKBGeometryType.wkbPointM)
					{
						Console.Out.Write("wkbPointM");
						if (envelopeBytes > 0) Console.Out.Write("[({0},{1})]", minX.ToString("F6"), minY.ToString("F6"));
					else
					{
							double x = 0.0, y = 0.0;
							x = ReadDouble(data,ref idx, littleEndianOrder);
							y = ReadDouble(data,ref idx, littleEndianOrder);
							Console.Out.Write("[({0},{1})]", x.ToString("F6"), y.ToString("F6"));
						}
					}
					else if (wkbType == (int)WKBGeometryType.wkbPointZM)
					{
						Console.Out.Write("wkbPointZM");
						if (envelopeBytes > 0) Console.Out.Write("[({0},{1})]", minX.ToString("F6"), minY.ToString("F6"));
					else
					{
							double x = 0.0, y = 0.0;
							x = ReadDouble(data,ref idx, littleEndianOrder);
							y = ReadDouble(data,ref idx, littleEndianOrder);
							Console.Out.Write("[({0},{1})]", x.ToString("F6"), y.ToString("F6"));
						}
					}
					else if (wkbType == (int)WKBGeometryType.wkbLineString)
					{
						Console.Out.Write("wkbLineString");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbPolygon)
					{
						Console.Out.Write("wkbPolygon");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbTriangle)
					{
						Console.Out.Write("wkbTriangle");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiPoint)
					{
						Console.Out.Write("wkbMultiPoint");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiLineString)
					{
						Console.Out.Write("wkbMultiLineString");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiPolygon)
					{
						Console.Out.Write("wkbMultiPolygon");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbGeometryCollection)
					{
						Console.Out.Write("wkbGeometryCollection");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbPolyhedralSurface)
					{
						Console.Out.Write("wkbPolyhedralSurface");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbTIN)
					{
						Console.Out.Write("wkbTIN");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbLineStringZ)
					{
						Console.Out.Write("wkbLineStringZ");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbPolygonZ)
					{
						Console.Out.Write("wkbPolygonZ");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbTrianglez)
					{
						Console.Out.Write("wkbTrianglez");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiPointZ)
					{
						Console.Out.Write("wkbMultiPointZ");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiLineStringZ)
					{
						Console.Out.Write("wkbMultiLineStringZ");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiPolygonZ)
					{
						Console.Out.Write("wkbMultiPolygonZ");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbGeometryCollectionZ)
					{
						Console.Out.Write("wkbGeometryCollectionZ");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbPolyhedralSurfaceZ)
					{
						Console.Out.Write("wkbPolyhedralSurfaceZ");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbTINZ)
					{
						Console.Out.Write("wkbTINZ");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbLineStringM)
					{
						Console.Out.Write("wkbLineStringM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbPolygonM)
					{
						Console.Out.Write("wkbPolygonM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbTriangleM)
					{
						Console.Out.Write("wkbTriangleM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiPointM)
					{
						Console.Out.Write("wkbMultiPointM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiLineStringM)
					{
						Console.Out.Write("wkbMultiLineStringM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiPolygonM)
					{
						Console.Out.Write("wkbMultiPolygonM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbGeometryCollectionM)
					{
						Console.Out.Write("wkbGeometryCollectionM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbPolyhedralSurfaceM)
					{
						Console.Out.Write("wkbPolyhedralSurfaceM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbTINM)
					{
						Console.Out.Write("wkbTINM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbLineStringZM)
					{
						Console.Out.Write("wkbLineStringZM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbPolygonZM)
					{
						Console.Out.Write("wkbPolygonZM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbTriangleZM)
					{
						Console.Out.Write("wkbTriangleZM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiPointZM)
					{
						Console.Out.Write("wkbMultiPointZM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiLineStringZM)
					{
						Console.Out.Write("wkbMultiLineStringZM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbMultiPolygonZM)
					{
						Console.Out.Write("wkbMultiPolygonZM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbGeometryCollectionZM)
					{
						Console.Out.Write("wkbGeometryCollectionZM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbPolyhedralSurfaceZM)
					{
						Console.Out.Write("wkbPolyhedralSurfaceZM");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else if (wkbType == (int)WKBGeometryType.wkbTinZM)
					{
						Console.Out.Write("wkbTinZM");
					}
					else if (wkbType == (int)WKBGeometryType.ewkbLegacyText)
					{
						Console.Out.Write("ewkbLegacyText");
						Console.Out.Write("[({0},{1})({2},{3})]", minX.ToString("F6"), minY.ToString("F6"), maxX.ToString("F6"), maxY.ToString("F6"));
					}
					else {
						Console.Out.Write("Unknown wkbType");
					}
				}
			}
		}
		/*
		################################################################################################
		################################################################################################
		#############                                                                      #############
		#############          Select, Insert, Update and Delete                           #############
		#############                                                                      #############
		################################################################################################
		################################################################################################
		*/
		private void Select(String txt)
		{
			Stopwatch stopwatch = Stopwatch.StartNew(); //creates and start the instance of Stopwatch
			ulong hCursor = NFAL.Select(hSession, txt);
			if (!MyReportErrors() && (hCursor > 0))
			{
				Console.Out.Write("MI_KEY\t");
				for (uint i = 0, n = NFAL.GetCursorColumnCount(hSession, hCursor); i < n; i++)
				{
					Console.Out.Write("{0}\t", NFAL.GetCursorColumnName(hSession, hCursor, i));
				}
				Console.Out.Write("\n");
				long recordCount = 0;
				long nbytes;
				byte[] data;
				EFALDATE efalDate;
				EFALTIME efalTime;
				EFALDATETIME efalDateTime;
				while (NFAL.FetchNext(hSession, hCursor))
				{
					MyReportErrors();
					recordCount++;
					Console.Out.Write("{0}\t", NFAL.GetCursorCurrentKey(hSession, hCursor));

					for (uint i = 0, n = NFAL.GetCursorColumnCount(hSession, hCursor); i < n; i++)
					{
						if (NFAL.GetCursorIsNull(hSession, hCursor, i))
						{
							Console.Out.Write("(null)");
						}
						else
						{
							switch (NFAL.GetCursorColumnType(hSession, hCursor, i))
							{
								case ALLTYPE_TYPE.OT_CHAR:
									Console.Out.Write("{0}", NFAL.GetCursorValueString(hSession, hCursor, i));
									break;
								case ALLTYPE_TYPE.OT_LOGICAL:
									Console.Out.Write("{0}", (NFAL.GetCursorValueBoolean(hSession, hCursor, i) ? "True" : "False"));
									break;
								case ALLTYPE_TYPE.OT_FLOAT:
								case ALLTYPE_TYPE.OT_DECIMAL:
									Console.Out.Write("{0}", NFAL.GetCursorValueDouble(hSession, hCursor, i));
									break;
								case ALLTYPE_TYPE.OT_INTEGER64:
									Console.Out.Write("{0}", NFAL.GetCursorValueInt64(hSession, hCursor, i));
									break;
								case ALLTYPE_TYPE.OT_INTEGER:
									Console.Out.Write("{0}", NFAL.GetCursorValueInt32(hSession, hCursor, i));
									break;
								case ALLTYPE_TYPE.OT_SMALLINT:
									Console.Out.Write("{0}", NFAL.GetCursorValueInt16(hSession, hCursor, i));
									break;
								case ALLTYPE_TYPE.OT_BINARY:
									nbytes = NFAL.PrepareCursorValueBinary(hSession, hCursor, i);
									Console.Out.Write("{0} bytes", nbytes);
									break;
								case ALLTYPE_TYPE.OT_OBJECT:
									nbytes = NFAL.PrepareCursorValueGeometry(hSession, hCursor, i);
									data = new byte[(int)nbytes];
									NFAL.GetData(hSession, data);
									PrintGeometry(data, nbytes);
									break;
								case ALLTYPE_TYPE.OT_STYLE:
									Console.Out.Write("{0}", NFAL.GetCursorValueStyle(hSession, hCursor, i));
									break;
								case ALLTYPE_TYPE.OT_TIMESPAN:
									Console.Out.Write("{0}", NFAL.GetCursorValueTimespanInMilliseconds(hSession, hCursor, i) / 1000.0);
									break;
								case ALLTYPE_TYPE.OT_DATE:
									efalDate = NFAL.GetCursorValueDate(hSession, hCursor, i);
									Console.Out.Write("{0}-{1}-{2}", efalDate.year, efalDate.month, efalDate.day);
									break;
								case ALLTYPE_TYPE.OT_DATETIME:
									efalDateTime = NFAL.GetCursorValueDateTime(hSession, hCursor, i);
									Console.Out.Write("{0}-{1}-{2} {3}:{4}:{5}.{6}", efalDateTime.year, efalDateTime.month, efalDateTime.day, efalDateTime.hour, efalDateTime.minute, efalDateTime.second, efalDateTime.millisecond);
									break;
								case ALLTYPE_TYPE.OT_TIME:
									efalTime = NFAL.GetCursorValueTime(hSession, hCursor, i);
									Console.Out.Write("{0}:{1}:{2}.{3}", efalTime.hour, efalTime.minute, efalTime.second, efalTime.millisecond);
									break;
							}
						}
						Console.Out.Write("\t");
					}
					Console.Out.Write("\n");
				}
				Console.Out.Write("{0} records\n", recordCount);
				NFAL.DisposeCursor(hSession, hCursor);
			}

			stopwatch.Stop();
			Console.WriteLine("{0} seconds", (int)(stopwatch.ElapsedMilliseconds / 1000));
		}
		private void Insert(String txt)
		{
			Stopwatch stopwatch = Stopwatch.StartNew(); //creates and start the instance of Stopwatch
			long recordCount = NFAL.Insert(hSession, txt);
			if (!MyReportErrors())
			{
				Console.Out.Write("{0} records affected.\n", recordCount);
			}
			stopwatch.Stop();
			Console.WriteLine("{0} seconds", (int)(stopwatch.ElapsedMilliseconds / 1000));
		}
		private void Update(String txt)
		{
			Stopwatch stopwatch = Stopwatch.StartNew(); //creates and start the instance of Stopwatch
			long recordCount = NFAL.Update(hSession, txt);
			if (!MyReportErrors())
			{
				Console.Out.Write("{0} records affected.\n", recordCount);
			}
			stopwatch.Stop();
			Console.WriteLine("{0} seconds", (int)(stopwatch.ElapsedMilliseconds / 1000));
		}
		private void Delete(String txt)
		{
			Stopwatch stopwatch = Stopwatch.StartNew(); //creates and start the instance of Stopwatch
			long recordCount = NFAL.Delete(hSession, txt);
			if (!MyReportErrors())
			{
				Console.Out.Write("{0} records affected.\n", recordCount);
			}
			stopwatch.Stop();
			Console.WriteLine("{0} seconds", (int)(stopwatch.ElapsedMilliseconds / 1000));
		}
		/*
		################################################################################################
		################################################################################################
		#############                                                                      #############
		#############         Variable Handling                                            #############
		#############                                                                      #############
		################################################################################################
		################################################################################################
		*/
		private void Set(String txt)
		{
			Stopwatch stopwatch = Stopwatch.StartNew(); //creates and start the instance of Stopwatch
			int idxEq = txt.IndexOf("=");
			if (idxEq > 0)
			{
				String vname = txt.Substring(0, idxEq);
				String vvalue = txt.Substring(idxEq + 1);
				NFAL.ClearErrors(hSession);
				NFAL.CreateVariable(hSession, vname);
				NFAL.SetVariableValue(hSession, vname, vvalue);
				if (!MyReportErrors())
				{
					stopwatch.Stop();
					Console.WriteLine("{0} seconds", (int)(stopwatch.ElapsedMilliseconds / 1000));
				}
			}
		}
		private void PrintVariableValue(String name)
		{
			if (NFAL.GetVariableIsNull(hSession, name))
			{
				Console.Out.Write("(null)");
				return;
			}
			long nbytes = 0;
			byte[] data = null;
			EFALDATE efalDate;
			EFALTIME efalTime;
			EFALDATETIME efalDateTime;

			switch (NFAL.GetVariableType(hSession, name))
			{
				case ALLTYPE_TYPE.OT_CHAR:
					Console.Out.Write("{0}", NFAL.GetVariableValueString(hSession, name));
					break;
				case ALLTYPE_TYPE.OT_LOGICAL:
					Console.Out.Write("{0}", (NFAL.GetVariableValueBoolean(hSession, name) ? "True" : "False"));
					break;
				case ALLTYPE_TYPE.OT_FLOAT:
				case ALLTYPE_TYPE.OT_DECIMAL:
					Console.Out.Write("{0}", NFAL.GetVariableValueDouble(hSession, name));
					break;
				case ALLTYPE_TYPE.OT_INTEGER64:
					Console.Out.Write("{0}", NFAL.GetVariableValueInt64(hSession, name));
					break;
				case ALLTYPE_TYPE.OT_INTEGER:
					Console.Out.Write("{0}", NFAL.GetVariableValueInt32(hSession, name));
					break;
				case ALLTYPE_TYPE.OT_SMALLINT:
					Console.Out.Write("{0}", NFAL.GetVariableValueInt16(hSession, name));
					break;
				case ALLTYPE_TYPE.OT_BINARY:
					nbytes = NFAL.PrepareVariableValueBinary(hSession, name);
					Console.Out.Write("{0} bytes", nbytes);
					break;
				case ALLTYPE_TYPE.OT_OBJECT:
					nbytes = NFAL.PrepareVariableValueGeometry(hSession, name);
					data = new byte[(int)nbytes];
					NFAL.GetData(hSession, data);
					//PrintGeometry(data, nbytes);
					break;
				case ALLTYPE_TYPE.OT_STYLE:
					Console.Out.Write("{0}", NFAL.GetVariableValueStyle(hSession, name));
					break;
				case ALLTYPE_TYPE.OT_TIMESPAN:
					Console.Out.Write("{0}", NFAL.GetVariableValueTimespanInMilliseconds(hSession, name) / 1000.0);
					break;
				case ALLTYPE_TYPE.OT_DATE:
					efalDate = NFAL.GetVariableValueDate(hSession, name);
					Console.Out.Write("{0}-{1}-{2}", efalDate.year, efalDate.month, efalDate.day);
					break;
				case ALLTYPE_TYPE.OT_DATETIME:
					efalDateTime = NFAL.GetVariableValueDateTime(hSession, name);
					Console.Out.Write("{0}-{1}-{2} {3}:{4}:{5}.{6}", efalDateTime.year, efalDateTime.month, efalDateTime.day, efalDateTime.hour, efalDateTime.minute, efalDateTime.second, efalDateTime.millisecond);
					break;
				case ALLTYPE_TYPE.OT_TIME:
					efalTime = NFAL.GetVariableValueTime(hSession, name);
					Console.Out.Write("{0}:{1}:{2}.{3}", efalTime.hour, efalTime.minute, efalTime.second, efalTime.millisecond);
					break;
			}
		}
		private void Show(String txt)
		{
			if (!String.IsNullOrEmpty(txt))
			{
				for (uint i = 0, n = NFAL.GetVariableCount(hSession); i < n; i++)
				{
					if (NFAL.GetVariableName(hSession, i).Equals(txt, StringComparison.InvariantCultureIgnoreCase))
					{
						String name = NFAL.GetVariableName(hSession, i);
						Console.Out.Write(name);
						Console.Out.Write("=");
						PrintVariableValue(name);
						Console.Out.Write("\n");
					}
				}
			}
			else
			{
				Console.Out.Write("\nVariables:\n----------------------------------------\n");
				for (uint i = 0, n = NFAL.GetVariableCount(hSession); i < n; i++)
				{
					String name = NFAL.GetVariableName(hSession, i);
					Console.Out.Write(name);
					Console.Out.Write("=");
					PrintVariableValue(name);
					Console.Out.Write("\n");
				}
			}
		}
		/*
		################################################################################################
		################################################################################################
		#############                                                                      #############
		#############      Main Processing and Entry Point                                 #############
		#############                                                                      #############
		################################################################################################
		################################################################################################
		*/
		private void Help()
		{
			Console.Out.Write("\n");
			Console.Out.Write("\n");
			Console.Out.Write("Lines beginning with ' or # are comment lines and will be ignored.\n");
			Console.Out.Write("A blank line terminates the execution.\n");
			Console.Out.Write("\n");
			Console.Out.Write("Commands:\n");
			Console.Out.Write("  Open [Table] <<filename>> [as <<alias>>]\n");
			Console.Out.Write("  Pack <<table>>\n");
			Console.Out.Write("  Close {<<table>> | ALL}\n");
			Console.Out.Write("  Set <<var>> = <<expr>>\n");
			Console.Out.Write("  Show [<<var>>]\n");
			Console.Out.Write("  Tables\n");
			Console.Out.Write("  Desc <<table>>\n");
			Console.Out.Write("  Save [table] <<table>> in <<filename>> [charset << charset >> ][NOINSERT]\n");
			Console.Out.Write("  Save [table] <<table>> as nativex table in <<filename>> [UTF16 | UTF8 | charset << charset >> ][NOINSERT]\n");
			Console.Out.Write("  Save [table] <<table>> as geopackage table <<dbtable>> in <<file>>\n");
			Console.Out.Write("                                     [coordsys <<codespace:code>>]\n");
			Console.Out.Write("                                     [UTF16 | UTF8 | charset << charset >> ][NOINSERT]\n");
			Console.Out.Write("  Select ...\n");
			Console.Out.Write("  Insert ...\n");
			Console.Out.Write("  Update ...\n");
			Console.Out.Write("  Delete ...\n");
			Console.Out.Write("\n");
		}
		private void ProcessCommand(String command)
		{
			// Process any commands that our command processor doesn't know how to handle...
			if (command.ToLower().StartsWith("'")) return;
			else if (command.ToLower().StartsWith("#")) return;
			else if (command.ToLower().StartsWith("open ")) Open(command.Substring(5));
			else if (command.ToLower().StartsWith("pack ")) Pack(command.Substring(5));
			else if (command.ToLower().StartsWith("close ")) Close(command.Substring(6));
			else if (command.ToLower().StartsWith("tables")) Tables();
			else if (command.ToLower().StartsWith("desc ")) Describe(command.Substring(5));
			else if (command.ToLower().StartsWith("save table ")) SaveAs(command.Substring(11));
			else if (command.ToLower().StartsWith("save ")) SaveAs(command.Substring(5));
			else if (command.ToLower().StartsWith("select ")) Select(command);
			else if (command.ToLower().StartsWith("insert ")) Insert(command);
			else if (command.ToLower().StartsWith("update ")) Update(command);
			else if (command.ToLower().StartsWith("delete ")) Delete(command);

			else if (command.ToLower().StartsWith("tokenize ")) dbg_tokenize(command.Substring(9));

			else if (command.ToLower().StartsWith("set ")) Set(command.Substring(4));
			else if (command.ToLower().StartsWith("show ")) Show(command.Substring(5));
			else if (command.Equals("show", StringComparison.InvariantCultureIgnoreCase)) Show(null);

			else if (command.ToLower().StartsWith("help ")) Help();
			else if (command.Equals("help", StringComparison.InvariantCultureIgnoreCase)) Help();

			MyReportErrors();
		}
		public void Prompt()
		{
			bool done = false;

			while (!done)
			{
				Console.Out.Write("> ");
				String command = Console.In.ReadLine().Trim();
				if (String.IsNullOrEmpty(command) || String.IsNullOrEmpty((command = command.Trim())))
				{
					Console.Out.WriteLine();
					break;
				}
				else ProcessCommand(command);
			}
		}
	}
}

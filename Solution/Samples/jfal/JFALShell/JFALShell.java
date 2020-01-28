import java.io.File;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.lang.StringBuffer;
import java.nio.ByteBuffer;
import java.util.Properties;
import java.util.Scanner;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.ArrayList;
import java.util.List;

public class JFALShell {
	private long hSession = 0;
	private static final double NANOSECONDS_TO_SECONDS = 1.0e9;
	
	public JFALShell(long sesid) {
		hSession = sesid;
	}
	
	public static void main(String[] args) {
		long hSession = JFAL.InitializeSession(null);
		JFALShell app = new JFALShell(hSession);
		app.Prompt();
		JFAL.DestroySession(hSession);
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
	private boolean MyReportErrors()
	{
		if (!JFAL.HaveErrors(hSession)) return false;
		System.out.printf("ERRORS: ");
		for (int ii = 0, nn = JFAL.NumErrors(hSession); ii < nn; ii++)
		{
			System.out.printf("%1$s\n", JFAL.GetError(hSession, ii));
		}
		JFAL.ClearErrors(hSession);
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
		if (txt.toLowerCase().startsWith("table ")) txt = txt.substring(6);
		long hTable = JFAL.OpenTable(hSession, txt);
		if (!MyReportErrors() && (hTable > 0))
		{
			System.out.print("Opened table as ");
			System.out.print(JFAL.GetTableName(hSession, hTable));
			System.out.print("\n");
		}
	}
	private long TableAlias2Handle(String name)
	{
		long nTables = JFAL.GetTableCount(hSession);
		for (long i = 0; i < nTables; i++)
		{
			long hTable = JFAL.GetTableHandle(hSession, i);
			if (JFAL.GetTableName(hSession, hTable).equalsIgnoreCase(name))
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
			txt = txt.trim();
		}
		if ((txt.length() == 0) || (txt.equalsIgnoreCase("al")))
		{
			while (JFAL.GetTableCount(hSession) > 0)
			{
				JFAL.CloseTable(hSession, JFAL.GetTableHandle(hSession, (long) 0));
			}
		}
		else
		{
			long hTable = TableAlias2Handle(txt);
			if (hTable > 0)
			{
				JFAL.CloseTable(hSession, hTable);
			}
		}
	}
	private void Pack(String txt)
	{
		if (txt != null) 
		{
			txt = txt.trim();
		}
		long hTable = TableAlias2Handle(txt);
		if (hTable > 0)
		{
			JFAL.Pack(hSession, hTable, ETablePackType.eTablePackTypeAll);
		}
	}
	private void Tables()
	{
		System.out.print("Tables:\n----------------------------------------\n");
		long nTables = JFAL.GetTableCount(hSession);
		for (long i = 0; i < nTables; i++)
		{
			long hTable = JFAL.GetTableHandle(hSession, i);
			System.out.print(JFAL.GetTableName(hSession, hTable));
			System.out.print("\t");
			System.out.print(JFAL.GetTablePath(hSession, hTable));
			System.out.print("\n");
		}
	}
	private void Describe(String txt)
	{
		txt = txt.trim();
		long hTable = TableAlias2Handle(txt);

		System.out.printf("Table %1$s:\n", JFAL.GetTableName(hSession, hTable));
		System.out.printf("   Description   : %1$s\n", JFAL.GetTableDescription(hSession, hTable));
		System.out.printf("   GUID          : %1$s\n", JFAL.GetTableGUID(hSession, hTable));
		System.out.printf("   IsMappable    : %1$s\n", JFAL.IsVector(hSession, hTable) ? "Yes" : "No");
		System.out.print("   Columns:\n");
		for (long i = 0, n = JFAL.GetColumnCount(hSession, hTable); i < n; i++)
		{
			System.out.printf("      [%1$2s] %2$-32.32s ", i, JFAL.GetColumnName(hSession, hTable, i));
			switch (JFAL.GetColumnType(hSession, hTable, i))
			{
			case OT_DECIMAL:
				System.out.printf("Decimal[%1$2d,%2$2d]   ", JFAL.GetColumnWidth(hSession, hTable, i), JFAL.GetColumnDecimals(hSession, hTable, i));
				break;
			case OT_CHAR:
				System.out.printf("Char[%1$3d]        ", JFAL.GetColumnWidth(hSession, hTable, i));
				break;
			case OT_INTEGER:
				System.out.print("Integer          ");
				break;
			case OT_INTEGER64:
				System.out.print("Integer64        ");
				break;
			case OT_SMALLINT:
				System.out.print("SmallInt         ");
				break;
			case OT_FLOAT:
				System.out.print("Float            ");
				break;
			case OT_LOGICAL:
				System.out.print("Boolean          ");
				break;
			case OT_DATE:
				System.out.print("Date             ");
				break;
			case OT_TIME:
				System.out.print("Time             ");
				break;
			case OT_DATETIME:
				System.out.print("DateTime         ");
				break;
			case OT_OBJECT:
				System.out.print("Object           ");
				break;
			case OT_STYLE:
				System.out.print("Style            ");
				break;
			}
			System.out.print("\n");

			if (JFAL.GetColumnType(hSession, hTable, i) == ALLTYPE_TYPE.OT_OBJECT)
			{
				long nPoints = JFAL.GetPointObjectCount(hSession, hTable, i);
				long nLines = JFAL.GetLineObjectCount(hSession, hTable, i);
				long nPolys = JFAL.GetAreaObjectCount(hSession, hTable, i);
				long nMisc = JFAL.GetMiscObjectCount(hSession, hTable, i);
				System.out.printf("                                            LineCount     : %1$d\n", nPoints);
				System.out.printf("                                            AreaCount     : %1$d\n", nLines);
				System.out.printf("                                            PointCount    : %1$d\n", nPolys);
				System.out.printf("                                            MiscCount     : %1$d\n", nMisc);
				System.out.printf("                                            CoordSys      : %1$s\n", JFAL.GetColumnCSys(hSession, hTable, i));
			}
		}
		System.out.print("   Metadata:\n");
		long hEnumerator = JFAL.EnumerateMetadata(hSession, hTable);
		while (JFAL.GetNextEntry(hSession, hEnumerator))
		{
			System.out.print("   ");
			System.out.print(JFAL.GetCurrentMetadataKey(hSession, hEnumerator));
			System.out.print("=");
			System.out.print(JFAL.GetCurrentMetadataValue(hSession, hEnumerator));
			System.out.print("\n");
		}
		JFAL.DisposeMetadataEnumerator(hSession, hEnumerator);
	}
	
	private List<String> tokenize(String str)
	{
		List<String> list = new ArrayList<String>();
		Matcher m = Pattern.compile("([^\"]\\S*|\".+?\")\\s*").matcher(str);
		while (m.find())
		    list.add(m.group(1).replace("\"", ""));
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
		long hSourceTable = 0;
		boolean saveData = true;
		boolean convertUnsupportedObjects = false;

		/* ---------------------------------------------------------------------------
		* Parse and validate the input command
		* ---------------------------------------------------------------------------
		*/
		List<String> strings = tokenize(txt);
		
		// Keyword Save and optional keyword Table are not part of the input text.
		if (strings.size() < 3)
		{
			System.out.print("Malformed command\n");
			Help();
			return;
		}
		// first token is the table alias we are saving
		sourceTableAlias = strings.get(0);
		if ((hSourceTable = JFAL.GetTableHandle(hSession, sourceTableAlias)) == 0)
		{
			System.out.printf("Table %1$s not found.\n", sourceTableAlias);
			Help();
			return;
		}
		for (int i = 1, n = strings.size(); i < n; i++)
		{
			if (strings.get(i).equalsIgnoreCase("in"))
			{
				if (++i >= n)
				{
					System.out.print("Malformed command\n");
					Help();
					return;
				}
				pDestinationFileName = strings.get(i);
			}
			else if (strings.get(i).equalsIgnoreCase("as"))
			{
				if (++i >= n)
				{
					System.out.print("Malformed command\n");
					Help();
					return;
				}
				if (strings.get(i).equalsIgnoreCase("nativex"))
				{
					if ((++i >= n) || (! strings.get(i).equalsIgnoreCase("table")))
					{
						System.out.print("Malformed command\n");
						Help();
						return;
					}
					tableType = 1;
				}
				else if (strings.get(i).equalsIgnoreCase("geopackage"))
				{
					if ((++i >= n) || (! strings.get(i).equalsIgnoreCase("table") || (++i >= n)))
					{
						System.out.print("Malformed command\n");
						Help();
						return;
					}
					pDestinationTableName = strings.get(i);
					tableType = 2;
					if (destinationCharset == MICHARSET.CHARSET_NONE) destinationCharset = MICHARSET.CHARSET_UTF8;
				}
			}
			else if (strings.get(i).equalsIgnoreCase("UTF8"))
			{
				destinationCharset = MICHARSET.CHARSET_UTF8;
			}
			else if (strings.get(i).equalsIgnoreCase("UTF16"))
			{
				destinationCharset = MICHARSET.CHARSET_UTF16;
			}
			else if (strings.get(i).equalsIgnoreCase("charset"))
			{
				if (++i >= n)
				{
					System.out.print("Malformed command\n");
					Help();
					return;
				}
				if (strings.get(i).equalsIgnoreCase("ISO8859_1")) destinationCharset = MICHARSET.CHARSET_ISO8859_1;
				else if (strings.get(i).equalsIgnoreCase("ISO8859_2")) destinationCharset = MICHARSET.CHARSET_ISO8859_2;
				else if (strings.get(i).equalsIgnoreCase("ISO8859_3")) destinationCharset = MICHARSET.CHARSET_ISO8859_3;
				else if (strings.get(i).equalsIgnoreCase("ISO8859_4")) destinationCharset = MICHARSET.CHARSET_ISO8859_4;
				else if (strings.get(i).equalsIgnoreCase("ISO8859_5")) destinationCharset = MICHARSET.CHARSET_ISO8859_5;
				else if (strings.get(i).equalsIgnoreCase("ISO8859_6")) destinationCharset = MICHARSET.CHARSET_ISO8859_6;
				else if (strings.get(i).equalsIgnoreCase("ISO8859_7")) destinationCharset = MICHARSET.CHARSET_ISO8859_7;
				else if (strings.get(i).equalsIgnoreCase("ISO8859_8")) destinationCharset = MICHARSET.CHARSET_ISO8859_8;
				else if (strings.get(i).equalsIgnoreCase("ISO8859_9")) destinationCharset = MICHARSET.CHARSET_ISO8859_9;
				else if (strings.get(i).equalsIgnoreCase("WLATIN1")) destinationCharset = MICHARSET.CHARSET_WLATIN1;
				else if (strings.get(i).equalsIgnoreCase("WLATIN2")) destinationCharset = MICHARSET.CHARSET_WLATIN2;
				else if (strings.get(i).equalsIgnoreCase("WARABIC")) destinationCharset = MICHARSET.CHARSET_WARABIC;
				else if (strings.get(i).equalsIgnoreCase("WCYRILLIC")) destinationCharset = MICHARSET.CHARSET_WCYRILLIC;
				else if (strings.get(i).equalsIgnoreCase("WGREEK")) destinationCharset = MICHARSET.CHARSET_WGREEK;
				else if (strings.get(i).equalsIgnoreCase("WHEBREW")) destinationCharset = MICHARSET.CHARSET_WHEBREW;
				else if (strings.get(i).equalsIgnoreCase("WTURKISH")) destinationCharset = MICHARSET.CHARSET_WTURKISH;
				else if (strings.get(i).equalsIgnoreCase("WTCHINESE")) destinationCharset = MICHARSET.CHARSET_WTCHINESE;
				else if (strings.get(i).equalsIgnoreCase("WSCHINESE")) destinationCharset = MICHARSET.CHARSET_WSCHINESE;
				else if (strings.get(i).equalsIgnoreCase("WJAPANESE")) destinationCharset = MICHARSET.CHARSET_WJAPANESE;
				else if (strings.get(i).equalsIgnoreCase("WKOREAN")) destinationCharset = MICHARSET.CHARSET_WKOREAN;
				else if (strings.get(i).equalsIgnoreCase("CP437")) destinationCharset = MICHARSET.CHARSET_CP437;
				else if (strings.get(i).equalsIgnoreCase("CP850")) destinationCharset = MICHARSET.CHARSET_CP850;
				else if (strings.get(i).equalsIgnoreCase("CP852")) destinationCharset = MICHARSET.CHARSET_CP852;
				else if (strings.get(i).equalsIgnoreCase("CP857")) destinationCharset = MICHARSET.CHARSET_CP857;
				else if (strings.get(i).equalsIgnoreCase("CP860")) destinationCharset = MICHARSET.CHARSET_CP860;
				else if (strings.get(i).equalsIgnoreCase("CP861")) destinationCharset = MICHARSET.CHARSET_CP861;
				else if (strings.get(i).equalsIgnoreCase("CP863")) destinationCharset = MICHARSET.CHARSET_CP863;
				else if (strings.get(i).equalsIgnoreCase("CP865")) destinationCharset = MICHARSET.CHARSET_CP865;
				else if (strings.get(i).equalsIgnoreCase("CP855")) destinationCharset = MICHARSET.CHARSET_CP855;
				else if (strings.get(i).equalsIgnoreCase("CP864")) destinationCharset = MICHARSET.CHARSET_CP864;
				else if (strings.get(i).equalsIgnoreCase("CP869")) destinationCharset = MICHARSET.CHARSET_CP869;
				else if (strings.get(i).equalsIgnoreCase("WTHAI")) destinationCharset = MICHARSET.CHARSET_WTHAI;
				else if (strings.get(i).equalsIgnoreCase("WBALTICRIM")) destinationCharset = MICHARSET.CHARSET_WBALTICRIM;
				else if (strings.get(i).equalsIgnoreCase("WVIETNAMESE")) destinationCharset = MICHARSET.CHARSET_WVIETNAMESE;
				else if (strings.get(i).equalsIgnoreCase("UTF8")) destinationCharset = MICHARSET.CHARSET_UTF8;
				else if (strings.get(i).equalsIgnoreCase("UTF16")) destinationCharset = MICHARSET.CHARSET_UTF16;
				else
				{
					System.out.print("Malformed command\n");
					Help();
					return;
				}
			}
			else if (strings.get(i).equalsIgnoreCase("CoordSys"))
			{
				if (++i >= n)
				{
					System.out.print("Malformed command\n");
					Help();
					return;
				}
				pDestinationCoordSys = strings.get(i);
			}
			else if (strings.get(i).equalsIgnoreCase("noinsert"))
			{
				saveData = false;
			}
			else if (strings.get(i).equalsIgnoreCase("ConvertUnsupportedObjects"))
			{
				convertUnsupportedObjects = true;
			}
			else
			{
				System.out.print("Malformed command\n");
				Help();
				return;
			}
		}

		if (destinationCharset == MICHARSET.CHARSET_NONE) destinationCharset = JFAL.GetTableCharset(hSession, hSourceTable);
		
		/* ---------------------------------------------------------------------------
		* Echo back the parsed input values
		* ---------------------------------------------------------------------------
		*/
		System.out.printf("Save Table %1$s ", sourceTableAlias);
		if (tableType == 1) System.out.print("as NativeX table ");
		if (tableType == 2) System.out.printf("as Geopackage table %1$s ", pDestinationTableName);
		System.out.printf(" in %1$s ", pDestinationFileName);
		if (pDestinationCoordSys != null) System.out.printf("CoordSys %1$s ", pDestinationCoordSys);
		switch (destinationCharset)
		{
		case CHARSET_ISO8859_1:
			System.out.print("Charset ISO8859_1 ");
			break;
		case CHARSET_ISO8859_2:
			System.out.print("Charset ISO8859_2 ");
			break;
		case CHARSET_ISO8859_3:
			System.out.print("Charset ISO8859_3 ");
			break;
		case CHARSET_ISO8859_4:
			System.out.print("Charset ISO8859_4 ");
			break;
		case CHARSET_ISO8859_5:
			System.out.print("Charset ISO8859_5 ");
			break;
		case CHARSET_ISO8859_6:
			System.out.print("Charset ISO8859_6 ");
			break;
		case CHARSET_ISO8859_7:
			System.out.print("Charset ISO8859_7 ");
			break;
		case CHARSET_ISO8859_8:
			System.out.print("Charset ISO8859_8 ");
			break;
		case CHARSET_ISO8859_9:
			System.out.print("Charset ISO8859_9 ");
			break;
		case CHARSET_WLATIN1:
			System.out.print("Charset WLATIN1 ");
			break;
		case CHARSET_WLATIN2:
			System.out.print("Charset WLATIN2 ");
			break;
		case CHARSET_WARABIC:
			System.out.print("Charset WARABIC ");
			break;
		case CHARSET_WCYRILLIC:
			System.out.print("Charset WCYRILLIC ");
			break;
		case CHARSET_WGREEK:
			System.out.print("Charset WGREEK ");
			break;
		case CHARSET_WHEBREW:
			System.out.print("Charset WHEBREW ");
			break;
		case CHARSET_WTURKISH:
			System.out.print("Charset WTURKISH ");
			break;
		case CHARSET_WTCHINESE:
			System.out.print("Charset WTCHINESE ");
			break;
		case CHARSET_WSCHINESE:
			System.out.print("Charset WSCHINESE ");
			break;
		case CHARSET_WJAPANESE:
			System.out.print("Charset WJAPANESE ");
			break;
		case CHARSET_WKOREAN:
			System.out.print("Charset WKOREAN ");
			break;
		case CHARSET_CP437:
			System.out.print("Charset CP437 ");
			break;
		case CHARSET_CP850:
			System.out.print("Charset CP850 ");
			break;
		case CHARSET_CP852:
			System.out.print("Charset CP852 ");
			break;
		case CHARSET_CP857:
			System.out.print("Charset CP857 ");
			break;
		case CHARSET_CP860:
			System.out.print("Charset CP860 ");
			break;
		case CHARSET_CP861:
			System.out.print("Charset CP861 ");
			break;
		case CHARSET_CP863:
			System.out.print("Charset CP863 ");
			break;
		case CHARSET_CP865:
			System.out.print("Charset CP865 ");
			break;
		case CHARSET_CP855:
			System.out.print("Charset CP855 ");
			break;
		case CHARSET_CP864:
			System.out.print("Charset CP864 ");
			break;
		case CHARSET_CP869:
			System.out.print("Charset CP869 ");
			break;
		case CHARSET_WTHAI:
			System.out.print("Charset WTHAI ");
			break;
		case CHARSET_WBALTICRIM:
			System.out.print("Charset WBALTICRIM ");
			break;
		case CHARSET_WVIETNAMESE:
			System.out.print("Charset WVIETNAMESE ");
			break;
		case CHARSET_UTF8:
			System.out.print("Charset UTF8 ");
			break;
		case CHARSET_UTF16:
			System.out.print("Charset UTF16 ");
			break;
		case CHARSET_NONE:
			break;
		default:
			System.out.print("Charset ??? ");
			break;
		}
		System.out.print("\n");

		/* ---------------------------------------------------------------------------
		* Create the new table
		* ---------------------------------------------------------------------------
		*/
		long hMetadata = 0;
		long start = System.nanoTime();
		
		File fDestinationFileName = new File(pDestinationFileName);
		if (tableType == 2) { // Geopackage
			File fTableFile = new File(fDestinationFileName.getParent(), pDestinationTableName +".TAB");
			
			System.out.print("fTableFile.getPath():'");
			System.out.print(fTableFile.getPath());
			System.out.print("'\n");

			hMetadata = JFAL.CreateGeopackageTableMetadata(hSession, pDestinationTableName, fTableFile.getPath(), pDestinationFileName, destinationCharset, convertUnsupportedObjects);
		}
		else if (tableType == 1) { // NativeX
			String acAlias = fDestinationFileName.getName();
			int pos = acAlias.lastIndexOf(".");
			if (pos > 0) {
				acAlias = acAlias.substring(0, pos);
			}
			hMetadata = JFAL.CreateNativeXTableMetadata(hSession, acAlias, pDestinationFileName, destinationCharset);
		}
		else { // Native
			String acAlias = fDestinationFileName.getName();
			int pos = acAlias.lastIndexOf(".");
			if (pos > 0) {
				acAlias = acAlias.substring(0, pos);
			}
			hMetadata = JFAL.CreateNativeTableMetadata(hSession, acAlias, pDestinationFileName, destinationCharset);
		}
		long hNewTable = 0;
		if (hMetadata != 0)
		{
			for (long idx = 0, n = JFAL.GetColumnCount(hSession, hSourceTable); idx < n; ++idx)  {
				String columnName = JFAL.GetColumnName(hSession, hSourceTable, idx);
				ALLTYPE_TYPE columnType = JFAL.GetColumnType(hSession, hSourceTable, idx);
				boolean isIndexed = JFAL.IsColumnIndexed(hSession, hSourceTable, idx);
				long columnWidth = JFAL.GetColumnWidth(hSession, hSourceTable, idx);
				long columnDecimals = JFAL.GetColumnDecimals(hSession, hSourceTable, idx);
				String columnCSys = JFAL.GetColumnCSys(hSession, hSourceTable, idx);
				if ((columnType == ALLTYPE_TYPE.OT_OBJECT) && (pDestinationCoordSys != null)) columnCSys = pDestinationCoordSys;
				if ((columnType == ALLTYPE_TYPE.OT_INTEGER64) && (tableType != 2 /*Not Geopackage*/)) columnType = ALLTYPE_TYPE.OT_INTEGER;
				JFAL.AddColumn(hSession, hMetadata, columnName, columnType, isIndexed, columnWidth, columnDecimals, columnCSys);
			}
			hNewTable = JFAL.CreateTable(hSession, hMetadata);
			JFAL.DestroyTableMetadata(hSession, hMetadata);
		}
		/* ---------------------------------------------------------------------------
		* Insert the records from the source table into the new destination table
		* ---------------------------------------------------------------------------
		*/
		long nrecs = -1;
		if ((hNewTable != 0) && saveData) {
			StringBuffer insert = new StringBuffer(), projection = new StringBuffer();
			boolean first = true;
			insert.append("Insert Into \"");
			insert.append(JFAL.GetTableName(hSession, hNewTable));
			insert.append("\" (");
			for (long idx = 0, cnt = JFAL.GetColumnCount(hSession, hNewTable); idx < cnt; ++idx)  {
				String newColumnName = JFAL.GetColumnName(hSession, hNewTable, idx);
				if (JFAL.IsColumnReadOnly(hSession, hNewTable, idx)) continue;

				for (long idx2 = 0, cnt2 = JFAL.GetColumnCount(hSession, hSourceTable); idx2 < cnt2; ++idx2)  {
					String srcColumnName = JFAL.GetColumnName(hSession, hSourceTable, idx2);
					if (newColumnName.equalsIgnoreCase(srcColumnName)) // geopackage tables use lower case
					{
						if (!first) projection.append(",");
						projection.append(newColumnName);
						first = false;
						break;
					}
				}
			}
			insert.append(projection);
			insert.append(") Select ");
			insert.append(projection);
			insert.append(" From \"");
			insert.append(sourceTableAlias);
			insert.append("\"");
			nrecs = -1;
			if (!first)
			{
				nrecs = JFAL.Insert(hSession, insert.toString());
			}
		}

		long finish = System.nanoTime();
		double duration = (double)(finish - start) / NANOSECONDS_TO_SECONDS;
		System.out.printf("%1$6.1f seconds to create the table with %2$d records\n", duration, nrecs);
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
	final protected static char[] hexArray = "0123456789ABCDEF".toCharArray();
	private static String bytesToHex(byte[] bytes) {
		char[] hexChars = new char[bytes.length * 2];
		for ( int j = 0; j < bytes.length; j++ ) {
			int v = bytes[j] & 0xFF;
			hexChars[j * 2] = hexArray[v >>> 4];
			hexChars[j * 2 + 1] = hexArray[v & 0x0F];
		}
		return new String(hexChars);
	}

	private void PrintGeometry(byte[] data, long nbytes)
	{
		//System.out.print("'0x");
		//System.out.print(bytesToHex(data));
		//System.out.print("'");
		ByteBuffer byteBuffer = ByteBuffer.wrap(data);
		byteBuffer.order(java.nio.ByteOrder.LITTLE_ENDIAN);
		
		if (nbytes < 7 /* sizeof(GeoPackageBinaryHeader)*/) {
			System.out.print("Malformed blob");
		}
		else if (byteBuffer.get() != (byte)'G' || byteBuffer.get() != (byte)'P') {
			System.out.print("Malformed blob");
		}
		else {
			byte version = byteBuffer.get();
			byte flags = byteBuffer.get();
			int srs_id = byteBuffer.getInt(); 
			int isExtendedGeometry, isEmptyGeometry, envelopeType, littleEndianOrder;
			int envelopeBytes = 0;
			isExtendedGeometry = (flags & 0x20);
			isEmptyGeometry = (flags & 0x10);
			envelopeType = (flags & 0x0E) >> 1;
			littleEndianOrder = (flags & 0x01);

			if (isEmptyGeometry != 0) {
				// Geometry is empty
				System.out.print("EMPTY");
			}
			else if (isExtendedGeometry != 0) {
				System.out.print("EXTENDED");
			}
			else {
				double minX=0, maxX=0, minY=0, maxY=0, minZ=0, maxZ=0, minM=0, maxM=0;
				int idxData = 7; /* sizeof(GeoPackageBinaryHeader)*/
				// Envelope
				if (envelopeType != 0) {
					// 1: envelope is [minx, maxx, miny, maxy], 32 bytes
					// 2: envelope is [minx, maxx, miny, maxy, minz, maxz], 48 bytes
					// 3: envelope is [minx, maxx, miny, maxy, minm, maxm], 48 bytes
					// 4: envelope is [minx, maxx, miny, maxy, minz, maxz, minm, maxm], 64 bytes
					if (envelopeType == 1) {
						envelopeBytes = 32;
						minX = byteBuffer.getDouble();
						maxX = byteBuffer.getDouble();
						minY = byteBuffer.getDouble();
						maxY = byteBuffer.getDouble();
					}
					else if (envelopeType == 2) {
						envelopeBytes = 48;
						minX = byteBuffer.getDouble();
						maxX = byteBuffer.getDouble();
						minY = byteBuffer.getDouble();
						maxY = byteBuffer.getDouble();
						minZ = byteBuffer.getDouble();
						maxZ = byteBuffer.getDouble();
					}
					else if (envelopeType == 3) {
						envelopeBytes = 48;
						minX = byteBuffer.getDouble();
						maxX = byteBuffer.getDouble();
						minY = byteBuffer.getDouble();
						maxY = byteBuffer.getDouble();
						minM = byteBuffer.getDouble();
						maxM = byteBuffer.getDouble();
					}
					else if (envelopeType == 4) {
						envelopeBytes = 64;
						minX = byteBuffer.getDouble();
						maxX = byteBuffer.getDouble();
						minY = byteBuffer.getDouble();
						maxY = byteBuffer.getDouble();
						minZ = byteBuffer.getDouble();
						maxZ = byteBuffer.getDouble();
						minM = byteBuffer.getDouble();
						maxM = byteBuffer.getDouble();
					}
				}
				littleEndianOrder = (int)byteBuffer.get();
				int wkbType = (int)byteBuffer.getInt();
				if (wkbType == WKBGeometryType.wkbPoint.swigValue())
				{
					System.out.print("wkbPoint");
					if (envelopeBytes > 0) System.out.printf("[(%1$12.6f,%2$12.6f)]", minX, minY);
					else
					{
						double x = 0.0, y = 0.0;
						x = byteBuffer.getDouble();
						y = byteBuffer.getDouble();
						System.out.printf("[(%1$12.6f,%2$12.6f)]", x, y);
					}
				} else if (wkbType == WKBGeometryType.wkbPointZ.swigValue()) {
					System.out.print("wkbPointZ");
					if (envelopeBytes > 0) System.out.printf("[(%1$12.6f,%2$12.6f)]", minX, minY);
					else
					{
						double x = 0.0, y = 0.0;
						x = byteBuffer.getDouble();
						y = byteBuffer.getDouble();
						System.out.printf("[(%1$12.6f,%2$12.6f)]", x, y);
					}
				} else if (wkbType == WKBGeometryType.wkbPointM.swigValue()) {
					System.out.print("wkbPointM");
					if (envelopeBytes > 0) System.out.printf("[(%1$12.6f,%2$12.6f)]", minX, minY);
					else
					{
						double x = 0.0, y = 0.0;
						x = byteBuffer.getDouble();
						y = byteBuffer.getDouble();
						System.out.printf("[(%1$12.6f,%2$12.6f)]", x, y);
					}
				} else if (wkbType == WKBGeometryType.wkbPointZM.swigValue()) {
					System.out.print("wkbPointZM");
					if (envelopeBytes > 0) System.out.printf("[(%1$12.6f,%2$12.6f)]", minX, minY);
					else
					{
						double x = 0.0, y = 0.0;
						x = byteBuffer.getDouble();
						y = byteBuffer.getDouble();
						System.out.printf("[(%1$12.6f,%2$12.6f)]", x, y);
					}
				} else if (wkbType == WKBGeometryType.wkbLineString.swigValue()) {
					System.out.print("wkbLineString");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbPolygon.swigValue()) {
					System.out.print("wkbPolygon");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbTriangle.swigValue()) {
					System.out.print("wkbTriangle");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiPoint.swigValue()) {
					System.out.print("wkbMultiPoint");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiLineString.swigValue()) {
					System.out.print("wkbMultiLineString");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiPolygon.swigValue()) {
					System.out.print("wkbMultiPolygon");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbGeometryCollection.swigValue()) {
					System.out.print("wkbGeometryCollection");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbPolyhedralSurface.swigValue()) {
					System.out.print("wkbPolyhedralSurface");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbTIN.swigValue()) {
					System.out.print("wkbTIN");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbLineStringZ.swigValue()) {
					System.out.print("wkbLineStringZ");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbPolygonZ.swigValue()) {
					System.out.print("wkbPolygonZ");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbTrianglez.swigValue()) {
					System.out.print("wkbTrianglez");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiPointZ.swigValue()) {
					System.out.print("wkbMultiPointZ");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiLineStringZ.swigValue()) {
					System.out.print("wkbMultiLineStringZ");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiPolygonZ.swigValue()) {
					System.out.print("wkbMultiPolygonZ");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbGeometryCollectionZ.swigValue()) {
					System.out.print("wkbGeometryCollectionZ");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbPolyhedralSurfaceZ.swigValue()) {
					System.out.print("wkbPolyhedralSurfaceZ");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbTINZ.swigValue()) {
					System.out.print("wkbTINZ");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbLineStringM.swigValue()) {
					System.out.print("wkbLineStringM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbPolygonM.swigValue()) {
					System.out.print("wkbPolygonM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbTriangleM.swigValue()) {
					System.out.print("wkbTriangleM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiPointM.swigValue()) {
					System.out.print("wkbMultiPointM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiLineStringM.swigValue()) {
					System.out.print("wkbMultiLineStringM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiPolygonM.swigValue()) {
					System.out.print("wkbMultiPolygonM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbGeometryCollectionM.swigValue()) {
					System.out.print("wkbGeometryCollectionM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbPolyhedralSurfaceM.swigValue()) {
					System.out.print("wkbPolyhedralSurfaceM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbTINM.swigValue()) {
					System.out.print("wkbTINM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbLineStringZM.swigValue()) {
					System.out.print("wkbLineStringZM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbPolygonZM.swigValue()) {
					System.out.print("wkbPolygonZM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbTriangleZM.swigValue()) {
					System.out.print("wkbTriangleZM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiPointZM.swigValue()) {
					System.out.print("wkbMultiPointZM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiLineStringZM.swigValue()) {
					System.out.print("wkbMultiLineStringZM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbMultiPolygonZM.swigValue()) {
					System.out.print("wkbMultiPolygonZM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbGeometryCollectionZM.swigValue()) {
					System.out.print("wkbGeometryCollectionZM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbPolyhedralSurfaceZM.swigValue()) {
					System.out.print("wkbPolyhedralSurfaceZM");
					System.out.printf("[(%1$12.6f,%2$12.6f)(%3$12.6f,%4$12.6f)]", minX, minY, maxX, maxY);
				} else if (wkbType == WKBGeometryType.wkbTinZM.swigValue()) {
					System.out.print("wkbTinZM");
				} else {
					System.out.print("Unknown wkbType");
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
		long start = System.nanoTime();
		long hCursor = JFAL.Select(hSession, txt);
		if (!MyReportErrors() && (hCursor > 0))
		{
			System.out.print("MI_KEY\t");
			for (long i = 0, n = JFAL.GetCursorColumnCount(hSession, hCursor); i < n; i++)
			{
				System.out.printf("%1$s\t", JFAL.GetCursorColumnName(hSession, hCursor, i));
			}
			System.out.print("\n");
			long recordCount = 0;
			long nbytes;
			byte [] data;
			EFALDATE efalDate;
			EFALTIME efalTime;
			EFALDATETIME efalDateTime;
			while (JFAL.FetchNext(hSession, hCursor))
			{
				MyReportErrors();
				recordCount++;
				System.out.printf("%1$s\t", JFAL.GetCursorCurrentKey(hSession, hCursor));

				for (long i = 0, n = JFAL.GetCursorColumnCount(hSession, hCursor); i < n; i++)
				{
					if (JFAL.GetCursorIsNull(hSession, hCursor, i))
					{
						System.out.print("(null)");
					}
					else 
					{
						switch (JFAL.GetCursorColumnType(hSession, hCursor, i))
						{
						case OT_CHAR:
							System.out.printf("%1$s", JFAL.GetCursorValueString(hSession, hCursor, i));
							break;
						case OT_LOGICAL:
							System.out.printf("%1$s", (JFAL.GetCursorValueBoolean(hSession, hCursor, i) ? "True" : "False"));
							break;
						case OT_FLOAT:
						case OT_DECIMAL:
							System.out.printf("%1$f", JFAL.GetCursorValueDouble(hSession, hCursor, i));
							break;
						case OT_INTEGER64:
							System.out.printf("%1$d", JFAL.GetCursorValueInt64(hSession, hCursor, i));
							break;
						case OT_INTEGER:
							System.out.printf("%1$d", JFAL.GetCursorValueInt32(hSession, hCursor, i));
							break;
						case OT_SMALLINT:
							System.out.printf("%1$d", JFAL.GetCursorValueInt16(hSession, hCursor, i));
							break;
						case OT_BINARY:
							nbytes = JFAL.PrepareCursorValueBinary(hSession, hCursor, i);
							System.out.printf("%1$d bytes", nbytes);
							break;
						case OT_OBJECT:
							nbytes = JFAL.PrepareCursorValueGeometry(hSession, hCursor, i);
							data = new byte[(int)nbytes];
							JFAL.GetData(hSession, data);
							PrintGeometry(data, nbytes);
							break;
						case OT_STYLE:
							System.out.printf("%1$s", JFAL.GetCursorValueStyle(hSession, hCursor, i));
							break;
						case OT_TIMESPAN:
							System.out.printf("%1$fs", JFAL.GetCursorValueTimespanInMilliseconds(hSession, hCursor, i) / 1000.0);
							break;
						case OT_DATE:
							efalDate = JFAL.GetCursorValueDate(hSession, hCursor, i);
							System.out.printf("%1$d-%2$d-%3$d", efalDate.getYear(), efalDate.getMonth(), efalDate.getDay());
							break;
						case OT_DATETIME:
							efalDateTime = JFAL.GetCursorValueDateTime(hSession, hCursor, i);
							System.out.printf("%1$d-%2$d-%3$d %4$d:%5$d:%6$d.%7$d", efalDateTime.getYear(), efalDateTime.getMonth(), efalDateTime.getDay(), efalDateTime.getHour(), efalDateTime.getMinute(), efalDateTime.getSecond(), efalDateTime.getMillisecond());
							break;
						case OT_TIME:
							efalTime = JFAL.GetCursorValueTime(hSession, hCursor, i);
							System.out.printf("%1$d:%2$d:%3$d.%4$d", efalTime.getHour(), efalTime.getMinute(), efalTime.getSecond(), efalTime.getMillisecond());
							break;
						}
					}
					System.out.print("\t");
				}
				System.out.print("\n");
			}
			System.out.printf("%1$d records\n", recordCount);
			JFAL.DisposeCursor(hSession, hCursor);
		}
		
		long finish = System.nanoTime();
		double duration = (double)(finish - start) / NANOSECONDS_TO_SECONDS;
		System.out.printf("%1$6.1f seconds\n", duration);
	}
	private void Insert(String txt)
	{
		long start = System.nanoTime();
		long recordCount = JFAL.Insert(hSession, txt);
		if (!MyReportErrors())
		{
			System.out.printf("%1$d records affected.\n", recordCount);
		}
		long finish = System.nanoTime();
		double duration = (double)(finish - start) / NANOSECONDS_TO_SECONDS;
		System.out.printf("%1$6.1f seconds\n", duration);
	}
	private void Update(String txt)
	{
		long start = System.nanoTime();
		long recordCount = JFAL.Update(hSession, txt);
		if (!MyReportErrors())
		{
			System.out.printf("%1$d records affected.\n", recordCount);
		}
		long finish = System.nanoTime();
		double duration = (double)(finish - start) / NANOSECONDS_TO_SECONDS;
		System.out.printf("%1$6.1f seconds\n", duration);
	}
	private void Delete(String txt)
	{
		long start = System.nanoTime();
		long recordCount = JFAL.Delete(hSession, txt);
		if (!MyReportErrors())
		{
			System.out.printf("%1$d records affected.\n", recordCount);
		}
		long finish = System.nanoTime();
		double duration = (double)(finish - start) / NANOSECONDS_TO_SECONDS;
		System.out.printf("%1$6.1f seconds\n", duration);
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
		long start = System.nanoTime();
		int idxEq = txt.indexOf("=");
		if (idxEq > 0)
		{
			String vname = txt.substring(0, idxEq);
			String vvalue = txt.substring(idxEq+1);
			JFAL.ClearErrors(hSession);
			JFAL.CreateVariable(hSession, vname);
			JFAL.SetVariableValue(hSession, vname, vvalue);
			if (!MyReportErrors())
			{
				long finish = System.nanoTime();
				double duration = (double)(finish - start) / NANOSECONDS_TO_SECONDS;
				System.out.printf("%1$6.1f seconds\n", duration);
			}
		}
	}
	private void PrintVariableValue(String name)
	{
		if (JFAL.GetVariableIsNull(hSession, name))
		{
			System.out.print("(null)");
			return;
		}
		long nbytes = 0;
		byte[] data = null;
		EFALDATE efalDate;
		EFALTIME efalTime;
		EFALDATETIME efalDateTime;
		
		switch (JFAL.GetVariableType(hSession, name))
		{
			case OT_CHAR:
				System.out.printf("%1$s", JFAL.GetVariableValueString(hSession, name));
				break;
			case OT_LOGICAL:
				System.out.printf("%1$s", (JFAL.GetVariableValueBoolean(hSession, name) ? "True" : "False"));
				break;
			case OT_FLOAT:
			case OT_DECIMAL:
				System.out.printf("%1$f", JFAL.GetVariableValueDouble(hSession, name));
				break;
			case OT_INTEGER64:
				System.out.printf("%1$d", JFAL.GetVariableValueInt64(hSession, name));
				break;
			case OT_INTEGER:
				System.out.printf("%1$d", JFAL.GetVariableValueInt32(hSession, name));
				break;
			case OT_SMALLINT:
				System.out.printf("%1$d", JFAL.GetVariableValueInt16(hSession, name));
				break;
			case OT_BINARY:
				nbytes = JFAL.PrepareVariableValueBinary(hSession, name);
				System.out.printf("%1$d bytes", nbytes);
				break;
			case OT_OBJECT:
				nbytes = JFAL.PrepareVariableValueGeometry(hSession, name);
				data = new byte[(int)nbytes];
				JFAL.GetData(hSession, data);
				PrintGeometry(data, nbytes);
				break;
			case OT_STYLE:
				System.out.printf("%1$s", JFAL.GetVariableValueStyle(hSession, name));
				break;
			case OT_TIMESPAN:
				System.out.printf("%1$fs", JFAL.GetVariableValueTimespanInMilliseconds(hSession, name) / 1000.0);
				break;
			case OT_DATE:
				efalDate = JFAL.GetVariableValueDate(hSession, name);
				System.out.printf("%1$d-%2$d-%3$d", efalDate.getYear(), efalDate.getMonth(), efalDate.getDay());
				break;
			case OT_DATETIME:
				efalDateTime = JFAL.GetVariableValueDateTime(hSession, name);
				System.out.printf("%1$d-%2$d-%3$d %4$d:%5$d:%6$d.%7$d", efalDateTime.getYear(), efalDateTime.getMonth(), efalDateTime.getDay(), efalDateTime.getHour(), efalDateTime.getMinute(), efalDateTime.getSecond(), efalDateTime.getMillisecond());
				break;
			case OT_TIME:
				efalTime = JFAL.GetVariableValueTime(hSession, name);
				System.out.printf("%1$d:%2$d:%3$d.%4$d", efalTime.getHour(), efalTime.getMinute(), efalTime.getSecond(), efalTime.getMillisecond());
				break;
		}
	}
	private void Show(String txt)
	{
		if ((txt != null) && (txt.length() > 0))
		{
			for (long i = 0, n = JFAL.GetVariableCount(hSession); i < n; i++)
			{
				if (JFAL.GetVariableName(hSession, i).equalsIgnoreCase(txt))
				{
					String name = JFAL.GetVariableName(hSession, i);
					System.out.print(name);
					System.out.print("=");
					PrintVariableValue(name);
					System.out.print("\n");
				}
			}
		}
		else
		{
			System.out.print("\nVariables:\n----------------------------------------\n");
			for (long i = 0, n = JFAL.GetVariableCount(hSession); i < n; i++)
			{
				String name = JFAL.GetVariableName(hSession, i);
				System.out.print(name);
				System.out.print("=");
				PrintVariableValue(name);
				System.out.print("\n");
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
		System.out.print("\n");
		System.out.print("\n");
		System.out.print("Lines beginning with ' or # are comment lines and will be ignored.\n");
		System.out.print("A blank line terminates the execution.\n");
		System.out.print("\n");
		System.out.print("Commands:\n");
		System.out.print("  Open [Table] <<filename>> [as <<alias>>]\n");
		System.out.print("  Pack <<table>>\n");
		System.out.print("  Close {<<table>> | ALL}\n");
		System.out.print("  Set <<var>> = <<expr>>\n");
		System.out.print("  Show [<<var>>]\n");
		System.out.print("  Tables\n");
		System.out.print("  Desc <<table>>\n");
		System.out.print("  Save [table] <<table>> in <<filename>> [charset << charset >> ][NOINSERT]\n");
		System.out.print("  Save [table] <<table>> as nativex table in <<filename>> [UTF16 | UTF8 | charset << charset >> ][NOINSERT]\n");
		System.out.print("  Save [table] <<table>> as geopackage table <<dbtable>> in <<file>>\n");
		System.out.print("                                     [coordsys <<codespace:code>>]\n");
		System.out.print("                                     [UTF16 | UTF8 | charset << charset >> ][NOINSERT]\n");
		System.out.print("  Select ...\n");
		System.out.print("  Insert ...\n");
		System.out.print("  Update ...\n");
		System.out.print("  Delete ...\n");
		System.out.print("\n");
	}
	private void ProcessCommand(String command)
	{
		// Process any commands that our command processor doesn't know how to handle...
		if (command.toLowerCase().startsWith("'")) return;
		else if (command.toLowerCase().startsWith("#")) return;
		else if (command.toLowerCase().startsWith("open ")) Open(command.substring(5)); 
		else if (command.toLowerCase().startsWith("pack ")) Pack(command.substring(5)); 
		else if (command.toLowerCase().startsWith("close ")) Close(command.substring(6)); 
		else if (command.toLowerCase().startsWith("tables")) Tables(); 
		else if (command.toLowerCase().startsWith("desc ")) Describe(command.substring(5));
		else if (command.toLowerCase().startsWith("save table ")) SaveAs(command.substring(11));
		else if (command.toLowerCase().startsWith("save ")) SaveAs(command.substring(5));
		else if (command.toLowerCase().startsWith("select ")) Select(command);
		else if (command.toLowerCase().startsWith("insert ")) Insert(command);
		else if (command.toLowerCase().startsWith("update ")) Update(command);
		else if (command.toLowerCase().startsWith("delete ")) Delete(command);
		
		else if (command.toLowerCase().startsWith("tokenize ")) tokenize(command.substring(9));

		else if (command.toLowerCase().startsWith("set ")) Set(command.substring(4)); 
		else if (command.toLowerCase().startsWith("show ")) Show(command.substring(5)); 
		else if (command.equalsIgnoreCase("show")) Show(null); 

		else if (command.toLowerCase().startsWith("help ")) Help(); 
		else if (command.equalsIgnoreCase("help")) Help(); 

		MyReportErrors();
	}
	public void Prompt()
	{
		boolean done = false;
		
		Scanner scanner = new Scanner(System.in);
		
		while (!done)
		{
			System.out.print("> ");
			String command = scanner.nextLine().trim();
			if (command.length() == 0) return;
			ProcessCommand(command);
		}
	}

}

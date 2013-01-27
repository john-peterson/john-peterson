// © John Peterson. License GNU GPL 3.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Security.Principal;
using System.Security.AccessControl;
using System.Security.Cryptography;
using System.Text;
using Shared;

namespace csumtree {
class csumtree {
	static String src = "", dst = "";
	static bool copyMissing = false, copyDifferent = false, ignoreReadError = false, ignoreWriteError = false;

	// usage
	private static void usage() {
		String str = "\nRecursive folder checksum comparison.\n" +
						"© John Peterson. License GNU GPL 3.\n\n" +
						"csumtree source destination [/f]\n\n" +
						"/m\tReplace missing files\n" +
						"/f\tReplace mismatched files\n" +
						"/ir\tIgnore read error\n" +
						"/iw\tIgnore write error\n";
		Console.WriteLine(str);
	}

	// files compared
	static bool isSystemFile(String file) {
		String ext = Path.GetExtension(file);
		if (ext == ".sys"
			|| ext == ".dll"
			|| ext == ".exe"
			|| ext == ".mum"
			|| ext == ".cat"
			|| ext == ".gpd"
			|| ext == ".xml"
			|| ext == ".ttf") return true;
		else
			return false;
	}

	static void Main(string[] args) {
		readArg(args);

		// reset log file
		FileStream log = new FileStream("csumtree.log", FileMode.Create);
		log.Close();
		Common.logFile = "csumtree.log";

		// check files
		CheckFiles(src);

		Common.Msg("Done");
	}

	// create list of files in path
	static void CheckFiles(string folder) {
		String srcPath, dstPath;
		bool dstExist = false, bSame;

		foreach (string file in Directory.GetFiles(folder)) {
			srcPath = file;

			// skip non system files
			if (!isSystemFile(srcPath)) continue;

			bSame = true;
			dstPath = dst + srcPath.Substring(src.Length);

			try {
				dstExist = File.Exists(dstPath);
			} catch (Exception e) {
				Common.errMsg(e.Message);
				Common.takeOwn(dstPath);
			}

			// compare files
			if (dstExist) {
				String srcCS = getCS(srcPath);
				String dstCS = getCS(dstPath);
				if (srcCS.Length > 0 && dstCS.Length > 0) bSame = srcCS == dstCS;
			}
			// destination file is missing
			if (!dstExist) {
				Common.Msg("► ", false); Common.Msg(String.Format("{0}", dstPath), ConsoleColor.Yellow, false);
				Common.Msg(String.Format(" missing [present at {0}]", srcPath), ConsoleColor.Gray);
				if (copyMissing)
					Common.copy(srcPath, dstPath, ignoreWriteError);
			}
			// file is different
			if (dstExist && !bSame) {
				Common.Msg("► ", false); Common.Msg(String.Format("{0}", dstPath), ConsoleColor.Red, false);
				Common.Msg(String.Format(" mismatch [{0}]", srcPath), ConsoleColor.Gray);
				if (copyDifferent)
					Common.copy(srcPath, dstPath, ignoreWriteError);
			}
		}

		// recurse
		foreach (string subDir in Directory.GetDirectories(folder)) {
			try {
				CheckFiles(subDir);
			} catch (Exception e) {
				Common.errMsg(e.Message);
			}
		}
	}

	static void readArg(string[] args) {
		if (args.Length == 0) {
			usage();
			Environment.Exit(0);
		}
		if (args[0] == "/?") {
			usage();
			Environment.Exit(0);
		}
		List<String> largs = new List<String>();
		for (int i = 0; i < args.Length; i++) {
			if (args[i].Equals("/m", StringComparison.CurrentCultureIgnoreCase)) copyMissing = true;
			if (args[i].Equals("/d", StringComparison.CurrentCultureIgnoreCase)) copyDifferent = true;
			if (args[i].Equals("/ir", StringComparison.CurrentCultureIgnoreCase)) ignoreReadError = true;
			if (args[i].Equals("/iw", StringComparison.CurrentCultureIgnoreCase)) ignoreWriteError = true;
			if (!args[i][0].Equals('/')) largs.Add(args[i]);
		}
		if (largs.Count > 0) src = largs[0];
		if (largs.Count > 1) dst = largs[1];

		// exceptions
		if (!Directory.Exists(src)) {
			Console.WriteLine("\nSource \"{0}\" missing", src);
			Environment.Exit(0);
		}
		if (!Directory.Exists(dst)) {
			Console.WriteLine("\nDestination \"{0}\" missing", dst);
			Environment.Exit(0);
		}
	}

	// checksum
	static String getCS(String file) {
		int r = 0;
		return getCS_(file, ref r);
	}
	static String getCS_(String file, ref int r) {
		try {
			FileStream fs = new FileStream(file, FileMode.Open, FileAccess.Read);
			MD5 md5 = new MD5CryptoServiceProvider();
			byte[] retVal = md5.ComputeHash(fs);
			fs.Close();
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < retVal.Length; i++)
				sb.Append(retVal[i].ToString("x2"));
			return sb.ToString();
		} catch (Exception e) {
			Common.errMsg(e.Message);
			if (r > 0) {
				Common.dbgMsg(String.Format("Can't read checksum for {0}", file));
				if (!ignoreReadError) Environment.Exit(0);
				return "";
			}
			Common.takeOwn(file);
			r++;
			getCS_(file, ref r);
			return "";
		}
	}
}
}
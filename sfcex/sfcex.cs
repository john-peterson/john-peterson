﻿// © John Peterson. License GNU GPL 3.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Security.Principal;
using System.Security.AccessControl;
using Shared;

namespace sfcex {
class sfcex {
	static bool bFix = false, bDrive = false, ignoreWriteError = false;
	static int ed = -1;
	static String dSrc = "", dDst = "", log = "";

	// usage
	private static void usage() {
		String str =	"\n© John Peterson. License GNU GPL 3.\n\n" +
						"Scans a System File Checker (sfc) log for corrupted files (including files in the source store) and replace them from a Windows Imaging Format installation file or a Windows installation file system.\n\n" +
						"sfcex /d:? (/w:#|/s:?) [/f]\n\n" +
						"/d:?\tCopy to ?\\windows\n" +
						"/w:?:#\tCopy from ?:\\sources\\install.wim\\# (# is edition (specified in ?:\\sources\\install.wim\\1.xml))\n" +
						"/s:?\tCopy from ?\\windows\n" +
						"/f\tFix errors. (Otherwise list errors only.)";
		Console.WriteLine(str);
	}

	static void Main(string[] args) {
		readArg(args);
		List<string> lines = new List<string>();
		try {
			using (StreamReader sr = new StreamReader(log)) {
				String line, src, dst;
				while ((line = sr.ReadLine()) != null) {
					if (line.IndexOf(" do not match") > -1) {
						src = line.Substring(line.IndexOf("\\"), line.IndexOf(" do not match") - line.IndexOf("\\"));
						src = src.Replace("C:\\", dSrc.ToUpper() + ":\\");
						src = src.Replace("\\SystemRoot", dSrc.ToUpper() + ":\\Windows");
						src = src.Replace("\\??\\", "");
						dst = src.Replace(dSrc.ToUpper() + ":", dDst.ToUpper() + ":");
						if (bDrive)
							readLocal(src, dst);
						else
							readInstall(src, dst);
					}
				}
			}
		} catch (Exception e) {
			Common.errMsg(e.Message);
		}
		Console.WriteLine("Done");
	}

	// copy from file system
	static void readLocal(String src, String dst) {
		String file;
		file = Path.GetFileName(src);
		if (File.Exists(src)) {
			Common.Msg("► copy /y ", false);
			Common.Msg(String.Format("{0}", src), bFix ? ConsoleColor.Green : ConsoleColor.Yellow, false);
			Common.Msg(String.Format(" {0}", dst));
			if (bFix) Common.copy(src, dst, true, ignoreWriteError);
		} else {
			Common.Msg(String.Format("{0}", src), ConsoleColor.Red, false);
			Common.Msg(String.Format(" not found", src));
		}
	}

	// copy from wim file
	static void readInstall(String src, String dst) {
		String relPath, file;
		file = Path.GetFileName(src);
		relPath = src.Substring(src.IndexOf("\\"));
		Console.WriteLine(String.Format("Extracting {0}", file));
		Common.extractFile(src, relPath, dSrc, ed);
		src = String.Format("{0}:\\sources\\install.wim\\{1}{2}", dSrc.ToUpper(), ed, relPath);
		if (!File.Exists(Path.GetFileName(file))) {
			Common.errMsg(String.Format("{0} not found", src));
			return;
		}
		Common.Msg("► copy /y ", false);
		Common.Msg(String.Format("{0}", src), bFix ? ConsoleColor.Green : ConsoleColor.Yellow, false);
		Common.Msg(String.Format(" {0}", dst));
		if (bFix) Common.copy(file, dst, false, ignoreWriteError);
		File.Delete(file);
	}

	// arguments

	static String parseArg(String arg) {
		if (arg.IndexOf(":") == -1) {
			Console.WriteLine("\nDrive in \"{0}\" missing", arg);
			Environment.Exit(0);
		}
		return arg.Substring(arg.IndexOf(":") + 1, 1);
	}
	static int parseEdition(String arg) {
		if ((arg.Split(':').Length - 1) != 2) {
			Console.WriteLine("\nEdition in \"{0}\" missing", arg);
			Environment.Exit(0);
		}
		return Convert.ToInt32(arg.Substring(arg.LastIndexOf(":") + 1, 1));
	}

	static void readArg(string[] args) {
		if (args.Length == 0) {
			usage();
			Environment.Exit(0);
		}
		for (int i = 0; i < args.Length; i++) {
			if (args[i] == "/?") {
				usage();
				Environment.Exit(0);
			}
			if (args[i].Equals("/f", StringComparison.CurrentCultureIgnoreCase)) bFix = true;
			if (args[i].IndexOf("/d", StringComparison.CurrentCultureIgnoreCase) == 0) dDst = parseArg(args[i]);
			if (args[i].IndexOf("/w", StringComparison.CurrentCultureIgnoreCase) == 0) {
				dSrc = parseArg(args[i]);
				ed = parseEdition(args[i]);
			}
			if (args[i].IndexOf("/s", StringComparison.CurrentCultureIgnoreCase) == 0) {
				dSrc = parseArg(args[i]);
				bDrive = true;
			}
			if (!args[i][0].Equals('/')) log = args[i];
		}

		// exceptions
		if (!File.Exists(log)) {
			Console.WriteLine("\nLog file \"{0}\" missing", log);
			Environment.Exit(0);
		}
		if (dSrc.Length == 0) {
			Console.WriteLine("\nSource drive letter not specified");
			Environment.Exit(0);
		}
		if (dDst.Length == 0) {
			Console.WriteLine("\nDestination drive letter not specified");
			Environment.Exit(0);
		}
	}
}
}
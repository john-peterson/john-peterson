// System File Checker (sfc) log scan and copy utility
// (C) John Peterson, GNU GPL 3
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Security.Principal;
using System.Security.AccessControl;

namespace sfcex {
	class sfcex {
		static bool bFix = false, bDrive = false;
		static int ed = -1;
		static String dSrc = "", dDst = "", log = "";

		// usage
		private static void usage() {
			String str =	"\n(C) John Peterson 2012, GNU GPL 3\n\n" +
							"Scans a System File Checker (sfc) log for corrupted files (including files in the source store) and replace them from a Windows Imaging Format installation file or a Windows installation file system.\n\n" +
							"SFCEX /S:? /D:? [/L:?] [/F] \n\n" +
							"/S:?:#\tUse ?:\\sources\\install.wim\\# as source where # signify the edition (specified in ?:\\sources\\install.wim\\1.xml).\n" +
							"/D:?\tUse drive letter ? as destination disk.\n" +
							"/L:?\tUse drive letter ? as source. (If specified /S is not required.)\n" +
							"/F\tFixes errors. Otherwise list changes only.";
			Console.WriteLine(str);
		}

		// take ownership and set write permission
		public static void TakeOwn(String file) {
			Process p = new Process();
			p.StartInfo.FileName = "takeown";
			p.StartInfo.Arguments = String.Format("/f {0}", file);
			p.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
			p.Start();
			p.WaitForExit();
			p.StartInfo.FileName = "icacls";
			p.StartInfo.Arguments = String.Format("{0} /grant administrators:F", file);
			p.Start();
			p.WaitForExit();
		}

		// extract wim file
		static void extractFile(String src, String relPath) {
			Process p = new Process();
			p.StartInfo.FileName = "7z";
			p.StartInfo.Arguments = String.Format("e -aoa {0}:\\sources\\install.wim {1}{2}", dSrc.ToUpper(), ed, relPath);
			p.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
			p.Start();
			p.PriorityClass = ProcessPriorityClass.Idle;
			p.WaitForExit();
		}

		// copy
		static void copy(String src, String dst, bool sourceTakeown = false) {
			try {
				if (sourceTakeown) TakeOwn(src);
				TakeOwn(dst);
				System.IO.File.Copy(src, dst, true);
			} catch (Exception e) {
				Console.ForegroundColor = ConsoleColor.Red;
				Console.WriteLine(e.Message);
				Console.ResetColor();
			}
		}

		// copy from local file system
		static void readLocal(String src, String dst) {
			String file;
			file = Path.GetFileName(src);
			if (File.Exists(src)) {
				Console.ForegroundColor = bFix ? ConsoleColor.Green : ConsoleColor.Yellow;
				Console.Write(String.Format("►Copy {0}", file));				
				Console.ForegroundColor = ConsoleColor.Gray;
				Console.WriteLine(String.Format(" [{0} to {1}]", src, dst));
				Console.ResetColor();
				if (bFix) copy(src, dst, true);
			} else {
				Console.ForegroundColor = ConsoleColor.Red;
				Console.Write(String.Format("{0}", file));
				Console.ForegroundColor = ConsoleColor.Gray;
				Console.WriteLine(String.Format(" not found [{0}]", src));
				Console.ResetColor();
			}
		}

		// copy from install file
		static void readInstall(String src, String dst) {
			String relPath, file;
			file = Path.GetFileName(src);
			relPath = src.Substring(src.IndexOf("\\"));
			Console.WriteLine(String.Format("Extracting {0} ...", file));
			extractFile(src, relPath);
			src = String.Format("{0}:\\sources\\install.wim\\{1}{2}", dSrc.ToUpper(), ed, relPath);
			if (!File.Exists(Path.GetFileName(file))) {
				Console.ForegroundColor = ConsoleColor.Red;
				Console.WriteLine(String.Format("{0} not found", src));
				Console.ResetColor();
				return;
			}
			Console.ForegroundColor = bFix ? ConsoleColor.Green : ConsoleColor.Yellow;
			Console.WriteLine(String.Format("►Copy {0} to {1}", src, dst));
			Console.ResetColor();
			if (bFix) copy(file, dst);
			File.Delete(file);
		}

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
				if (args[i].IndexOf("/s", StringComparison.CurrentCultureIgnoreCase) == 0) {
					dSrc = parseArg(args[i]);
					ed = parseEdition(args[i]);
				}
				if (args[i].IndexOf("/d", StringComparison.CurrentCultureIgnoreCase) == 0) dDst = parseArg(args[i]);
				if (args[i].IndexOf("/l", StringComparison.CurrentCultureIgnoreCase) == 0) {
					dSrc = parseArg(args[i]);
					bDrive = true;
				}
				if (!args[i][0].Equals('/')) log = args[i];
			}
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
							//Console.ReadKey();
						}
					}
				}
			}
			catch (Exception e) {
				Console.WriteLine(e.Message);
			}
			Console.WriteLine("Done. Press a key to continue . . .");
			Console.ReadKey();
		}
	}
}
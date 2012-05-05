// System File Checker (sfc) log scan and copy utility
// (C) John Peterson, GNU GPL 3
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Security.Principal;
using System.Security.AccessControl;
using System.Security.Cryptography;
using System.Text;

namespace csumtree {
	class csumtree {
		static String src = "", dst = "";
		static bool bFix = false;

		// usage
		private static void usage() {
			String str = "\nRecursive folder checksum comparison.\n" +
							"(C) John Peterson 2012, GNU GPL 3\n\n" +
							"csumtree source destination [/f]\n\n" +
							"/f\tReplace mismatched files. Otherwise list changes only.";
			Console.WriteLine(str);
		}

		static void dbgMsg(String s, bool newLine = true) {
			if (newLine) s = s + "\n";
#if (DEBUG)
			Console.Write(s);
#endif
			fileMsg(s);
		}
		static void errMsg(String s, bool newLine = true) {
			if (newLine) s = s + "\n";
			Console.ForegroundColor = ConsoleColor.Red;
			Console.Write(s);
			Console.ResetColor();
			fileMsg(s);
		}

		static void Msg(String s, ConsoleColor c = ConsoleColor.Gray, bool newLine = true) {
			if (newLine) s = s + "\n";
			Console.ForegroundColor = c;
			Console.Write(s);
			Console.ResetColor();
			fileMsg(s);
		}
		
		static void fileMsg(String s) {
			StreamWriter sw = File.AppendText("csumtree.log");
			sw.Write(s);
			sw.Flush();
			sw.Close();
		}

		static bool isSystemFile(String file) {
			String ext = Path.GetExtension(file);
			if (ext == ".sys"
				|| ext == ".dll"
				|| ext == ".exe"
				|| ext == ".mum"
				|| ext == ".cat"
				|| ext == ".ttf") return true;
			else
				return false;
		}

		static List<String> GetFilesEx(string folder, ref List<String> files) {
			foreach (string file in Directory.GetFiles(folder)) {
				files.Add(file);
			}
			foreach (string subDir in Directory.GetDirectories(folder)) {
				try {
					GetFilesEx(subDir, ref files);
				} catch (Exception e) {
					errMsg(e.Message);
				}
			}
			return files;
		}

		// take ownership and set write permission
		public static void TakeOwn(String file) {
			dbgMsg(String.Format("Taking ownership of {0}", file));
			Process p = new Process();
			p.StartInfo.FileName = "takeown";
			p.StartInfo.Arguments = String.Format("/f \"{0}\"", file);
			p.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
			p.Start();
			p.WaitForExit();
			p.StartInfo.FileName = "icacls";
			p.StartInfo.Arguments = String.Format("\"{0}\" /grant administrators:F", file);
			p.Start();
			p.WaitForExit();
		}

		// copy
		static void copy(String src, String dst, bool sourceTakeown = false) {
			Msg(String.Format("►Copy {0} to {1}", src, dst), bFix ? ConsoleColor.Green : ConsoleColor.Yellow);
			if (!bFix) return;
			try {
				if (sourceTakeown) TakeOwn(src);
				TakeOwn(dst);
				System.IO.File.Copy(src, dst, true);
			} catch (Exception e) {
				dbgMsg(e.Message);
			}
		}

		static String parseArg(String arg) {
			if (arg.IndexOf(":") == -1) {
				Console.WriteLine("\nDirectory in \"{0}\" missing", arg);
				Environment.Exit(0);
			}
			return arg.Substring(arg.IndexOf(":") + 1, 1);
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
				if (args[i].Equals("/f", StringComparison.CurrentCultureIgnoreCase)) bFix = true;
				if (!args[i][0].Equals('/')) largs.Add(args[i]);
			}
			if (largs.Count > 0) src = largs[0];
			if (largs.Count > 1) dst = largs[1];
			if (!Directory.Exists(src)) {
				Console.WriteLine("\nSource \"{0}\" missing", src);
				Environment.Exit(0);
			}
			if (!Directory.Exists(dst)) {
				Console.WriteLine("\nDestination \"{0}\" missing", dst);
				Environment.Exit(0);
			}
		}

		static String getCSEx2(String file, ref int i) {
			try {
				return getCS(file);
			} catch (Exception e) {
				if (i > 0) {
					dbgMsg(String.Format("Cannot read checksum for {0}", file));
					return "";
				}
				dbgMsg(e.Message);
				TakeOwn(file);
				i++;
				getCSEx2(file, ref i);
				return "";
			}
		}

		static String getCSEx(String file) {
			int i = 0;
			return getCSEx2(file, ref i);
		}

		static String getCS(String file) {
			FileStream fs = new FileStream(file, FileMode.Open);
			MD5 md5 = new MD5CryptoServiceProvider();
			byte[] retVal = md5.ComputeHash(fs);
			fs.Close();
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < retVal.Length; i++) {
				sb.Append(retVal[i].ToString("x2"));
			}
			return sb.ToString();
		}

		static void Main(string[] args) {
			String file, dstPath;
			bool bExist = false, bSame;
			readArg(args);
			List<String> files = new List<String>();
			GetFilesEx(src, ref files);
			FileStream log = new FileStream("csumtree.log", FileMode.Create);
			log.Close();
			for (int i = 0; i < files.Count; i++) {
				String srcPath = files[i];
				fileMsg(String.Format("{0}/{1} {2}\n", i, files.Count, srcPath));
				if (!isSystemFile(srcPath)) continue;
				bSame = true;
				dstPath = dst + srcPath.Substring(src.Length);
				file = Path.GetFileName(srcPath);
				try {
					bExist = File.Exists(dstPath);
				} catch (Exception e) {
					dbgMsg(e.Message);
					TakeOwn(dstPath);
				}
				if (bExist) {
					String srcCS = getCSEx(srcPath);
					String dstCS = getCSEx(dstPath);
					if (srcCS.Length > 0 && dstCS.Length > 0) bSame = srcCS == dstCS;
				}
				if (!bExist) {
					Msg(String.Format("{0}", file), ConsoleColor.Yellow, false);
					Msg(String.Format(" missing [{0}]", dstPath), ConsoleColor.Gray);
				}
				if (bExist && !bSame) {
					Msg(String.Format("{0}", file), ConsoleColor.Red, false);
					Msg(String.Format(" mismatch [{0}]", srcPath), ConsoleColor.Gray);
					copy(srcPath, dstPath);
				}
			}
			Console.WriteLine("Done. Press a key to continue . . .");
			Console.ReadKey();
		}
	}
}
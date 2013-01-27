// © John Peterson. License GNU GPL 3.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
//using System.Threading.Tasks;

namespace Shared
{
public class Common
{
	public static String logFile;

	public static void dbgMsg(String s, bool newLine = true) {
#if (DEBUG)
		Msg(s);
#endif
	}
	public static void errMsg(String s, bool newLine = true) {
		Msg(s, ConsoleColor.Red);
	}

	public static void Msg(String s, ConsoleColor c = ConsoleColor.Gray, bool newLine = true) {
		if (newLine) s = s + "\n";
		Console.ForegroundColor = c;
		Console.Write(s);
		Console.ResetColor();
		fileMsg(s);
	}
	public static void Msg(String s, bool newLine = true) {
		Msg(s, ConsoleColor.Gray, newLine);
	}
	public static void Msg(String s) {
		Msg(s, ConsoleColor.Gray, true);
	}

	public static void fileMsg(String s) {
		if (String.IsNullOrEmpty(logFile)) return;
		StreamWriter sw = File.AppendText(logFile);
		sw.Write(s);
		sw.Flush();
		sw.Close();
	}

	// files

	// take ownership and set write permission
	public static void takeOwn(String file) {
		String perm;
		Process p = new Process();
		p.StartInfo.FileName = "takeown";
		p.StartInfo.Arguments = String.Format("/f \"{0}\"", file);
		p.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
		p.Start();
		p.WaitForExit();
		dbgMsg(String.Format("{0} {1}", p.StartInfo.FileName, p.StartInfo.Arguments));
		p.StartInfo.FileName = "icacls";
		if (Common.IsDir(file))
			perm = "(oi)(ci)f";
		else
			perm = "f";
		p.StartInfo.Arguments = String.Format("\"{0}\" /grant administrators:{1}", file, perm);
		p.Start();
		p.WaitForExit();
		dbgMsg(String.Format("{0} {1}", p.StartInfo.FileName, p.StartInfo.Arguments));
	}

	// copy
	public static void copy(String src, String dst, bool sourceTakeown = false, bool ignoreError = false) {
		// create target path
		String dstDir = Path.GetDirectoryName(dst);
		if (String.IsNullOrEmpty(dstDir)) return;

		if (!String.IsNullOrEmpty(dstDir) && !Directory.Exists(dstDir))
			createDir(dstDir);

		try {
			if (sourceTakeown) takeOwn(src);
			takeOwn(dstDir);
			takeOwn(dst);
			Msg("► copy /y ", false); Msg(String.Format("{0}", src), ConsoleColor.Green, false); Msg(String.Format(" {0}", dst));
			System.IO.File.Copy(src, dst, true);
		} catch (Exception e) {
			errMsg(e.Message);
			if (!ignoreError) Environment.Exit(0);
		}
	}

	// create directory
	public static void createDir(String path) {
		int r = 0;
		createDir_(path, ref r);
	}
	public static void createDir_(String path, ref int r) {
		try {
			dbgMsg(String.Format("► mkdir \"{0}\"", path));
			Directory.CreateDirectory(path);
			takeOwn(path);
		} catch (Exception e) {
			errMsg(e.Message);
			if (r > 0) {
				dbgMsg(String.Format("Can't create {0}", path));
				Environment.Exit(0);
				return;
			}
			takeOwn(Path.GetDirectoryName(path));
			r++;
			createDir_(path, ref r);
			return;
		}
	}

	// extract wim file
	public static void extractFile(String src, String relPath, String dSrc, int ed) {
		if (!Common.Exists("7z.exe")) {
			Console.WriteLine("\n\"7z.exe\" missing");
			Environment.Exit(0);
		}
		Process p = new Process();
		p.StartInfo.FileName = "7z";
		p.StartInfo.Arguments = String.Format("e -aoa {0}:\\sources\\install.wim {1}{2}", dSrc.ToUpper(), ed, relPath);
		p.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
		p.Start();
		// should be called after start
		p.PriorityClass = ProcessPriorityClass.Idle;
		p.WaitForExit();
		Common.dbgMsg(String.Format("{0} {1}", p.StartInfo.FileName, p.StartInfo.Arguments));
	}

	public static bool Exists(string fileName) {
		if (GetFullPath(fileName) != null)
			return true;
		return false;
	}

	public static string GetFullPath(string fileName) {
		if (File.Exists(fileName))
			return Path.GetFullPath(fileName);

		var values = Environment.GetEnvironmentVariable("PATH");
		foreach (var path in values.Split(';')) {
			var fullPath = Path.Combine(path, fileName);
			if (File.Exists(fullPath))
				return fullPath;
		}
		return null;
	}

	public static bool IsDir(String path) {
		return (File.GetAttributes(path) & FileAttributes.Directory) == FileAttributes.Directory;
	}
}
}

// Sparse file tool
// (C) John Peterson. GNU GPL 3.
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Sparse {
class Sparse {
	// Interop
	[DllImport("kernel32.dll")]
	static extern uint GetCompressedFileSizeW([In, MarshalAs(UnmanagedType.LPWStr)] string lpFileName,
	[Out, MarshalAs(UnmanagedType.U4)] out uint lpFileSizeHigh);

	[DllImport("kernel32.dll", SetLastError = true, PreserveSig = true)]
	static extern int GetDiskFreeSpaceW([In, MarshalAs(UnmanagedType.LPWStr)] string lpRootPathName,
	out uint lpSectorsPerCluster, out uint lpBytesPerSector, out uint lpNumberOfFreeClusters,
	out uint lpTotalNumberOfClusters);

	// usage
	private static void usage() {
		String str = "\n(C) John Peterson 2012, GNU GPL 3\n\n" +
						"Sparse file tool.\n\n" +
						"sparse [/s] file\n\n" +
						"/s\tSet sparse flag. Otherwise show sparse flag and size.";
		Console.WriteLine(str);
	}

	static void Main(string[] args) {
		// print usage and exit
		if (args.Length == 0) {
			usage();
			Environment.Exit(0);
		}
		string f = args[args.Length-1];
		try {
			if (args.Length > 0)
				// set as sparse
				if (args[0] == "/s") {
					FileStream fs = File.Open(f, FileMode.Open);
					Console.WriteLine("Working ...", f);
					SparseProject.SparseFile.setAsSparse(fs);
					SparseProject.SparseFile.SetZeroAll(fs);
					fs.Close();
					Console.WriteLine("Done. {0} is now sparse.", f);
					FileSize(f);
					// print info
				} else if (SparseProject.SparseFile.checkSparse(f))
					FileSize(f);
				else {
					Console.WriteLine("{0} is not sparse.", f);
					FileSize(f);
				}
		} catch (Exception e) {
			Console.WriteLine("{0}", e.Message);
		}
	}

	// file size
	public static void FileSize(string f) {
		Console.WriteLine("Size: {0:##,#}\tSize on disk: {1:##,#}", new FileInfo(f).Length, GetFileSizeOnDisk(f));
	}

	// sparse file size
	public static long GetFileSizeOnDisk(string f) {
		FileInfo info = new FileInfo(f);
		uint dummy, sectorsPerCluster, bytesPerSector;
		int result = GetDiskFreeSpaceW(info.Directory.Root.FullName, out sectorsPerCluster, out bytesPerSector, out dummy, out dummy);
		if (result == 0) throw new Win32Exception();
		uint clusterSize = sectorsPerCluster * bytesPerSector;
		uint hosize;
		uint losize = GetCompressedFileSizeW(f, out hosize);
		long size;
		size = (long)hosize << 32 | losize;
		return ((size + clusterSize - 1) / clusterSize) * clusterSize;
	}
}
}

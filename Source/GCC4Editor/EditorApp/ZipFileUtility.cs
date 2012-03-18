//========================================================================
// ZipFileUtility.cs - creates a ZIP file readable by the GameCode4 engine
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Packaging;
using System.Xml;

namespace EditorApp
{
    //----------------------------------------------------------------
    // ZipFileUtility                       - Chapter 22, page 782
    //----------------------------------------------------------------

    class ZipFileUtility
    {
        public static void Create(string rootDirectoryName, string zipFileName)
        {
            DirectoryInfo rootDirectory = new DirectoryInfo(rootDirectoryName);
            int rootDirLen = rootDirectory.FullName.Length;

            using (Package package = ZipPackage.Open(zipFileName, FileMode.Create))
            {
                var stack = new Stack<string>();
                stack.Push(rootDirectory.FullName);

                while (stack.Count > 0)
                {
                    var currentNode = stack.Pop();
                    var directoryInfo = new DirectoryInfo(currentNode);
                    foreach (var directory in directoryInfo.GetDirectories())
                    {
                        FileAttributes attributes = File.GetAttributes(directory.FullName);
                        if ((attributes & FileAttributes.Hidden) == 0 && directory.Name != "Editor")
                        {
                            stack.Push(directory.FullName);
                        }
                    }
                    foreach (var file in directoryInfo.GetFiles())
                    {
                        FileAttributes attributes = File.GetAttributes(file.FullName);
                        if ((attributes & FileAttributes.Hidden) == 0)
                        {
                            Console.WriteLine("Packing " + file.FullName);
                            string relativeFromRoot = file.FullName.Substring(rootDirLen);
                            Uri relUri = GetRelativeUri(relativeFromRoot);

                            PackagePart packagePart =
                                package.CreatePart(relUri, System.Net.Mime.MediaTypeNames.Application.Octet, CompressionOption.Maximum);
                            using (FileStream fileStream = new FileStream(file.FullName, FileMode.Open, FileAccess.Read))
                            {
                                CopyStream(fileStream, packagePart.GetStream());
                            }
                        }
                    }
                }
            }
        }

        private static void CopyStream(Stream source, Stream target)
        {
            const int bufSize = 16384;
            byte[] buf = new byte[bufSize];
            int bytesRead = 0;
            while ((bytesRead = source.Read(buf, 0, bufSize)) > 0)
                target.Write(buf, 0, bytesRead);
        }

        private static Uri GetRelativeUri(string currentFile)
        {
            string pastBackslashes = currentFile.Substring(currentFile.IndexOf('\\'));
            string nukeDoubleBackslash = pastBackslashes.Replace('\\', '/');
            string nukeSpaces = nukeDoubleBackslash.Replace(' ', '_');

            return new Uri(RemoveAccents(nukeSpaces), UriKind.Relative);
        }

        private static string RemoveAccents(string input)
        {
            string normalized = input.Normalize(NormalizationForm.FormKD);
            Encoding removal = Encoding.GetEncoding(Encoding.ASCII.CodePage, new EncoderReplacementFallback(""), new DecoderReplacementFallback(""));
            byte[] bytes = removal.GetBytes(normalized);
            return Encoding.ASCII.GetString(bytes);
        }

    }

}

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Xml;
using System.Xml.Linq;

namespace MigrateDocumentation
{
    public static class Program
    {
        private class Replacement
        {
            Replacement(int start, int length, string value)
            {
                Start = start;
                Length = length;
                Value = value;
            }

            public int Start { get; }

            public int Length { get; }

            public string Value { get; }
        }

        public static void Main()
        {
            DirectoryInfo solutionFolder = GetSolutionFolder();
            DirectoryInfo docsFolder = new DirectoryInfo(Path.Combine(solutionFolder.FullName, "docs"));
            Environment.CurrentDirectory = docsFolder.FullName;

            ISet<string> fileNames = new HashSet<string>();

            IEnumerable<FileInfo> docFiles = docsFolder.GetFiles().ToList();
            foreach (FileInfo docFile in docFiles)
            {
                string oldName = docFile.Name;
                string newName = TransformName(oldName);
                if (oldName != newName)
                {
                    Console.WriteLine($"{oldName} -> {newName}");
                    File.Move(oldName, newName);
                }

                int dot = newName.LastIndexOf('.');
                fileNames.Add(newName.Substring(0, dot));
            }

            docFiles = docsFolder.GetFiles();
            foreach (FileInfo docFile in docFiles)
            {
                string name = docFile.Name;
                if (!name.EndsWith(".md"))
                {
                    continue;
                }

                Console.WriteLine($"{name}");
                string text = File.ReadAllText(docFile.FullName, Encoding.UTF8);

                StringBuilder sb = new StringBuilder();
                int pos = 0;
                Match match = Regex.Match(text, @"\(([^()]*)\)");
                while (match.Success)
                {
                    var length = match.Index + 1 - pos;
                    sb.Append(text.Substring(pos, length));

                    string innerValue = match.Value.Substring(1, match.Length - 2);
                    string link = innerValue;
                    string newName = TransformName(link);
                    if (fileNames.Contains(newName))
                    {
                        Console.WriteLine(newName);
                        sb.Append(newName).Append(')');
                    }
                    else
                    {
                        ConsoleColor color = Console.ForegroundColor;
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine(newName);
                        Console.ForegroundColor = color;
                        sb.Append(innerValue).Append(')');
                    }

                    pos = match.Index + match.Length;
                    match = match.NextMatch();
                }

                sb.Append(text.Substring(pos));
                string newText = sb.ToString();
                File.WriteAllText(docFile.FullName, newText, Encoding.UTF8);
            }
        }

        private static string TransformName(string oldName)
        {
            return oldName
                .Replace(" — ", "-")
                .Replace("-—-", "-")
                .Replace("_ ", "-")
                .Replace("_", "-")
                .Replace(",", "")
                .Replace(" ", "-");
        }

        private static DirectoryInfo GetSolutionFolder()
        {
            DirectoryInfo directoryInfo = new DirectoryInfo(Environment.CurrentDirectory);
            while (directoryInfo.Name != "textedit")
            {
                directoryInfo = directoryInfo.Parent;
                if (directoryInfo == null)
                {
                    throw new Exception("Unable to find solution root folder");
                }
            }

            return directoryInfo;
        }
    }
}

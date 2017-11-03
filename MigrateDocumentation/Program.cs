﻿using System;
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
        private const string CSharpTag = "{code:C#}";

        public static void Main()
        {
            DirectoryInfo solutionFolder = GetSolutionFolder();
            DirectoryInfo docsFolder = new DirectoryInfo(Path.Combine(solutionFolder.FullName, "docs"));
            Environment.CurrentDirectory = docsFolder.FullName;

            IDictionary<string, string> nameToFile = new Dictionary<string, string>();

            // Rename files

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
                string key = newName.Substring(dot) == ".md" ? newName.Substring(0, dot) : newName;
                nameToFile.Add(key, newName);
            }

            // Fixup links and code snippets

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
                    if (nameToFile.TryGetValue(newName, out var fileName))
                    {
                        Console.WriteLine(fileName);
                        sb.Append(fileName).Append(')');
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

                // Code snippets

                pos = 0;
                sb.Clear();
                bool opening = true;
                while (true)
                {
                    int prev = pos;
                    pos = newText.IndexOf(CSharpTag, pos, StringComparison.InvariantCulture);
                    if (pos < 0)
                    {
                        sb.Append(newText.Substring(prev));
                        break;
                    }

                    sb.Append(newText.Substring(prev, pos - prev)).Append("```");
                    if (opening)
                    {
                        sb.Append("C++");
                    }

                    opening = !opening;
                    pos += CSharpTag.Length;
                }

                newText = sb.ToString();
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

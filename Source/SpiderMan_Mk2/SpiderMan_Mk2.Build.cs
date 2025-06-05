// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class SpiderMan_Mk2 : ModuleRules
{
	public SpiderMan_Mk2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        // 添加包含路径配置
       
        // 添加模块下所有文件夹
        PublicIncludePaths.AddRange(GetFolders(ModuleDirectory, true, true));


		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
    // GetFolders start--------------------------------------------------------------------------------
    private System.Collections.Generic.List<string> GetFolders(string FolderPath, bool bRecurisive, bool bReturnRelativePath)
    {
        System.Collections.Generic.List<string> Folders = new System.Collections.Generic.List<string>();
        if (bRecurisive)
            GetFoldersRecurisive(FolderPath, Folders);
        else
        {
            System.IO.DirectoryInfo Directory = new System.IO.DirectoryInfo(FolderPath);
            Folders.Add(Directory.FullName);
            foreach (System.IO.DirectoryInfo SubDirectory in Directory.GetDirectories())
            {
                Folders.Add(SubDirectory.FullName);
            }
        }
        string CurRootPath = System.IO.Path.GetDirectoryName(ModuleDirectory);
        CurRootPath = CurRootPath.Replace('\\', '/');
        if (CurRootPath[CurRootPath.Length - 1] != '/')
            CurRootPath = CurRootPath + "/";

        for (int i = 0; i < Folders.Count; ++i)
        {
            string Folder = Folders[i];
            Folder = Folder.Replace('\\', '/');
            if (bReturnRelativePath)
            {
                int Index = Folder.IndexOf(CurRootPath);
                if (Index >= 0)
                    Folder = Folder.Substring(Index + CurRootPath.Length);
            }
            Folders[i] = Folder;
        }
        return Folders;
    }
    private void GetFoldersRecurisive(string CurPath, System.Collections.Generic.List<string> OutFolders)
    {
        System.IO.DirectoryInfo Directory = new System.IO.DirectoryInfo(CurPath);
        OutFolders.Add(Directory.FullName);
        foreach (System.IO.DirectoryInfo SubDirectory in Directory.GetDirectories())
        {
            GetFoldersRecurisive(SubDirectory.FullName, OutFolders);
        }
    }
    // GetFolders end--------------------------------------------------------------------------------
}

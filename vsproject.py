#!/usr/bin/env python3
import os
from pathlib import Path
import uuid

cl_compile = []
cl_include = []
source_dirs = set()
for root, subdirs, files in os.walk('src'):
	for file in [os.path.join(root, f) for f in files]:
		lowerfile = file.lower()
		add_source_dir = False
		if lowerfile.endswith('.cpp') or lowerfile.endswith('.c'):
			cl_compile.append(file)
			add_source_dir = True
		if lowerfile.endswith('.hpp') or lowerfile.endswith('.h'):
			cl_include.append(file)
			add_source_dir = True
		if add_source_dir:
			path = Path(root)
			for i in range(len(path.parents) - 1):
				parent = path.parents[i]
				if not str(parent) in source_dirs:
					source_dirs.add(str(parent))
			source_dirs.add(os.path.dirname(file))

sln = open("The-Powder-Toy.sln", 'w')
sln.write(r"""Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio 2013
VisualStudioVersion = 12.0.40629.0
MinimumVisualStudioVersion = 10.0.40219.1
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "The-Powder-Toy", "The-Powder-Toy.vcxproj", "{57F7954F-6975-4DEE-8C4F-F9B083E05985}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
		Release|Win32 = Release|Win32
		Static|Win32 = Static|Win32
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{57F7954F-6975-4DEE-8C4F-F9B083E05985}.Debug|Win32.ActiveCfg = Debug|Win32
		{57F7954F-6975-4DEE-8C4F-F9B083E05985}.Debug|Win32.Build.0 = Debug|Win32
		{57F7954F-6975-4DEE-8C4F-F9B083E05985}.Release|Win32.ActiveCfg = Release|Win32
		{57F7954F-6975-4DEE-8C4F-F9B083E05985}.Release|Win32.Build.0 = Release|Win32
		{57F7954F-6975-4DEE-8C4F-F9B083E05985}.Static|Win32.ActiveCfg = Static|Win32
		{57F7954F-6975-4DEE-8C4F-F9B083E05985}.Static|Win32.Build.0 = Static|Win32
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
EndGlobal
""")
sln.close()

vcxproj = open("The-Powder-Toy.vcxproj", 'w')
vcxproj.write(r"""<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Static|Win32">
      <Configuration>Static</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{57F7954F-6975-4DEE-8C4F-F9B083E05985}</ProjectGuid>
    <Keyword>Win32Proj</Keyword>
    <WindowsTargetPlatformVersion>10.0.17763.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v142</PlatformToolset>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v142</PlatformToolset>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Static|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v142</PlatformToolset>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Static|Win32'" Label="PropertySheets">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <LinkIncremental>false</LinkIncremental>
    <OutDir>$(SolutionDir)Build\</OutDir>
    <TargetName>Powder</TargetName>
    <IncludePath>$(ProjectDir)includes;$(ProjectDir)includes/SDL2;$(ProjectDir)includes/luajit-2.0;$(ProjectDir)data;$(ProjectDir)src;$(ProjectDir)resources;$(IncludePath)</IncludePath>
    <LibraryPath>$(ProjectDir)Libraries;$(LibraryPath)</LibraryPath>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <LinkIncremental>false</LinkIncremental>
    <OutDir>$(SolutionDir)Build\</OutDir>
    <TargetName>Powder</TargetName>
    <IncludePath>$(ProjectDir)includes;$(ProjectDir)includes/SDL2;$(ProjectDir)includes/luajit-2.0;$(ProjectDir)data;$(ProjectDir)src;$(ProjectDir)resources;$(IncludePath)</IncludePath>
    <LibraryPath>$(ProjectDir)Libraries;$(LibraryPath)</LibraryPath>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Static|Win32'">
    <LinkIncremental>false</LinkIncremental>
    <OutDir>$(SolutionDir)Build\</OutDir>
    <TargetName>Powder</TargetName>
    <IncludePath>$(ProjectDir)includes;$(ProjectDir)includes/SDL2;$(ProjectDir)includes/luajit-2.0;$(ProjectDir)data;$(ProjectDir)src;$(ProjectDir)resources;$(IncludePath)</IncludePath>
    <LibraryPath>$(ProjectDir)Staticlibs;$(LibraryPath)</LibraryPath>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <PreprocessorDefinitions>WIN;X86;X86_SSE2;USE_SDL;STABLE;GRAVFFT;LUACONSOLE;_SCL_SECURE_NO_WARNINGS;WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>
      <WarningLevel>Level1</WarningLevel>
      <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <Optimization>Disabled</Optimization>
      <FloatingPointModel>Fast</FloatingPointModel>
      <ObjectFileName>$(IntDir)\%(RelativeDir)</ObjectFileName>
    </ClCompile>
    <Link>
      <TargetMachine>MachineX86</TargetMachine>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <SubSystem>Windows</SubSystem>
      <AdditionalDependencies>SDL2.lib;SDL2main.lib;shell32.lib;libbz2.lib;pthreadVC2.lib;luajit2.0.lib;libfftw3f-3.lib;zlib.lib;libcurl.lib;ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <PreprocessorDefinitions>WIN;X86;X86_SSE2;USE_SDL;STABLE;GRAVFFT;LUACONSOLE;_SCL_SECURE_NO_WARNINGS;WIN32;NDEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
      <WarningLevel>Level1</WarningLevel>
      <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <FloatingPointModel>Fast</FloatingPointModel>
      <EnableEnhancedInstructionSet>StreamingSIMDExtensions2</EnableEnhancedInstructionSet>
      <ObjectFileName>$(IntDir)\%(RelativeDir)</ObjectFileName>
    </ClCompile>
    <Link>
      <TargetMachine>MachineX86</TargetMachine>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <SubSystem>Windows</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <AdditionalDependencies>SDL2.lib;SDL2main.lib;shell32.lib;libbz2.lib;pthreadVC2.lib;luajit2.0.lib;libfftw3f-3.lib;zlib.lib;libcurl.lib;ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Static|Win32'">
    <ClCompile>
      <PreprocessorDefinitions>WIN;X86;X86_SSE2;USE_SDL;STABLE;GRAVFFT;LUACONSOLE;ZLIB_WINAPI;_SCL_SECURE_NO_WARNINGS;WIN32;NDEBUG;_WINDOWS;PTW32_STATIC_LIB;CURL_STATICLIB;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
      <WarningLevel>Level1</WarningLevel>
      <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <FloatingPointModel>Fast</FloatingPointModel>
      <EnableEnhancedInstructionSet>StreamingSIMDExtensions2</EnableEnhancedInstructionSet>
      <ObjectFileName>$(IntDir)\%(RelativeDir)</ObjectFileName>
    </ClCompile>
    <Link>
      <TargetMachine>MachineX86</TargetMachine>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <SubSystem>Windows</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <AdditionalDependencies>SDL2.lib;SDL2main.lib;shell32.lib;libbz2.lib;pthreadVC2.lib;luajit2.0.lib;libfftw3f-3.lib;zlib.lib;libcurl.lib;ws2_32.lib;Wldap32.lib;crypt32.lib;winmm.lib;dxguid.lib;imm32.lib;version.lib;SetupApi.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <LinkTimeCodeGeneration>UseLinkTimeCodeGeneration</LinkTimeCodeGeneration>
      <IgnoreSpecificDefaultLibraries>
      </IgnoreSpecificDefaultLibraries>
      <ImageHasSafeExceptionHandlers>false</ImageHasSafeExceptionHandlers>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="data\font.cpp" />
    <ClCompile Include="data\hmap.cpp" />
    <ClCompile Include="data\icon.cpp" />
    <ClCompile Include="data\images.cpp" />
    """)
vcxproj.write('\n    '.join([('<ClCompile Include="' + p + '" />') for p in cl_compile]))
vcxproj.write(r"""
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="data\font.h" />
    <ClInclude Include="data\hmap.h" />
    <ClInclude Include="data\icon.h" />
    <ClInclude Include="data\icondoc.h" />
    <ClInclude Include="data\images.h" />
    <ClInclude Include="data\IntroText.h" />
    <ClInclude Include="data\Shaders.h" />
    <ClInclude Include="resources\resource.h" />
    """)
vcxproj.write('\n    '.join([('<ClInclude Include="' + p + '" />') for p in cl_include]))
vcxproj.write(r"""
  </ItemGroup>
  <ItemGroup>
    <ResourceCompile Include="resources\powder-res.rc" />
  </ItemGroup>
  <ItemGroup>
    <None Include="vsproject.py" />
    <None Include="README.md" />
    <None Include="SConscript" />
    <None Include="SConstruct" />
    <None Include="src\graphics\DrawMethodsDef.inc" />
    <None Include="src\graphics\OpenGLDrawMethods.inl" />
    <None Include="src\graphics\RasterDrawMethods.inl" />
    <None Include="src\lua\socket\socket.lua" />
    <None Include="src\SDLMain.m" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>
""")
vcxproj.close()

filters = open("The-Powder-Toy.vcxproj.filters", 'w')
filters.write(r"""<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="src">
      <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>
      <Extensions>cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx</Extensions>
    </Filter>
    <Filter Include="resources">
      <UniqueIdentifier>{67DA6AB6-F800-4c08-8B7A-83BB121AAD01}</UniqueIdentifier>
      <Extensions>rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav</Extensions>
    </Filter>
    <Filter Include="data">
      <UniqueIdentifier>{fc5911e1-d5ba-4da3-9cfa-5631c6914487}</UniqueIdentifier>
    </Filter>
    """)
filters.write('\n    '.join([('<Filter Include="' + p + '">\n      <UniqueIdentifier>{' + str(uuid.uuid4()) + '}</UniqueIdentifier>\n    </Filter>') for p in source_dirs]))
filters.write(r"""
  </ItemGroup>
  <ItemGroup>
    """)
filters.write('\n    '.join([('<ClCompile Include="' + p + '">\n      <Filter>' + os.path.dirname(p) + '</Filter>\n    </ClCompile>') for p in cl_compile]))
filters.write(r"""
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="src\simulation\elements\Element.h">
      <Filter>src\simulation</Filter>
    </ClInclude>
    <ClInclude Include="data\font.h">
      <Filter>data</Filter>
    </ClInclude>
    <ClInclude Include="data\hmap.h">
      <Filter>data</Filter>
    </ClInclude>
    <ClInclude Include="data\icon.h">
      <Filter>data</Filter>
    </ClInclude>
    <ClInclude Include="data\icondoc.h">
      <Filter>data</Filter>
    </ClInclude>
    <ClInclude Include="data\images.h">
      <Filter>data</Filter>
    </ClInclude>
    <ClInclude Include="data\IntroText.h">
      <Filter>data</Filter>
    </ClInclude>
    <ClInclude Include="data\Shaders.h">
      <Filter>data</Filter>
    </ClInclude>
    """)
filters.write('\n    '.join([('<ClInclude Include="' + p + '">\n      <Filter>' + os.path.dirname(p) + '</Filter>\n    </ClInclude>') for p in cl_include]))
filters.write(r"""
    <ClInclude Include="resources\resource.h">
      <Filter>resources</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <ResourceCompile Include="resources\powder-res.rc">
      <Filter>resources</Filter>
    </ResourceCompile>
  </ItemGroup>
  <ItemGroup>
    <None Include="src\graphics\DrawMethodsDef.inc">
      <Filter>src\graphics</Filter>
    </None>
    <None Include="src\graphics\OpenGLDrawMethods.inl">
      <Filter>src\graphics</Filter>
    </None>
    <None Include="src\graphics\RasterDrawMethods.inl">
      <Filter>src\graphics</Filter>
    </None>
    <None Include="vsproject.py" />
    <None Include="SConstruct" />
    <None Include="SConscript" />
    <None Include="README.md" />
    <None Include="src\SDLMain.m" />
	<None Include="src\lua\socket\socket.lua">
      <Filter>src\lua\socket</Filter>
    </None>
  </ItemGroup>
</Project>
""")
filters.close()

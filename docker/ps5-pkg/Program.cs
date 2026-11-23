// Packs a BennuGD PS5 homebrew folder into a debug fPKG.
// Links LibProsperoPkg (GPL-3.0-or-later); this program is GPL-3.0-or-later too.
// https://github.com/SvenGDK/LibProsperoPKG

using LibProsperoPkg;
using LibProsperoPkg.PKG;

if (args.Length != 6)
{
    Console.Error.WriteLine("usage: bennugd-ps5-pkg <source> <output> <content-id> <title-id> <title> <version>");
    return 1;
}

string source = args[0];
string output = args[1];
string contentId = args[2];
string titleId = args[3];
string title = args[4];
string version = args[5];

if (!ProsperoPackageBuilder.IsValidContentId(contentId))
{
    Console.Error.WriteLine($"invalid content id: {contentId}");
    return 1;
}
if (!ProsperoPackageBuilder.IsValidTitleId(titleId))
{
    Console.Error.WriteLine($"invalid title id: {titleId}");
    return 1;
}

Directory.CreateDirectory(output);

ProsperoBuildResult result = ProsperoPackageBuilder.Build(new ProsperoBuildOptions
{
    Mode = ProsperoPackageMode.Homebrew,
    OutputFormat = ProsperoOutputFormat.DebugImage,
    SourceFolder = source,
    OutputFolder = output,
    ContentId = contentId,
    TitleId = titleId,
    Title = title,
    Version = version,
    ApplicationType = ProsperoApplicationType.DemoApp,
    LicenseFree = true,
    GenerateParamJsonIfMissing = true,
}, Console.WriteLine);

var failed = false;
foreach (string warning in result.Warnings)
{
    Console.Error.WriteLine("warning: " + warning);
    if (warning.Contains("fake-sign", StringComparison.OrdinalIgnoreCase)
        || warning.Contains("no raw ELF", StringComparison.OrdinalIgnoreCase))
        failed = true;
}

if (failed || !File.Exists(result.OutputPath) || new FileInfo(result.OutputPath).Length == 0)
{
    Console.Error.WriteLine("PS5 packaging error: package was not produced");
    return 1;
}

ProsperoAcceptanceReport report = ProsperoPkgValidator.Validate(result.OutputPath, contentId);
if (!report.Accepted)
{
    Console.Error.WriteLine("PS5 packaging error: package failed structural validation");
    return 1;
}

Console.WriteLine(result.OutputPath);
return 0;

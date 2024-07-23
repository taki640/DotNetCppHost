using System.Runtime.InteropServices;

namespace DotNetLib;

public static class DotNetLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    public struct LibArgs
    {
        public IntPtr Message;
        public int Number;
    }

    public static int EntryPoint(IntPtr args, int argsLength)
    {
        if (argsLength < Marshal.SizeOf(typeof(LibArgs)))
            return 1;

        LibArgs libArgs = Marshal.PtrToStructure<LibArgs>(args);
        Console.WriteLine($"Hello from the C# side!");
        PrintLibArgs(libArgs);
        return 0;
    }

    private static void PrintLibArgs(LibArgs libArgs)
    {
        string? message = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? Marshal.PtrToStringUni(libArgs.Message)
            : Marshal.PtrToStringUTF8(libArgs.Message);

        Console.WriteLine($"-- message: {(string.IsNullOrEmpty(message) ? "null/empty" : message)}");
        Console.WriteLine($"-- number: {libArgs.Number}");
    }
}

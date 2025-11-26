AudioAnalyzer Minimal - Build & Usage
====================================

要求:
- Windows 10/11 x64
- Visual Studio 2022 (MSVC) 或者支持 CMake 的编译器
- CMake >= 3.15

构建步骤 (命令行):
--------------------
mkdir build
cd build
cmake .. -A x64
cmake --build . --config Release

输出:
- build/Release/audioanalyzer.dll
- 同时会复制为 build/Release/soundtouch.dll 和 build/Release/libkeyfinder.dll

把生成的 DLL 放在你的 C# 可执行文件同目录，或放到 PATH 能找到的位置。

C# 调用 (示例):
-----------------
[DllImport("soundtouch.dll", CallingConvention = CallingConvention.Cdecl)]
private static extern float soundtouch_detectBPMFile([MarshalAs(UnmanagedType.LPStr)] string filename);

[DllImport("libkeyfinder.dll", CallingConvention = CallingConvention.Cdecl)]
private static extern IntPtr analyze_key([MarshalAs(UnmanagedType.LPStr)] string wavFilePath);

[DllImport("libkeyfinder.dll", CallingConvention = CallingConvention.Cdecl)]
private static extern void free_string(IntPtr ptr);

示例:
string wav = "test.wav";
float bpm = soundtouch_detectBPMFile(wav);
IntPtr p = analyze_key(wav);
string key = Marshal.PtrToStringAnsi(p);
free_string(p);

注意:
- 仅支持 16-bit PCM WAV 输入。如果你的文件是其他格式（MP3/OGG/FLAC），请先用 ffmpeg 转成 16-bit WAV:
  ffmpeg -i input.mp3 -ac 2 -ar 44100 temp.wav

- 这是轻量化实现；如需更高准确度/性能，可以替换更成熟算法或引入 optimized FFT 库。

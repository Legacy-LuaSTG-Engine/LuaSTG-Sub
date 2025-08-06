# 关于这个文件

这是一个损坏的 wav 文件，它的 `fmt ` 块的内容如下：

| field                | content | description                          |
|:--------------------:|:-------:|:------------------------------------:|
| chunk id             | "fmt "  |                                      |
| chunk size           | 18      |                                      |
| format tag           | 1       | PCM                                  |
| channels             | 1       |                                      |
| sample rate          | 22050   |                                      |
| AVG bytes per sample | 44100   | = (block align) * (sample rate)      |
| block align          | 2       | = (channels) * (bits per sample) / 8 |
| bits per sample      | 16      |                                      |
| extended data size   | 24932   |                                      |

可以发现 `extended data size` 的值为 24932，理论上后续应该还有其他拓展内容，但实际上紧接着下一个块就是 `data`，也就是音频数据。

```
Address   00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F

00000000  52 49 46 46 C2 2D 00 00  57 41 56 45 66 6D 74 20  RIFF.-.. WAVEfmt 
00000010  12 00 00 00 01 00 01 00  22 56 00 00 44 AC 00 00  ........ "V..D...
00000020  02 00 10 00 64 61 64 61  74 61 9C 2D 00 00 23 00  ....dada ta.-..#.
```

这会导致一些软件无法解析该文件。

要修复该文件，只需要将 `extended data size` 的值设为 0 并保存即可（地址：00000024 到 00000025）。

# Image I/O

Supported image container formats:

- bmp
- jpeg
- png
- webp
- qoi

## JPEG

JPEG I/O support is powered by [libjpeg-turbo](https://libjpeg-turbo.org/).

## PNG

PNG I/O support is powered by [libpng](https://www.libpng.org/pub/png/libpng.html).

## WebP

WebP I/O support is powered by [libwebp](https://developers.google.com/speed/webp).

## QOI

QOI I/O support is powered by [qoi](https://qoiformat.org/).

## Windows Imaging Component

If [Windows Imaging Component](https://learn.microsoft.com/en-us/windows/win32/wic/-wic-lh) is used, then libpng and libjpeg-turbo are optional. This reduces the size of the executable.

However, note that the feature set supported by the Windows Imaging Component varies across different versions of Windows. **In particular, codecs for WebP and RAW formats must be installed separately**.

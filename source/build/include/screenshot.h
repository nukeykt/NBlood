#pragma once

#ifndef screenshot_h__
#define screenshot_h__

extern char screenshot_dir[BMAX_PATH];

int videoCaptureScreen(const char* filename, char inverseit) ATTRIBUTE((nonnull(1)));
int videoCaptureScreenTGA(const char* filename, char inverseit) ATTRIBUTE((nonnull(1)));

#endif // screenshot_h__

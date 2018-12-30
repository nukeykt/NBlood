#pragma once
void *ResReadLine(char *buffer, unsigned long nBytes, void **pRes);
bool FileRead(FILE *, void *, unsigned long);
bool FileWrite(FILE *, void *, unsigned long);
bool FileLoad(const char *, void *, unsigned long);
int FileLength(FILE *);
unsigned long qrand(void);
void ChangeExtension(char *pzFile, const char *pzExt);

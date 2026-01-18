#pragma once

#include <map>
#include <string>

#define IDS_GENERICERROR      100
#define IDS_FILENOTFOUND      150
#define IDS_IOERROR           200
#define IDS_ERR_TYPE_MISMATCH 250
#define IDS_TEXT_OK           300
#define IDS_TEXT_CANCEL       350

typedef std::map<std::u16string, std::map<long, std::u16string> > TextMap;
std::u16string load_wstring(const std::u16string& lang, long id);

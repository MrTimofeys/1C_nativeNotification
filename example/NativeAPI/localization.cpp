
#include "localization.h"
#include "localization/common.cxx"

std::u16string load_wstring(const std::u16string& lang, long id)
{
    std::u16string res;

    auto stringVals = stLocalizedText.find(lang);
    if (stringVals == stLocalizedText.end())
        stringVals = stLocalizedText.find(u"en");

    const auto textVal = stringVals->second.find(id);
    res = textVal != stringVals->second.end() ? textVal->second : u"Undocumented error";
    return res;
}

#pragma once

#include <juce_core/juce_core.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmacro-redefined", "-Wdeprecated-declarations", "-Wdeprecated-dynamic-exception-spec")
#include <fileref.h>
#include <tpropertymap.h>
#include <utf8-cpp/checked.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

static inline int get_file_length (std::u8string_view file_path)
{
    TagLib::FileRef file_ref { (const char*) std::u8string { file_path }.c_str(),
                               true,
                               TagLib::AudioProperties::Accurate };
    if (auto* props = file_ref.audioProperties())
        return props->length();
    return 0;
}

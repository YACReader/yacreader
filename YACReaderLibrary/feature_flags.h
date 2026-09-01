#ifndef YACREADER_LIBRARY_FEATURE_FLAGS_H
#define YACREADER_LIBRARY_FEATURE_FLAGS_H

namespace YACReader::FeatureFlags {

// The file organization workflow is still experimental. Keep its actions out
// of menus and shortcut management until the feature is ready for production.
inline constexpr bool organizeFiles = true;

} // namespace YACReader::FeatureFlags

#endif // YACREADER_LIBRARY_FEATURE_FLAGS_H

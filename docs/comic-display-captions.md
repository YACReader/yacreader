# Comic display captions

Polish follow-up for the 10.5 / API v3 stack. Captions are presentation strings, never replacement metadata.

## Contexts

| View | Caption | Number placement |
| --- | --- | --- |
| Desktop grid | Title, otherwise series, volume, then filename / last known filename | Dedicated number field; do not add a prefix to the caption |
| Desktop table Title column | Actual title metadata; blank when absent | Number, series, volume and filename have separate columns |
| Desktop info heading | Number + title, retaining series context when present; otherwise numbered series/volume or filename | Heading must identify the issue on its own |
| Desktop continue-reading card and comic action labels | Numbered metadata caption, otherwise filename | No dedicated number field |
| Exported reading-list HTML | Plain caption | Series / issue / volume are in its separate details line |
| Web cards and reader/details headings | Numbered metadata caption, otherwise filename | No dedicated number field on cards |
| iOS / Android remote cards and table rows | Numbered metadata caption, otherwise filename | No dedicated number field on cards; available and missing comics share the formatter |
| Android local cards and rows | Same formatter, using locally stored metadata | No dedicated number field on cards |
| iOS local grid and table | Stored name by default; readable caption for the readable-title preference or explicit reading order | Both layouts honor the same choice |

Plain metadata caption priority: title > series > volume > filename. Numbered captions preserve the existing `#N - Title`, `Series #N`, and `#N - Volume` forms. Filename fallbacks remain intact because they commonly already identify the issue. Blank fields are absent; issue `0` and fractional/string numbers are valid. With only a number, standalone captions show `#N`. A completely unknown caption displays `-`. The table Title column is raw metadata, so absent titles stay blank.

The server sends raw optional title/series/volume/number fields, never `-` or a fabricated "Missing comic" title. API v3 available-comic summaries now include optional raw series and volume, matching missing-comic summaries and full comic details. Frozen v2 summary responses remain unchanged. Mobile PATCH actions reload content; web actions merge returned fields into the current item, retaining v3 caption metadata.

## Existing data limits

- iOS local storage has title/volume/number/name but no separate series field. Its formatter uses those available fields; this change does not migrate its database or invent a series from a volume.
- For legacy iOS records without a universal number, integer zero remains the historical unknown-number sentinel. An explicit universal number `"0"` remains a real issue number.
- Missing entries cannot be imported into local mobile libraries; local caption rules apply to imported/available comics.
- This does not implement the deferred A-Z navigation redesign or change explicit reading-list ordering.

## Validation

- Desktop: full Release build; all 55 CTest targets pass, including frozen v2 responses and v3 routes. Regression cases cover grid/table versus standalone captions, volume-only metadata, blank fields, issue zero, fractional numbers, filename fallback, and raw API metadata.
- Web: syntax check and nine caption cases executed against the actual formatter; source objects remain unchanged.
- Android: debug app and instrumentation APKs build; all 149 unit tests pass, including shared normal/missing caption cases. Six Parcel instrumentation tests also pass on the existing API 25 emulator.
- iOS: regression cases added for remote normal/missing captions and local legacy/universal numbers. Xcode build and XCTest execution still require macOS; not run on Windows.

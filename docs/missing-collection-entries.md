# Missing reading-list and label entries

Normal reading lists and labels retain entries when a comic leaves the library. Scanning the same content hash again relinks the entry and restores its position. Database version 9.16.2 adds `label_entry` and migrates existing label memberships.

Both JSON collection content and legacy text import endpoints include missing entries in collection order. The server presents each as a downloadable one-page placeholder, with a valid JPEG cover/page, a `[MISSING]` filename, preserved title/issue metadata, and the synopsis �This comic is missing from your collection.� This supports clients that require a nonzero page count and cover before importing.

JSON retains `type: comic`, sets `missing: true`, exposes `entry_id` and `missing_message`, and uses `parent_id: null`. Available comics keep their existing ids and metadata. Missing navigation rows omit the optional title so mobile browsing uses the descriptive filename. Both required issue-number fields remain present; full import metadata retains the descriptive title.

Server placeholder ids reserve the range [2^52, 2^53), encode the durable entry id and a bit distinguishing labels from reading lists, and remain exactly representable by JSON clients. These are wire ids only; no placeholder comic or archive is inserted in the library database. Import/open/full-info/page/cover routes resolve them while the entry is missing; page zero returns the cover JPEG, and other pages return 404. Deleted or relinked entries stop resolving. Placeholder progress updates are acknowledged without changing real comics.

Each placeholder uses a synthetic hash distinct from the retained real comic hash. It encodes the wire id in its hexadecimal prefix and the image byte count in the normal file-size suffix. Hash-only progress sync skips placeholders. Relinking on the server does not automatically replace a placeholder already downloaded onto an iPhone/iPad.

The current web UI browses folders and comics and does not expose normal reading-list or label pages. Adding those pages is separate work.

Verification should cover desktop grid/info/flow views for lists and labels, importing collections containing multiple missing entries on iPhone/iPad, opening their placeholder page, and restoring the real file followed by a scan. Device import still requires validation on the actual iOS app.

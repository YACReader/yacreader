(function () {
  "use strict";

  var root = document.documentElement;

  function preferredTheme() {
    try {
      var savedTheme = localStorage.getItem("yacreader-server-theme");
      if (savedTheme === "light" || savedTheme === "dark") {
        return savedTheme;
      }
    } catch (error) {
    }

    return window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
  }

  function applyTheme(theme) {
    root.dataset.theme = theme;

    var toggle = document.querySelector("[data-theme-toggle]");
    if (toggle) {
      toggle.setAttribute("aria-label", theme === "dark" ? "Use light theme" : "Use dark theme");
    }
  }

  function element(tagName, className, text) {
    var node = document.createElement(tagName);
    if (className) {
      node.className = className;
    }
    if (text !== undefined && text !== null) {
      node.textContent = text;
    }
    return node;
  }

  function initLibraryBrowser() {
    var browserRoot = document.querySelector("[data-browser-root]");
    if (!browserRoot) {
      return;
    }

    var libraryId = document.body.dataset.browserLibraryId;
    var libraryName = document.body.dataset.browserLibraryName;
    var breadcrumbs = document.querySelector("[data-browser-breadcrumbs]");
    var pageTitle = document.querySelector("[data-browser-title]");
    var browserBack = document.querySelector("[data-browser-back]");
    var navigationVersion = 0;
    var folderMetadataCache = {};
    var browserBackAction = null;

    if (browserBack) {
      browserBack.addEventListener("click", function () {
        if (browserBackAction) {
          browserBackAction();
        }
      });
    }

    function setBrowserBack(parentFolderId) {
      if (!browserBack) {
        return;
      }

      if (!parentFolderId) {
        browserBack.hidden = true;
        browserBackAction = null;
        return;
      }

      browserBack.hidden = false;
      browserBackAction = function () {
        showFolder(String(parentFolderId), true);
      };
    }

    function requestId() {
      try {
        var existingId = sessionStorage.getItem("yacreader-webui-request-id");
        if (existingId) {
          return existingId;
        }

        var newId = window.crypto && window.crypto.randomUUID
          ? window.crypto.randomUUID()
          : String(Date.now()) + "-" + String(Math.random()).slice(2);
        sessionStorage.setItem("yacreader-webui-request-id", newId);
        return newId;
      } catch (error) {
        return "";
      }
    }

    function fetchJson(url) {
      var headers = { "Accept": "application/json" };
      var id = requestId();
      if (id) {
        headers["X-Request-Id"] = id;
      }

      return fetch(url, { headers: headers }).then(function (response) {
        if (!response.ok) {
          throw new Error("Request failed with status " + response.status);
        }
        return response.json();
      });
    }

    function libraryUrl() {
      return "/webui/library/" + encodeURIComponent(libraryId);
    }

    function folderUrl(folderId) {
      return folderId === "1"
        ? libraryUrl()
        : libraryUrl() + "/folder/" + encodeURIComponent(folderId);
    }

    function comicUrl(comicId) {
      return libraryUrl() + "/comic/" + encodeURIComponent(comicId);
    }

    function folderContentApi(folderId) {
      return "/v2/library/" + encodeURIComponent(libraryId) + "/folder/" + encodeURIComponent(folderId) + "/content";
    }

    function folderMetadataApi(folderId) {
      return "/v2/library/" + encodeURIComponent(libraryId) + "/folder/" + encodeURIComponent(folderId) + "/metadata";
    }

    function comicInfoApi(comicId) {
      return "/v2/library/" + encodeURIComponent(libraryId) + "/comic/" + encodeURIComponent(comicId) + "/fullinfo";
    }

    function safeCoverPath(path) {
      return String(path || "")
        .split("/")
        .filter(function (part) {
          return part && part !== "." && part !== "..";
        })
        .map(encodeURIComponent)
        .join("/");
    }

    function coverUrl(item) {
      var filePath = "";
      if (item.type === "comic" && item.hash) {
        filePath = encodeURIComponent(item.hash) + ".jpg";
      } else if (item.type === "folder") {
        if (item.custom_image) {
          filePath = safeCoverPath(item.custom_image);
        } else if (item.first_comic_hash) {
          filePath = encodeURIComponent(item.first_comic_hash) + ".jpg";
        }
      }

      if (!filePath) {
        return "";
      }

      return "/v2/library/" + encodeURIComponent(libraryId) + "/cover/" + filePath;
    }

    function readableComicTitle(comic) {
      var number = comic.universal_number;
      var title = comic.title && String(comic.title).trim();
      var volume = comic.volume && String(comic.volume).trim();

      if (number !== undefined && number !== null && String(number).trim() !== "") {
        if (title) {
          return "#" + number + " — " + title;
        }
        if (volume) {
          return "#" + number + " — " + volume;
        }
      }

      return title || volume || comic.file_name || "Untitled comic";
    }

    function setPageHeading(title) {
      pageTitle.textContent = title;
      document.title = title + " · YACReaderLibrary";
    }

    function renderBreadcrumbs(parts) {
      breadcrumbs.replaceChildren();

      parts.forEach(function (part, index) {
        if (index > 0) {
          breadcrumbs.appendChild(element("span", "breadcrumb-separator", "/"));
        }

        if (part.href) {
          var link = element("a", "", part.label);
          link.href = part.href;
          if (part.action) {
            link.addEventListener("click", function (event) {
              event.preventDefault();
              part.action();
            });
          }
          breadcrumbs.appendChild(link);
        } else {
          breadcrumbs.appendChild(element("span", "breadcrumb-current", part.label));
        }
      });
    }

    function showLoading() {
      browserRoot.setAttribute("aria-busy", "true");
      browserRoot.replaceChildren();

      var loading = element("div", "browser-loading");
      loading.appendChild(element("div", "browser-loading-header"));
      var grid = element("div", "browser-loading-grid");
      for (var i = 0; i < 8; i++) {
        grid.appendChild(element("div", "browser-loading-card"));
      }
      loading.appendChild(grid);
      browserRoot.appendChild(loading);
    }

    function showError(retry) {
      browserRoot.removeAttribute("aria-busy");
      browserRoot.replaceChildren();

      var state = element("div", "browser-state");
      var icon = element("div", "browser-state-icon", "!");
      var heading = element("h2", "", "Couldn’t load this library");
      var description = element("p", "", "The server could not read this content. The library may be updating or no longer available.");
      var button = element("button", "primary-button", "Try again");
      button.type = "button";
      button.addEventListener("click", retry);

      state.append(icon, heading, description, button);
      browserRoot.appendChild(state);
    }

    function folderPlaceholder() {
      var placeholder = element("div", "cover-placeholder folder-placeholder");
      placeholder.appendChild(element("span", "folder-glyph"));
      return placeholder;
    }

    function comicPlaceholder() {
      var placeholder = element("div", "cover-placeholder comic-placeholder");
      placeholder.appendChild(element("span", "comic-glyph", "Y"));
      return placeholder;
    }

    function coverImage(item, placeholder) {
      var url = coverUrl(item);
      if (!url) {
        return null;
      }

      var image = element("img", "browser-cover-image");
      image.alt = "";
      image.loading = "lazy";
      image.src = url;
      image.addEventListener("load", function () {
        placeholder.hidden = true;
      });
      image.addEventListener("error", function () {
        image.remove();
        placeholder.hidden = false;
      });
      return image;
    }

    function folderCard(folder) {
      var card = element("a", "browser-card folder-card");
      card.href = folderUrl(String(folder.id));
      card.addEventListener("click", function (event) {
        event.preventDefault();
        showFolder(String(folder.id), true);
      });

      var cover = element("div", "browser-cover folder-cover");
      cover.appendChild(element("div", "folder-cover-back"));
      cover.appendChild(element("div", "folder-cover-middle"));
      var front = element("div", "folder-cover-front");
      var placeholder = folderPlaceholder();
      front.appendChild(placeholder);
      var image = coverImage(folder, placeholder);
      if (image) {
        front.appendChild(image);
      }
      cover.appendChild(front);

      var copy = element("div", "browser-card-copy");
      copy.appendChild(element("div", "browser-card-title", folder.folder_name || "Untitled folder"));
      var count = Number(folder.num_children) || 0;
      copy.appendChild(element("div", "browser-card-meta", count === 1 ? "1 item" : count + " items"));

      card.append(cover, copy);
      return card;
    }

    function comicCard(comic) {
      var card = element("a", "browser-card comic-card");
      card.href = comicUrl(String(comic.id));
      card.addEventListener("click", function (event) {
        event.preventDefault();
        showComic(String(comic.id), true);
      });

      var cover = element("div", "browser-cover comic-cover");
      var placeholder = comicPlaceholder();
      cover.appendChild(placeholder);
      var image = coverImage(comic, placeholder);
      if (image) {
        cover.appendChild(image);
      }

      if (comic.read) {
        cover.appendChild(element("span", "comic-status read", "Read"));
      } else if (Number(comic.current_page) > 1) {
        cover.appendChild(element("span", "comic-status reading", "Page " + comic.current_page));
      }

      var currentPage = Number(comic.current_page) || 0;
      var numPages = Number(comic.num_pages) || 0;
      if (!comic.read && currentPage > 0 && numPages > 0) {
        var progress = element("span", "comic-progress");
        var progressValue = element("span", "comic-progress-value");
        progressValue.style.width = Math.min(100, Math.round((currentPage / numPages) * 100)) + "%";
        progress.appendChild(progressValue);
        cover.appendChild(progress);
      }

      var copy = element("div", "browser-card-copy");
      copy.appendChild(element("div", "browser-card-title", readableComicTitle(comic)));
      copy.appendChild(element("div", "browser-card-meta", numPages === 1 ? "1 page" : numPages + " pages"));

      card.append(cover, copy);
      return card;
    }

    function getFolderMetadata(folderId) {
      if (folderMetadataCache[folderId]) {
        return Promise.resolve(folderMetadataCache[folderId]);
      }

      return fetchJson(folderMetadataApi(folderId)).then(function (folder) {
        folderMetadataCache[folderId] = folder;
        return folder;
      });
    }

    function folderTrail(folderId) {
      if (folderId === "1") {
        return Promise.resolve([]);
      }

      var trail = [];
      var visited = {};

      function load(currentId) {
        if (!currentId || currentId === "0" || currentId === "1" || visited[currentId]) {
          return Promise.resolve(trail);
        }
        visited[currentId] = true;

        return getFolderMetadata(currentId).then(function (folder) {
          trail.unshift({
            id: String(folder.id),
            name: folder.folder_name || "Untitled folder"
          });
          return load(String(folder.parent_id || "1"));
        });
      }

      return load(folderId);
    }

    function browserBreadcrumbParts(trail, finalLabel) {
      var parts = [{
        label: "Libraries",
        href: "/webui#libraries"
      }];

      if (trail.length === 0 && !finalLabel) {
        parts.push({ label: libraryName });
        return parts;
      }

      parts.push({
        label: libraryName,
        href: libraryUrl(),
        action: function () {
          showFolder("1", true);
        }
      });

      trail.forEach(function (folder, index) {
        var isLastFolder = index === trail.length - 1 && !finalLabel;
        parts.push({
          label: folder.name,
          href: isLastFolder ? "" : folderUrl(folder.id),
          action: isLastFolder ? null : function () {
            showFolder(folder.id, true);
          }
        });
      });

      if (finalLabel) {
        parts.push({ label: finalLabel });
      }

      return parts;
    }

    function showFolder(folderId, pushHistory) {
      var version = ++navigationVersion;
      showLoading();

      Promise.all([
        fetchJson(folderContentApi(folderId)),
        folderTrail(folderId)
      ]).then(function (results) {
        if (version !== navigationVersion) {
          return;
        }

        var items = results[0];
        var trail = results[1];
        var folderName = trail.length ? trail[trail.length - 1].name : libraryName;
        var folders = items.filter(function (item) { return item.type === "folder"; });
        var comics = items.filter(function (item) { return item.type === "comic"; });
        var parentFolderId = folderId === "1"
          ? null
          : trail.length > 1
            ? trail[trail.length - 2].id
            : "1";

        setPageHeading(folderName);
        setBrowserBack(parentFolderId);
        renderBreadcrumbs(browserBreadcrumbParts(trail));

        browserRoot.removeAttribute("aria-busy");
        browserRoot.replaceChildren();

        var header = element("section", "browser-library-header");
        header.appendChild(element("div", "section-title", folderId === "1" ? "Library" : "Folder"));
        header.appendChild(element("h2", "", folderName));
        var summaryParts = [];
        if (folders.length) {
          summaryParts.push(folders.length === 1 ? "1 folder" : folders.length + " folders");
        }
        if (comics.length) {
          summaryParts.push(comics.length === 1 ? "1 comic" : comics.length + " comics");
        }
        header.appendChild(element("p", "", summaryParts.join(" · ") || "No items"));
        browserRoot.appendChild(header);

        if (!items.length) {
          var empty = element("div", "browser-state compact");
          empty.appendChild(element("div", "browser-state-icon folder-state-icon"));
          empty.appendChild(element("h2", "", "This folder is empty"));
          empty.appendChild(element("p", "", "There are no folders or comics here yet."));
          browserRoot.appendChild(empty);
        } else {
          var grid = element("div", "browser-grid");
          items.forEach(function (item) {
            if (item.type === "folder") {
              grid.appendChild(folderCard(item));
            } else if (item.type === "comic") {
              grid.appendChild(comicCard(item));
            }
          });
          browserRoot.appendChild(grid);
        }

        var url = folderUrl(folderId);
        var state = { view: "folder", itemId: folderId };
        if (pushHistory) {
          history.pushState(state, "", url);
        } else {
          history.replaceState(state, "", url);
        }
      }).catch(function () {
        if (version !== navigationVersion) {
          return;
        }
        showError(function () {
          showFolder(folderId, false);
        });
      });
    }

    function hasValue(value) {
      return value !== undefined && value !== null && String(value).trim() !== "";
    }

    function fileTypeName(fileType) {
      var names = ["Comic", "Manga", "Western manga", "Web comic", "Yonkoma"];
      return names[Number(fileType)] || "";
    }

    function detailField(label, value) {
      var field = element("div", "comic-metadata-field");
      field.appendChild(element("dt", "", label));
      field.appendChild(element("dd", "", value));
      return field;
    }

    function sanitizedHtml(value) {
      var allowedElements = {
        A: true,
        BLOCKQUOTE: true,
        BR: true,
        CODE: true,
        EM: true,
        H1: true,
        H2: true,
        H3: true,
        H4: true,
        H5: true,
        H6: true,
        HR: true,
        LI: true,
        OL: true,
        P: true,
        PRE: true,
        STRONG: true,
        TABLE: true,
        TBODY: true,
        TD: true,
        TFOOT: true,
        TH: true,
        THEAD: true,
        TR: true,
        UL: true
      };
      var discardedElements = {
        BUTTON: true,
        EMBED: true,
        FORM: true,
        IFRAME: true,
        IMG: true,
        INPUT: true,
        LINK: true,
        MATH: true,
        META: true,
        OBJECT: true,
        SCRIPT: true,
        STYLE: true,
        SVG: true,
        TEXTAREA: true,
        VIDEO: true
      };
      var source = new DOMParser().parseFromString(String(value), "text/html");
      var result = document.createDocumentFragment();

      if (source.body.children.length === 0) {
        var plainText = element("p", "synopsis-plain", source.body.textContent);
        result.appendChild(plainText);
        return result;
      }

      function copyNode(node, target) {
        if (node.nodeType === Node.TEXT_NODE) {
          target.appendChild(document.createTextNode(node.textContent));
          return;
        }

        if (node.nodeType !== Node.ELEMENT_NODE || discardedElements[node.tagName]) {
          return;
        }

        if (!allowedElements[node.tagName]) {
          Array.from(node.childNodes).forEach(function (child) {
            copyNode(child, target);
          });
          return;
        }

        var clean = document.createElement(node.tagName.toLowerCase());
        if (node.tagName === "A") {
          try {
            var url = new URL(node.getAttribute("href"), window.location.href);
            if (url.protocol === "http:" || url.protocol === "https:") {
              clean.href = url.href;
              clean.target = "_blank";
              clean.rel = "noopener noreferrer";
            }
          } catch (error) {
          }
        } else if (node.tagName === "TD" || node.tagName === "TH") {
          ["colspan", "rowspan"].forEach(function (attribute) {
            var span = Number(node.getAttribute(attribute));
            if (Number.isInteger(span) && span > 1 && span <= 20) {
              clean.setAttribute(attribute, String(span));
            }
          });
          var scope = node.getAttribute("scope");
          if (scope === "row" || scope === "col" || scope === "rowgroup" || scope === "colgroup") {
            clean.setAttribute("scope", scope);
          }
        }

        Array.from(node.childNodes).forEach(function (child) {
          copyNode(child, clean);
        });
        target.appendChild(clean);
      }

      Array.from(source.body.childNodes).forEach(function (node) {
        copyNode(node, result);
      });
      return result;
    }

    function showComic(comicId, pushHistory) {
      var version = ++navigationVersion;
      showLoading();

      fetchJson(comicInfoApi(comicId)).then(function (comic) {
        return Promise.all([
          Promise.resolve(comic),
          folderTrail(String(comic.parent_id || "1"))
        ]);
      }).then(function (results) {
        if (version !== navigationVersion) {
          return;
        }

        var comic = results[0];
        var trail = results[1];
        var title = readableComicTitle(comic);
        var parentId = String(comic.parent_id || "1");

        setPageHeading(title);
        setBrowserBack(parentId);
        renderBreadcrumbs(browserBreadcrumbParts(trail, title));

        browserRoot.removeAttribute("aria-busy");
        browserRoot.replaceChildren();

        var detail = element("article", "comic-detail");
        var coverColumn = element("div", "comic-detail-cover-column");
        var cover = element("div", "comic-detail-cover");
        var placeholder = comicPlaceholder();
        cover.appendChild(placeholder);
        var image = coverImage(comic, placeholder);
        if (image) {
          cover.appendChild(image);
        }
        coverColumn.appendChild(cover);

        var back = element("a", "secondary-button comic-back-button", "Back to folder");
        back.href = folderUrl(parentId);
        back.addEventListener("click", function (event) {
          event.preventDefault();
          showFolder(parentId, true);
        });
        coverColumn.appendChild(back);

        var mobilePromo = element("aside", "mobile-app-promo");
        mobilePromo.appendChild(element("div", "section-title", "Read on mobile"));
        mobilePromo.appendChild(element("h3", "", "Take your library with you"));
        mobilePromo.appendChild(element("p", "", "Browse this library and read your comics with YACReader for iOS or Android."));

        var mobileLinks = element("div", "mobile-app-links");
        var iosLink = element("a", "mobile-app-link", "iOS app");
        iosLink.href = "https://apps.apple.com/app/id635717885";
        iosLink.target = "_blank";
        iosLink.rel = "noopener noreferrer";
        iosLink.setAttribute("aria-label", "Get YACReader for iOS on the App Store");

        var androidLink = element("a", "mobile-app-link", "Android app");
        androidLink.href = "https://play.google.com/store/apps/details?id=com.yacreader.yacreader";
        androidLink.target = "_blank";
        androidLink.rel = "noopener noreferrer";
        androidLink.setAttribute("aria-label", "Get YACReader for Android on Google Play");

        mobileLinks.append(iosLink, androidLink);
        mobilePromo.appendChild(mobileLinks);
        coverColumn.appendChild(mobilePromo);

        var copy = element("div", "comic-detail-copy");
        copy.appendChild(element("div", "section-title", "Comic information"));
        copy.appendChild(element("h2", "", title));
        if (comic.file_name && comic.file_name !== title) {
          copy.appendChild(element("p", "comic-file-name", comic.file_name));
        }

        var facts = element("div", "comic-facts");
        var numPages = Number(comic.num_pages) || 0;
        if (numPages) {
          facts.appendChild(element("span", "comic-fact", numPages === 1 ? "1 page" : numPages + " pages"));
        }
        if (comic.read) {
          facts.appendChild(element("span", "comic-fact success", "Read"));
        } else if (Number(comic.current_page) > 0) {
          facts.appendChild(element("span", "comic-fact accent", "Page " + comic.current_page));
        }
        if (hasValue(comic.format)) {
          facts.appendChild(element("span", "comic-fact", comic.format));
        }
        var fileType = fileTypeName(comic.file_type);
        if (fileType) {
          facts.appendChild(element("span", "comic-fact", fileType));
        }
        copy.appendChild(facts);

        if (hasValue(comic.synopsis)) {
          var synopsis = element("section", "comic-copy-section");
          synopsis.appendChild(element("h3", "", "Synopsis"));
          var synopsisContent = element("div", "comic-synopsis");
          synopsisContent.appendChild(sanitizedHtml(comic.synopsis));
          synopsis.appendChild(synopsisContent);
          copy.appendChild(synopsis);
        }

        var metadata = [
          ["Series", comic.series],
          ["Issue", comic.universal_number],
          ["Volume", comic.volume],
          ["Publisher", comic.publisher],
          ["Imprint", comic.imprint],
          ["Date", comic.date],
          ["Story arc", comic.story_arc],
          ["Arc number", comic.arc_number],
          ["Genre", comic.genre],
          ["Writer", comic.writer],
          ["Penciller", comic.penciller],
          ["Inker", comic.inker],
          ["Colorist", comic.colorist],
          ["Letterer", comic.letterer],
          ["Cover artist", comic.cover_artist],
          ["Editor", comic.editor],
          ["Age rating", comic.age_rating],
          ["Language", comic.language_iso],
          ["Characters", comic.characters],
          ["Teams", comic.teams],
          ["Locations", comic.locations],
          ["Tags", comic.tags],
          ["Rating", Number(comic.rating) > 0 ? comic.rating + " / 5" : ""],
          ["Color", comic.color === true ? "Color" : comic.color === false ? "Black and white" : ""]
        ].filter(function (entry) {
          return hasValue(entry[1]);
        });

        if (metadata.length) {
          var metadataSection = element("section", "comic-copy-section");
          metadataSection.appendChild(element("h3", "", "Metadata"));
          var metadataList = element("dl", "comic-metadata-grid");
          metadata.forEach(function (entry) {
            metadataList.appendChild(detailField(entry[0], entry[1]));
          });
          metadataSection.appendChild(metadataList);
          copy.appendChild(metadataSection);
        }

        if (hasValue(comic.notes)) {
          var notes = element("section", "comic-copy-section");
          notes.appendChild(element("h3", "", "Notes"));
          notes.appendChild(element("p", "", comic.notes));
          copy.appendChild(notes);
        }

        detail.append(coverColumn, copy);
        browserRoot.appendChild(detail);

        var url = comicUrl(comicId);
        var state = { view: "comic", itemId: comicId };
        if (pushHistory) {
          history.pushState(state, "", url);
        } else {
          history.replaceState(state, "", url);
        }
      }).catch(function () {
        if (version !== navigationVersion) {
          return;
        }
        showError(function () {
          showComic(comicId, false);
        });
      });
    }

    function routeFromLocation() {
      var escapedLibraryId = libraryId.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
      var match = window.location.pathname.match(new RegExp("^/webui/library/" + escapedLibraryId + "(?:/(folder|comic)/([0-9]+))?/?$"));
      if (!match) {
        return { view: "folder", itemId: "1" };
      }
      return {
        view: match[1] || "folder",
        itemId: match[2] || "1"
      };
    }

    window.addEventListener("popstate", function () {
      var route = routeFromLocation();
      if (route.view === "comic") {
        showComic(route.itemId, false);
      } else {
        showFolder(route.itemId, false);
      }
    });

    var initialView = document.body.dataset.browserInitialView;
    var initialItemId = document.body.dataset.browserInitialItemId || "1";
    if (initialView === "comic") {
      showComic(initialItemId, false);
    } else {
      showFolder(initialItemId, false);
    }
  }

  function initLibraryMenus() {
    var menus = Array.prototype.slice.call(document.querySelectorAll("[data-menu]"));
    if (!menus.length) {
      return;
    }

    function closeAll(except) {
      menus.forEach(function (menu) {
        if (menu === except) {
          return;
        }
        var toggle = menu.querySelector("[data-menu-toggle]");
        var popover = menu.querySelector("[data-menu-popover]");
        if (popover) {
          popover.hidden = true;
        }
        if (toggle) {
          toggle.setAttribute("aria-expanded", "false");
        }
      });
    }

    menus.forEach(function (menu) {
      var toggle = menu.querySelector("[data-menu-toggle]");
      var popover = menu.querySelector("[data-menu-popover]");
      if (!toggle || !popover) {
        return;
      }

      toggle.addEventListener("click", function (event) {
        event.stopPropagation();
        var willOpen = popover.hidden;
        closeAll(menu);
        popover.hidden = !willOpen;
        toggle.setAttribute("aria-expanded", willOpen ? "true" : "false");
      });
    });

    document.addEventListener("click", function () {
      closeAll(null);
    });

    document.addEventListener("keydown", function (event) {
      if (event.key === "Escape") {
        closeAll(null);
      }
    });

    return { closeAll: closeAll };
  }

  function initUpdateLibraries(menus) {
    var container = document.querySelector("[data-update-libraries]");
    if (!container) {
      return;
    }

    var allButton = container.querySelector("[data-update-all]");
    var cards = Array.prototype.slice.call(container.querySelectorAll("[data-library-card]"));
    var triggers = Array.prototype.slice.call(container.querySelectorAll("[data-update-all], [data-update-library]"));
    if (!triggers.length) {
      return;
    }

    var polling = false;
    var active = null; // null | "all" | the card being updated

    function setTriggersDisabled(disabled) {
      triggers.forEach(function (trigger) {
        trigger.disabled = disabled;
      });
    }

    function showIndicators(target) {
      // "all": spin the header button only (per-card bars in lock-step add no
      // information). A single card: run the ping-pong bar on that card alone.
      if (allButton) {
        allButton.classList.toggle("is-busy", target === "all");
      }
      cards.forEach(function (card) {
        card.classList.toggle("is-updating", card === target);
      });
      active = target || null;
    }

    function clearIndicators() {
      if (allButton) {
        allButton.classList.remove("is-busy");
      }
      cards.forEach(function (card) {
        card.classList.remove("is-updating");
      });
      active = null;
    }

    function schedulePoll(delay) {
      if (polling) {
        return;
      }
      polling = true;
      window.setTimeout(poll, delay);
    }

    // The server tracks a single global "running" flag and updates libraries
    // sequentially, so it can't say which library is in progress. We show the
    // indicator for whatever started the run (or fall back to "all" if a run was
    // already going) and disable every trigger until it finishes.
    function applyRunning(running) {
      if (running) {
        setTriggersDisabled(true);
        if (!active) {
          showIndicators("all");
        }
        schedulePoll(2000);
      } else {
        clearIndicators();
        setTriggersDisabled(false);
      }
    }

    function checkStatus() {
      return fetch("/v2/libraries/update/status", { headers: { "Accept": "application/json" } })
        .then(function (response) {
          return response.ok ? response.json() : { running: false };
        })
        .then(function (data) {
          applyRunning(!!(data && data.running));
        });
    }

    function poll() {
      polling = false;
      checkStatus().catch(function () {
        polling = false;
        clearIndicators();
        setTriggersDisabled(false);
      });
    }

    function trigger(target, url) {
      if (menus) {
        menus.closeAll(null);
      }
      setTriggersDisabled(true);
      showIndicators(target);

      fetch(url, { method: "POST", headers: { "Accept": "application/json" } })
        .then(function (response) {
          if (response.status === 202) {
            schedulePoll(1200);
          } else if (response.status === 409) {
            clearIndicators();
            return checkStatus();
          } else {
            throw new Error("Unexpected status " + response.status);
          }
        })
        .catch(function () {
          clearIndicators();
          setTriggersDisabled(false);
        });
    }

    triggers.forEach(function (button) {
      button.addEventListener("click", function (event) {
        event.stopPropagation();
        if (button.disabled) {
          return;
        }
        var libraryId = button.getAttribute("data-update-library");
        if (libraryId) {
          var card = button.closest("[data-library-card]");
          trigger(card, "/v2/library/" + encodeURIComponent(libraryId) + "/update");
        } else {
          trigger("all", "/v2/libraries/update");
        }
      });
    });

    // Reflect an update that may already be running (started elsewhere or before reload).
    checkStatus().catch(function () {});
  }

  applyTheme(preferredTheme());

  document.addEventListener("DOMContentLoaded", function () {
    var toggle = document.querySelector("[data-theme-toggle]");
    if (toggle) {
      toggle.addEventListener("click", function () {
        var theme = root.dataset.theme === "dark" ? "light" : "dark";
        applyTheme(theme);

        try {
          localStorage.setItem("yacreader-server-theme", theme);
        } catch (error) {
        }
      });
    }

    document.querySelectorAll("[data-controls]").forEach(function (checkbox) {
      var control = document.getElementById(checkbox.dataset.controls);
      if (!control) {
        return;
      }

      function updateControl() {
        control.disabled = !checkbox.checked;
      }

      checkbox.addEventListener("change", updateControl);
      updateControl();
    });

    initLibraryBrowser();
    var libraryMenus = initLibraryMenus();
    initUpdateLibraries(libraryMenus);
  });
}());

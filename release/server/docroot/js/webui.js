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

  function svgIcon(className, iconMarkup) {
    var icon = document.createElementNS("http://www.w3.org/2000/svg", "svg");
    icon.setAttribute("class", className);
    icon.setAttribute("viewBox", "0 0 24 24");
    icon.setAttribute("fill", "none");
    icon.setAttribute("stroke", "currentColor");
    icon.setAttribute("stroke-width", "1.8");
    icon.setAttribute("stroke-linecap", "round");
    icon.setAttribute("stroke-linejoin", "round");
    icon.setAttribute("aria-hidden", "true");
    icon.innerHTML = iconMarkup;
    return icon;
  }

  function storeBadge(className, imageSource, href, ariaLabel) {
    var link = element("a", "store-badge-link " + className);
    link.href = href;
    link.target = "_blank";
    link.rel = "noopener noreferrer";
    link.setAttribute("aria-label", ariaLabel);

    var badge = element("img");
    badge.src = imageSource;
    badge.alt = "";
    link.appendChild(badge);
    return link;
  }

  var firstReaderPromotionShown = false;

  function showFirstReaderPromotion(returnFocus) {
    if (firstReaderPromotionShown) {
      return;
    }

    try {
      if (localStorage.getItem("yacreader-webui-reader-promotion-v2") === "shown") {
        firstReaderPromotionShown = true;
        return;
      }
      localStorage.setItem("yacreader-webui-reader-promotion-v2", "shown");
    } catch (error) {
    }
    firstReaderPromotionShown = true;

    var overlay = element("div", "mobile-promotion-overlay");
    var dialog = element("section", "mobile-promotion-dialog");
    dialog.setAttribute("role", "dialog");
    dialog.setAttribute("aria-modal", "true");
    dialog.setAttribute("aria-labelledby", "first-comic-promotion-title");
    dialog.setAttribute("aria-describedby", "first-comic-promotion-copy");

    var closeButton = element("button", "mobile-promotion-close");
    closeButton.type = "button";
    closeButton.setAttribute("aria-label", "Close mobile app promotion");
    closeButton.appendChild(svgIcon("mobile-promotion-close-icon", '<path d="m7 7 10 10M17 7 7 17"/>'));

    var intro = element("header", "mobile-promotion-intro");
    var title = element("h2", "", "Upgrade your reading");
    title.id = "first-comic-promotion-title";
    var introCopy = element(
      "p",
      "",
      "The best YACReader reading experience is on iOS and Android. Go far beyond the basic web reader with a fast, deeply customizable reader engineered for comics, manga and webtoons."
    );
    introCopy.id = "first-comic-promotion-copy";
    intro.append(title, introCopy);

    var featureList = element("ul", "mobile-promotion-features");
    [
      [
        "Guided, panel by panel",
        "Automatic panel detection, configurable framing and optional full-page stops."
      ],
      [
        "A layout for every comic",
        "Fit and fill modes, single or double pages, manga direction, auto-scroll and continuous webtoon reading."
      ],
      [
        "Make every page look its best",
        "Automatic margin trimming and powerful image filters for faded colors, dark scans and imperfect pages."
      ],
      [
        "Your library, everywhere",
        "Browse and stream remotely, import for offline reading, and keep progress and settings synchronized."
      ]
    ].forEach(function (feature) {
      var item = element("li");
      var featureCopy = element("span", "mobile-promotion-feature-copy");
      featureCopy.append(element("strong", "", feature[0]), element("span", "", feature[1]));
      item.append(
        svgIcon("mobile-promotion-check", '<path d="m5 12 4 4L19 6"/>'),
        featureCopy
      );
      featureList.appendChild(item);
    });

    var moreFeatures = element("p", "mobile-promotion-more");
    moreFeatures.append(
      element("strong", "", "And much more."),
      document.createTextNode(" Discover the complete feature set for your device.")
    );

    function platformCard(platform, devices, website, storeLink) {
      var card = element("section", "mobile-platform-card");
      var cardCopy = element("div", "mobile-platform-copy");
      cardCopy.append(element("h3", "", platform), element("span", "", devices));

      var websiteLink = element("a", "mobile-platform-website", "Discover every " + platform + " feature");
      websiteLink.href = website;
      websiteLink.target = "_blank";
      websiteLink.rel = "noopener noreferrer";
      websiteLink.appendChild(svgIcon("mobile-platform-link-icon", '<path d="M7 17 17 7M8 7h9v9"/>'));

      card.append(cardCopy, websiteLink, storeLink);
      return card;
    }

    var platforms = element("div", "mobile-platform-grid");
    platforms.append(
      platformCard(
        "iOS",
        "iPhone and iPad",
        "https://ios.yacreader.com/",
        storeBadge(
          "app-store-badge",
          "/images/webui/app-store-badge.svg",
          "https://apps.apple.com/app/id635717885",
          "Download YACReader on the App Store"
        )
      ),
      platformCard(
        "Android",
        "Phones, tablets and desktop layouts",
        "https://android.yacreader.com/",
        storeBadge(
          "google-play-badge",
          "/images/webui/google-play-badge.png",
          "https://play.google.com/store/apps/details?id=com.yacreader.yacreader",
          "Get YACReader on Google Play"
        )
      )
    );

    var continueButton = element("button", "secondary-button mobile-promotion-continue", "Continue on the web");
    continueButton.type = "button";

    dialog.append(closeButton, intro, featureList, moreFeatures, platforms, continueButton);
    overlay.appendChild(dialog);
    document.body.appendChild(overlay);
    document.body.classList.add("mobile-promotion-open");

    function closePromotion() {
      document.removeEventListener("keydown", handlePromotionKeys);
      document.body.classList.remove("mobile-promotion-open");
      overlay.remove();
      if (returnFocus && returnFocus.isConnected) {
        returnFocus.focus();
      }
    }

    function handlePromotionKeys(event) {
      if (event.key === "Escape") {
        event.preventDefault();
        closePromotion();
        return;
      }
      if (event.key !== "Tab") {
        return;
      }

      var focusable = Array.from(dialog.querySelectorAll("a[href], button:not([disabled])"));
      if (!focusable.length) {
        return;
      }
      var first = focusable[0];
      var last = focusable[focusable.length - 1];
      if (event.shiftKey && document.activeElement === first) {
        event.preventDefault();
        last.focus();
      } else if (!event.shiftKey && document.activeElement === last) {
        event.preventDefault();
        first.focus();
      }
    }

    closeButton.addEventListener("click", closePromotion);
    continueButton.addEventListener("click", closePromotion);
    overlay.addEventListener("pointerdown", function (event) {
      if (event.target === overlay) {
        closePromotion();
      }
    });
    document.addEventListener("keydown", handlePromotionKeys);
    closeButton.focus();
  }

  function initSidebarActions() {
    var sidebar = document.querySelector(".sidebar");
    var serverSummary = sidebar && sidebar.querySelector(".server-summary");
    if (!sidebar || !serverSummary) {
      return;
    }

    var promos = element("div", "sidebar-promos");

    var mobilePromo = element("section", "sidebar-mobile-promo");
    mobilePromo.setAttribute("aria-labelledby", "mobile-promo-title");
    var mobileHeader = element("div", "sidebar-mobile-header");
    var mobileCopy = element("div");
    var mobileTitle = element("h2", "", "Upgrade your reading");
    mobileTitle.id = "mobile-promo-title";
    mobileCopy.append(mobileTitle, element("p", "", "Unlock a richer reading experience with panel-by-panel navigation, flexible fit modes, image filters, fluid animations, and much more."));
    mobileHeader.appendChild(mobileCopy);

    var storeLinks = element("nav", "store-links");
    storeLinks.setAttribute("aria-label", "Get the YACReader mobile apps");
    storeLinks.append(
      storeBadge(
        "app-store-badge",
        "/images/webui/app-store-badge.svg",
        "https://apps.apple.com/app/id635717885",
        "Download YACReader on the App Store"
      ),
      storeBadge(
        "google-play-badge",
        "/images/webui/google-play-badge.png",
        "https://play.google.com/store/apps/details?id=com.yacreader.yacreader",
        "Get YACReader on Google Play"
      )
    );
    mobilePromo.append(mobileHeader, storeLinks);

    var donation = element("section", "sidebar-donation");
    var donationCopy = element("div", "sidebar-donation-copy");
    donationCopy.append(element("strong", "", "Love YACReader?"), element("span", "", "Help keep it independent."));
    var donateLink = element("a", "donate-button");
    donateLink.href = "https://www.paypal.com/donate?business=5TAMNQCDDMVP8&item_name=Support+YACReader";
    donateLink.target = "_blank";
    donateLink.rel = "noopener noreferrer";
    donateLink.append(
      svgIcon("donate-button-icon", '<path d="M20.8 4.6a5.5 5.5 0 0 0-7.8 0L12 5.7l-1.1-1.1a5.5 5.5 0 0 0-7.8 7.8l1.1 1.1L12 21l7.8-7.5 1.1-1.1a5.5 5.5 0 0 0-.1-7.8z"/>'),
      element("span", "", "Donate")
    );
    donation.append(donationCopy, donateLink);

    promos.append(mobilePromo, donation);
    sidebar.insertBefore(promos, serverSummary);

    var sidebarHeader = sidebar.querySelector(".sidebar-header");
    var themeToggle = sidebar.querySelector("[data-theme-toggle]");
    if (!sidebarHeader || !themeToggle) {
      return;
    }

    var compactActions = element("div", "compact-promo-actions");
    var compactDonate = element("a", "compact-promo-button compact-donate-button");
    compactDonate.href = donateLink.href;
    compactDonate.target = "_blank";
    compactDonate.rel = "noopener noreferrer";
    compactDonate.title = "Support YACReader";
    compactDonate.setAttribute("aria-label", "Support YACReader with a donation");
    compactDonate.appendChild(svgIcon("compact-promo-icon", '<path d="M20.8 4.6a5.5 5.5 0 0 0-7.8 0L12 5.7l-1.1-1.1a5.5 5.5 0 0 0-7.8 7.8l1.1 1.1L12 21l7.8-7.5 1.1-1.1a5.5 5.5 0 0 0-.1-7.8z"/>'));

    var compactMobile = element("button", "compact-promo-button");
    compactMobile.type = "button";
    compactMobile.title = "Get the mobile apps";
    compactMobile.setAttribute("aria-label", "Get the YACReader mobile apps");
    compactMobile.setAttribute("aria-haspopup", "dialog");
    compactMobile.setAttribute("aria-expanded", "false");
    compactMobile.setAttribute("aria-controls", "compact-mobile-popover");
    compactMobile.appendChild(svgIcon("compact-promo-icon", '<rect x="6.5" y="2.5" width="11" height="19" rx="2.5"/><path d="M10 5h4M11 18.5h2"/>'));

    var compactPopover = element("div", "compact-mobile-popover");
    compactPopover.id = "compact-mobile-popover";
    compactPopover.hidden = true;
    compactPopover.setAttribute("role", "dialog");
    compactPopover.setAttribute("aria-labelledby", "compact-mobile-popover-title");

    var compactPopoverHeader = element("div", "compact-mobile-popover-header");
    var compactPopoverHeading = element("div");
    var compactPopoverTitle = element("h2", "", "Upgrade your reading");
    compactPopoverTitle.id = "compact-mobile-popover-title";
    compactPopoverHeading.appendChild(compactPopoverTitle);

    var compactPopoverClose = element("button", "compact-popover-close");
    compactPopoverClose.type = "button";
    compactPopoverClose.setAttribute("aria-label", "Close mobile apps");
    compactPopoverClose.appendChild(svgIcon("compact-popover-close-icon", '<path d="m7 7 10 10M17 7 7 17"/>'));
    compactPopoverHeader.append(compactPopoverHeading, compactPopoverClose);

    compactPopover.append(
      compactPopoverHeader,
      element("p", "", "Unlock a richer reading experience with panel-by-panel navigation, flexible fit modes, image filters, fluid animations, and much more."),
      storeBadge(
        "app-store-badge",
        "/images/webui/app-store-badge.svg",
        "https://apps.apple.com/app/id635717885",
        "Download YACReader on the App Store"
      ),
      storeBadge(
        "google-play-badge",
        "/images/webui/google-play-badge.png",
        "https://play.google.com/store/apps/details?id=com.yacreader.yacreader",
        "Get YACReader on Google Play"
      )
    );

    function closeCompactPopover(returnFocus) {
      if (compactPopover.hidden) {
        return;
      }
      compactPopover.hidden = true;
      compactMobile.setAttribute("aria-expanded", "false");
      if (returnFocus) {
        compactMobile.focus();
      }
    }

    compactMobile.addEventListener("click", function (event) {
      event.stopPropagation();
      var willOpen = compactPopover.hidden;
      if (willOpen) {
        compactPopover.hidden = false;
        compactMobile.setAttribute("aria-expanded", "true");
      } else {
        closeCompactPopover(false);
      }
    });
    compactPopoverClose.addEventListener("click", function () {
      closeCompactPopover(true);
    });
    compactPopover.addEventListener("click", function (event) {
      event.stopPropagation();
    });
    document.addEventListener("click", function () {
      closeCompactPopover(false);
    });
    document.addEventListener("keydown", function (event) {
      if (event.key === "Escape") {
        closeCompactPopover(true);
      }
    });
    var compactLayout = window.matchMedia("(max-width: 800px)");
    if (compactLayout.addEventListener) {
      compactLayout.addEventListener("change", function (event) {
        if (!event.matches) {
          closeCompactPopover(false);
        }
      });
    }

    compactActions.append(compactDonate, compactMobile, compactPopover);
    sidebarHeader.insertBefore(compactActions, themeToggle);
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
    var readerCleanup = null;

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

    function readerUrl(comicId) {
      return comicUrl(comicId) + "/read";
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

    function comicOpenApi(comicId) {
      return "/v2/library/" + encodeURIComponent(libraryId) + "/comic/" + encodeURIComponent(comicId) + "/remote";
    }

    function comicPageApi(comicId, page) {
      return "/v2/library/" + encodeURIComponent(libraryId) + "/comic/" + encodeURIComponent(comicId) + "/page/" + page + "/remote";
    }

    function comicProgressApi(comicId) {
      return "/v2/library/" + encodeURIComponent(libraryId) + "/comic/" + encodeURIComponent(comicId) + "/update";
    }

    function apiHeaders(accept) {
      var headers = {};
      if (accept) {
        headers.Accept = accept;
      }
      var id = requestId();
      if (id) {
        headers["X-Request-Id"] = id;
      }
      return headers;
    }

    function leaveReader() {
      if (readerCleanup) {
        readerCleanup();
        readerCleanup = null;
      }
      document.body.classList.remove("web-reader-active");
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
      leaveReader();
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

    function showReader(comicId, pushHistory, existingComic) {
      leaveReader();
      var version = ++navigationVersion;
      showLoading();

      Promise.resolve(existingComic || fetchJson(comicInfoApi(comicId))).then(function (comic) {
        if (version !== navigationVersion) {
          return;
        }

        var title = readableComicTitle(comic);
        var numPages = Math.max(0, Number(comic.num_pages) || 0);
        if (!numPages) {
          throw new Error("This comic has no readable pages.");
        }

        var savedPage = Number(comic.current_page) || 1;
        var currentPage = Math.min(numPages - 1, Math.max(0, savedPage - 1));
        var requestedPage = currentPage;
        var pageRequestVersion = 0;
        var pageCache = Object.create(null);
        var pageLoads = Object.create(null);
        var pageAbortControllers = Object.create(null);
        var destroyed = false;
        var hasDisplayedPage = false;
        var progressSynced = false;
        var comicOpened = false;

        setPageHeading(title);
        setBrowserBack(null);
        browserRoot.removeAttribute("aria-busy");
        browserRoot.replaceChildren();
        document.body.classList.add("web-reader-active");

        var reader = element("article", "web-reader");
        var toolbar = element("header", "web-reader-toolbar");
        var exitButton = element("button", "reader-toolbar-button reader-exit-button");
        exitButton.type = "button";
        exitButton.setAttribute("aria-label", "Back to comic details");
        exitButton.append(svgIcon("reader-toolbar-icon", '<path d="m15 18-6-6 6-6"/>'), element("span", "", "Back"));

        var readerHeading = element("div", "web-reader-heading");
        readerHeading.append(element("div", "web-reader-title", title));
        var pageIndicator = element("div", "web-reader-page-indicator");
        pageIndicator.setAttribute("aria-live", "polite");
        toolbar.append(exitButton, readerHeading, pageIndicator);

        var stage = element("div", "web-reader-stage");
        var previousButton = element("button", "reader-page-button reader-previous-button");
        previousButton.type = "button";
        previousButton.setAttribute("aria-label", "Previous page");
        previousButton.appendChild(svgIcon("reader-page-button-icon", '<path d="m15 18-6-6 6-6"/>'));

        var imageFrame = element("div", "web-reader-image-frame");
        var pageImage = element("img", "web-reader-page");
        pageImage.alt = "";
        pageImage.hidden = true;
        var loading = element("div", "web-reader-loading");
        loading.setAttribute("role", "status");
        loading.append(element("span", "web-reader-spinner"), element("span", "", "Loading page…"));
        var errorState = element("div", "web-reader-error");
        errorState.hidden = true;
        errorState.append(element("strong", "", "Couldn’t load this page"), element("span", "", "The comic may still be opening on the server."));
        var retryButton = element("button", "secondary-button", "Try again");
        retryButton.type = "button";
        errorState.appendChild(retryButton);
        imageFrame.append(pageImage, loading, errorState);

        var nextButton = element("button", "reader-page-button reader-next-button");
        nextButton.type = "button";
        nextButton.setAttribute("aria-label", "Next page");
        nextButton.appendChild(svgIcon("reader-page-button-icon", '<path d="m9 18 6-6-6-6"/>'));
        stage.append(previousButton, imageFrame, nextButton);
        reader.append(toolbar, stage);
        browserRoot.appendChild(reader);
        showFirstReaderPromotion(exitButton);

        function updateNavigation() {
          pageIndicator.textContent = (requestedPage + 1) + " / " + numPages;
          previousButton.disabled = !comicOpened || requestedPage <= 0;
          nextButton.disabled = !comicOpened || requestedPage >= numPages - 1;
        }

        function syncProgress() {
          if (!hasDisplayedPage || progressSynced) {
            return;
          }
          progressSynced = true;
          var headers = apiHeaders("text/plain");
          headers["Content-Type"] = "text/plain; charset=utf-8";
          fetch(comicProgressApi(comicId), {
            method: "POST",
            headers: headers,
            body: "currentPage:" + (currentPage + 1) + "\n",
            keepalive: true
          }).catch(function () {
          });
        }

        function cancelledPageError() {
          var error = new Error("Page request cancelled.");
          error.name = "AbortError";
          return error;
        }

        function pageFetch(page, controller, attempt) {
          if (destroyed || (controller && controller.signal.aborted)) {
            return Promise.reject(cancelledPageError());
          }
          return fetch(comicPageApi(comicId, page), {
            headers: apiHeaders("image/jpeg"),
            cache: "no-store",
            signal: controller ? controller.signal : undefined
          }).then(function (response) {
            if (response.status === 412 && attempt < 120) {
              return new Promise(function (resolve) {
                window.setTimeout(resolve, 500);
              }).then(function () {
                return pageFetch(page, controller, attempt + 1);
              });
            }
            if (!response.ok) {
              var error = new Error("Page request failed with status " + response.status);
              error.status = response.status;
              throw error;
            }
            return response.blob();
          });
        }

        function decodePage(blob) {
          return new Promise(function (resolve, reject) {
            var objectUrl = URL.createObjectURL(blob);
            var decodedImage = new Image();
            decodedImage.onload = function () {
              resolve(objectUrl);
            };
            decodedImage.onerror = function () {
              URL.revokeObjectURL(objectUrl);
              reject(new Error("Page image could not be decoded."));
            };
            decodedImage.src = objectUrl;
          });
        }

        function ensurePage(page) {
          if (Object.prototype.hasOwnProperty.call(pageCache, page)) {
            return Promise.resolve(pageCache[page]);
          }
          if (pageLoads[page]) {
            return pageLoads[page];
          }

          var controller = window.AbortController ? new AbortController() : null;
          pageAbortControllers[page] = controller;
          pageLoads[page] = pageFetch(page, controller, 0).then(decodePage).then(function (objectUrl) {
            delete pageLoads[page];
            delete pageAbortControllers[page];
            if (destroyed) {
              URL.revokeObjectURL(objectUrl);
              throw cancelledPageError();
            }
            pageCache[page] = objectUrl;
            return objectUrl;
          }).catch(function (error) {
            delete pageLoads[page];
            delete pageAbortControllers[page];
            throw error;
          });
          return pageLoads[page];
        }

        function preloadAdjacentPages(page) {
          [page - 1, page + 1].forEach(function (adjacentPage) {
            if (adjacentPage >= 0 && adjacentPage < numPages) {
              ensurePage(adjacentPage).catch(function () {
              });
            }
          });
        }

        function prunePageCache(page) {
          Object.keys(pageCache).forEach(function (cachedPage) {
            if (Math.abs(Number(cachedPage) - page) > 1) {
              URL.revokeObjectURL(pageCache[cachedPage]);
              delete pageCache[cachedPage];
            }
          });
          Object.keys(pageAbortControllers).forEach(function (loadingPage) {
            if (Math.abs(Number(loadingPage) - page) > 1 && pageAbortControllers[loadingPage]) {
              pageAbortControllers[loadingPage].abort();
            }
          });
        }

        function loadPage(page) {
          if (destroyed) {
            return;
          }
          requestedPage = Math.min(numPages - 1, Math.max(0, page));
          updateNavigation();
          errorState.hidden = true;
          var pageWasCached = Object.prototype.hasOwnProperty.call(pageCache, requestedPage);
          loading.hidden = pageWasCached;
          imageFrame.classList.toggle("is-loading", !pageWasCached);
          pageRequestVersion += 1;
          var requestVersion = pageRequestVersion;
          var pageToDisplay = requestedPage;

          ensurePage(pageToDisplay).then(function (objectUrl) {
            if (destroyed || requestVersion !== pageRequestVersion) {
              return;
            }
            pageImage.src = objectUrl;
            pageImage.alt = title + ", page " + (pageToDisplay + 1);
            pageImage.hidden = false;
            loading.hidden = true;
            imageFrame.classList.remove("is-loading");
            currentPage = pageToDisplay;
            hasDisplayedPage = true;
            prunePageCache(currentPage);
            preloadAdjacentPages(currentPage);
          }).catch(function (error) {
            if (destroyed || requestVersion !== pageRequestVersion || (error && error.name === "AbortError")) {
              return;
            }
            if (error && (error.status === 404 || error.status === 424)) {
              comicOpened = false;
              updateNavigation();
            }
            loading.hidden = true;
            imageFrame.classList.remove("is-loading");
            errorState.hidden = false;
          });
        }

        function openComicAndLoad() {
          errorState.hidden = true;
          loading.hidden = false;
          imageFrame.classList.add("is-loading");
          fetch(comicOpenApi(comicId), {
            headers: apiHeaders("text/plain"),
            cache: "no-store"
          }).then(function (response) {
            if (!response.ok) {
              throw new Error("Comic request failed with status " + response.status);
            }
            if (destroyed) {
              return;
            }
            comicOpened = true;
            loadPage(requestedPage);
          }).catch(function () {
            if (destroyed) {
              return;
            }
            comicOpened = false;
            updateNavigation();
            loading.hidden = true;
            imageFrame.classList.remove("is-loading");
            errorState.hidden = false;
          });
        }

        function exitReader() {
          if (history.state && history.state.view === "reader" && history.state.fromComicDetail) {
            history.back();
          } else {
            showComic(comicId, false);
          }
        }

        function handleReaderKeys(event) {
          if (event.key === "ArrowLeft") {
            event.preventDefault();
            if (requestedPage > 0) {
              loadPage(requestedPage - 1);
            }
          } else if (event.key === "ArrowRight" || event.key === " ") {
            event.preventDefault();
            if (requestedPage < numPages - 1) {
              loadPage(requestedPage + 1);
            }
          } else if (event.key === "Escape") {
            event.preventDefault();
            exitReader();
          }
        }

        function handleReaderPageHide(event) {
          if (!event.persisted) {
            syncProgress();
          }
        }

        previousButton.addEventListener("click", function () {
          loadPage(requestedPage - 1);
        });
        nextButton.addEventListener("click", function () {
          loadPage(requestedPage + 1);
        });
        retryButton.addEventListener("click", function () {
          if (comicOpened) {
            loadPage(requestedPage);
          } else {
            openComicAndLoad();
          }
        });
        exitButton.addEventListener("click", exitReader);
        document.addEventListener("keydown", handleReaderKeys);
        window.addEventListener("pagehide", handleReaderPageHide);

        readerCleanup = function () {
          syncProgress();
          destroyed = true;
          pageRequestVersion += 1;
          Object.keys(pageAbortControllers).forEach(function (page) {
            if (pageAbortControllers[page]) {
              pageAbortControllers[page].abort();
            }
          });
          Object.keys(pageCache).forEach(function (page) {
            URL.revokeObjectURL(pageCache[page]);
          });
          document.removeEventListener("keydown", handleReaderKeys);
          window.removeEventListener("pagehide", handleReaderPageHide);
        };

        var existingReaderState = !pushHistory && history.state && history.state.view === "reader"
          ? Boolean(history.state.fromComicDetail)
          : false;
        var url = readerUrl(comicId);
        var state = { view: "reader", itemId: comicId, fromComicDetail: pushHistory || existingReaderState };
        if (pushHistory) {
          history.pushState(state, "", url);
        } else {
          history.replaceState(state, "", url);
        }

        updateNavigation();
        openComicAndLoad();
      }).catch(function () {
        if (version !== navigationVersion) {
          return;
        }
        leaveReader();
        showError(function () {
          showReader(comicId, false);
        });
      });
    }

    function showComic(comicId, pushHistory) {
      leaveReader();
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

        var read = element("button", "secondary-button comic-read-button", "Read");
        read.type = "button";
        read.addEventListener("click", function () {
          showReader(comicId, true, comic);
        });
        coverColumn.appendChild(read);

        var compactMobileActions = element("div", "comic-mobile-compact-actions");
        var compactReadMobile = element("button", "primary-button comic-read-mobile-button");
        compactReadMobile.type = "button";
        compactReadMobile.setAttribute("aria-expanded", "false");
        compactReadMobile.setAttribute("aria-controls", "comic-mobile-popover");
        compactReadMobile.append(
          svgIcon("comic-read-mobile-icon", '<rect x="6.5" y="2.5" width="11" height="19" rx="2.5"/><path d="M10 5h4M11 18.5h2"/>'),
          element("span", "", "Read on mobile")
        );

        var compactComicPopover = element("div", "compact-mobile-popover comic-mobile-popover");
        compactComicPopover.id = "comic-mobile-popover";
        compactComicPopover.hidden = true;
        compactComicPopover.setAttribute("role", "dialog");
        compactComicPopover.setAttribute("aria-labelledby", "comic-mobile-popover-title");

        var compactComicPopoverHeader = element("div", "compact-mobile-popover-header");
        var compactComicPopoverHeading = element("div");
        var compactComicPopoverTitle = element("h2", "", "Upgrade your reading");
        compactComicPopoverTitle.id = "comic-mobile-popover-title";
        compactComicPopoverHeading.appendChild(compactComicPopoverTitle);
        var compactComicPopoverClose = element("button", "compact-popover-close");
        compactComicPopoverClose.type = "button";
        compactComicPopoverClose.setAttribute("aria-label", "Close mobile apps");
        compactComicPopoverClose.appendChild(svgIcon("compact-popover-close-icon", '<path d="m7 7 10 10M17 7 7 17"/>'));
        compactComicPopoverHeader.append(compactComicPopoverHeading, compactComicPopoverClose);

        compactComicPopover.append(
          compactComicPopoverHeader,
          element("p", "", "Unlock a richer reading experience with panel-by-panel navigation, flexible fit modes, image filters, fluid animations, and much more."),
          storeBadge(
            "app-store-badge",
            "/images/webui/app-store-badge.svg",
            "https://apps.apple.com/app/id635717885",
            "Download YACReader on the App Store"
          ),
          storeBadge(
            "google-play-badge",
            "/images/webui/google-play-badge.png",
            "https://play.google.com/store/apps/details?id=com.yacreader.yacreader",
            "Get YACReader on Google Play"
          )
        );

        function closeComicMobilePopover(returnFocus) {
          compactComicPopover.hidden = true;
          compactReadMobile.setAttribute("aria-expanded", "false");
          document.removeEventListener("pointerdown", handleComicMobileOutside);
          if (returnFocus) {
            compactReadMobile.focus();
          }
        }

        function handleComicMobileOutside(event) {
          if (!compactMobileActions.contains(event.target)) {
            closeComicMobilePopover(false);
          }
        }

        compactReadMobile.addEventListener("click", function () {
          if (compactComicPopover.hidden) {
            compactComicPopover.hidden = false;
            compactReadMobile.setAttribute("aria-expanded", "true");
            document.addEventListener("pointerdown", handleComicMobileOutside);
          } else {
            closeComicMobilePopover(false);
          }
        });
        compactComicPopoverClose.addEventListener("click", function () {
          closeComicMobilePopover(true);
        });
        compactMobileActions.addEventListener("keydown", function (event) {
          if (event.key === "Escape") {
            closeComicMobilePopover(true);
          }
        });

        compactMobileActions.append(compactReadMobile, compactComicPopover);
        coverColumn.appendChild(compactMobileActions);

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
      var match = window.location.pathname.match(new RegExp("^/webui/library/" + escapedLibraryId + "(?:/(folder|comic)/([0-9]+)(?:/(read))?)?/?$"));
      if (!match) {
        return { view: "folder", itemId: "1" };
      }
      return {
        view: match[3] ? "reader" : match[1] || "folder",
        itemId: match[2] || "1"
      };
    }

    window.addEventListener("popstate", function () {
      var route = routeFromLocation();
      if (route.view === "reader") {
        showReader(route.itemId, false);
      } else if (route.view === "comic") {
        showComic(route.itemId, false);
      } else {
        showFolder(route.itemId, false);
      }
    });

    var initialView = document.body.dataset.browserInitialView;
    var initialItemId = document.body.dataset.browserInitialItemId || "1";
    if (initialView === "reader") {
      showReader(initialItemId, false);
    } else if (initialView === "comic") {
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
    initSidebarActions();

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

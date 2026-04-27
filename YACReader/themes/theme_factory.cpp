#include "theme_factory.h"

#include "icon_utils.h"
#include "theme_meta.h"

#include <QApplication>

struct ToolbarParams {
    ToolbarThemeTemplates t;

    QColor iconColor;
    QColor iconDisabledColor;
    QColor iconCheckedColor;
    QColor backgroundColor;
    QColor separatorColor;
    QColor checkedButtonColor;
    QColor menuIndicatorColor;
};

struct ViewerParams {
    ViewerThemeTemplates t;

    QColor defaultBackgroundColor;
    QColor defaultTextColor;
    QColor infoBackgroundColor;
    QColor infoTextColor;
};

struct GoToFlowWidgetParams {
    GoToFlowWidgetThemeTemplates t;

    QColor flowBackgroundColor;
    QColor flowTextColor;
    QColor toolbarBackgroundColor;
    QColor sliderBorderColor;
    QColor sliderGrooveColor;
    QColor sliderHandleColor;
    QColor editBorderColor;
    QColor editBackgroundColor;
    QColor editTextColor;
    QColor labelTextColor;
    QColor iconColor;
};

struct WhatsNewDialogParams {
    QColor backgroundColor;
    QColor headerTextColor;
    QColor versionTextColor;
    QColor contentTextColor;
    QColor linkColor;
    QColor closeButtonColor;
    QColor headerDecorationColor;
};

struct ShortcutsIconsParams {
    QColor iconColor; // Main icon color (replaces #f0f)
};

struct DialogIconsParams {
    QColor iconColor; // Main icon color (replaces #f0f)
};

struct TranslatorParams {
    TranslatorThemeTemplates t;

    QColor backgroundColor { 0x40, 0x40, 0x40 };
    QColor inputBackgroundColor { 0x2a, 0x2a, 0x2a };
    QColor inputDarkerBackgroundColor { 0x27, 0x27, 0x27 };
    QColor selectionBackgroundColor { 0x20, 0x20, 0x20 };
    QColor primaryTextColor { Qt::white };
    QColor secondaryTextColor { 0xe3, 0xe3, 0xe3 };
    QColor scrollbarHandleColor { 0xdd, 0xdd, 0xdd };
    QColor borderColor { 0x21, 0x21, 0x21 };
    QColor iconColor { 0xcc, 0xcc, 0xcc };
};

struct ThemeParams {
    ThemeMeta meta;

    ToolbarParams toolbarParams;
    ViewerParams viewerParams;
    GoToFlowWidgetParams goToFlowWidgetParams;
    HelpAboutDialogTheme helpAboutDialogParams;
    WhatsNewDialogParams whatsNewDialogParams;
    ShortcutsIconsParams shortcutsIconsParams;
    DialogIconsParams dialogIconsParams;
    TranslatorParams translatorParams;
};

void setToolbarIconPair(QIcon &icon,
                        QIcon &icon18,
                        const QString &basePath,
                        const QColor &iconColor,
                        const QColor &disabledColor,
                        const QColor &checkedColor,
                        const QString &themeId)
{
    QString path18 = basePath;
    if (path18.endsWith(".svg"))
        path18.insert(path18.size() - 4, "_18x18");
    else
        path18.append("_18x18");

    // Normal
    const QString normalPath = recoloredSvgToThemeFile(basePath, iconColor, themeId);
    const QString normalPath18 = recoloredSvgToThemeFile(path18, iconColor, themeId);
    // Disabled
    const QString disabledPath = recoloredSvgToThemeFile(basePath, disabledColor, themeId, { .suffix = "_disabled" });
    const QString disabledPath18 = recoloredSvgToThemeFile(path18, disabledColor, themeId, { .suffix = "_disabled" });
    // Checked (On state)
    const QString checkedPath = recoloredSvgToThemeFile(basePath, checkedColor, themeId, { .suffix = "_checked" });
    const QString checkedPath18 = recoloredSvgToThemeFile(path18, checkedColor, themeId, { .suffix = "_checked" });

    icon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(disabledPath, QSize(), QIcon::Disabled, QIcon::Off);
    icon.addFile(checkedPath, QSize(), QIcon::Normal, QIcon::On);
    icon.addFile(disabledPath, QSize(), QIcon::Disabled, QIcon::On);

    icon18.addFile(normalPath18, QSize(), QIcon::Normal, QIcon::Off);
    icon18.addFile(disabledPath18, QSize(), QIcon::Disabled, QIcon::Off);
    icon18.addFile(checkedPath18, QSize(), QIcon::Normal, QIcon::On);
    icon18.addFile(disabledPath18, QSize(), QIcon::Disabled, QIcon::On);
}

Theme makeTheme(const ThemeParams &params)
{
    Theme theme;

    theme.meta = params.meta;

    // Toolbar & actions
    theme.toolbar.toolbarQSS = params.toolbarParams.t.toolbarQSS.arg(params.toolbarParams.backgroundColor.name(), params.toolbarParams.separatorColor.name(), params.toolbarParams.checkedButtonColor.name(), recoloredSvgToThemeFile(params.toolbarParams.t.menuArrowPath, params.toolbarParams.menuIndicatorColor, params.meta.id));

    auto setToolbarIconPairT = [&](QIcon &icon, QIcon &icon18, const QString &basePath) {
        setToolbarIconPair(icon, icon18, basePath, params.toolbarParams.iconColor, params.toolbarParams.iconDisabledColor, params.toolbarParams.iconCheckedColor, params.meta.id);
    };

    setToolbarIconPairT(theme.toolbar.openAction, theme.toolbar.openAction18x18, ":/images/viewer_toolbar/open.svg");
    setToolbarIconPairT(theme.toolbar.openFolderAction, theme.toolbar.openFolderAction18x18, ":/images/viewer_toolbar/openFolder.svg");
    setToolbarIconPairT(theme.toolbar.openLatestComicAction, theme.toolbar.openLatestComicAction18x18, ":/images/viewer_toolbar/openNext.svg");
    setToolbarIconPairT(theme.toolbar.saveImageAction, theme.toolbar.saveImageAction18x18, ":/images/viewer_toolbar/save.svg");
    setToolbarIconPairT(theme.toolbar.openComicOnTheLeftAction, theme.toolbar.openComicOnTheLeftAction18x18, ":/images/viewer_toolbar/openPrevious.svg");
    setToolbarIconPairT(theme.toolbar.openComicOnTheRightAction, theme.toolbar.openComicOnTheRightAction18x18, ":/images/viewer_toolbar/openNext.svg");
    setToolbarIconPairT(theme.toolbar.goToPageOnTheLeftAction, theme.toolbar.goToPageOnTheLeftAction18x18, ":/images/viewer_toolbar/previous.svg");
    setToolbarIconPairT(theme.toolbar.goToPageOnTheRightAction, theme.toolbar.goToPageOnTheRightAction18x18, ":/images/viewer_toolbar/next.svg");
    setToolbarIconPairT(theme.toolbar.adjustHeightAction, theme.toolbar.adjustHeightAction18x18, ":/images/viewer_toolbar/toHeight.svg");
    setToolbarIconPairT(theme.toolbar.adjustWidthAction, theme.toolbar.adjustWidthAction18x18, ":/images/viewer_toolbar/toWidth.svg");
    setToolbarIconPairT(theme.toolbar.leftRotationAction, theme.toolbar.leftRotationAction18x18, ":/images/viewer_toolbar/rotateL.svg");
    setToolbarIconPairT(theme.toolbar.rightRotationAction, theme.toolbar.rightRotationAction18x18, ":/images/viewer_toolbar/rotateR.svg");
    setToolbarIconPairT(theme.toolbar.doublePageAction, theme.toolbar.doublePageAction18x18, ":/images/viewer_toolbar/doublePage.svg");
    setToolbarIconPairT(theme.toolbar.doubleMangaPageAction, theme.toolbar.doubleMangaPageAction18x18, ":/images/viewer_toolbar/doubleMangaPage.svg");
    setToolbarIconPairT(theme.toolbar.showZoomSliderlAction, theme.toolbar.showZoomSliderlAction18x18, ":/images/viewer_toolbar/zoom.svg");
    setToolbarIconPairT(theme.toolbar.goToPageAction, theme.toolbar.goToPageAction18x18, ":/images/viewer_toolbar/goto.svg");
    setToolbarIconPairT(theme.toolbar.optionsAction, theme.toolbar.optionsAction18x18, ":/images/viewer_toolbar/options.svg");
    setToolbarIconPairT(theme.toolbar.helpAboutAction, theme.toolbar.helpAboutAction18x18, ":/images/viewer_toolbar/help.svg");
    setToolbarIconPairT(theme.toolbar.showMagnifyingGlassAction, theme.toolbar.showMagnifyingGlassAction18x18, ":/images/viewer_toolbar/magnifyingGlass.svg");
    setToolbarIconPairT(theme.toolbar.setBookmarkAction, theme.toolbar.setBookmarkAction18x18, ":/images/viewer_toolbar/bookmark.svg");
    setToolbarIconPairT(theme.toolbar.showBookmarksAction, theme.toolbar.showBookmarksAction18x18, ":/images/viewer_toolbar/showBookmarks.svg");
    setToolbarIconPairT(theme.toolbar.showShorcutsAction, theme.toolbar.showShorcutsAction18x18, ":/images/viewer_toolbar/shortcuts.svg");
    setToolbarIconPairT(theme.toolbar.showInfoAction, theme.toolbar.showInfoAction18x18, ":/images/viewer_toolbar/info.svg");
    setToolbarIconPairT(theme.toolbar.closeAction, theme.toolbar.closeAction18x18, ":/images/viewer_toolbar/close.svg");
    setToolbarIconPairT(theme.toolbar.showDictionaryAction, theme.toolbar.showDictionaryAction18x18, ":/images/viewer_toolbar/translator.svg");
    setToolbarIconPairT(theme.toolbar.adjustToFullSizeAction, theme.toolbar.adjustToFullSizeAction18x18, ":/images/viewer_toolbar/full.svg");
    setToolbarIconPairT(theme.toolbar.fitToPageAction, theme.toolbar.fitToPageAction18x18, ":/images/viewer_toolbar/fitToPage.svg");
    setToolbarIconPairT(theme.toolbar.continuousScrollAction, theme.toolbar.continuousScrollAction18x18, ":/images/viewer_toolbar/toContinuousScroll.svg");
    setToolbarIconPairT(theme.toolbar.showFlowAction, theme.toolbar.showFlowAction18x18, ":/images/viewer_toolbar/flow.svg");
    // end Toolbar & actions

    // Viewer
    theme.viewer.defaultBackgroundColor = params.viewerParams.defaultBackgroundColor;
    theme.viewer.defaultTextColor = params.viewerParams.defaultTextColor;
    theme.viewer.infoBackgroundColor = params.viewerParams.infoBackgroundColor;
    theme.viewer.infoLabelQSS = params.viewerParams.t.infoLabelQSS.arg(params.viewerParams.infoTextColor.name());
    // end Viewer

    // GoToFlowWidget
    auto &gotoParams = params.goToFlowWidgetParams;
    theme.goToFlowWidget.flowBackgroundColor = gotoParams.flowBackgroundColor;
    theme.goToFlowWidget.flowTextColor = gotoParams.flowTextColor;
    theme.goToFlowWidget.toolbarBackgroundColor = gotoParams.toolbarBackgroundColor;
    theme.goToFlowWidget.sliderQSS = gotoParams.t.sliderQSS.arg(
            gotoParams.sliderBorderColor.name(QColor::HexArgb),
            gotoParams.sliderGrooveColor.name(QColor::HexArgb),
            gotoParams.sliderHandleColor.name(QColor::HexArgb));
    theme.goToFlowWidget.editQSS = gotoParams.t.editQSS.arg(
            gotoParams.editBorderColor.name(QColor::HexArgb),
            gotoParams.editBackgroundColor.name(QColor::HexArgb),
            gotoParams.editTextColor.name());
    theme.goToFlowWidget.buttonQSS = gotoParams.t.buttonQSS;
    theme.goToFlowWidget.labelQSS = gotoParams.t.labelQSS.arg(gotoParams.labelTextColor.name());

    const QString centerIconPath = recoloredSvgToThemeFile(":/images/centerFlow.svg", gotoParams.iconColor, params.meta.id);
    const QString goToIconPath = recoloredSvgToThemeFile(":/images/gotoFlow.svg", gotoParams.iconColor, params.meta.id);
    theme.goToFlowWidget.centerIcon = QIcon(centerIconPath);
    theme.goToFlowWidget.goToIcon = QIcon(goToIconPath);
    // end GoToFlowWidget

    // HelpAboutDialog
    theme.helpAboutDialog = params.helpAboutDialogParams;
    // end HelpAboutDialog

    // WhatsNewDialog
    const auto &wn = params.whatsNewDialogParams;
    theme.whatsNewDialog.backgroundColor = wn.backgroundColor;
    theme.whatsNewDialog.headerTextColor = wn.headerTextColor;
    theme.whatsNewDialog.versionTextColor = wn.versionTextColor;
    theme.whatsNewDialog.contentTextColor = wn.contentTextColor;
    theme.whatsNewDialog.linkColor = wn.linkColor;
    theme.whatsNewDialog.closeButtonIcon = QPixmap(recoloredSvgToThemeFile(":/images/custom_dialog/custom_close_button.svg", wn.closeButtonColor, params.meta.id));
    theme.whatsNewDialog.headerDecoration = QPixmap(recoloredSvgToThemeFile(":/images/whats_new/whatsnew_header.svg", wn.headerDecorationColor, params.meta.id));
    // end WhatsNewDialog

    // ShortcutsIcons
    const auto &sci = params.shortcutsIconsParams;
    auto makeShortcutsIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, sci.iconColor, params.meta.id);
        return QIcon(path);
    };

    theme.shortcutsIcons.comicsIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_comics.svg");
    theme.shortcutsIcons.generalIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_general.svg");
    theme.shortcutsIcons.magnifyingGlassIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_mglass.svg");
    theme.shortcutsIcons.pageIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_page.svg");
    theme.shortcutsIcons.readingIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_reading.svg");
    // end ShortcutsIcons

    // FindFolder icon (used in OptionsDialog)
    {
        const QString path = recoloredSvgToThemeFile(":/images/find_folder.svg", params.toolbarParams.iconColor, params.meta.id);
        const qreal dpr = qApp->devicePixelRatio();
        theme.dialogIcons.findFolderIcon = QIcon(renderSvgToPixmap(path, 13, 13, dpr));
    }

    // Translator
    {
        const auto &tr = params.translatorParams;
        theme.translator.backgroundColor = tr.backgroundColor;
        theme.translator.inputBackgroundColor = tr.inputBackgroundColor;
        theme.translator.scrollBarQSS = tr.t.scrollBarQSS.arg(
                tr.backgroundColor.name(),
                tr.scrollbarHandleColor.name());
        theme.translator.textEditQSS = tr.t.textEditQSS.arg(
                tr.inputBackgroundColor.name(),
                tr.primaryTextColor.name());
        const QString dropDownArrowPath = recoloredSvgToThemeFile(
                ":/images/translator/dropDownArrow.svg", tr.iconColor, params.meta.id);
        theme.translator.comboBoxQSS = tr.t.comboBoxQSS.arg(
                tr.inputBackgroundColor.name(),
                tr.primaryTextColor.name(),
                dropDownArrowPath,
                tr.inputDarkerBackgroundColor.name(),
                tr.selectionBackgroundColor.name());
        theme.translator.clearButtonQSS = tr.t.clearButtonQSS.arg(
                tr.borderColor.name(),
                tr.inputBackgroundColor.name(),
                tr.primaryTextColor.name());
        theme.translator.titleQSS = tr.t.titleQSS.arg(tr.primaryTextColor.name());
        theme.translator.resultsTitleQSS = tr.t.resultsTitleQSS.arg(tr.secondaryTextColor.name());
        theme.translator.resultTextQSS = tr.t.resultTextQSS.arg(tr.primaryTextColor.name());
        theme.translator.closeIcon = QIcon(recoloredSvgToThemeFile(
                ":/images/translator/close.svg", tr.iconColor, params.meta.id));
        theme.translator.speakerIcon = QIcon(recoloredSvgToThemeFile(
                ":/images/translator/speaker.svg", tr.iconColor, params.meta.id));
        theme.translator.searchIcon = QIcon(recoloredSvgToThemeFile(
                ":/images/translator/translatorSearch.svg", tr.iconColor, params.meta.id));
        theme.translator.fromToPixmap = QPixmap(recoloredSvgToThemeFile(
                ":/images/translator/fromTo.svg", tr.iconColor, params.meta.id));
    }
    // end Translator

    return theme;
}

// JSON helpers ---------------------------------------------------------------

static QColor colorFromJson(const QJsonObject &obj, const QString &key, const QColor &fallback)
{
    if (!obj.contains(key))
        return fallback;
    QColor c(obj[key].toString());
    return c.isValid() ? c : fallback;
}

Theme makeTheme(const QJsonObject &json)
{
    ThemeParams p;

    if (json.contains("toolbar")) {
        const auto t = json["toolbar"].toObject();
        auto &tp = p.toolbarParams;
        tp.iconColor = colorFromJson(t, "iconColor", tp.iconColor);
        tp.iconDisabledColor = colorFromJson(t, "iconDisabledColor", tp.iconDisabledColor);
        tp.iconCheckedColor = colorFromJson(t, "iconCheckedColor", tp.iconCheckedColor);
        tp.backgroundColor = colorFromJson(t, "backgroundColor", tp.backgroundColor);
        tp.separatorColor = colorFromJson(t, "separatorColor", tp.separatorColor);
        tp.checkedButtonColor = colorFromJson(t, "checkedButtonColor", tp.checkedButtonColor);
        tp.menuIndicatorColor = colorFromJson(t, "menuIndicatorColor", tp.menuIndicatorColor);
    }

    if (json.contains("viewer")) {
        const auto v = json["viewer"].toObject();
        auto &vp = p.viewerParams;
        vp.defaultBackgroundColor = colorFromJson(v, "defaultBackgroundColor", vp.defaultBackgroundColor);
        vp.defaultTextColor = colorFromJson(v, "defaultTextColor", vp.defaultTextColor);
        vp.infoBackgroundColor = colorFromJson(v, "infoBackgroundColor", vp.infoBackgroundColor);
        vp.infoTextColor = colorFromJson(v, "infoTextColor", vp.infoTextColor);
    }

    if (json.contains("goToFlowWidget")) {
        const auto g = json["goToFlowWidget"].toObject();
        auto &gp = p.goToFlowWidgetParams;
        gp.flowBackgroundColor = colorFromJson(g, "flowBackgroundColor", gp.flowBackgroundColor);
        gp.flowTextColor = colorFromJson(g, "flowTextColor", gp.flowTextColor);
        gp.toolbarBackgroundColor = colorFromJson(g, "toolbarBackgroundColor", gp.toolbarBackgroundColor);
        gp.sliderBorderColor = colorFromJson(g, "sliderBorderColor", gp.sliderBorderColor);
        gp.sliderGrooveColor = colorFromJson(g, "sliderGrooveColor", gp.sliderGrooveColor);
        gp.sliderHandleColor = colorFromJson(g, "sliderHandleColor", gp.sliderHandleColor);
        gp.editBorderColor = colorFromJson(g, "editBorderColor", gp.editBorderColor);
        gp.editBackgroundColor = colorFromJson(g, "editBackgroundColor", gp.editBackgroundColor);
        gp.editTextColor = colorFromJson(g, "editTextColor", gp.editTextColor);
        gp.labelTextColor = colorFromJson(g, "labelTextColor", gp.labelTextColor);
        gp.iconColor = colorFromJson(g, "iconColor", gp.iconColor);
    }

    if (json.contains("helpAboutDialog")) {
        const auto h = json["helpAboutDialog"].toObject();
        p.helpAboutDialogParams.headingColor = colorFromJson(h, "headingColor", p.helpAboutDialogParams.headingColor);
        p.helpAboutDialogParams.linkColor = colorFromJson(h, "linkColor", p.helpAboutDialogParams.linkColor);
    }

    if (json.contains("whatsNewDialog")) {
        const auto w = json["whatsNewDialog"].toObject();
        auto &wn = p.whatsNewDialogParams;
        wn.backgroundColor = colorFromJson(w, "backgroundColor", wn.backgroundColor);
        wn.headerTextColor = colorFromJson(w, "headerTextColor", wn.headerTextColor);
        wn.versionTextColor = colorFromJson(w, "versionTextColor", wn.versionTextColor);
        wn.contentTextColor = colorFromJson(w, "contentTextColor", wn.contentTextColor);
        wn.linkColor = colorFromJson(w, "linkColor", wn.linkColor);
        wn.closeButtonColor = colorFromJson(w, "closeButtonColor", wn.closeButtonColor);
        wn.headerDecorationColor = colorFromJson(w, "headerDecorationColor", wn.headerDecorationColor);
    }

    if (json.contains("shortcutsIcons")) {
        const auto s = json["shortcutsIcons"].toObject();
        p.shortcutsIconsParams.iconColor = colorFromJson(s, "iconColor", p.shortcutsIconsParams.iconColor);
    }

    if (json.contains("dialogIcons")) {
        const auto o = json["dialogIcons"].toObject();
        p.dialogIconsParams.iconColor = colorFromJson(o, "iconColor", p.dialogIconsParams.iconColor);
    }

    if (json.contains("translator")) {
        const auto t = json["translator"].toObject();
        auto &tp = p.translatorParams;
        tp.backgroundColor = colorFromJson(t, "backgroundColor", tp.backgroundColor);
        tp.inputBackgroundColor = colorFromJson(t, "inputBackgroundColor", tp.inputBackgroundColor);
        tp.inputDarkerBackgroundColor = colorFromJson(t, "inputDarkerBackgroundColor", tp.inputDarkerBackgroundColor);
        tp.selectionBackgroundColor = colorFromJson(t, "selectionBackgroundColor", tp.selectionBackgroundColor);
        tp.primaryTextColor = colorFromJson(t, "primaryTextColor", tp.primaryTextColor);
        tp.secondaryTextColor = colorFromJson(t, "secondaryTextColor", tp.secondaryTextColor);
        tp.scrollbarHandleColor = colorFromJson(t, "scrollbarHandleColor", tp.scrollbarHandleColor);
        tp.borderColor = colorFromJson(t, "borderColor", tp.borderColor);
        tp.iconColor = colorFromJson(t, "iconColor", tp.iconColor);
    }

    if (json.contains("meta")) {
        const auto o = json["meta"].toObject();
        p.meta.id = o["id"].toString(p.meta.id);
        p.meta.displayName = o["displayName"].toString(p.meta.displayName);
        const QString variantStr = o["variant"].toString();
        if (variantStr == "light")
            p.meta.variant = ThemeVariant::Light;
        else if (variantStr == "dark")
            p.meta.variant = ThemeVariant::Dark;
        p.meta.targetApp = o["targetApp"].toString(p.meta.targetApp);
        p.meta.version = o["version"].toString(p.meta.version);
    }

    Theme theme = makeTheme(p);
    theme.sourceJson = json;
    return theme;
}

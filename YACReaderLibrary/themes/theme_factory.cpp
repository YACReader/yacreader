#include "theme_factory.h"

#include "icon_utils.h"

struct ComicVineParams {
    ComicVineThemeTemplates t;

    QColor contentTextColor;
    QColor contentBackgroundColor;
    QColor contentAltBackgroundColor;
    QColor dialogBackgroundColor;

    QColor tableBackgroundColor;
    QColor tableAltBackgroundColor;
    QColor tableBorderColor;
    QColor tableSelectedColor;
    QColor tableHeaderBackgroundColor;
    QColor tableHeaderBorderColor;
    QColor tableHeaderTextColor;
    QColor tableScrollHandleColor;
    QColor tableScrollBackgroundColor;
    QColor tableSectionBorderLight;
    QColor tableSectionBorderDark;

    QColor labelTextColor;
    QColor labelBackgroundColor;
    QColor hyperlinkColor; // TODO: it doesn't work

    QColor buttonBackgroundColor;
    QColor buttonTextColor;
    QColor buttonBorderColor;

    QString radioUncheckedPath;
    QColor radioUncheckedColor;

    QString radioCheckedPath;
    QColor radioCheckedBackgroundColor;
    QColor radioCheckedIndicatorColor;

    QColor checkBoxTickColor;

    QColor toolButtonAccentColor;

    QColor downArrowColor;
    QColor upArrowColor;

    QColor busyIndicatorColor;
    QColor navIconColor;
    QColor rowIconColor;
};

struct ThemeParams {
    QString themeName;

    ComicFlowColors comicFlowColors;
    ComicVineParams comicVineParams;
    HelpAboutDialogTheme helpAboutDialogParams;
};

Theme makeTheme(const ThemeParams &params)
{
    Theme theme;

    // Comic Flow
    const auto &cf = params.comicFlowColors;
    theme.comicFlow.backgroundColor = cf.backgroundColor;
    theme.comicFlow.textColor = cf.textColor;

    // Comic Vine
    const auto &cv = params.comicVineParams;
    const auto &t = cv.t;

    auto recolor = [&](const QString &path, const QColor &color) {
        return recoloredSvgToThemeFile(path, color, params.themeName);
    };

    theme.comicVine.defaultLabelQSS = t.defaultLabelQSS.arg(cv.labelTextColor.name());
    theme.comicVine.titleLabelQSS = t.titleLabelQSS.arg(cv.labelTextColor.name());
    theme.comicVine.coverLabelQSS = t.coverLabelQSS.arg(cv.labelBackgroundColor.name(), cv.labelTextColor.name());
    theme.comicVine.radioButtonQSS = t.radioButtonQSS.arg(cv.buttonTextColor.name(), recolor(cv.radioUncheckedPath, cv.radioUncheckedColor), recoloredSvgToThemeFile(cv.radioCheckedPath, cv.radioCheckedBackgroundColor, cv.radioCheckedIndicatorColor, params.themeName));
    theme.comicVine.checkBoxQSS = t.checkBoxQSS.arg(cv.buttonTextColor.name(), cv.buttonBorderColor.name(), cv.buttonBackgroundColor.name(), recolor(":/images/comic_vine/checkBoxTick.svg", cv.checkBoxTickColor));

    theme.comicVine.scraperLineEditTitleLabelQSS = t.scraperLineEditTitleLabelQSS.arg(cv.contentTextColor.name());
    theme.comicVine.scraperLineEditQSS = t.scraperLineEditQSS.arg(cv.contentAltBackgroundColor.name(), cv.contentTextColor.name(), "%1");

    theme.comicVine.scraperToolButtonQSS = t.scraperToolButtonQSS.arg(cv.buttonBackgroundColor.name(), cv.buttonTextColor.name(), cv.toolButtonAccentColor.name());
    theme.comicVine.scraperToolButtonSeparatorQSS = t.scraperToolButtonSeparatorQSS.arg(cv.toolButtonAccentColor.name());
    theme.comicVine.scraperToolButtonFillColor = cv.buttonBackgroundColor;

    theme.comicVine.scraperScrollLabelTextQSS = t.scraperScrollLabelTextQSS.arg(cv.contentBackgroundColor.name(), cv.contentTextColor.name(), cv.hyperlinkColor.name());
    theme.comicVine.scraperScrollLabelScrollAreaQSS = t.scraperScrollLabelScrollAreaQSS.arg(cv.contentBackgroundColor.name(), cv.tableScrollHandleColor.name(), cv.tableScrollBackgroundColor.name());

    theme.comicVine.scraperTableViewQSS = t.scraperTableViewQSS
                                                  .arg(cv.tableHeaderTextColor.name())
                                                  .arg(cv.tableAltBackgroundColor.name())
                                                  .arg(cv.tableBackgroundColor.name())
                                                  .arg(cv.tableSelectedColor.name())
                                                  .arg(cv.tableHeaderBackgroundColor.name())
                                                  .arg(cv.tableHeaderBorderColor.name())
                                                  .arg(cv.tableHeaderTextColor.name())
                                                  .arg(cv.tableSectionBorderDark.name())
                                                  .arg(cv.tableSectionBorderLight.name())
                                                  .arg(cv.tableScrollHandleColor.name())
                                                  .arg(cv.tableScrollBackgroundColor.name())
                                                  .arg(recolor(":/images/comic_vine/downArrow.svg", cv.downArrowColor))
                                                  .arg(recolor(":/images/comic_vine/upArrow.svg", cv.upArrowColor));

    theme.comicVine.dialogQSS = t.dialogQSS.arg(cv.dialogBackgroundColor.name());
    theme.comicVine.dialogButtonsQSS = t.dialogButtonsQSS.arg(cv.buttonBorderColor.name(), cv.buttonBackgroundColor.name(), cv.buttonTextColor.name());

    theme.comicVine.busyIndicatorColor = cv.busyIndicatorColor;

    theme.comicVine.nextPageIcon = { QIcon(recolor(t.nextPageIcon, cv.navIconColor)), t.pageIconSize };
    theme.comicVine.previousPageIcon = { QIcon(recolor(t.previousPageIcon, cv.navIconColor)), t.pageIconSize };

    theme.comicVine.rowUpIcon = { QIcon(recolor(t.rowUpIcon, cv.rowIconColor)), t.rowIconSize };
    theme.comicVine.rowDownIcon = { QIcon(recolor(t.rowDownIcon, cv.rowIconColor)), t.rowIconSize };

    // HelpAboutDialog
    theme.helpAboutDialog = params.helpAboutDialogParams;
    // end HelpAboutDialog

    return theme;
}

ThemeParams classicThemeParams();
ThemeParams lightThemeParams();
ThemeParams darkThemeParams();

Theme makeTheme(ThemeId themeId)
{
    switch (themeId) {
    case ThemeId::Classic:
        return makeTheme(classicThemeParams());
    case ThemeId::Light:
        return makeTheme(lightThemeParams());
    case ThemeId::Dark:
        return makeTheme(darkThemeParams());
    }

    return makeTheme(classicThemeParams());
}

ThemeParams classicThemeParams()
{
    ThemeParams params;
    params.themeName = "classic";

    ComicFlowColors cf;
    cf.backgroundColor = Qt::black;
    cf.textColor = QColor(0x4C4C4C);

    ComicVineParams cv;
    cv.contentTextColor = Qt::white;
    cv.contentBackgroundColor = QColor(0x2B2B2B);
    cv.contentAltBackgroundColor = QColor(0x2B2B2B);
    cv.dialogBackgroundColor = QColor(0x404040);

    cv.tableBackgroundColor = QColor(0x2B2B2B);
    cv.tableAltBackgroundColor = QColor(0x2E2E2E);
    cv.tableBorderColor = QColor(0x242424);
    cv.tableSelectedColor = QColor(0x555555);
    cv.tableHeaderBackgroundColor = QColor(0x292929);
    cv.tableHeaderBorderColor = QColor(0x1F1F1F);
    cv.tableHeaderTextColor = QColor(0xEBEBEB);
    cv.tableScrollHandleColor = QColor(0xDDDDDD);
    cv.tableScrollBackgroundColor = QColor(0x404040);
    cv.tableSectionBorderLight = QColor(0xFEFEFE);
    cv.tableSectionBorderDark = QColor(0xDFDFDF);

    cv.labelTextColor = Qt::white;
    cv.labelBackgroundColor = QColor(0x2B2B2B);
    cv.hyperlinkColor = QColor(0xFFCC00);

    cv.buttonBackgroundColor = QColor(0x2E2E2E);
    cv.buttonTextColor = Qt::white;
    cv.buttonBorderColor = QColor(0x242424);

    cv.radioUncheckedPath = ":/images/comic_vine/radioUnchecked.png";
    cv.radioUncheckedColor = QColor(0xE5E5E5);

    cv.radioCheckedPath = ":/images/comic_vine/radioChecked.svg";
    cv.radioCheckedBackgroundColor = QColor(0xE5E5E5);
    cv.radioCheckedIndicatorColor = QColor(0x5F5F5F);

    cv.checkBoxTickColor = Qt::white;

    cv.toolButtonAccentColor = QColor(0x282828);

    cv.downArrowColor = QColor(0x9F9F9F);
    cv.upArrowColor = QColor(0x9F9F9F);

    cv.busyIndicatorColor = Qt::white;
    cv.navIconColor = Qt::white;
    cv.rowIconColor = QColor(0xE5E5E5);

    cv.t = ComicVineThemeTemplates();

    params.comicFlowColors = cf;
    params.comicVineParams = cv;

    params.helpAboutDialogParams.headingColor = QColor(0x302f2d);
    params.helpAboutDialogParams.linkColor = QColor(0xC19441);

    return params;
}

ThemeParams lightThemeParams()
{
    ThemeParams params;
    params.themeName = "light";

    ComicFlowColors cf;
    cf.backgroundColor = Qt::white;
    cf.textColor = Qt::black;

    ComicVineParams cv;
    cv.contentTextColor = Qt::black;
    cv.contentBackgroundColor = QColor(0xECECEC);
    cv.contentAltBackgroundColor = QColor(0xE0E0E0);
    cv.dialogBackgroundColor = QColor(0xFBFBFB);

    cv.tableBackgroundColor = QColor(0xF4F4F4);
    cv.tableAltBackgroundColor = QColor(0xFAFAFA);
    cv.tableBorderColor = QColor(0xCCCCCC);
    cv.tableSelectedColor = QColor(0xDDDDDD);
    cv.tableHeaderBackgroundColor = QColor(0xE0E0E0);
    cv.tableHeaderBorderColor = QColor(0xC0C0C0);
    cv.tableHeaderTextColor = QColor(0x333333);
    cv.tableScrollHandleColor = QColor(0x888888);
    cv.tableScrollBackgroundColor = QColor(0xD0D0D0);
    cv.tableSectionBorderLight = QColor(0xFFFFFF);
    cv.tableSectionBorderDark = QColor(0xCCCCCC);

    cv.labelTextColor = Qt::black;
    cv.labelBackgroundColor = QColor(0xECECEC);
    cv.hyperlinkColor = QColor(0xFFCC00);

    cv.buttonBackgroundColor = QColor(0xE0E0E0);
    cv.buttonTextColor = Qt::black;
    cv.buttonBorderColor = QColor(0xCCCCCC);

    cv.radioUncheckedPath = ":/images/comic_vine/radioUnchecked.svg";
    cv.radioUncheckedColor = QColor(0xE0E0E0);

    cv.radioCheckedPath = ":/images/comic_vine/radioChecked.svg";
    cv.radioCheckedBackgroundColor = QColor(0xE0E0E0);
    cv.radioCheckedIndicatorColor = QColor(0x222222);

    cv.checkBoxTickColor = Qt::black;

    cv.toolButtonAccentColor = QColor(0xA0A0A0);

    cv.downArrowColor = QColor(0x222222);
    cv.upArrowColor = QColor(0x222222);

    cv.busyIndicatorColor = Qt::black;
    cv.navIconColor = QColor(0x222222);
    cv.rowIconColor = QColor(0x222222);

    cv.t = ComicVineThemeTemplates();

    params.comicFlowColors = cf;
    params.comicVineParams = cv;

    params.helpAboutDialogParams.headingColor = QColor(0x302f2d);
    params.helpAboutDialogParams.linkColor = QColor(0xC19441);

    return params;
}

ThemeParams darkThemeParams()
{
    ThemeParams params;
    params.themeName = "dark";

    ComicFlowColors cf;
    cf.backgroundColor = QColor(0x111111);
    cf.textColor = QColor(0x888888);

    ComicVineParams cv;
    cv.contentTextColor = Qt::white;
    cv.contentBackgroundColor = QColor(0x2B2B2B);
    cv.contentAltBackgroundColor = QColor(0x2E2E2E);
    cv.dialogBackgroundColor = QColor(0x404040);

    cv.tableBackgroundColor = QColor(0x2B2B2B);
    cv.tableAltBackgroundColor = QColor(0x2E2E2E);
    cv.tableBorderColor = QColor(0x242424);
    cv.tableSelectedColor = QColor(0x555555);
    cv.tableHeaderBackgroundColor = QColor(0x292929);
    cv.tableHeaderBorderColor = QColor(0x1F1F1F);
    cv.tableHeaderTextColor = QColor(0xEBEBEB);
    cv.tableScrollHandleColor = QColor(0xDDDDDD);
    cv.tableScrollBackgroundColor = QColor(0x404040);
    cv.tableSectionBorderLight = QColor(0xFEFEFE);
    cv.tableSectionBorderDark = QColor(0xDFDFDF);

    cv.labelTextColor = Qt::white;
    cv.labelBackgroundColor = QColor(0x2B2B2B);
    cv.hyperlinkColor = QColor(0xFFCC00);

    cv.buttonBackgroundColor = QColor(0x2E2E2E);
    cv.buttonTextColor = Qt::white;
    cv.buttonBorderColor = QColor(0x242424);

    cv.radioUncheckedPath = ":/images/comic_vine/radioUnchecked.png";
    cv.radioUncheckedColor = QColor(0xE5E5E5);

    cv.radioCheckedPath = ":/images/comic_vine/radioChecked.svg";
    cv.radioCheckedBackgroundColor = QColor(0xE5E5E5);
    cv.radioCheckedIndicatorColor = QColor(0x5F5F5F);

    cv.checkBoxTickColor = Qt::white;

    cv.toolButtonAccentColor = QColor(0x282828);

    cv.downArrowColor = QColor(0x9F9F9F);
    cv.upArrowColor = QColor(0x9F9F9F);

    cv.busyIndicatorColor = Qt::white;
    cv.navIconColor = Qt::white;
    cv.rowIconColor = QColor(0xE5E5E5);

    cv.t = ComicVineThemeTemplates();

    params.comicFlowColors = cf;
    params.comicVineParams = cv;

    params.helpAboutDialogParams.headingColor = QColor(0xE0E0E0);
    params.helpAboutDialogParams.linkColor = QColor(0xD4A84B);

    return params;
}

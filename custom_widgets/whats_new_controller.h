#ifndef WHATSNEWCONTROLLER_H
#define WHATSNEWCONTROLLER_H

class QWidget;

namespace YACReader {

class WhatsNewController
{
public:
    WhatsNewController();
    void showWhatsNewIfNeeded(QWidget *fromParent = nullptr);
    void showWhatsNew(QWidget *fromParent = nullptr);
};

} // namespace YACReader

#endif // WHATSNEWCONTROLLER_H

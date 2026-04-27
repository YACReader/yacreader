#ifndef __BUNDLE_CREATOR_H
#define __BUNDLE_CREATOR_H

#include <QObject>

class BundleCreator : public QObject
{
    Q_OBJECT
public:
    BundleCreator(void);
    ~BundleCreator(void) override;
};

#endif
#ifndef FLOORPLANREPOSITORY_H
#define FLOORPLANREPOSITORY_H

#include "FloorPlanDocument.h"

#include <QString>

class FloorPlanRepository
{
public:
    explicit FloorPlanRepository(QString filePath);

    bool load(FloorPlanDocument *document, bool *fileMissing = nullptr, QString *errorMessage = nullptr) const;
    bool save(const FloorPlanDocument &document, QString *errorMessage = nullptr) const;

private:
    QString filePath;
};

#endif

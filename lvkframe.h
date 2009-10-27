#ifndef LVKFRAME_H
#define LVKFRAME_H

#include <QString>

#include "types.h"

/// Frame abstraction.
/// ox, oy, w, and h are intended for future use to create
/// frames that are a portion of the original input image.
struct LvkFrame
{
    LvkFrame(Id id = NullId, Id imgId = NullId,
             int ox = 0, int oy = 0, int w = 0, int h = 0,
             const QString& name = "");

    LvkFrame(const QString& str);

    Id  id;             /* frame id */
    Id  imgId;          /* input image id */
    int ox;             /* offset x */
    int oy;             /* offset y */
    int w;              /* width */
    int h;              /* height */
    QString name;       /* frame name */

    /// returns the string representation
    QString toString() const;

    /// initializes the current instance from the string @param str
    bool fromString(const QString& str);
};

#endif // LVKFRAME_H
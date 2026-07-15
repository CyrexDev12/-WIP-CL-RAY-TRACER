#ifndef CLRT_SCENE_OBJECT_RESOLVER_H
#define CLRT_SCENE_OBJECT_RESOLVER_H

#include "scene/StableIds.h"

class Shape;

namespace clrt::scene {

class ObjectResolver {
public:
    virtual ~ObjectResolver() = default;
    [[nodiscard]] virtual const Shape& resolve(ObjectId id) const = 0;
};

} // namespace clrt::scene

#endif

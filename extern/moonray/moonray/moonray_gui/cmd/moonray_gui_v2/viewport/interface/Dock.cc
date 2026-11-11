// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "Dock.h"
#include "Component.h"

namespace moonray_gui_v2 {

Dock::Dock()
{
    updateSize(); // initialize size
}

int
Dock::updateSize()
{
    int size = 0;
    for (const Component* component : mComponents) {
        if (component->isOpen()) {
            size += getComponentSize(component);
        }
    }
    mSizeDiff = size - mSize;
    mSize = size;
    return size;
}

int
Dock::getOffset(const Component* inputComp) const
{
    int offset = 0;
    for (const Component* comp : mComponents) {
        if (comp == inputComp) {
            break;
        }
        if (comp->isOpen()) {
            offset += getComponentSize(comp);
        }
    }
    return offset;
}

void 
Dock::addComponent(const Component* component)
{
    mComponents.push_back(component);
}

} // end namespace moonray_gui_v2

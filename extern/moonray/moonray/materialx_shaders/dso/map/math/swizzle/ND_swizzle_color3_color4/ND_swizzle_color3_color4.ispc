// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the Moonray Project

/// @file "ND_swizzle_color3_color4.ispc"

#include "attributes.isph"

#include <moonray/rendering/shading/ispc/MapApi.isph>

struct ND_swizzle_color3_color4 {
    uniform bool disableMode;
    uniform int rChannel;
    uniform int gChannel;
    uniform int bChannel;
    uniform int aChannel;
};

ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ND_swizzle_color3_color4);

static varying float pickChannel(varying Color input, uniform int channel)
{
    float result;
    if (channel == 0) {
       result = input.r;
    } else if (channel == 1) {
       result = input.g;
    } else {
       result = input.b;
    }
    return result;
}

static varying Col4f
sample(const uniform Map*            uniform map,
             uniform ShadingTLState* uniform tls,
       const varying State&                  state)
{
    const uniform ND_swizzle_color3_color4* uniform me = MAP_GET_ISPC_CPTR(ND_swizzle_color3_color4, map);
    // SdfType is color3f
    // NdrType is color
    varying Color inValue;
    inValue = evalInAttr(map, tls, state);

    // SdfType is string
    // NdrType is string
    // uniform String channelsValue;
    // channelsValue = getChannelsAttr(map);

    // SdfType is color4f
    // NdrType is color4
    varying Col4f outValue;
    if (me->disableMode) {
        outValue = Col4f_ctor(0.0f, 0.0f, 0.0f,0.0f);
    } else if (me->rChannel < 0) {
        // the swizzle string was invalid
        outValue = Col4f_ctor(1.0f, 0.0f, 1.0f,0.0f);
    } else {
        outValue = Col4f_ctor(pickChannel(inValue, me->rChannel),
                              pickChannel(inValue, me->gChannel),
                              pickChannel(inValue, me->bChannel),
                              pickChannel(inValue, me->aChannel));
    }
    return outValue;
/*
    family: swizzle
    category: 
    role: channel
    help: 
    context: pattern
    source type: mtlx
    implementationName: ND_swizzle_color3_color4
    sourceCode: 
    departments:
    primvars:
    Inputs:
        bool disable = false; // implicit
        Color in= Color(0.0f, 0.0f, 0.0f);
            Metadata
                widget = default
        String channels= "rrrr";
            Metadata
                widget = default
    Outputs:
        Rgba out= Rgba(0.0f, 0.0f, 0.0f,0.0f);
            Metadata
            widget = default
*/
}

DEFINE_MAP_SHADER_WITH_TYPE(ND_swizzle_color3_color4, sample, Col4f)


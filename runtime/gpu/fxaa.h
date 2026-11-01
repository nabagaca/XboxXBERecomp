#pragma once

// Shared presentation-only FXAA. img, ls and uvrect are declared by the blit shader.
// Clamp to the visible source rectangle so overscan never bleeds into its edges.
inline constexpr const char* kFxaaHlsl = R"(
float3 AaSample(float2 uv, float2 texel) {
    float2 lo = uvrect.xy + texel * 0.5;
    float2 hi = max(lo, uvrect.xy + uvrect.zw - texel * 0.5);
    return img.SampleLevel(ls, clamp(uv, lo, hi), 0).rgb;
}
float3 Fxaa(float2 uv) {
    uint w, h; img.GetDimensions(w, h);
    float2 texel = 1.0 / float2(w, h);
    float3 m = AaSample(uv, texel);
    float3 nw = AaSample(uv + float2(-1, -1) * texel, texel);
    float3 ne = AaSample(uv + float2( 1, -1) * texel, texel);
    float3 sw = AaSample(uv + float2(-1,  1) * texel, texel);
    float3 se = AaSample(uv + float2( 1,  1) * texel, texel);
    float3 luma = float3(0.299, 0.587, 0.114);
    float lm = dot(m, luma), lnw = dot(nw, luma), lne = dot(ne, luma);
    float lsw = dot(sw, luma), lse = dot(se, luma);
    float low = min(lm, min(min(lnw, lne), min(lsw, lse)));
    float high = max(lm, max(max(lnw, lne), max(lsw, lse)));
    if (high - low < max(0.0312, high * 0.125)) return m;
    float2 dir = float2(-((lnw + lne) - (lsw + lse)), (lnw + lsw) - (lne + lse));
    float reduce = max((lnw + lne + lsw + lse) * (0.25 / 8.0), 1.0 / 128.0);
    dir = clamp(dir / (min(abs(dir.x), abs(dir.y)) + reduce), -8.0, 8.0) * texel;
    float3 a = 0.5 * (AaSample(uv - dir / 6.0, texel) + AaSample(uv + dir / 6.0, texel));
    float3 b = a * 0.5 + 0.25 * (AaSample(uv - dir * 0.5, texel) + AaSample(uv + dir * 0.5, texel));
    float lb = dot(b, luma);
    return (lb < low || lb > high) ? a : b;
}
)";

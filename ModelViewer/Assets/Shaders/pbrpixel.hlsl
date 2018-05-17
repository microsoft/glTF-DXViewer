//
// This fragment shader defines a reference implementation for Physically Based Shading of
// a microfacet surface material defined by a glTF model.
//
// References:
// [1] Real Shading in Unreal Engine 4
//     http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// [2] Physically Based Shading at Disney
//     http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
// [3] README.md - Environment Maps
//     https://github.com/KhronosGroup/glTF-WebGL-PBR/#environment-maps
// [4] "An Inexpensive BRDF Model for Physically based Rendering" by Christophe Schlick
//     https://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf

#define NORMALS
#define UV
#define HAS_NORMALS
#define USE_IBL
#define USE_TEX_LOD

Texture2D baseColourTexture : register(t0);
SamplerState baseColourSampler : register(s0);

Texture2D normalTexture : register(t1);
SamplerState normalSampler : register(s1);

Texture2D emissionTexture : register(t2);
SamplerState emissionSampler : register(s2);

Texture2D occlusionTexture : register(t3);
SamplerState occlusionSampler : register(s3);

Texture2D metallicRoughnessTexture : register(t4);
SamplerState metallicRoughnessSampler : register(s4);

TextureCube envDiffuseTexture : register(t8);
SamplerState envDiffuseSampler : register(s8);

Texture2D brdfLutTexture : register(t9);
SamplerState brdfLutSampler : register(s9);

TextureCube envSpecularTexture : register(t10);
SamplerState envSpecularSampler : register(s10);

struct Light
{
    float3 dir;
    float padding1;
    float3 colour;
    float padding2;
};

cbuffer cbPerFrame : register(b0)
{
    Light light;
};

cbuffer cbPerObject : register(b1)
{
    float normalScale;
    float3 emissiveFactor;
    float occlusionStrength;
    float2 metallicRoughnessValues;
    float padding1;
    float4 baseColorFactor;
    float3 camera;
    float padding2;

    // debugging flags used for shader output of intermediate PBR variables
    float4 scaleDiffBaseMR;
    float4 scaleFGDSpec;
    float4 scaleIBLAmbient;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 poswithoutw : POSITION;

#ifdef NORMALS
    float3 normal : NORMAL;
#endif
#ifdef UV
    float2 texcoord : TEXCOORD0;
#endif
};

#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
varying mat3 v_TBN;
#else
#endif
#endif

// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms, outlined in the Readme.MD Appendix.
struct PBRInfo
{
    float NdotL; // cos angle between normal and light direction
    float NdotV; // cos angle between normal and view direction
    float NdotH; // cos angle between normal and half vector
    float LdotH; // cos angle between light direction and half vector
    float VdotH; // cos angle between view direction and half vector
    float perceptualRoughness; // roughness value, as authored by the model creator (input to shader)
    float metalness; // metallic value at the surface
    float3 reflectance0; // full reflectance color (normal incidence angle)
    float3 reflectance90; // reflectance color at grazing angle
    float alphaRoughness; // roughness mapped to a more linear change in the roughness (proposed by [2])
    float3 diffuseColor; // color contribution from diffuse lighting
    float3 specularColor; // color contribution from specular lighting
};

static const float M_PI = 3.141592653589793;
static const float c_MinRoughness = 0.04;

float4 SRGBtoLINEAR(float4 srgbIn)
{
#ifdef MANUAL_SRGB
#ifdef SRGB_FAST_APPROXIMATION
    float3 linOut = pow(srgbIn.xyz,float3(2.2, 2.2, 2.2));
#else //SRGB_FAST_APPROXIMATION
    float3 bLess = step(float3(0.04045, 0.04045, 0.04045), srgbIn.xyz);
    float3 linOut = lerp(srgbIn.xyz / float3(12.92, 12.92, 12.92), pow((srgbIn.xyz + float3(0.055, 0.055, 0.055)) / float3(1.055, 1.055, 1.055), float3(2.4, 2.4, 2.4)), bLess);
#endif //SRGB_FAST_APPROXIMATION
    return float4(linOut,srgbIn.w);;
#else //MANUAL_SRGB
    return srgbIn;
#endif //MANUAL_SRGB
}

// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
float3 getNormal(float3 position, float3 normal, float2 uv)
{
    // Retrieve the tangent space matrix
#ifndef HAS_TANGENTS
    float3 pos_dx = ddx(position);
    float3 pos_dy = ddy(position);
    float3 tex_dx = ddx(float3(uv, 0.0));
    float3 tex_dy = ddy(float3(uv, 0.0));
    float3 t = (tex_dy.y * pos_dx - tex_dx.y * pos_dy) / (tex_dx.x * tex_dy.y - tex_dy.x * tex_dx.y);

#ifdef HAS_NORMALS
    float3 ng = normalize(normal);
#else
    float3 ng = cross(pos_dx, pos_dy);
#endif

    t = normalize(t - ng * dot(ng, t));
    float3 b = normalize(cross(ng, t));
    row_major float3x3 tbn = float3x3(t, b, ng);

#else // HAS_TANGENTS
    mat3 tbn = v_TBN;
#endif

#ifdef HAS_NORMALMAP
    float3 n = normalTexture.Sample(normalSampler, uv).rgb;

    // Need to check the multiplication is equivalent..
    n = normalize(mul(((2.0 * n - 1.0) * float3(normalScale, normalScale, 1.0)), tbn));
#else
    float3 n = tbn[2].xyz;
#endif

    return n;
}

#ifdef USE_IBL
// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
float3 getIBLContribution(PBRInfo pbrInputs, float3 n, float3 reflection)
{
    float mipCount = 9.0; // resolution of 512x512
    float lod = (pbrInputs.perceptualRoughness * mipCount);
  
    // retrieve a scale and bias to F0. See [1], Figure 3
    float2 val = float2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness);
    float3 brdf = SRGBtoLINEAR(brdfLutTexture.Sample(brdfLutSampler, val)).rgb;

    float3 diffuseLight = SRGBtoLINEAR(envDiffuseTexture.Sample(envDiffuseSampler, n)).rgb;

#ifdef USE_TEX_LOD
    float3 specularLight = SRGBtoLINEAR(envSpecularTexture.SampleLevel(envSpecularSampler, reflection, 0)).rgb;
#else
    float3 specularLight = SRGBtoLINEAR(envSpecularTexture.Sample(envSpecularSampler, reflection)).rgb;
#endif

    float3 diffuse = diffuseLight * pbrInputs.diffuseColor;
    float3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

    // For presentation, this allows us to disable IBL terms
    diffuse *= scaleIBLAmbient.x;
    specular *= scaleIBLAmbient.y;

    return diffuse + specular;
}
#endif

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
float3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / M_PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
float3 specularReflection(PBRInfo pbrInputs)
{
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    // Metallic and Roughness material properties are packed together
    // In glTF, these factors can be specified by fixed scalar values
    // or from a metallic-roughness map
    float perceptualRoughness = metallicRoughnessValues.y;
    float metallic = metallicRoughnessValues.x;

#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    float4 mrSample = metallicRoughnessTexture.Sample(metallicRoughnessSampler, input.texcoord);

	// Had to reverse the order of the channels here - TODO: investigate..
    perceptualRoughness = mrSample.g * perceptualRoughness;
    metallic = mrSample.b * metallic;
#endif

    perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);

    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness [2].
    float alphaRoughness = perceptualRoughness * perceptualRoughness;

    // The albedo may be defined from a base texture or a flat color

#ifdef HAS_BASECOLORMAP
    float4 baseColor = SRGBtoLINEAR(baseColourTexture.Sample(baseColourSampler, input.texcoord)) * baseColorFactor;
#else
    float4 baseColor = baseColorFactor;
#endif

    float3 f0 = float3(0.04, 0.04, 0.04);
    float3 diffuseColor = baseColor.rgb * (float3(1.0, 1.0, 1.0) - f0);

    diffuseColor *= 1.0 - metallic;

    float3 specularColor = lerp(f0, baseColor.rgb, metallic);

    // Compute reflectance.
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
    // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    float3 specularEnvironmentR0 = specularColor.rgb;
    float3 specularEnvironmentR90 = float3(1.0, 1.0, 1.0) * reflectance90;

    float3 n = getNormal(input.poswithoutw, input.normal, input.texcoord); // normal at surface point
    float3 v = normalize(camera - input.poswithoutw); // Vector from surface point to camera
    
    float3 l = normalize(light.dir); // Vector from surface point to light
    float3 h = normalize(l + v); // Half vector between both l and v
    float3 reflection = -normalize(reflect(v, n));

    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotV = abs(dot(n, v)) + 0.001;
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);

    PBRInfo pbrInputs;
    pbrInputs.NdotL = NdotL;
    pbrInputs.NdotV = NdotV;
    pbrInputs.NdotH = NdotH;
    pbrInputs.LdotH = LdotH;
    pbrInputs.VdotH = VdotH;
    pbrInputs.perceptualRoughness = perceptualRoughness;
    pbrInputs.metalness = metallic;
    pbrInputs.reflectance0 = specularEnvironmentR0;
    pbrInputs.reflectance90 = specularEnvironmentR90;
    pbrInputs.alphaRoughness = alphaRoughness;
    pbrInputs.diffuseColor = diffuseColor;
    pbrInputs.specularColor = specularColor;

    // Calculate the shading terms for the microfacet specular shading model
    float3 F = specularReflection(pbrInputs);
    
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    // Calculation of analytical lighting contribution
    float3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    float3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
    float3 color = NdotL * light.colour * (diffuseContrib + specContrib);

    
    // Calculate lighting contribution from image based lighting source (IBL)
#ifdef USE_IBL
    color += getIBLContribution(pbrInputs, n, reflection);
#endif

    // Apply optional PBR terms for additional (optional) shading
#ifdef HAS_OCCLUSIONMAP
    float ao = occlusionTexture.Sample(occlusionSampler, input.texcoord).r;
    color = lerp(color, color * ao, occlusionStrength);
#endif

#ifdef HAS_EMISSIVEMAP
    float3 emissive = SRGBtoLINEAR(emissionTexture.Sample(emissionSampler, input.texcoord)).rgb * emissiveFactor;
    color += emissive;
#endif

    // This section uses lerp to override final color for reference app visualization
    // of various parameters in the lighting equation.
    color = lerp(color, F, scaleFGDSpec.x);
    color = lerp(color, float3(G, G, G), scaleFGDSpec.y);
    color = lerp(color, float3(D, D, D), scaleFGDSpec.z);
    color = lerp(color, specContrib, scaleFGDSpec.w);
    color = lerp(color, diffuseContrib, scaleDiffBaseMR.x);
    color = lerp(color, baseColor.rgb, scaleDiffBaseMR.y);
    color = lerp(color, float3(metallic, metallic, metallic), scaleDiffBaseMR.z);
    color = lerp(color, float3(perceptualRoughness, perceptualRoughness, perceptualRoughness), scaleDiffBaseMR.w);

    return float4(color, 1.0);
}
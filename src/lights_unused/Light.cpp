/*Spectrum MicrofacetReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                        const Point2f &u, Float *pdf,
                                        BxDFType *sampledType) const {
    // Sample microfacet orientation $\wh$ and reflected direction $\wi$
    Vector3f wh = distribution->Sample_wh(wo, u);
    *wi = Reflect(wo, wh);
    if (!SameHemisphere(wo, *wi)) return Spectrum(0.f);

    // Compute PDF of _wi_ for microfacet reflectance
    *pdf = distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));
    return f(wo, *wi);
}

Spectrum MicrofacetReflection::f(const Vector3f &wo, const Vector3f &wi) const {
    Float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
    Vector3f wh = wi + wo;
    // Handle degenerate cases for microfacet reflectance
    if (cosThetaI == 0 || cosThetaO == 0) return Spectrum(0.);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return Spectrum(0.);
    wh = Normalize(wh);
    Spectrum F = fresnel->Evaluate(Dot(wi, wh));
    return R * distribution->D(wh) * distribution->G(wo, wi) * F /
           (4 * cosThetaI * cosThetaO);
}


Float MicrofacetReflection::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    if (!SameHemisphere(wo, wi)) return 0;
    Vector3f wh = Normalize(wo + wi);
    return distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));
}

Float MicrofacetDistribution::Pdf(const Vector3f &wo, const Vector3f &wh) const {
        return D(wh) * AbsCosTheta(wh);
}

Float BeckmannDistribution::D(const Vector3f &wh) const {
    Float tan2Theta = Tan2Theta(wh);
    if (std::isinf(tan2Theta)) return 0.;
    Float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
    return std::exp(-tan2Theta * (Cos2Phi(wh) / (alphax * alphax) +
                                  Sin2Phi(wh) / (alphay * alphay))) /
           (Pi * alphax * alphay * cos4Theta);
}

Vector3f BeckmannDistribution::Sample_wh(const Vector3f &wo, const Point2f &u) const {
    // Sample full distribution of normals for Beckmann distribution

    // Compute $\tan^2 \theta$ and $\phi$ for Beckmann distribution sample
    Float tan2Theta, phi;

    Float logSample = std::log(u[0]);
    if (std::isinf(logSample)) logSample = 0;
    tan2Theta = -alphax * alphax * logSample;
    phi = u[1] * 2 * Pi;

    // Map sampled Beckmann angles to normal direction _wh_
    Float cosTheta = 1 / std::sqrt(1 + tan2Theta);
    Float sinTheta = std::sqrt(std::max((Float) 0, 1 - cosTheta * cosTheta));
    Vector3f wh = SphericalDirection(sinTheta, cosTheta, phi);
    if (!SameHemisphere(wo, wh)) wh = -wh;
    return wh;
}

// ----------------------------- BLENDER --------------
ccl_device float3 bsdf_microfacet_beckmann_eval_reflect(const ShaderClosure *sc, const float3 I, const float3 omega_in, float *pdf)
{
    float m_ab = max(sc->data0, 1e-4f);
    int m_refractive = sc->type == CLOSURE_BSDF_MICROFACET_BECKMANN_REFRACTION_ID;
    float3 N = sc->N;

    if(m_refractive || m_ab <= 1e-4f)
        return make_float3 (0, 0, 0);
    float cosNO = dot(N, I);
    float cosNI = dot(N, omega_in);
    if(cosNO > 0 && cosNI > 0) {
        // get half vector
        float3 Hr = normalize(omega_in + I);
        // eq. 20: (F*G*D)/(4*in*on)
        // eq. 25: first we calculate D(m) with m=Hr:
        float alpha2 = m_ab * m_ab;
        float cosThetaM = dot(N, Hr);
        float cosThetaM2 = cosThetaM * cosThetaM;
        float tanThetaM2 = (1 - cosThetaM2) / cosThetaM2;
        float cosThetaM4 = cosThetaM2 * cosThetaM2;
        float D = expf(-tanThetaM2 / alpha2) / (M_PI_F * alpha2 *  cosThetaM4);
        // eq. 26, 27: now calculate G1(i,m) and G1(o,m)
        float ao = 1 / (m_ab * safe_sqrtf((1 - cosNO * cosNO) / (cosNO * cosNO)));
        float ai = 1 / (m_ab * safe_sqrtf((1 - cosNI * cosNI) / (cosNI * cosNI)));
        float G1o = ao < 1.6f ? (3.535f * ao + 2.181f * ao * ao) / (1 + 2.276f * ao + 2.577f * ao * ao) : 1.0f;
        float G1i = ai < 1.6f ? (3.535f * ai + 2.181f * ai * ai) / (1 + 2.276f * ai + 2.577f * ai * ai) : 1.0f;
        float G = G1o * G1i;
        float out = (G * D) * 0.25f / cosNO;
        // eq. 24
        float pm = D * cosThetaM;
        // convert into pdf of the sampled direction
        // eq. 38 - but see also:
        // eq. 17 in http://www.graphics.cornell.edu/~bjw/wardnotes.pdf
        *pdf = pm * 0.25f / dot(Hr, I);
        return make_float3 (out, out, out);
    }
    return make_float3 (0, 0, 0);
}
*/
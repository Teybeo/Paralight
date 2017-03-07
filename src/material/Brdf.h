#ifndef OPENCL_BRDF_H
#define OPENCL_BRDF_H

#include <iostream>

#include "core/Random.h"
#include "math/Vec3.h"

using std::sqrt;
using std::cout;
using std::endl;

struct alignas(16) CLBrdf {
    Vec3 albedo;                // 0  - 11
    float pad1;                 // 12 - 15
    Vec3 reflection;            // 16 - 27
    float pad2;                 // 28 - 31
    float roughness;            // 32 - 35
    char type;                  // 36
    char albedo_map_index;      // 37
    char roughness_map_index;   // 38
    char reflection_map_index;  // 39
    char normal_map_index;      // 40
    char pad[7];                // 41 - 47
};

#define LAMBERTIAN    0b0001
#define MICROFACET    0b0010
#define MIRROR        0b0100
#define FRESNEL_BLEND 0b1000

#define MATCH_BITFIELD(V, B) ( ((V) & (B)) == (B) )

// Schlick approximation
// In the microfacet model, the normal is the one sampled from distribution (also half_vector), not the geometrical one
// The incoming_dir is the outgoing_dir reflected around the normal so N.I always > 0
// But due to FP rounding, the dot can be slightly negative, and make the result > 1, so clamp it
inline Vec3 Fresnel(Vec3 F0, const Vec3& incoming_dir, const Vec3& normal) {
    float n_dot_i = std::max(0.f, normal.dot(incoming_dir));
    return F0 + (Vec3 {1.f} - F0) * pow((1.f - n_dot_i), 5.f);
}

class Brdf {
public:
    const char type;
    Brdf(char type) : type{type} {}
    virtual ~Brdf() { }

    virtual Vec3 Sample_f(Vec3 outgoing_dir, Vec3& incoming_dir, float& pdf, Vec3 normal) = 0;
};
class Lambertian : public Brdf {

private:
    Vec3 albedo;

public:

    Lambertian(const Vec3& albdeo) : Brdf{LAMBERTIAN}, albedo(albdeo) { }

    Vec3 Sample_f(Vec3 outgoing_dir, Vec3& incoming_dir, float& pdf, Vec3 normal) override {

        // Cosine Hemisphere sampling, PDF = cos(theta) / Pi
        incoming_dir = Random::GetInstance().GetWorldRandomHemishpereDirectionCosine(normal);

        float cos_theta = incoming_dir.dot(normal);

        pdf = cos_theta / M_PI_F;

        return albedo / M_PI_F;
    }

    void setAlbedo(const Vec3& albedo) {
        this->albedo = albedo;
    }
    const Vec3& getAlbedo() const {
        return albedo;
    }
};


// Aka Microfacet
class CookTorrance : public Brdf {

private:
    Vec3 reflectance;
    float roughness;

public:

    CookTorrance(Vec3 reflectance, float roughness) : Brdf{MICROFACET},
                                                     reflectance{reflectance*reflectance},
                                                     roughness{std::max(0.001f, roughness*roughness)}
    { }

    Vec3 Sample_f(Vec3 outgoing_dir, Vec3& incoming_dir, float& pdf, Vec3 normal) override {

        Vec3 micro_normal = Random::GetInstance().BeckmannSample(roughness); // Ok
        micro_normal = micro_normal.ToTangentSpace(normal); // OK

        incoming_dir = micro_normal.reflect(outgoing_dir); // OK

        // Due to the random micronormal, the reflected incoming_dir can go under the normal hemisphere
        // And due to normal mapping, normal != geom_normal, the outgoing dir can be under the normal hemisphere (independently of the first problem)
        // These can causes the denominator and the result to be negative
        float n_dot_i = normal.dot(incoming_dir);
        float n_dot_o = normal.dot(outgoing_dir);

        if (n_dot_i <= 0 || n_dot_o <= 0)
            return 0;

        // As the microfacets are modeled as perfect specular surfaces, H = M
        Vec3 half_vector = micro_normal;
//        Vec3 half_vector = (incoming_dir + outgoing_dir).normalize();
//        half_vector.checkNormal();

        Vec3 fresnel = Fresnel(reflectance, incoming_dir, micro_normal);
        float ndf = Beckmann(normal, micro_normal, roughness);
        float geom = GeometryCookTorrance(normal, outgoing_dir, incoming_dir);
//        float geom = GeometrySmithOrGGX(normal, outgoing_dir, incoming_dir);
        float denominator = 4 * n_dot_o * n_dot_i;

//        pdf = (ndf * normal.dot(half_vector)) / (4 * half_vector.dot(outgoing_dir));
        pdf = (ndf * normal.dot(half_vector)) / (4 * fmaxf(0.001f, half_vector.dot(outgoing_dir)));
        pdf = pdf ? pdf : 1;
//        pdf = 1;

        return (fresnel * geom * ndf) / denominator;
        if (denominator <= 0) {
            std::cout.setf( std::ios::fixed, std:: ios::floatfield ); // floatfield set to fixed
            std::cout.precision(26);
            std::cout << " denominator: " << denominator << std::endl;
            std::cout << " n_dot_i: " << n_dot_i << std::endl;
//            return 0;
        }
    } //-V591
    /*
     * a = angle between N and H
     * exp(-tan(a)² / m²) / ( PI * m² * cos(a)^4)
     */
    float Beckmann(const Vec3& normal, const Vec3& half_vector, float roughness) {
        // FP precision can make this slightly (7th decimal or less) > 1
        float n_dot_h = std::min(1.f, normal.dot(half_vector));

        #ifdef USE_TRIGO_LOOKUP
        float alpha = ACOS_LOOKUP(n_dot_h);
        float tan_a_2 = TAN_LOOKUP(alpha) * TAN_LOOKUP(alpha);
        float cos_a_4 = COS_LOOKUP(alpha) * COS_LOOKUP(alpha) * COS_LOOKUP(alpha) * COS_LOOKUP(alpha);
        #else
        float alpha = acosf(n_dot_h);
        float tan_a_2 = tanf(alpha) * tanf(alpha);
        float cos_a_4 = cosf(alpha) * cosf(alpha) * cosf(alpha) * cosf(alpha);
        #endif

        float roughness_2 = roughness * roughness;

        return expf(-tan_a_2 / roughness_2) / (M_PI_F * roughness_2 * cos_a_4);
    }

    // V-Cavity model
    float GeometryCookTorrance(const Vec3& normal, const Vec3& outgoing_dir, const Vec3& incoming_dir) {

        Vec3 half_vector = (incoming_dir + outgoing_dir).normalize();

        float n_dot_h = normal.dot(half_vector);
        float v_dot_h = outgoing_dir.dot(half_vector);
        float n_dot_v = normal.dot(outgoing_dir);
        float n_dot_l = normal.dot(incoming_dir);

        float res = (2 * n_dot_h) / v_dot_h;
        return fminf(1.f, fminf(res * n_dot_v, res * n_dot_l));
    }
    float GeometrySmithOrGGX(const Vec3& normal, const Vec3& outgoing_dir, const Vec3& incoming_dir) {
        float cosNO = normal.dot(outgoing_dir);
        float cosNI = normal.dot(incoming_dir);
        float ao = 1 / (roughness * sqrt((1 - cosNO * cosNO) / (cosNO * cosNO)));
        float ai = 1 / (roughness * sqrt((1 - cosNI * cosNI) / (cosNI * cosNI)));
        float G1o = ao < 1.6f ? (3.535f * ao + 2.181f * ao * ao) / (1 + 2.276f * ao + 2.577f * ao * ao) : 1.0f;
        float G1i = ai < 1.6f ? (3.535f * ai + 2.181f * ai * ai) / (1 + 2.276f * ai + 2.577f * ai * ai) : 1.0f;
        return G1o * G1i;
    }

    void setReflectance(const Vec3& reflection) {
        CookTorrance::reflectance = reflection * reflection;
    }

    void setRoughness(float roughness) {
        CookTorrance::roughness = std::max(0.001f, roughness * roughness);
    }

    void setRawRoughness(float roughness) {
        CookTorrance::roughness = std::max(0.001f, roughness);
    }

    void setRawReflectance(const Vec3& reflection) {
        CookTorrance::reflectance = reflection;
    }

    const Vec3& getReflection() const {
        return reflectance;
    }

    float getRoughness() const {
        return roughness;
    }

};

class FresnelBlend : public Brdf {

private:

    Vec3 albedo;
    Vec3 reflectance;
    float roughness;

public:
    FresnelBlend(char type) : Brdf(FRESNEL_BLEND) { }

    Vec3 Sample_f(Vec3 outgoing_dir, Vec3& incoming_dir, float& pdf, Vec3 normal) override {
        Vec3 diffuse;
        Vec3 specular;

        return diffuse + specular;
    }

};

class Mirror : public Brdf {

private:
    float reflectance;

public:

    Mirror() : Brdf{MIRROR}, reflectance{0.8f} { }
    Mirror(float reflectance) : Brdf{MIRROR}, reflectance{reflectance} { }

    Vec3 Sample_f(Vec3 outgoing_dir, Vec3& incoming_dir, float& pdf, Vec3 normal) override {

        incoming_dir = normal.reflect(outgoing_dir);

        pdf = 1;
        // Don't apply N.L light attenuation for perfect mirror
        //            if (typeid(*brdf) == typeid(Mirror)) {
        //                cos_factor =  1;
        //            }
        return Vec3(reflectance);
    }

    float getReflectance() const {
        return reflectance;
    }

    void setReflectance(float reflectance) {
        Mirror::reflectance = reflectance;
    }

};


#endif //OPENCL_BRDF_H

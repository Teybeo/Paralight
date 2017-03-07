#ifndef OPENCL_BRDFSTACK_H
#define OPENCL_BRDFSTACK_H

#include "Brdf.h"

class BrdfStack {

    static const int BRDF_MAX_COUNT = 2;

protected:
    Brdf* brdf[BRDF_MAX_COUNT] = {nullptr};

public:

    const int brdf_count = 0;

    BrdfStack() = default;
    BrdfStack(int brdf_count) : brdf_count{brdf_count}
    { };

    virtual ~BrdfStack() {
        for (int i = 0; i < brdf_count; ++i)
            delete brdf[i];
    }

    virtual BrdfStack* copy() = 0;

    virtual Brdf* Sample_Brdf(Vec3 outgoing_dir, Vec3 normal, char brdf_bitfield, Vec3& weight) {

        // [matching_brdf_count] is always <= [brdf_count]
        int matching_brdf_count = MatchingBrdfCount(brdf_bitfield);

        // 0 matching brdf
        if (matching_brdf_count == 0) {
            return nullptr;
        } // Past this, we know there are 1 or more matching brdfs

        // If only 1 brdf is present, we can deduce it's the only one active
        if (brdf_count == 1) {
            weight = 1;
            return brdf[0];
        }
        // Only 1 active brdf out of the 2, so just return it
        if (matching_brdf_count == 1) {
            weight = 1;
            if (brdf[0]->type & brdf_bitfield)
                return brdf[0];
            else
                return brdf[1];
        }

        // Take a random number in [0.f, 1.f[
        // Transform it to [0, match_count[
        // Mult weight by matching count
        float rand = Random::GetInstance().GetUniformRandom();
        int index = int(rand * matching_brdf_count);
        weight = matching_brdf_count;

        // Use it as the brdf index to sample
        return brdf[index];
    };

    virtual char Sample_BrdfType(Vec3 outgoing_dir, Vec3 normal, char brdf_bitfield, Vec3& weight) {

        // [matching_brdf_count] is always <= [brdf_count]
        int matching_brdf_count = MatchingBrdfCount(brdf_bitfield);

        // 0 matching brdf
        if (matching_brdf_count == 0) {
            return 0;
        } // Past this, we know there are 1 or more matching brdfs

        // If only 1 brdf is present, we can deduce it's the only one active
        if (brdf_count == 1) {
            weight = 1;
            return brdf[0]->type;
        }
        // Only 1 active brdf out of the 2, so just return it
        if (matching_brdf_count == 1) {
            weight = 1;
            if (brdf[0]->type & brdf_bitfield)
                return brdf[0]->type;
            else
                return brdf[1]->type;
        }

        // Take a random number in [0.f, 1.f[
        // Transform it to [0, match_count[
        // Mult weight by matching count
        float rand = Random::GetInstance().GetUniformRandom();
        int index = int(rand * matching_brdf_count);
        weight = matching_brdf_count;

        // Use it as the brdf index to sample
        return brdf[index]->type;
    };
    virtual Vec3 Sample_f(Vec3 outgoing_dir, Vec3 normal, Vec3& incoming_dir, float& pdf_out, char brdf_bitfield) {
        Vec3 weight = 0;
        Brdf* sampled_brdf = Sample_Brdf(outgoing_dir, normal, brdf_bitfield, weight);

        if (sampled_brdf == nullptr || weight == 0)
            return 0;

        return weight * sampled_brdf->Sample_f(outgoing_dir, incoming_dir, pdf_out, normal);
    }

    Brdf* const* getBrdfArray() const {
        return brdf;
    }

    inline int MatchingBrdfCount(char brdf_bitfield) const {
        int count = 0;
        for (int i = 0; i < brdf_count; ++i)
            if (brdf[i]->type & brdf_bitfield)
                ++count;
        return count;
    }
};

class StandardStack : public BrdfStack {

public:

    BrdfStack* copy() override {
        BrdfStack* stack = new StandardStack{0, 0, 0};
        return stack;
    }

    StandardStack (Vec3 albedo, Vec3 reflection, float roughness) : BrdfStack{2} {
        brdf[0] = new Lambertian(albedo);
        brdf[1] = new CookTorrance(reflection, roughness);
    }

    Brdf* Sample_Brdf(Vec3 outgoing_dir, Vec3 normal, char brdf_bitfield, Vec3& weight_out) override {

        Brdf* sampled_brdf = BrdfStack::Sample_Brdf(outgoing_dir, normal, brdf_bitfield, weight_out);

        // Light hitting a surface can either be reflected without entering the material ("specular")
        // Or be refracted, bounce inside the material and eventually exit the material ("diffuse")
        // Due to the Fresnel effect, the ratio of reflected/refracted light changes with the light angle
        // The MicroFacet model already includes a Fresnel term but not the Lambertian brdf
        // So we need to weight the Lambertian by (1 - F) in order to preserve energy conservation

        if (sampled_brdf && (sampled_brdf->type == LAMBERTIAN) && MATCH_BITFIELD(brdf_bitfield, LAMBERTIAN | MICROFACET)) {
            Vec3 reflection = ((CookTorrance*)brdf[1])->getReflection();
            float roughness = ((CookTorrance*)brdf[1])->getRoughness();
//            std::cout << "reflection: " << reflection << std::endl;
//            std::cout << "roughness: " << roughness << std::endl;
            weight_out *= Vec3{1.f} - Sample_FresnelBeckmann(reflection, roughness, outgoing_dir, normal);
        }
        return sampled_brdf;
    }

    Vec3 Sample_FresnelBeckmann(Vec3 reflectance, float roughness, const Vec3& outgoing_dir, const Vec3& normal) {

        Vec3 micro_normal = Random::GetInstance().BeckmannSample(roughness);
        micro_normal = micro_normal.ToTangentSpace(normal);
        Vec3 specular_ray = micro_normal.reflect(outgoing_dir);

        Vec3 half_vector = (specular_ray + outgoing_dir).normalize();
        return Fresnel(reflectance, specular_ray, half_vector);
    }

};
// When only one Brdf is needed, the logic can be simplified
//TODO: Bench to see if using this class yields any significant difference
class SingleBrdf : public BrdfStack {

public:

    virtual BrdfStack* copy() override {
        BrdfStack* stack = new SingleBrdf{new Lambertian{Vec3{1, 0, 0}}};
        return stack;
    }

    SingleBrdf(Brdf* _brdf) : BrdfStack(1)
    {
        brdf[0] = _brdf;
    }

    Brdf* Sample_Brdf(Vec3 outgoing_dir, Vec3 normal, char brdf_bitfield, Vec3& weight) override {
        weight = 1;
        return (brdf[0]->type & brdf_bitfield) ? brdf[0] : nullptr;
    };

    Vec3 Sample_f(Vec3 outgoing_dir, Vec3 normal, Vec3& incoming_dir, float& pdf_out, char brdf_bitfield) override {
        if (brdf[0]->type & brdf_bitfield)
            return brdf[0]->Sample_f(outgoing_dir, incoming_dir, pdf_out, normal);
        else
            return 0;
    }

};


#endif //OPENCL_BRDFSTACK_H

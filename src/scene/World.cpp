#include "World.h"
#include "scene/LightShadeVector.h"
#include <stdexcept>
#include <cmath>

// Default World constructor 

World::World() {
    PointLight* ptLight = new PointLight({{-10, 10, -10, 1}, Color(1, 1, 1)});

    Lighting *ling = new Lighting(*ptLight); 

    lighting = ling; 
    
}


// Intersect World Function 
// Iterate over all of the objects that have been added to the world, intersecting them with the ray. 
// Aggregate the intersections into a single collection 
Intersections World::intersect_world(const Ray& ray) {
    Intersections intersectionList; 

    for (auto& s : shapesList) {
        s->intersect(ray, intersectionList); // Call intersect on each shape in the list, and add the intersections to the intersection list
    }

    intersectionList.Sort(); // Sort, then return in ascending order
    return intersectionList; 
}

// Measure the distance from point to the light source by subtracting point from the light position, and take the magnitude of the resulting vector (distance)
// Create a ray from point toward the light source by normalizing the vector from step 1
// Intersect the World with that Ray
// Check to see if there was a hit, and if so wether t is less than distance. If so, the hit lies between the point and the light source, and the point is in shadow. 
bool World::is_shadowed(const vector<double>& pt) {
    vector<double> v = SubtractTuples(lighting->getPos(), pt); 
    double mag = GetMagnitude(v); // Distance
    vector<double> dir = NormalizeTuple(v); 

    Ray ray(pt, dir); 
    Intersections ints; 
    ints = intersect_world(ray); 

   const Intersection* intersection = ints.hit(); 

   if (intersection != nullptr && intersection->getT() < mag) {
        return true; 
   }

   return false; 
}


// Create a new ray originating at the hits location and pointing in the diretion of reflectv. Find the color of the new ray via color_at(). 
// Then multiply the result by the reflective value. If reflective is set to something between 0-1, it will give you partial reflection. 
Color World::reflected_color(const Computations& comps, int remaining) {
    if (remaining <= 0) {
        return Color{0, 0, 0}; // Return black 
    }

    if (comps.object->getMaterial().reflective == 0) {
        return Color{0, 0, 0}; 
    }

    Ray reflect_ray(comps.overPt, comps.reflectv); 
    Color color = Color_at(reflect_ray, remaining - 1); 

    return color * comps.object->getMaterial().reflective; 
}

/*CODE REVIEW; refracted_color() calculates the color contribution from light passing through a transparent object */

// NEW: calculate the color contributed by a ray passing through a transparent object
Color World::refracted_color(const Computations& comps,int remaining) {
    const Material& material = comps.object->getMaterial();

    // stop recursion or skip completely opaque materials
    if (remaining <= 0 || material.transparency <= 0.0) {
        return Color{0, 0, 0};
    }

    // Snell's law.. ratio of the two refractive indices
    const double nRatio = comps.n1 / comps.n2;

    // Angle between the eye vector and surface normal
    const double cosI =
        CalculateDotProd(comps.eyev, comps.normalv);

    // Calculate the squared sine of the transmitted angle
    const double sin2T =
        nRatio * nRatio * (1.0 - cosI * cosI);

    // Total internal reflection means no refracted ray exists
    if (sin2T > 1.0) {
        return Color{0, 0, 0};
    }

    const double cosT = std::sqrt(1.0 - sin2T);

    // implemented to calculate the direction of the refracted ray
    const vector<double> direction =
        comps.normalv * (nRatio * cosI - cosT)
        - comps.eyev * nRatio;

    // underPt begins slightly beneath the surface so the ray
    // does not immediately intersect the same object again
    Ray refractRay(comps.underPt, direction);

    const Color color =
        Color_at(refractRay, remaining - 1);

    return color * material.transparency;
}

//Implementing shading... We check if pt is a shadow or not, then pass it to process lighting
// NEW CODE REVIEW: we are adding the refracted color contribution
Color World::shade_hit(const Computations& comps, int remaining) {
    if (lighting == nullptr) {
        throw std::runtime_error("World has no lighting configured."); // Commented out, as lighting starts of null to get configured 
     }

    LightShadeVector lsv;
    lsv.E = comps.eyev;
    lsv.N = comps.normalv;


   Color surface = lighting->ProcessLighting(
    comps.object,
    comps.object->getMaterial(),
    lsv,
    comps.point, 
    is_shadowed(comps.overPt)
);

    Material mat = comps.object->getMaterial();

    Color emissive = mat.emissiveColor * mat.emissiveStrength;

    Color reflected = reflected_color(comps, remaining); 
    Color refracted = refracted_color(comps, remaining);

    if (mat.reflective > 0 && mat.transparency > 0) {
        double reflectance = schlick(comps); 
        return surface + emissive + reflected * reflectance + refracted * (1 - reflectance);
    }

    // added the refracted 
    return surface + emissive + reflected + refracted;
}





// Find color_at the hit()
Color World::Color_at(const Ray& ray, int remaining) {
    Intersections intersections = intersect_world(ray);

    // Get a pointer to the closest hit
    const Intersection* intersection = intersections.hit(); 

    // If no valid positive intersection exists, return black immediately
    if (intersection == nullptr) {
        return Color{0, 0, 0};
    }

    /*
    // --- CRITICAL SURGICAL DEBUG PRINT ---
    // If intersection->object is an invalid pointer, this print will trigger the crash,
    // telling you exactly where your tracking pipeline broke.
    if (intersection->getObject() == nullptr) {
        std::cout << "[DEBUG CRASH] Critical Error: Intersection exists but object pointer is null!" << std::endl;
    } else {
        std::cout << "[DEBUG HIT] Hit detected on shape address: " << intersection->getObject() << std::endl;
        
        // Let's verify the material and pattern address inside the shape safely
        const Material& mat = intersection-> getObject() ->getMaterial();
        std::cout << "[DEBUG MAT] Material read successfully. Pattern address: " << mat.pattern.get() << std::endl;
        
        if (mat.pattern != nullptr) {
            std::cout << "[DEBUG PATTERN] Executing pattern pipeline..." << std::endl;
        }
    }
        */ 

    
    
    // the complete intersection list is what allows your nested-object code to calculate the correct n1 and n2
    Computations comp = prepareComputations(*intersection, ray, intersections);

    return shade_hit(comp, remaining);
}

// ENGGEN131 (2020) - Lab 9 (5th - 9th October, 2020)
// EXERCISE SIX - Da Vinci Code
//
// tinyray.c - A raytracer in exactly 400 lines of C 
// Author: Matthew Jakeman
//
// Entirely original code, inspired by the following resources:
//  - https://github.com/ssloy/tinyraytracer
//  - https://www.gabrielgambetta.com/computer-graphics-from-scratch/basic-ray-tracing.html
//  - https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <math.h>
#include <sys/time.h>

// A single byte-type representing one channel of a pixel
typedef unsigned char byte;

/* Minimal Floating Point Vector Maths - Author: Matthew Jakeman */
typedef struct {
    float x;
    float y;
    float z;
} Vec3;

Vec3 vec3_new(float x, float y, float z) {
    Vec3 vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    return vec;
}

Vec3 vec3_divide_scalar(Vec3 a, float b) {
    return vec3_new(a.x/b, a.y/b, a.z/b);
}

float vec3_len(Vec3 v) {
    return (float)sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

Vec3 vec3_normalise(Vec3 v) {
    float r = 1/sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    return (Vec3) {v.x*r, v.y*r, v.z*r};
}

// END - vectors

/* Struct Definitions */
typedef Vec3 RgbColour;

typedef struct {
    RgbColour diffuse;
    float specular;
} Material;

typedef struct {
    Vec3 centre;
    float radius;
    Material material;
} Sphere;

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;

typedef enum {
    PointLight,
    DirectionalLight,
    AmbientLight
} LightType;

typedef struct {
    LightType type;
    Vec3 position; // point only
    Vec3 direction; // directional only
    float intensity;
} Light;

/* Globals */
const static Vec3 Zero = {0};
const static Vec3 Invalid = {-1, -1, -1};
const static int FOV = 1; //3.1415/2;
const static int MAX_DIST = 1000;
const static unsigned int WIDTH = 1920;
const static unsigned int HEIGHT = 1920;

/* Constructors */
Sphere sphere_new(Vec3 centre, float radius, Material material) {
    Sphere sphere = {0};
    sphere.centre = centre;
    sphere.radius = radius;
    sphere.material = material;
    return sphere;
}

Light light_point_new(float intensity, Vec3 position) {
    Light light = {0};
    light.type = PointLight;
    light.position = position;
    light.intensity = intensity;
    return light;
}

Light light_ambient_new(float intensity) {
    Light light = {0};
    light.type = AmbientLight;
    light.intensity = intensity;
    return light;
}

Light light_directional_new(float intensity, Vec3 direction) {
    Light light = {0};
    light.type = DirectionalLight;
    light.direction = direction;
    light.intensity = intensity;
    return light;
}

Material material_new(Vec3 diffuse, float specular) {
    Material material = {0};
    material.diffuse = diffuse;
    material.specular = specular;
    return material;
}

Ray ray_new(Vec3 origin, Vec3 direction) {
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    return ray;
}

// Lighting Compute Algorithm
// Adapted from https://www.gabrielgambetta.com/
//
// ARGS:
//  - point = point of intersection
//  - normal = normal vector at point
// RETURNS:
//  - intensity of light
float lighting_compute(Vec3 point, Vec3 normal,
                       Vec3 view, float specular,
                       Light *lights, int num_lights) {
    
    // Intensity of light for the given pixel
    float intensity = 0.0f;

    // Iterate over lights
    for (int i = 0; i < num_lights; i++)
    {
        Light *light = &lights[i];
        if (light->type == AmbientLight)
        {
            // Simply add ambient light to total
            intensity += light->intensity;
        }
        else
        {
            Vec3 light_ray;
            if (light->type == PointLight)
                // Point Light: Direction of ray from light to point
                light_ray = (Vec3) {light->position.x - point.x, light->position.y - point.y, light->position.z - point.z};
            else
                // Directional Light: Direction
                light_ray = light->direction;		

            // Diffuse
            float reflect = normal.x * light_ray.x + normal.y * light_ray.y + normal.z * light_ray.z;
            intensity += (light->intensity * reflect)/(vec3_len(normal) * vec3_len(light_ray));

            // Specular
            if (specular != -1)
            {
                Vec3 r = { normal.x * 2 * reflect - light_ray.x, normal.y * 2 * reflect - light_ray.y, normal.z * 2 * reflect - light_ray.z };
                float reflect_view_proj = r.x*view.x + r.y*view.y + r.z*view.z;
                if (reflect_view_proj > 0)
                {
                    float cosine = reflect_view_proj/(vec3_len(r) * vec3_len(view));
                    intensity += light->intensity * powf(cosine, specular);
                }
            }
        }
    }

    return intensity;
}

// Sphere-Ray Intersection
// Returns 1 if intersection found, otherwise 0
//
// ARGS:
//  - sphere = sphere to intersect
//  - ray = description of ray properties (e.g. direction)
// RETURNS:
//  - boolean of whether ray intersected a sphere
//  - [out] dist0, dist 1 = perpendicular distances to points of intersection
int do_sphere_raycast(Sphere sphere, Ray ray, float *dist0, float *dist1) {
    
    // Please see sphere_ray_intersection.bmp
    *dist0 = 0;
    *dist1 = 0;

    // Find L and tca
    Vec3 L = (Vec3) { sphere.centre.x - ray.origin.x, sphere.centre.y - ray.origin.y, sphere.centre.z - ray.origin.z };
    float tca = L.x * ray.direction.x + L.y * ray.direction.y + L.z * ray.direction.z;

    // Discard if intersection is behind origin
    if (tca < 0)
        return 0;

    // Find d
    float d = sqrtf(L.x*L.x + L.y*L.y + L.z*L.z - tca * tca);
    if (d > sphere.radius)
        return 0;

    // Calculate thc using pythagoras
    float thc = sqrtf(sphere.radius * sphere.radius - d * d);

    // Calculate t0 and t1 (perpendicular distance to
    // the 0th and 1st intersection)
    float t0 = tca - thc;
    float t1 = tca + thc;

    // Ensure at least one of t0 and t1 is greater than zero
    if (t0 < 0 && t1 < 0)
        return 0;

    *dist0 = t0;
    *dist1 = t1;
    
    return 1; // Intersection found
}

// Raytrace Scene at Point
//
// ARGS:
//  - origin = location of camera
//  - dir = direction of projectile
//  - min_t, max_t = min and max clipping planes
//  - spheres, num_spheres = array of spheres to test
//  - lights, num_lights = array of lights in the scene
// RETURNS:
//  - rgb colour of pixel being raytraced
RgbColour raytrace(Vec3 origin, Vec3 dir, float min_t, float max_t,
                   Sphere *spheres, int num_spheres,
                   Light *lights, int num_lights) {

    // Closest sphere to screen (for depth-testing)
    Sphere *closest = 0;
    
    // We use t_comp to store the t-depth of the closest
    // sphere and compare it with other spheres to perform
    // primitive depth testing (where 't' is perpendicular
    // distance to the point of intersection)
    float t_comp = (float)MAX_DIST;

    // Ray to test
    Ray ray = ray_new(origin, dir);

    // Cycle through all spheres and depth-test
    for (int i = 0; i < num_spheres; i++)
    {
        float dist0, dist1;
        if (do_sphere_raycast(spheres[i], ray, &dist0, &dist1))
        {
            // Check dist0
            if ((min_t < dist0 && dist0 < max_t) &&
                dist0 < t_comp)
            {
                t_comp = dist0;
                closest = &spheres[i];
            }

            // Now check dist1
            if ((min_t < dist1 && dist1 < max_t) &&
                dist1 < t_comp)
            {
                t_comp = dist1;
                closest = &spheres[i];
            }
        }
    }

    if (!closest) {
        return Invalid;
    }

    Vec3 point = (Vec3) { origin.x + dir.x * t_comp, origin.y + dir.y * t_comp, origin.z + t_comp * dir.z };
    Vec3 normal = vec3_normalise((Vec3) {point.x - closest->centre.x, point.y - closest->centre.y, point.z - closest->centre.z});
    Material material = closest->material;

    float intensity = lighting_compute(
        point, normal,
        (Vec3) {-1*dir.x, -1*dir.y, -1*dir.z},
        material.specular,
        lights, num_lights
    );

    return (Vec3) { fmin(255, fmax(0, material.diffuse.x * intensity)), fmin(255, fmax(0, material.diffuse.y * intensity)), fmin(255, fmax(0, material.diffuse.z * intensity)) };
}


int main(void)
{
    byte *data = malloc(sizeof(byte) * 3 * WIDTH * HEIGHT);

    // Materials
    Material blue = material_new(vec3_new(69, 161, 255), 500);
    Material white = material_new(vec3_new(240, 240, 240), 180);
    Material red = material_new(vec3_new(255, 0, 57), 10);
    Material ground = material_new(vec3_new(0, 57, 89), 1000);

    // Scene
    #define NUM_SPHERES 4
    Sphere spheres[NUM_SPHERES];
    spheres[0] = sphere_new(vec3_new(-0.75f, -0.2f, 6.5f), 1.5f, red);
    spheres[1] = sphere_new(vec3_new(0, -1, 5), 1.0f, blue);
    spheres[2] = sphere_new(vec3_new(2, -0.5, 8), 3.0f, white);
    spheres[3] = sphere_new(vec3_new(0, -4001, 0), 4000, ground);

    // Lights
    #define NUM_LIGHTS 3
    Light lights[NUM_LIGHTS];
    lights[0] = light_ambient_new(0.2f);
    lights[1] = light_point_new(0.6f, vec3_new(-8, 1, 0));
    lights[2] = light_directional_new(0.2f, vec3_new(1, 4, -8));

    // For non-square images (future-proofing?)
    float aspect_ratio = (float)WIDTH/(float)HEIGHT;
    float screen_dim = tanf(FOV / (float)2);

    Vec3 origin = Zero;

    struct timeval before, after;
    
    gettimeofday(&before, NULL); 

    // Render
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {

            // Background
            data[(y*WIDTH + x) * 3 + 0] = (byte)(y/(float)WIDTH * 255);
            data[(y*WIDTH + x) * 3 + 1] = (byte)(x/(float)HEIGHT * 255);
            data[(y*WIDTH + x) * 3 + 2] = (byte)160;
            
            // Get Pixel in World Coords
            float x_world_coord = (2*(x + 0.5f)/(float)HEIGHT - 1) * screen_dim * aspect_ratio;
            float y_world_coord = -(2*(y + 0.5f)/(float)WIDTH - 1) * screen_dim;

            //Vec3 dir = vec3_normalise((Vec3) { x_world_coord, y_world_coord, 1 });
            float r = 1/sqrtf(x_world_coord*x_world_coord + y_world_coord*y_world_coord + 1);
            Vec3 dir = (Vec3) {x_world_coord*r, y_world_coord*r, r};
            //Vec3 dir = (Vec3) {x_world_coord/len, y_world_coord/len, len};

            // Raytrace Pixel
            RgbColour colour = raytrace(origin, dir, 1.0f, (float)MAX_DIST,
                                        spheres, NUM_SPHERES,
                                        lights, NUM_LIGHTS);

            // Draw Geometry
            if (colour.x != -1) {
                data[(y*WIDTH + x) * 3 + 0] = (byte)colour.x;
                data[(y*WIDTH + x) * 3 + 1] = (byte)colour.y;
                data[(y*WIDTH + x) * 3 + 2] = (byte)colour.z;
            }
        }
    }

    gettimeofday(&after, NULL); 

    float exec_time = ((after.tv_sec + (after.tv_usec / 1000000.0)) -
                            (before.tv_sec + (before.tv_usec / 1000000.0)));

    printf("Total time: %f\n",exec_time);

    // Write to file
    printf("tinyray: writing to file!");
    if (!stbi_write_bmp("output.bmp", WIDTH, HEIGHT, 3, data))
        printf("tinyray: failed to write image!");

    return 0;
}

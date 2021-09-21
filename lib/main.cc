

#include "src/bitmap.h"
#include "src/camera.h"
#include "src/hittable_list.h"
#include "src/hittable.h"
#include "src/material.h"
#include "src/ray.h"
#include "src/rtweekend.h"
#include "src/sphere.h"
#include "src/vec3.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

color ray_color(const ray &r, const hittable &world, int depth)
{
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
    {
        return color(0, 0, 0);
    }

    if (world.hit(r, 0.001, infinity, rec))
    {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list readWorld(string fileName)
{
    ifstream infile(fileName);
    hittable_list world;
    string myText;

    map<string, shared_ptr<material>> materialTypes;

    materialTypes.insert(pair<string, shared_ptr<material>>("material_ground", make_shared<lambertian>(color(0.8, 0.8, 0.0))));
    materialTypes.insert(pair<string, shared_ptr<material>>("material_center", make_shared<lambertian>(color(0.7, 0.3, 0.3))));
    materialTypes.insert(pair<string, shared_ptr<material>>("material_left", make_shared<metal>(color(0.8, 0.8, 0.8))));
    materialTypes.insert(pair<string, shared_ptr<material>>("material_right", make_shared<metal>(color(0.8, 0.6, 0.2))));

    while (getline(infile, myText))
    {
        double arr[4];
        string aux;
        int i = 0;
        stringstream ss(myText);
        while (ss.good() && i < 5)
        {
            if (i < 4)
            {
                ss >> arr[i];
            }
            else
            {
                ss >> aux;
            }
            ++i;
        }

        world.add(make_shared<sphere>(point3(arr[0], arr[1], arr[2]), arr[3], materialTypes.find(aux)->second));
    }

    return world;
}

void render(int image_height, int image_width, int samples_per_pixel, int max_depth, camera cam, hittable_list world, int startColumn, int endColumn)
{
    // Render
    for (int j = image_height - 1; j >= 0; --j)
    {
        for (int i = startColumn; i < endColumn; ++i)
        {
            color pixel_color(0, 0, 0);

            for (int s = 0; s < samples_per_pixel; ++s)
            {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }

            auto r = pixel_color.x();
            auto g = pixel_color.y();
            auto b = pixel_color.z();

            // Divide the color by the number of samples and gamma-correct for gamma=2.0.
            auto scale = 1.0 / samples_per_pixel;
            r = sqrt(scale * r);
            g = sqrt(scale * g);
            b = sqrt(scale * b);

            unsigned char *pos = &img[j * line_size + i * cel_size];

            pos[0] = (unsigned char)(256 * clamp(r, 0.0, 0.999));
            pos[1] = (unsigned char)(256 * clamp(g, 0.0, 0.999));
            pos[2] = (unsigned char)(256 * clamp(b, 0.0, 0.999));
        }
    }
}

int main()
{

    // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 600;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100;
    const int max_depth = 50;

    img = new unsigned char[image_width * image_height * 3 * sizeof(int)];
    cel_size = sizeof(unsigned char) * 3;
    line_size = cel_size * image_width;

    // Read world from file
    hittable_list world = readWorld("input.txt");

    // Camera
    camera cam;

    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::seconds;

    auto t1 = high_resolution_clock::now();

    // Render
    const int threadCount = 8;
    std::thread threadList[threadCount];

    for (int i = 0; i < threadCount; i++)
    {
        if (i == threadCount - 1)
        {
            threadList[i] = std::thread(render, image_height, image_width, samples_per_pixel, max_depth, cam, world, ((image_width / threadCount) * i), image_width);
        }
        else
        {
            threadList[i] = std::thread(render, image_height, image_width, samples_per_pixel, max_depth, cam, world, ((image_width / threadCount) * i), (((image_width / threadCount)) * i) + (image_width / threadCount));
        }
    }

    for (int i = 0; i < threadCount; i++)
    {
        threadList[i].join();
    }

    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<seconds>(t2 - t1);
    std::cout << ms_int.count() << "s\n";

    ofstream outfile;
    outfile.open("image.bmp", ios::binary | ios::out);
    writeBitmapFile(outfile, image_width, image_height);
    outfile.close();
}

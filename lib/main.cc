#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <Windows.h>

using namespace std;

#include "src/bitmap.h"
#include "src/camera.h"
#include "src/hittable_list.h"
#include "src/hittable.h"
#include "src/material.h"
#include "src/moving_sphere.h"
#include "src/ray.h"
#include "src/rtweekend.h"
#include "src/sphere.h"
#include "src/vec3.h"

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

    // diffuse
    auto albedoDiffuse = color::random() * color::random();
    auto material_diffuse = make_shared<lambertian>(albedoDiffuse);

    // metal
    auto albedoMetal = color::random(0.5, 1);
    auto fuzz = random_double(0, 0.5);
    auto material_metal = make_shared<metal>(albedoMetal, fuzz);

    // glass
    auto material_glass = make_shared<dielectric>(1.5);

    materialTypes.insert(pair<string, shared_ptr<material>>("material_diffuse", material_diffuse));
    materialTypes.insert(pair<string, shared_ptr<material>>("material_metal", material_metal));
    materialTypes.insert(pair<string, shared_ptr<material>>("material_glass", material_glass));

    std::cout << "Reading scene file\n";

    /// for each line of file
    while (getline(infile, myText))
    {
        string text;
        string geometry;
        double arr[4];
        string materialType;
        int i = 0;
        stringstream ss(myText);

        /// for each word of line
        while (ss.good() && i < 6)
        {

            /// define geometry type
            if (i == 0)
            {
                ss >> text;

                /// skip comments
                if (text == "#")
                {
                    break;
                }
                else
                {
                    geometry = text;
                }
            }

            /// define geometry size and position
            else if (i > 0 && i < 5)
            {
                ss >> arr[i - 1];
            }

            /// define geometry material
            else
            {
                ss >> materialType;
            }

            ++i;
        }

        /// add geometry to list
        if (geometry == "sphere")
        {
            world.add(make_shared<sphere>(point3(-arr[0], arr[1], arr[2]), arr[3], materialTypes.find(materialType)->second));
        }
    }

    return world;
}

hittable_list random_scene()
{
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -5; a < 5; a++)
    {
        for (int b = -5; b < 5; b++)
        {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9)
            {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0, .5), 0);
                    world.add(make_shared<moving_sphere>(
                        center, center2, 0.0, 1.0, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

void render(int image_height, int image_width, int samples_per_pixel, int max_depth, camera cam, hittable_list world, int startColumn, int endColumn)
{
    int aniCount = 0;

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
            Sleep(5);
        }
    }
}

int main()
{

    // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 720;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100;
    const int max_depth = 5;

    img = new unsigned char[image_width * image_height * 3 * sizeof(int)];
    cel_size = sizeof(unsigned char) * 3;
    line_size = cel_size * image_width;

    // Read world from file
    hittable_list world;
    string readWorldFromFile;
    std::cout << "Read input file? [Y] / [N]: ";
    std::cin >> readWorldFromFile;

    if (readWorldFromFile == "Y" || readWorldFromFile == "y")
    {
        world = readWorld("input.txt");
    }
    else if (readWorldFromFile == "N" || readWorldFromFile == "n")
    {
        world = random_scene();
    }

    // Camera
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Output image
    string imgPreffix = "images/image";
    string imgSuffix = ".bmp";

    // Render
    std::thread *threadList;
    int threads;

    std::cout << "Threads to use: ";
    std::cin >> threads;

    threadList = new std::thread[threads];
    int pixel_per_thread = (image_width / threads);

    double x = 5;
    double y = 1;
    double z = 2;

    double images;
    std::cout << "Images to render: ";
    std::cin >> images;
    double moveSize = (x * 2) / images;

    /// get time before start rendering
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::seconds;
    auto t0 = high_resolution_clock::now();
    auto t1 = t0;
    auto sumRenderTime = duration_cast<seconds>(t1 - t0);

    std::cout << "Rendering " << images << " images with " << threads << " threads\n";
\
    for (int c = 0; c < images; c++)
    {
        camera cam(vec3(x, y, z), lookat, vup, 90, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

        for (int i = 0; i < threads; i++)
        {
            if (i == threads - 1)
            {
                threadList[i] = std::thread(render, image_height, image_width, samples_per_pixel, max_depth, cam, world, pixel_per_thread * i, image_width);
            }
            else
            {
                threadList[i] = std::thread(render, image_height, image_width, samples_per_pixel, max_depth, cam, world, pixel_per_thread * i, (pixel_per_thread * i) + pixel_per_thread);
            }
        }

        for (int i = 0; i < threads; i++)
        {
            threadList[i].join();
        }

        x -= moveSize;

        /// get time after each render
        t1 = high_resolution_clock::now();
        sumRenderTime = duration_cast<seconds>(t1 - t0);

        /// print image number
        std::cout << (c + 1) << "/" << images;
        /// print render time
        std::cout << " - render time: " << sumRenderTime.count() << "s\n";

        ofstream outfile;
        outfile.open(imgPreffix + to_string(c) + imgSuffix, ios::binary | ios::out);
        writeBitmapFile(outfile, image_width, image_height);
        outfile.close();
    }
}

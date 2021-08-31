

#include "src/bitmap.h"
#include "src/camera.h"
#include "src/hittable_list.h"
#include "src/ray.h"
#include "src/rtweekend.h"
#include "src/sphere.h"
#include "src/vec3.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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
        point3 target = rec.p + rec.normal + random_unit_vector();
        return 0.5 * ray_color(ray(rec.p, target - rec.p), world, depth - 1);
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

    while (getline(infile, myText))
    {
        double arr[4];
        int i = 0;
        stringstream ss(myText);
        while (ss.good() && i < 4)
        {
            ss >> arr[i];
            ++i;
        }

        world.add(make_shared<sphere>(point3(arr[0], arr[1], arr[2]), arr[3]));
    }

    return world;
}

int main()
{

    // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
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

    // Render
    for (int j = image_height - 1; j >= 0; --j)
    {
        for (int i = 0; i < image_width; ++i)
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

    ofstream outfile;
    outfile.open("image.bmp", ios::binary | ios::out);
    writeBitmapFile(outfile, image_width, image_height);
    outfile.close();
}

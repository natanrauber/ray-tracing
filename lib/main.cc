

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

color ray_color(const ray &r, const hittable &world)
{
    hit_record rec;
    if (world.hit(r, 0, infinity, rec))
    {
        return 0.5 * (rec.normal + color(1, 1, 1));
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
                pixel_color += ray_color(r, world);
            }

            int ir = static_cast<int>(255.999 * pixel_color.x());
            int ig = static_cast<int>(255.999 * pixel_color.y());
            int ib = static_cast<int>(255.999 * pixel_color.z());

            // Divide the color by the number of samples.
            auto scale = 1.0 / samples_per_pixel;
            ir *= scale;
            ig *= scale;
            ib *= scale;

            unsigned char *pos = &img[j * line_size + i * cel_size];

            pos[0] = (unsigned char)(ir);
            pos[1] = (unsigned char)(ig);
            pos[2] = (unsigned char)(ib);
        }
    }

    ofstream outfile;
    outfile.open("image.bmp", ios::binary | ios::out);
    writeBitmapFile(outfile, image_width, image_height);
    outfile.close();
}

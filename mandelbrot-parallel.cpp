
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <tuple>
#include <time.h>
#include <cmath>
#include <complex>
#include <chrono>

#include "mandelbrot-helpers.hpp"

using namespace std;

/**
 * Static work allocation
 *
 * @param[inout] image
 * @param[in] thread_id
 * @param[in] num_threads
 * @param[inout] pixels_inside
 *
 */
void worker_static(Image &image, int thread_id, int num_threads, int &pixels_inside)
{
    int rows = image.height;
    int cols = image.width;

    std::array<int, 3> pixel = {0, 0, 0};
    complex<double> c;
    int color = (255 * thread_id) / num_threads; // 0 means { 0, 0, 0 } -> black

    // static work distribution (see https://moped.par.univie.ac.at/test.html?taskcode=cpp10)
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            double dx = ((double)col / cols - 0.75) * 2;
            double dy = ((double)row / rows - 0.5) * 2;

            c = complex<double>(dx, dy);

            if (mandelbrot_kernel(c, pixel, color)) // the actual mandelbrot kernel, 0 means black color for mandelbrot pixels
                pixels_inside++;

            // apply to the image
            image[row][col] = pixel;
        }
    }
}

/**
 * Dynamic work allocation
 *
 * @param[inout] image
 * @param[in] thread_id
 * @param[in] num_threads
 * @param[inout] idx
 * @param[inout] pixel_inside
 *
 * @return (pixels_inside could also be used as a return value)
 */
void worker_dynamic(Image &image, int thread_id, int num_threads, int &pixels_inside) // <-- you might need to adjust/extend this!
{
    int rows = image.height;
    int cols = image.width;

    // pixel to be passed to the mandelbrot function
    std::array<int, 3> pixel = {0, 0, 0}; // red, green, blue (each range 0-255)
    complex<double> c;
    int color = (255 * thread_id) / num_threads; // 0 means { 0, 0, 0 } -> black

    // dynamic - put your code here (see https://moped.par.univie.ac.at/test.html?taskcode=cpp11)
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            double dx = ((double)col / cols - 0.75) * 2;
            double dy = ((double)row / rows - 0.5) * 2;

            c = complex<double>(dx, dy);

            if (mandelbrot_kernel(c, pixel, 0)) // the actual mandelbrot kernel, 0 means black color for mandelbrot pixels
                pixels_inside++;

            // apply to the image
            image[row][col] = pixel;
        }
    }
}

int main(int argc, char **argv)
{
    // arguments
    int num_threads = 1;
    std::string work_allocation = "static";
    int print_level = 2; // 0 exec time only, 1 exec time and pixel count, 2 exec time, pixel cound and work allocation

    // height and width of the output image
    int width = 960, height = 720;

    parse_args(argc, argv, num_threads, work_allocation, height, width, print_level);

    double time;
    int pixels_inside = 0;

    // Generate Mandelbrot set in this image
    Image image(height, width);

    auto t1 = chrono::high_resolution_clock::now();

    if (work_allocation.compare("static") == 0)
    {
        // spawn threads and pass parameters
        for (int tid = 0; tid < num_threads; ++tid)
        {
            worker_static(image, tid, num_threads, pixels_inside);
        }
    }

    if (work_allocation.compare("dynamic") == 0)
    {
        // spawn threads and pass parameters
        for (int tid = 0; tid < num_threads; ++tid)
        {
            worker_dynamic(image, tid, num_threads, pixels_inside);
        }
    }

    auto t2 = chrono::high_resolution_clock::now();

    // save image
    image.save_to_ppm("mandelbrot.ppm");

    if (print_level >= 2)
        cout << "Work allocation: " << work_allocation << endl;
    if (print_level >= 1)
        cout << "Total Mandelbrot pixels: " << pixels_inside << endl;
    cout << chrono::duration<double>(t2 - t1).count() << endl;

    return 0;
}
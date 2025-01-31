#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// Structure to hold RGB values
struct Color {
    double r, g, b;
};

// Function to fill the 2D vector with random color values added, based on the level
void fill_with_random_values(vector<vector<Color>>& texture, int level) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0, level);
    for (auto& row : texture) {
        for (auto& val : row) {
            val.r += dis(gen);
            val.g += dis(gen);
            val.b += dis(gen);
        }
    }
}

// Function to apply a horizontal blur
void blur_horizontal(vector<vector<Color>>& texture, vector<vector<Color>>& temp) {
    int height = texture.size();
    int width = texture[0].size();
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            double sum_r = 0.0, sum_g = 0.0, sum_b = 0.0;
            int count = 0;
            for (int dj = -1; dj <= 1; ++dj) {
                int nj = (j + dj + width) % width;
                sum_r += texture[i][nj].r;
                sum_g += texture[i][nj].g;
                sum_b += texture[i][nj].b;
                ++count;
            }
            temp[i][j].r = sum_r / count;
            temp[i][j].g = sum_g / count;
            temp[i][j].b = sum_b / count;
        }
    }
}

// Function to apply a vertical blur
void blur_vertical(vector<vector<Color>>& texture, vector<vector<Color>>& temp) {
    int height = texture.size();
    int width = texture[0].size();
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            double sum_r = 0.0, sum_g = 0.0, sum_b = 0.0;
            int count = 0;
            for (int di = -1; di <= 1; ++di) {
                int ni = (i + di + height) % height;
                sum_r += temp[ni][j].r;
                sum_g += temp[ni][j].g;
                sum_b += temp[ni][j].b;
                ++count;
            }
            texture[i][j].r = sum_r / count;
            texture[i][j].g = sum_g / count;
            texture[i][j].b = sum_b / count;
        }
    }
}

// Function to blur the 2D vector based on the level, ensuring the texture is seamless
void blur_texture(vector<vector<Color>>& texture, int level) {
    vector<vector<Color>> temp = texture;
    blur_horizontal(texture, temp);
    blur_vertical(texture, temp);
}

// Function to save the 2D vector as an image in PPM ASCII colored format
void save_texture_as_ppm(const vector<vector<Color>>& texture, string filename) {
    int height = texture.size();
    int width = texture[0].size();

    // Check if the file already exists and find a unique filename
    if (fs::exists(filename)) {
        int file_index = 1;
        string base_name = filename.substr(0, filename.size() - 4); // Remove the ".ppm"
        while (fs::exists(filename)) {
            filename = base_name + to_string(file_index++) + ".ppm";
        }
    }

    // Open the file in write mode
    ofstream ofs(filename);
    if (!ofs) {
        cerr << "Error: Could not open file " << filename << " for writing." << endl;
        return;
    }

    // Write the PPM header with max color value 65535
    ofs << "P3\n" << width << " " << height << "\n65535\n";

    // Find the min and max values in the 2D vector for each color channel
    double min_r = numeric_limits<double>::max(), max_r = numeric_limits<double>::min();
    double min_g = numeric_limits<double>::max(), max_g = numeric_limits<double>::min();
    double min_b = numeric_limits<double>::max(), max_b = numeric_limits<double>::min();
    for (const auto& row : texture) {
        for (const auto& val : row) {
            if (val.r < min_r) min_r = val.r;
            if (val.r > max_r) max_r = val.r;
            if (val.g < min_g) min_g = val.g;
            if (val.g > max_g) max_g = val.g;
            if (val.b < min_b) min_b = val.b;
            if (val.b > max_b) max_b = val.b;
        }
    }

    // Normalize the values to the range [0, 65535] and write the pixel data
    for (const auto& row : texture) {
        for (const auto& val : row) {
            int pixel_r = static_cast<int>(65535.0 * (val.r - min_r) / (max_r - min_r));
            int pixel_g = static_cast<int>(65535.0 * (val.g - min_g) / (max_g - min_g));
            int pixel_b = static_cast<int>(65535.0 * (val.b - min_b) / (max_b - min_b));
            ofs << pixel_r << " " << pixel_g << " " << pixel_b << " ";
        }
        ofs << "\n";
    }
    ofs.close();
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <width> <height> <file>" << endl;
        return 1;
    }

    int width = stoi(argv[1]);
    int height = stoi(argv[2]);
    string filename = argv[3];
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".ppm") {
        filename += ".ppm";
    }

    clog << "width = " << width << endl;
    clog << "height = " << height << endl;

  //  int level = (height + width) / 2;
    int level = (height + width) / 1;

    vector<vector<Color>> texture(height, vector<Color>(width));
    fill_with_random_values(texture, level);
    while (level > 0) {
        blur_texture(texture, level);
       // level = static_cast<int>(level / 1.5);
        level = static_cast<int>(level / 2);
        fill_with_random_values(texture, level);
        cout << "Texture at level " << level << ":\n";
    }

    save_texture_as_ppm(texture, filename);

    return 0;
}


#include "imageReader.hpp"
#include "iostream"

using namespace std;

int main() {
    Image img = loadImage("images/panda.jpg");
    cout << "Image width: " << img.width << endl;
    cout << "Image height: " << img.height << endl;
    cout << "Image channels: " << img.channels << endl;

    Partition p(img, 1024);
    savePartition(p, "blocks");

    stbi_image_free(img.data);

    return 0;
}

#include "octree.h"
#include "myrandom.h"

#include <iostream>

struct Data
{
    int i;
};

std::ostream& operator<< (std::ostream &out, const octree::Point &p)
{
    return out << p.x << ' ' << p.y << ' ' << p.z;
}

int main()
{
    RandomNumberGenerator rng;
    rng.init(1);
    octree::Octree<Data> ot{octree::CuboidalBoundry{{-10, -10, -10}, {10, 10, 10}}};

    Data d{1};
    ot.insert({-10, 0, -0}, d);

    for (auto &pd: ot.find(octree::Sphere{{0, 0, 0}, 10.0f}))
    {
        std::cout << pd.point << ' ' << pd.data.i << '\n';
    }

    return 0;
}
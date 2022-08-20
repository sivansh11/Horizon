#ifndef OCTREEE_H
#define OCTREEE_H

#include <iostream>
#include <vector>
#include <math.h>
#include <cassert>

namespace octree
{

struct Point
{
    float x, y, z;
    Point() = default;
    Point(float x, float y, float z) : x(x), y(y), z(z) {}
};

template <typename Data>
struct Node
{
    Node *subNodes{};
    std::vector<Data> data;
};

struct CuboidalBoundry
{
    Point topRightFront, bottomLeftBack, center;

    CuboidalBoundry(Point bottomLeftBack, Point topRightFront) : topRightFront(topRightFront), bottomLeftBack(bottomLeftBack) 
    { center = Point{(topRightFront.x + bottomLeftBack.x) / 2.0f, (topRightFront.y + bottomLeftBack.y) / 2.0f, (topRightFront.z + bottomLeftBack.z) / 2.0f}; }
    CuboidalBoundry() = default;

    bool isValid()
    {
        return topRightFront.x >= bottomLeftBack.x &&
               topRightFront.y >= bottomLeftBack.y &&
               topRightFront.z >= bottomLeftBack.z;
    }
    bool isContaining(Point &point)
    {
        if (point.x >= bottomLeftBack.x && point.x <= topRightFront.x &&
            point.y >= bottomLeftBack.y && point.y <= topRightFront.y &&
            point.z >= bottomLeftBack.z && point.z <= topRightFront.z)
            return true;
        return false;
    }

    CuboidalBoundry subTopRightFront()
    {
        return {center, topRightFront};
    }
    CuboidalBoundry subTopRightBack()
    {
        return {Point{center.x, center.y, bottomLeftBack.z}, Point{topRightFront.x, topRightFront.y, center.z}};
    }
    CuboidalBoundry subTopLeftFront()
    {
        return {Point{bottomLeftBack.x, center.y, center.z}, Point{center.x, topRightFront.y, topRightFront.z}};
    }
    CuboidalBoundry subTopLeftBack()
    {
        return {Point{bottomLeftBack.x, center.y, bottomLeftBack.z}, Point{center.x, topRightFront.y, center.z}};
    }
    CuboidalBoundry subBottomRightFront()
    {
        return {Point{center.x, bottomLeftBack.y, center.z}, Point{topRightFront.x, center.y, topRightFront.z}};
    }
    CuboidalBoundry subBottomRightBack()
    {
        return {Point{center.x, bottomLeftBack.y, bottomLeftBack.z}, Point{topRightFront.x, center.y, center.z}};
    }
    CuboidalBoundry subBottomLeftFront()
    {
        return {Point{bottomLeftBack.x, bottomLeftBack.y, center.z}, Point{center.x, center.y, topRightFront.z}};
    }
    CuboidalBoundry subBottomLeftBack()
    {
        return {bottomLeftBack, center};
    }
};


template <typename BoundryType>  // CRTP base
struct Boundry
{
    bool isContaining(Point &point)
    {
        return static_cast<BoundryType>(*this).isContaining(point);
    }
    bool isContainedBy(CuboidalBoundry &cuboidalBoundry)
    {
        return static_cast<BoundryType>(*this).isContainedBy(cuboidalBoundry);
    }
};


struct Cuboid : public Boundry<Cuboid> 
{
    Point topRightFront, bottomLeftBack, center;

    Cuboid(Point bottomLeftBack, Point topRightFront) : topRightFront(topRightFront), bottomLeftBack(bottomLeftBack) 
    { center = Point{(topRightFront.x + bottomLeftBack.x) / 2.0f, (topRightFront.y + bottomLeftBack.y) / 2.0f, (topRightFront.z + bottomLeftBack.z) / 2.0f}; }
    
    bool isContaining(Point &point)
    {
        if (point.x >= bottomLeftBack.x && point.x <= topRightFront.x &&
            point.y >= bottomLeftBack.y && point.y <= topRightFront.y &&
            point.z >= bottomLeftBack.z && point.z <= topRightFront.z)
            return true;
        return false;
    }
    bool isContainedBy(CuboidalBoundry &boundry)
    {
        return (((bottomLeftBack.x >= boundry.topRightFront.x && boundry.bottomLeftBack.x >= topRightFront.x) &&
                 (bottomLeftBack.y >= boundry.topRightFront.y && boundry.bottomLeftBack.y >= topRightFront.y) &&
                 (bottomLeftBack.z >= boundry.topRightFront.z && boundry.bottomLeftBack.z >= topRightFront.z)) ||
                ((bottomLeftBack.x <= boundry.topRightFront.x && topRightFront.x >= boundry.bottomLeftBack.x) &&
                 (bottomLeftBack.y <= boundry.topRightFront.y && topRightFront.y >= boundry.bottomLeftBack.y) &&
                 (bottomLeftBack.z <= boundry.topRightFront.z && topRightFront.z >= boundry.bottomLeftBack.z)));
    }
};

struct Sphere : public Boundry<Sphere>
{
    Point origin;
    float radius;
    Sphere(Point origin, float radius) : origin(origin), radius(radius) {}

    bool isContaining(Point &point)
    {
        float dist2 = pow(origin.x - point.x, 2) + pow(origin.y - point.y, 2) + pow(origin.z - point.z, 2);
        if (dist2 <= radius * radius) 
            return true;
        return false;
    }

    bool isContainedBy(CuboidalBoundry &boundry)
    {
        float r2 = radius * radius;
        float dmin = 0;

        if (origin.x < boundry.bottomLeftBack.x) dmin += sq(origin.x - boundry.bottomLeftBack.x);
        else if (origin.x > boundry.topRightFront.x) dmin += sq(origin.x - boundry.topRightFront.x);
        if (origin.y < boundry.bottomLeftBack.y) dmin += sq(origin.y - boundry.bottomLeftBack.y);
        else if (origin.y > boundry.topRightFront.y) dmin += sq(origin.y - boundry.topRightFront.y);
        if (origin.z < boundry.bottomLeftBack.z) dmin += sq(origin.z - boundry.bottomLeftBack.z);
        else if (origin.z > boundry.topRightFront.z) dmin += sq(origin.z - boundry.topRightFront.z);
        return dmin <= r2;
    }
private:
    float sq(float v)
    {
        return v * v;
    }
};

template <typename Data>
class Octree
{
public:
    struct PointData
    {
        Point point;
        Data data;
    };
    Octree() = default;
    Octree(CuboidalBoundry &boundry, int capacityPerNode = 4) : mBoundry(boundry), mCapacityPerNode(capacityPerNode)
    {
        assert(boundry.isValid());
        subOctrees = nullptr;
        mPointData.reserve(mCapacityPerNode);
        
        // TODO test memory pools
        // mPool = new std::vector<PointData>();
    }
    Octree(CuboidalBoundry &&boundry, int capacityPerNode = 4) : mBoundry(boundry), mCapacityPerNode(capacityPerNode)
    {
        assert(boundry.isValid());
        subOctrees = nullptr;
        mPointData.reserve(mCapacityPerNode);
        
        // TODO test memory pools
        // mPool = new std::vector<PointData>();
    }

    bool insert(Point &p, Data &data)
    {
        if (!mBoundry.isContaining(p)) return false;
        if (mPointData.size() < mCapacityPerNode)
        {
            mPointData.push_back({p, data});
            return true;
        }

        if (subOctrees == nullptr)
        {
            subOctrees = allocateSubTrees();

            subOctrees[0] = Octree<Data>{mBoundry.subTopRightFront(), mCapacityPerNode};
            subOctrees[1] = Octree<Data>{mBoundry.subTopRightBack(), mCapacityPerNode};
            subOctrees[2] = Octree<Data>{mBoundry.subTopLeftFront(), mCapacityPerNode};
            subOctrees[3] = Octree<Data>{mBoundry.subTopLeftBack(), mCapacityPerNode};
            subOctrees[4] = Octree<Data>{mBoundry.subBottomRightFront(), mCapacityPerNode};
            subOctrees[5] = Octree<Data>{mBoundry.subBottomRightBack(), mCapacityPerNode};
            subOctrees[6] = Octree<Data>{mBoundry.subBottomLeftFront(), mCapacityPerNode};
            subOctrees[7] = Octree<Data>{mBoundry.subBottomLeftBack(), mCapacityPerNode};

            // TODO test memory pools
            // subOctrees[0] = Octree<Data>{mBoundry.subTopRightFront(), mCapacityPerNode, mPool};
            // subOctrees[1] = Octree<Data>{mBoundry.subTopRightBack(), mCapacityPerNode, mPool};
            // subOctrees[2] = Octree<Data>{mBoundry.subTopLeftFront(), mCapacityPerNode, mPool};
            // subOctrees[3] = Octree<Data>{mBoundry.subTopLeftBack(), mCapacityPerNode, mPool};
            // subOctrees[4] = Octree<Data>{mBoundry.subBottomRightFront(), mCapacityPerNode, mPool};
            // subOctrees[5] = Octree<Data>{mBoundry.subBottomRightBack(), mCapacityPerNode, mPool};
            // subOctrees[6] = Octree<Data>{mBoundry.subBottomLeftFront(), mCapacityPerNode, mPool};
            // subOctrees[7] = Octree<Data>{mBoundry.subBottomLeftBack(), mCapacityPerNode, mPool};
        }

        // todo: hardcode the checks for faster subtree finding
        for (int i = 0; i < 8; i++)
        {
            if (subOctrees[i].insert(p, data)) return true;
        }
        assert(false);  // flow should never reach here
    }
    bool insert(Point &&p, Data &data)
    {
        return insert(p, data);
    }

    template <typename Boundry>
    std::vector<PointData> find(Boundry &boundry)
    {
        std::vector<PointData> outPointData;
        subTreeFind(boundry, outPointData);
        return outPointData;
    }
    template <typename Boundry>
    std::vector<PointData> find(Boundry &&boundry)
    {
        return find(boundry);
    }

private:
    // TODO test memory pools
    // Octree(CuboidalBoundry &boundry, int capacityPerNode, std::vector<PointData> *memoryPool)
    // {
    //     assert(boundry.isValid());
    //     subOctrees = nullptr;
    //     mPointData.reserve(mCapacityPerNode);
    //     mPool = memoryPool;
    // }

    Octree<Data>* allocateSubTrees()
    {
        return new Octree<Data>[8];
    }

    template <typename Boundry>
    void subTreeFind(Boundry &boundry, std::vector<PointData> &outPointData)
    {
        if (!boundry.isContainedBy(mBoundry)) return;
        if (mPointData.size() > 0)
        {
            for (int i = 0; i < mPointData.size(); i++)
            {
                if (boundry.isContaining(mPointData[i].point))
                {
                    outPointData.push_back(mPointData[i]);
                }
            }
        }
        if (subOctrees != nullptr)
        {
            for (int i = 0; i < 8; i++)
            {
                subOctrees[i].subTreeFind(boundry, outPointData);
            }
        }
    }
    

private:
    int mCapacityPerNode{};
    CuboidalBoundry mBoundry{}; 
    std::vector<PointData> mPointData{};
    Octree<Data> *subOctrees{nullptr};

    // TODO test memory pools
    // std::vector<PointData> *mPool;
};

} // namespace octree


#endif
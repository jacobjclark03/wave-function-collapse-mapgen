// Fill out your copyright notice in the Description page of Project Settings.

// Code from
// https://www.geeksforgeeks.org/convex-hull-algorithm-in-cpp/

#include "ConvexHull.h"

ConvexHull::ConvexHull()
{
}

ConvexHull::~ConvexHull()
{
}

int ConvexHull::Quad(std::pair<int, int> p)
{
    if (p.first >= 0 && p.second >= 0)
        return 1;
    if (p.first <= 0 && p.second >= 0)
        return 2;
    if (p.first <= 0 && p.second <= 0)
        return 3;
    return 4;
}

int ConvexHull::Orientation(std::pair<int, int> a, std::pair<int, int> b, std::pair<int, int> c)
{
    int res = (b.second - a.second) * (c.first - b.first) - (c.second - b.second) * (b.first - a.first);

    if (res == 0)
        return 0;
    if (res > 0)
        return 1;
    return -1;
}

bool ConvexHull::Compare(std::pair<int, int> p1, std::pair<int, int> q1)
{
    std::pair<int, int> p = std::make_pair(p1.first - _mid.first, p1.second - _mid.second);
    std::pair<int, int> q = std::make_pair(q1.first - _mid.first, q1.second - _mid.second);

    int one = Quad(p);
    int two = Quad(q);

    if (one != two)
        return (one < two);
    return (p.second * q.first < q.second * p.first);
}

std::vector<std::pair<int, int>> ConvexHull::Merger(std::vector<std::pair<int, int>> a, std::vector<std::pair<int, int>> b)
{
    // n1 -> number of points in polygon a
        // n2 -> number of points in polygon b
   int n1 = a.size(), n2 = b.size();

    int ia = 0, ib = 0;
    for (int i = 1; i < n1; i++)
        if (a[i].first > a[ia].first)
            ia = i;

    // ib -> leftmost point of b
    for (int i = 1; i < n2; i++)
        if (b[i].first < b[ib].first)
            ib = i;

    // finding the upper tangent
    int inda = ia, indb = ib;
    bool done = 0;
    while (!done)
    {
        done = true;
        while (Orientation(b[indb], a[inda], a[(inda + 1) % n1]) >= 0)
            inda = (inda + 1) % n1;

        while (Orientation(a[inda], b[indb], b[(n2 + indb - 1) % n2]) <= 0)
        {
            indb = (n2 + indb - 1) % n2;
            done = false;
        }
    }

    int uppera = inda, upperb = indb;
    inda = ia, indb = ib;
    done = 0;
    int g = 0;
    while (!done) // finding the lower tangent
    {
        done = true;
        while (Orientation(a[inda], b[indb], b[(indb + 1) % n2]) >= 0)
            indb = (indb + 1) % n2;

        while (Orientation(b[indb], a[inda], a[(n1 + inda - 1) % n1]) <= 0)
        {
            inda = (n1 + inda - 1) % n1;
            done = false;
        }
    }

    int lowera = inda, lowerb = indb;
    std::vector<std::pair<int, int>> ret;
    
    // ret contains the convex hull after merging the two convex hulls
    // with the points sorted in anti-clockwise order
    ret.push_back(a[uppera]);
    int ind = uppera;
    int maxIterations = 1000;  // Maximum allowed iterations to avoid infinite loops
    int iterations = 0;

    while (ind != lowera)
    {
        ind = (ind + 1) % n1;
        ret.push_back(a[ind]);
        iterations++;
    }

    ind = lowerb;
    ret.push_back(b[lowerb]);
    while (ind != upperb)
    {
        ind = (ind + 1) % n2;
        ret.push_back(b[ind]);
    }
    return ret;
}

std::vector<std::pair<int, int>> ConvexHull::BruteHull(std::vector<std::pair<int, int>> a)
{
    // Take any pair of points from the set and check
      // whether it is the edge of the convex hull or not.
      // if all the remaining points are on the same side
      // of the line then the line is the edge of convex
      // hull otherwise not
    std::set<std::pair<int, int>> s;

    for (int i = 0; i < a.size(); i++)
    {
        for (int j = i + 1; j < a.size(); j++)
        {
            int x1 = a[i].first, x2 = a[j].first;
            int y1 = a[i].second, y2 = a[j].second;

            int a1 = y1 - y2;
            int b1 = x2 - x1;
            int c1 = x1 * y2 - y1 * x2;
            int pos = 0, neg = 0;
            for (int k = 0; k < a.size(); k++)
            {
                if (a1 * a[k].first + b1 * a[k].second + c1 <= 0)
                    neg++;
                if (a1 * a[k].first + b1 * a[k].second + c1 >= 0)
                    pos++;
            }
            if (pos == a.size() || neg == a.size())
            {
                s.insert(a[i]);
                s.insert(a[j]);
            }
        }
    }

    std::vector<std::pair<int, int>> ret;
    for (auto e : s)
        ret.push_back(e);

    // Sorting the points in the anti-clockwise order
    _mid = std::make_pair(0, 0);
    int n = ret.size();
    for (int i = 0; i < n; i++)
    {
        _mid.first += ret[i].first;
        _mid.second += ret[i].second;
        ret[i].first *= n;
        ret[i].second *= n;
    }
    std::sort(ret.begin(), ret.end(), [this](const std::pair<int, int>& p1, const std::pair<int, int>& p2)
        {
        return Compare(p1, p2);
        });
    for (int i = 0; i < n; i++)
        ret[i] = std::make_pair(ret[i].first / n, ret[i].second / n);

    return ret;
}

std::vector<std::pair<int, int>> ConvexHull::Divide(std::vector<std::pair<int, int>> a)
{
    // If the number of points is less than 6 then the
      // function uses the brute algorithm to find the
      // convex hull
    if (a.size() <= 5)
        return BruteHull(a);


    std::vector<std::pair<int, int>> cleanedPoints;
    cleanedPoints.push_back(a[0]);  // Add the first point

    for (int i = 1; i < a.size() - 1; i++)
    {
        std::pair<int, int> p = cleanedPoints.back();  // Last point added to the cleaned list
        std::pair<int, int> q = a[i];
        std::pair<int, int> r = a[i + 1];

        // If the points are collinear, skip the current point
        if (Orientation(p, q, r) != 0)
        {
            cleanedPoints.push_back(q);  // Add q if not collinear
        }
    }

    // Add the last point
    cleanedPoints.push_back(a.back());
 
    if (cleanedPoints.size() <= 5)
        return BruteHull(cleanedPoints);

    // left contains the left half points
    // right contains the right half points
    std::vector<std::pair<int, int>> left, right;
    for (int i = 0; i < cleanedPoints.size() / 2; i++)
        left.push_back(cleanedPoints[i]);
    for (int i = cleanedPoints.size() / 2; i < cleanedPoints.size(); i++)
        right.push_back(cleanedPoints[i]);

    // convex hull for the left and right sets
    std::vector<std::pair<int, int>> left_hull = Divide(left);
    std::vector<std::pair<int, int>> right_hull = Divide(right);

    return Merger(left_hull, right_hull);
}


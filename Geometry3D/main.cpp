#include "geometry.h"
#include <vector>
#include <queue>

using namespace geometry;

bool isIntersect(Segment seg, Point top, long double R) {
    if (seg.a.z > seg.b.z)
        std::swap(seg.a.z, seg.b.z);
    if (seg.a.z > top.z)
        return false;
    if (seg.b.z > top.z)
        seg.b = seg.a + (seg.b - seg.a) * (top.z - seg.a.z) / (seg.b.z - seg.a.z);
    Point d = seg.b - seg.a;
    long double r = R * (1.0 - seg.a.z / top.z);
    long double dr = R * d.z / top.z;
    long double a = d.x * d.x + d.y * d.y - dr * dr;
    long double dx = (seg.a.x - top.x);
    long double dy = (seg.a.y - top.y);
    long double b = d.x * dx + d.y * dy + dr * r;
    long double c = dx * dx + dy * dy - r * r;
    if (c <= 0 || a + 2 * b + c <= 0)
        return true;
    long double t = -b / a;
    return 0 < t && t < 1 && a * t * t + 2 * b * t + c <= 0;
}

void bfs(const std::vector< std::vector< size_t > > &graph, std::vector< size_t > &layer, size_t start) {
    layer.assign(graph.size(), -1);
    layer[start] = 0;
    std::queue< size_t > q;
    q.push(start);
    while (!q.empty()) {
        size_t vertex = q.front();
        q.pop();
        for (size_t i = 0; i < graph[vertex].size(); i++) {
            size_t neighbor = graph[vertex][i];
            if (layer[neighbor] == -1) {
                layer[neighbor] = layer[vertex] + 1;
                q.push(neighbor);
            }
        }
    }
}

void cones() {
    size_t n;
    std::cin >> n;
    std::vector< Point > top(n);
    std::vector< long double > radius(n);
    for (size_t i = 0; i < n; i++)
        std::cin >> top[i].x >> top[i].y >> radius[i] >> top[i].z;
    std::vector< std::vector< size_t > > graph(n);
    for (size_t i = 0; i < n - 1; i++)
        for (size_t j = i + 1; j < n; j++) {
            bool hasIntersection = false;
            for (size_t k = 0; k < n && !hasIntersection; k++)
                if (k != i && k != j && isIntersect(Segment(top[i], top[j]), top[k], radius[k]))
                    hasIntersection = true;
            if (!hasIntersection) {
                std::cout << i + 1 << ' ' << j + 1 << std::endl;
                graph[i].push_back(j);
                graph[j].push_back(i);
            }
        }
    std::vector< size_t > layer(n);
    bfs(graph, layer, 0);
    std::cout << layer.back() << std::endl;
}

void hull() {
    size_t n, m = 1;
    //std::cin >> m;
    while (m--) {
        std::cin >> n;
        SetOfPoints setOfPoints;
        setOfPoints.points.resize(n);
        for (size_t i = 0; i < n; i++)
            std::cin >> setOfPoints.points[i];
        std::vector< std::vector< size_t > > planes;
        setOfPoints.convexHull(planes);
        std::vector< std::vector< size_t > > ans(planes.size(), std::vector< size_t > (3));
        std::cout << planes.size() << std::endl;
        for (size_t i = 0; i < planes.size(); i++) {
            //std::cout << 3 << ' ';
            size_t minNumb = 0;
            for (size_t j = 0; j < 3; j++)
                if (planes[i][j] < planes[i][minNumb])
                    minNumb = j;
            for (size_t j = 0; j < 3; j++)
                //std::cout << planes[i][(minNumb + j) % 3] << ' ';
                ans[i][j] = planes[i][(minNumb + j) % 3];
            //std::cout << std::endl;
        }
        sort(ans.begin(), ans.end());
        for (size_t i = 0; i < ans.size(); i++) {
            std::cout << 3;
            for (size_t j = 0; j < 3; j++)
                std::cout << ' ' << ans[i][j];
            std::cout << std::endl;
        }
    }
}

int main() {
    freopen("input.txt", "rt", stdin);
    freopen("output.txt", "wt", stdout);
    hull();
    //cones();
    return 0;
}
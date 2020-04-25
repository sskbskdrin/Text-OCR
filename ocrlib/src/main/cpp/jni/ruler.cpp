//
// Created by sskbskdrin on 2020/4/24.
//

#include "ruler.h"
#include "log.h"
#include <math.h>

#define TAG "ruler--"

namespace ocr {
    Point minY(INT_MAX, INT_MAX);

    struct Rectangle {
        int p1;
        int p2;
        int left;
        int right;
        int parallel;
        double w;
        double h;
    };

    inline int X(int x1, int y1, int x2, int y2) {
      return x1 * y2 - y1 * x2;
    }

/**
 * 两点间的距离
 */
    inline double distance(double x1, double y1, double x2, double y2) {
      double x = x1 - x2;
      double y = y1 - y2;
      return sqrt(x * x + y * y);
    }

/**
 * p点到直线p1，p2的距离，p在顺时针方向时为正数，在逆时针方向时为负数
 */
    inline double distance(Point p, Point p1, Point p2) {
      double a = p2.y - p1.y;
      double b = p1.x - p2.x;
      double c = p2.x * p1.y - p1.x * p2.y;
      double x = p.x;
      double y = p.y;
      return (a * x + b * y + c) / sqrt(a * a + b * b);
    }

/**
 * 计算p点与直线p1、p2的垂直交点（即垂足）坐标
 */
    template<typename T>
    static Point_<T> foot(Point_<T> p, Point_<T> p1, Point_<T> p2) {
      T A = p2.y - p1.y;
      T B = p1.x - p2.x;
      if (A == 0 && B == 0) {
        return Point_<T>(p1.x, p1.y);
      }
      T C = p2.x * p1.y - p1.x * p2.y;
      T x = (B * B * p.x - A * B * p.y - A * C) / (A * A + B * B);
      T y = (-A * B * p.x + A * A * p.y - B * C) / (A * A + B * B);
      return Point_<T>(x, y);
    }

    inline int getPre(int size, int position) {
      return position == 0 ? size - 1 : position - 1;
    }

    inline int getNext(int size, int position) {
      return position == size - 1 ? 0 : position + 1;
    }

    bool cmp(const Point p1, const Point p2) {
      int x1 = p1.x - minY.x;
      int y1 = p1.y - minY.y;
      int x2 = p2.x - minY.x;
      int y2 = p2.y - minY.y;
      int x = X(x1, y1, x2, y2);
      if (x > 0) return false;
      if (x == 0) return distance(x1, y1, minY.x, minY.y) < distance(x2, y2, minY.x, minY.y);
      return true;
    }

    std::vector<Point> convexHull(std::vector<Point> vector) {
      LOGD(TAG, "convexHull size=%ld", vector.size());
      for (Point point : vector) {
        if (minY.y > point.y || (minY.y == point.y && minY.x > point.x)) {
          minY = point;
        }
      }
      LOGD(TAG, "minY %d,%d", minY.x, minY.y);
      std::remove(vector.begin(), vector.end(), minY);
      vector.pop_back();
      // 按minY点顺时针排序
      std::sort(vector.begin(), vector.end(), cmp);

      // 计算凸多边形的点
      //std::vector<Point> list;
      int size = static_cast<int>(vector.size());
      Point *array = (Point *) malloc(size * sizeof(Point));
      int index = 2;
      array[0] = minY;
      array[1] = vector[0];
      array[2] = vector[1];
      //list.push_back(minY);
      //list.push_back(vector[0]);
      //list.push_back(vector[1]);
      for (int i = 2; i < size; i++) {
        Point newP = vector[i];
        Point p1 = array[index - 1];
        //list[list.size() - 2];
        Point p2 = array[index];
        //list[list.size() - 1];
        int x = X(newP.x - p1.x, newP.y - p1.y, p2.x - p1.x, p2.y - p1.y);
        if (x == 0) {
          array[index] = newP;
          //list.pop_back();
          //list.push_back(newP);
        } else if (x > 0) {
          //list.push_back(newP);
          array[++index] = newP;
        } else {
          //list.pop_back();
          index--;
          i--;
        }
      }
      LOGD(TAG, "convexHull end size=%d", index + 1);
      free(array);
      std::vector<Point> list(array, array + index + 1);
      LOGD(TAG, "list size=%ld", list.size());
      return list;
    }

    Rectangle getRect(std::vector<Point> vector, int p, BOOL isLeft) {
      int size = static_cast<int>(vector.size());
      int pp = getPre(size, p);
      if (!isLeft) {
        pp = p;
        p = getNext(size, pp);
      }

      Rectangle rect;
      rect.p1 = p;
      rect.p2 = pp;

      double max = -1;
      for (int i = getNext(size, p); i != pp; i = getNext(size, i)) {
        double dis = distance(vector[i], vector[pp], vector[p]);
        if (dis > max) {
          max = dis;
          rect.parallel = i;
        }
      }
      rect.w = abs(max);

      Point zu = foot(vector[rect.parallel], vector[p], vector[pp]);

      max = INT_MIN;
      double min = INT_MAX;
      for (int i = getNext(size, p); i != pp; i = getNext(size, i)) {
        double dis = distance(vector[i], zu, vector[rect.parallel]);
        if (dis > max) {
          max = dis;
          rect.left = i;
        }
        if (dis < min) {
          min = dis;
          rect.right = i;
        }
      }
      rect.h = max - min;
      return rect;
    }

    inline void check(Rectangle &min, Rectangle &temp) {
      if (min.w * min.h > temp.w * temp.h) {
        min = temp;
      }
    }

    RectD minAreaRect(std::vector<Point> vector) {
      LOGD(TAG, "minAreaRect");
      // 计算出凸多边形
      std::vector<Point> list = convexHull(vector);

      int left = 0, right = 0, top = 0, bottom = 0;
      int size = static_cast<int>(list.size());
      // 找到最边上的四个点
      for (int i = 0; i < size; i++) {
        Point t = list[i];
        if (list[left].x >= t.x) {
          left = i;
        }
        if (list[right].x < t.x) {
          right = i;
        }
        if (list[top].y > t.y) {
          top = i;
        }
        if (list[bottom].y <= t.y) {
          bottom = i;
        }
      }
      // 根据最边上的四个点算出连接这四个点的八条边的矩形，并找到最小的一个
      Rectangle min;
      min.w = INT_MAX;
      min.h = INT_MAX;
      Rectangle temp = getRect(list, left, TRUE);
      check(min, temp);
      temp = getRect(list, left, FALSE);
      check(min, temp);
      temp = getRect(list, bottom, TRUE);
      check(min, temp);
      temp = getRect(list, bottom, FALSE);
      check(min, temp);
      temp = getRect(list, right, TRUE);
      check(min, temp);
      temp = getRect(list, right, FALSE);
      check(min, temp);
      temp = getRect(list, top, TRUE);
      check(min, temp);
      temp = getRect(list, top, FALSE);
      check(min, temp);

      PointD p1 = foot(list[min.left].toPointD(), list[min.p1].toPointD(), list[min.p2].toPointD());
      PointD p2 = foot(list[min.right].toPointD(), list[min.p1].toPointD(), list[min.p2].toPointD());
      PointD p3 = foot(list[min.parallel].toPointD(), p1, list[min.left].toPointD());
      PointD p4 = foot(list[min.parallel].toPointD(), p2, list[min.right].toPointD());

      RectD rect;
      rect.setPoint(p1, p2, p3, p4);
      return rect;
    }
}

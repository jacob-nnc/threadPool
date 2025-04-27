#pragma once
#include <vector>
#include <initializer_list>
#include <string>
#include <cmath>
template <size_t n> struct point {
    double x[n];

    point() {
        for (int i = 0; i < n; i++)
            x[i] = 0;
    }

    point(double* a) {
        for (int i = 0; i < n; i++)
            x[i] = a[i];
    }

    point(std::initializer_list<double> a) {
        int i = 0;
        for (auto it = a.begin(); it != a.end(); it++) {
            x[i] = *it;
            i++;
        }
    }

    double length() {
        double sum = 0;
        for (int i = 0; i < n; i++)
            sum += ((x[i]) * (x[i]));
        return sqrt(sum);
    }

    double sum() {
        double sum = 0;
        for (int i = 0; i < n; i++)
            sum += x[i];
        return sum;
    }


    double length(point<n>& a) { return (*this - a).length(); }

    point<n> normalize() {
        point<n> ret;
        double len = length();
        for (int i = 0; i < n; i++)
            ret.x[i] = x[i] / len;
        return ret;
    }

    void operator=(const point<n>& a) {
        for (int i = 0; i < n; i++)
            x[i] = a.x[i];
    }

    double& operator[](size_t i) {
        return x[i];
    }

    const double& operator[](size_t i)const {
        return x[i];
    }


    std::string to_string() {
        std::string s = "[";
        for (int i = 0; i < n - 1; i++) {
            s += ::to_string(x[i]);
            s += ",";
        }
        s += ::to_string(x[n - 1]);
        s += "]";
        return s;
    }
};

template <size_t n> point<n> operator+(const point<n>& a, const point<n>& b) {
    double c[n];
    for (int i = 0; i < n; i++)
        c[i] = a.x[i] + b.x[i];
    return point<n>(c);
}

template <size_t n> point<n> operator-(const point<n>& a, const point<n>& b) {
    double c[n];
    for (int i = 0; i < n; i++)
        c[i] = a.x[i] - b.x[i];
    return point<n>(c);
}

template <size_t n> bool operator==(const point<n>& a, const point<n>& b) {
    for (int i = 0; i < n; i++)
        if (a.x[i] != b.x[i])
            return false;
    return true;
}

template <size_t n> bool operator!=(const point<n>& a, const point<n>& b) {
    return !(a == b);
}

template <size_t n> point<n> operator*(const point<n>& a, double b) {
    point<n> ret;
    for (int i = 0; i < n; i++)
        ret.x[i] = a.x[i] * b;
    return ret;
}

template <size_t n> double operator*(const point<n>& a, const point<n>& b) {
    double sum = 0;
    for (int i = 0; i < n; i++)
        sum+= a.x[i] * b.x[i];
    return sum;
}

template <size_t n> point<n> operator*(double b, const point<n>& a) {
    point<n> ret;
    for (int i = 0; i < n; i++)
        ret.x[i] = a.x[i] * b;
    return ret;
}

template <size_t n> point<n> operator/(const point<n>& a, double b) {
    point<n> ret;
    for (int i = 0; i < n; i++)
        ret.x[i] = a.x[i] / b;
    return ret;
}



template <size_t n> std::vector<point<n>> cross(std::vector<point<n>> a) {
    int isFree = 0;
    for (int i = 0; i < a.size() - 1; i++) {
        for (int j = i + 1; j < a.size(); j++) {
            if (a[i].x[i + isFree] < a[j].x[i + isFree]) {
                std::swap(a[i], a[j]);
            }
        }

        if (std::abs(a[i].x[i + isFree]) <= 1e-8) {
            isFree += 1;
            i--;
            continue;
        }
        for (int j = i + 1; j < n - 1; j++) {
            a[j] = a[j] - a[i] / a[i].x[i + isFree] * a[j].x[i + isFree];
        }
    }

    std::vector<int> nofreeptr;
    for (int i = 0, j = 0; i < a.size() && j < n;) {
        if (a[i].x[j] <= 1e-8) {
            j++;
        }
        else {
            nofreeptr.push_back(j);
            i++;
        }
    }

    for (int i = 1; i < nofreeptr.size(); i++) {
        for (int j = 0; j < i; j++)
        {
            a[j] = a[j] - a[i] / a[i].x[nofreeptr[i]] * a[j].x[nofreeptr[i]];
        }
    }

    for (int i = 0; i < nofreeptr.size(); i++) {
        a[i] = a[i] / a[i].x[nofreeptr[i]];
    }

    std::vector<int> freeptr;
    int i, j;
    for (i = 0, j = 0; i < n && j < nofreeptr.size();)
    {
        if (i == nofreeptr[j]) {
            j++; i++;
        }
        else {
            freeptr.push_back(i);
            i++;
        }
    }

    for (; i < n; i++)freeptr.push_back(i);

    std::vector<point<n>> ret;

    for (int i = 0; i < freeptr.size(); i++)
    {
        point<n> temp;
        temp.x[freeptr[i]] = 1;
        for (int j = 0; j < nofreeptr.size(); j++)
        {
            temp.x[nofreeptr[j]] = -a[j][freeptr[i]];
        }
        ret.push_back(temp);
    }

    return ret;
}

template<>
std::vector<point<3>> cross<3>(std::vector<point<3>> a) {
    return { {
       a[0][1] * a[1][2] - a[0][2] * a[1][1],
       a[0][2] * a[1][0] - a[0][0] * a[1][2],
       a[0][0] * a[1][1] - a[0][1] * a[1][0]
    } };
}

template <size_t n> point<n> solve(std::vector<point<n + 1>> a) {
    using std::abs;
    if (a.size() != n)
        return point<n>{};

    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (abs(a[i].x[i]) < abs(a[j].x[i])) {
                std::swap(a[i], a[j]);
            }
        }

        if (std::abs(a[i].x[i]) <= 1e-8)
        {
            
            continue;
        }

        for (int j = i + 1; j < n; j++) {
            a[j] = a[j] - a[i] / a[i].x[i] * a[j].x[i];
        }
    }

    for (int i = n - 1; i >= 0; i--) {
        if (std::abs(a[i].x[i]) <= 1e-8)
            continue;
        for (int j = 0; j < i; j++) {
            a[j] = a[j] - a[i] / a[i].x[i] * a[j].x[i];
        }
    }

    point<n> ret;

    for (int i = 0; i < n; i++) {
        if (std::abs(a[i].x[i]) <= 1e-8)
            continue;
        ret.x[i] = a[i].x[n] / a[i].x[i];
    }

    return ret;
};


template <size_t n>  std::vector<point<n>> inv(std::vector<point<n>> a) {
    using std::abs;
    using std::vector;

    if (a.size() != n)
        return {};
    vector<point<n>> ret(n);
    for (int i = 0; i < n; i++)
    {
        ret[i][i] = 1;
    }


    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (abs(a[i].x[i]) < abs(a[j].x[i])) {
                std::swap(a[i], a[j]);
                std::swap(ret[i], ret[j]);
            }
        }

        if (std::abs(a[i].x[i]) <= 1e-8)
        {
            continue;
        }

        for (int j = i + 1; j < n; j++) {
            auto t=a[j].x[i] / a[i].x[i];
            a[j] = a[j] - a[i] * t;
            ret[j] = ret[j] - ret[i] * t;
        }
    }

    for (int i = n - 1; i >= 0; i--) {
        if (std::abs(a[i].x[i]) <= 1e-8)
            continue;
        for (int j = 0; j < i; j++) {
            auto t = a[j].x[i] / a[i].x[i];

            a[j] = a[j] - a[i] * t;
            ret[j] = ret[j] - ret[i] *t;
        }
    }

    for (int i = 0; i < n; i++) {
        ret[i] = ret[i] / a[i][i];
    }

    return ret;
};

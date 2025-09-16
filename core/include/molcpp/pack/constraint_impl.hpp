// Clean, non-templated implementation of value-based constraints

#include "constraint.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <xtensor/core/xmath.hpp>

namespace molcpp::pack {

inline Constraint make_inside_box(const Vec3f& lengths, const Vec3f& origin) {
    Vec3f upper = origin + lengths;
    Constraint c;
    c.name = "inside_box";
    c.penalty = [origin, upper](const Array& pts) -> float {
        if (pts.size() == 0) return 0.0f;
        float p = 0.0f;
        const auto n = pts.shape(0);
        for (size_t i = 0; i < n; ++i) {
            bool outside = false;
            float x = pts(i,0), y = pts(i,1), z = pts(i,2);
            if (x < origin[0] || x > upper[0]) outside = true;
            if (y < origin[1] || y > upper[1]) outside = true;
            if (z < origin[2] || z > upper[2]) outside = true;
            if (outside) p += 1.0f;
        }
        return p;
    };
    c.dpenalty = [origin, upper](const Array& pts) -> Array {
        if (pts.size() == 0) return Array::from_shape({0,3});
        const auto n = pts.shape(0);
        auto g = Array::from_shape({n,3});
        for (size_t i = 0; i < n; ++i) {
            float x = pts(i,0), y = pts(i,1), z = pts(i,2);
            g(i,0) = (x < origin[0]) ? 1.0f : (x > upper[0] ? -1.0f : 0.0f);
            g(i,1) = (y < origin[1]) ? 1.0f : (y > upper[1] ? -1.0f : 0.0f);
            g(i,2) = (z < origin[2]) ? 1.0f : (z > upper[2] ? -1.0f : 0.0f);
        }
        return g;
    };
    c.sampler = [origin, upper](Array& sub, std::mt19937& rng){
        std::uniform_real_distribution<float> dx(origin[0], upper[0]);
        std::uniform_real_distribution<float> dy(origin[1], upper[1]);
        std::uniform_real_distribution<float> dz(origin[2], upper[2]);
        const auto n = sub.shape(0);
        for (size_t i = 0; i < n; ++i) {
            sub(i,0) = dx(rng); sub(i,1) = dy(rng); sub(i,2) = dz(rng);
        }
    };
    return c;
}

inline Constraint make_outside_box(const Vec3f& origin, const Vec3f& lengths) {
    Vec3f upper = origin + lengths;
    Constraint c; c.name = "outside_box";
    c.penalty = [origin, upper](const Array& pts){
        if (pts.size() == 0) return 0.0f;
        float p = 0.0f;
        const auto n = pts.shape(0);
        for (size_t i=0;i<n;++i){
            float x=pts(i,0), y=pts(i,1), z=pts(i,2);
            bool inside = (x>=origin[0] && x<=upper[0] && y>=origin[1] && y<=upper[1] && z>=origin[2] && z<=upper[2]);
            if (inside) p += 1.0f;
        }
        return p;
    };
    c.dpenalty = [origin, upper](const Array& pts){
        if (pts.size()==0) return Array::from_shape({0,3});
        const auto n=pts.shape(0);
        auto g = Array::from_shape({n,3});
        for (size_t i=0;i<n;++i){
            float x=pts(i,0), y=pts(i,1), z=pts(i,2);
            bool inside = (x>=origin[0] && x<=upper[0] && y>=origin[1] && y<=upper[1] && z>=origin[2] && z<=upper[2]);
            if (inside){
                float dx0 = x-origin[0], dx1 = upper[0]-x;
                float dy0 = y-origin[1], dy1 = upper[1]-y;
                float dz0 = z-origin[2], dz1 = upper[2]-z;
                float dx = std::min(dx0, dx1);
                float dy = std::min(dy0, dy1);
                float dz = std::min(dz0, dz1);
                float m = std::min(dx, std::min(dy, dz));
                g(i,0) = (m==dx) ? ((dx0 < dx1)? -1.0f: 1.0f) : 0.0f;
                g(i,1) = (m==dy) ? ((dy0 < dy1)? -1.0f: 1.0f) : 0.0f;
                g(i,2) = (m==dz) ? ((dz0 < dz1)? -1.0f: 1.0f) : 0.0f;
            } else { g(i,0)=g(i,1)=g(i,2)=0.0f; }
        }
        return g;
    };
    return c;
}

inline Constraint make_inside_sphere(float radius, const Vec3f& center){
    Constraint c; c.name = "inside_sphere";
    c.penalty = [radius, center](const Array& pts){
        if (pts.size()==0) return 0.0f;
        float p=0.0f; const auto n=pts.shape(0);
        for (size_t i=0;i<n;++i){
            float dx=pts(i,0)-center[0]; float dy=pts(i,1)-center[1]; float dz=pts(i,2)-center[2];
            float d = std::sqrt(dx*dx+dy*dy+dz*dz);
            if (d>radius) p += 1.0f;
        }
        return p;
    };
    c.dpenalty = [radius, center](const Array& pts){
        if (pts.size()==0) return Array::from_shape({0,3});
        const auto n=pts.shape(0); auto g=Array::from_shape({n,3});
        for (size_t i=0;i<n;++i){
            float dx=pts(i,0)-center[0]; float dy=pts(i,1)-center[1]; float dz=pts(i,2)-center[2];
            float d = std::sqrt(dx*dx+dy*dy+dz*dz);
            if (d>radius && d>0.0f){ g(i,0) = -dx/d; g(i,1)=-dy/d; g(i,2)=-dz/d; }
            else { g(i,0)=g(i,1)=g(i,2)=0.0f; }
        }
        return g;
    };
    c.sampler = [radius, center](Array& sub, std::mt19937& rng){
        std::uniform_real_distribution<float> u(-radius, radius);
        const auto n=sub.shape(0);
        for (size_t i=0;i<n;++i){
            while (true){
                float x=u(rng), y=u(rng), z=u(rng);
                if (x*x+y*y+z*z <= radius*radius){
                    sub(i,0)=center[0]+x; sub(i,1)=center[1]+y; sub(i,2)=center[2]+z; break;
                }
            }
        }
    };
    return c;
}

inline Constraint make_outside_sphere(float radius, const Vec3f& center){
    Constraint c; c.name="outside_sphere";
    c.penalty = [radius, center](const Array& pts){
        if (pts.size()==0) return 0.0f;
        float p=0.0f;
        const auto n=pts.shape(0);
        for (size_t i=0;i<n;++i){
            float dx=pts(i,0)-center[0]; float dy=pts(i,1)-center[1]; float dz=pts(i,2)-center[2];
            float d=std::sqrt(dx*dx+dy*dy+dz*dz); if (d<radius) p += 1.0f;
        } return p;
    };
    c.dpenalty = [radius, center](const Array& pts){
        if (pts.size()==0) return Array::from_shape({0,3});
        const auto n=pts.shape(0);
        auto g=Array::from_shape({n,3});
        for (size_t i=0;i<n;++i){
            float dx=pts(i,0)-center[0]; float dy=pts(i,1)-center[1]; float dz=pts(i,2)-center[2];
            float d=std::sqrt(dx*dx+dy*dy+dz*dz);
            if (d<radius && d>0.0f){ g(i,0)=dx/d; g(i,1)=dy/d; g(i,2)=dz/d; } else { g(i,0)=g(i,1)=g(i,2)=0.0f; }
        } return g;
    };
    return c;
}

inline Constraint make_min_distance(float min_distance){
    Constraint c; c.name="min_distance";
    c.penalty = [min_distance](const Array& pts){
        if (pts.size()==0) return 0.0f;
        float p=0.0f;
        const auto n=pts.shape(0);
        for (size_t i=0;i<n;++i){
            for (size_t j=i+1;j<n;++j){
                float dx=pts(i,0)-pts(j,0); float dy=pts(i,1)-pts(j,1); float dz=pts(i,2)-pts(j,2);
                float d=std::sqrt(dx*dx+dy*dy+dz*dz); if (d<min_distance) p += (min_distance - d);
            }
        } return p;
    };
    c.dpenalty = [min_distance](const Array& pts){
        if (pts.size()==0) return Array::from_shape({0,3});
        const auto n=pts.shape(0);
        auto g=Array::from_shape({n,3});
        for (size_t i=0;i<n;++i){ g(i,0)=g(i,1)=g(i,2)=0.0f; }
        for (size_t i=0;i<n;++i){
            for (size_t j=i+1;j<n;++j){
                float dx=pts(i,0)-pts(j,0); float dy=pts(i,1)-pts(j,1); float dz=pts(i,2)-pts(j,2);
                float d=std::sqrt(dx*dx+dy*dy+dz*dz); if (d<min_distance && d>0.0f){
                    float s = 1.0f/d;
                    g(i,0) += -dx*s; g(i,1)+= -dy*s; g(i,2)+= -dz*s;
                    g(j,0) -= -dx*s; g(j,1)-= -dy*s; g(j,2)-= -dz*s;
                }
            }
        } return g;
    };
    return c;
}

inline Constraint make_inter_molecular_min_distance(float min_distance, std::size_t group_size){
    Constraint c; c.name="inter_mol_min_distance";
    c.penalty = [min_distance, group_size](const Array& pts){
        if (pts.size()==0 || group_size==0) return 0.0f;
        const auto n=pts.shape(0);
        const std::size_t ng = n / group_size; float p=0.0f;
        for (size_t gi=0; gi<ng; ++gi){
            for (size_t gj=gi+1; gj<ng; ++gj){
                for (size_t ai=0; ai<group_size; ++ai){ size_t i=gi*group_size+ai;
                    for (size_t aj=0; aj<group_size; ++aj){ size_t j=gj*group_size+aj;
                        float dx=pts(i,0)-pts(j,0); float dy=pts(i,1)-pts(j,1); float dz=pts(i,2)-pts(j,2);
                        float d=std::sqrt(dx*dx+dy*dy+dz*dz); if (d<min_distance) p += (min_distance - d);
                    }
                }
            }
        } return p;
    };
    c.dpenalty = [min_distance, group_size](const Array& pts){
        if (pts.size()==0 || group_size==0) return Array::from_shape({0,3});
        const auto n=pts.shape(0);
        auto g=Array::from_shape({n,3}); for (size_t i=0;i<n;++i){ g(i,0)=g(i,1)=g(i,2)=0.0f; }
        const std::size_t ng = n / group_size;
        for (size_t gi=0; gi<ng; ++gi){
            for (size_t gj=gi+1; gj<ng; ++gj){
                for (size_t ai=0; ai<group_size; ++ai){ size_t i=gi*group_size+ai;
                    for (size_t aj=0; aj<group_size; ++aj){ size_t j=gj*group_size+aj;
                        float dx=pts(i,0)-pts(j,0); float dy=pts(i,1)-pts(j,1); float dz=pts(i,2)-pts(j,2);
                        float d=std::sqrt(dx*dx+dy*dy+dz*dz); if (d<min_distance && d>0.0f){ float s=1.0f/d;
                            g(i,0)+= -dx*s; g(i,1)+= -dy*s; g(i,2)+= -dz*s;
                            g(j,0)-= -dx*s; g(j,1)-= -dy*s; g(j,2)-= -dz*s;
                        }
                    }
                }
            }
        } return g;
    };
    return c;
}

inline Constraint operator&(const Constraint& a, const Constraint& b){
    Constraint c; c.name = "("+a.name+"&"+b.name+")";
    c.penalty = [a,b](const Array& pts){ return a.penalty(pts) + b.penalty(pts); };
    c.dpenalty = [a,b](const Array& pts){ return a.dpenalty(pts) + b.dpenalty(pts); };
    c.sampler = [a,b](Array& sub, std::mt19937& rng){ if (a.sampler) a.sampler(sub, rng); if (b.sampler) b.sampler(sub, rng); };
    return c;
}

inline Constraint operator|(const Constraint& a, const Constraint& b){
    Constraint c; c.name = "("+a.name+"|"+b.name+")";
    c.penalty = [a,b](const Array& pts){ float pa=a.penalty(pts), pb=b.penalty(pts); return std::min(pa,pb); };
    c.dpenalty = [a,b](const Array& pts){ float pa=a.penalty(pts), pb=b.penalty(pts); return (pa<pb)? a.dpenalty(pts) : b.dpenalty(pts); };
    c.sampler = [a,b](Array& sub, std::mt19937& rng){ if (a.sampler) a.sampler(sub, rng); else if (b.sampler) b.sampler(sub, rng); };
    return c;
}

} // namespace molcpp::pack



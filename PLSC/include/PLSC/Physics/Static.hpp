#pragma once

#include "Collider.hpp"

#include <memory>
#include <vector>

namespace PLSC::Static
{
    class Definition
    {
    public:
        std::vector<std::shared_ptr<Collider::ICollider>> m_interfaces;
        template <typename T>
        std::vector<std::shared_ptr<T>> Register(T x)
        {
            AssertICollider<T>();

            auto sp = std::make_shared<T>(x);
            auto ip = std::static_pointer_cast<Collider::ICollider>(sp);
            m_interfaces.push_back(ip);
            return {sp};
        }

        template <typename T>
        std::vector<std::shared_ptr<T>> Register(T (*f)())
        {
            AssertICollider<T>();
            return Register(f());
        }

        template <typename T, typename Integral>
        std::vector<std::shared_ptr<T>> Register(T (*f)(Integral), Integral i)
        {
            AssertICollider<T>();

            std::vector<std::shared_ptr<T>> ret;
            for (Integral j = 0; j < i; ++j) { ret.push_back(_spRegister(f(j))); }
            return ret;
        }

    private:
        template <typename T>
        inline static constexpr void AssertICollider()
        {
            static_assert(std::is_base_of<Collider::ICollider, T>::value,
                          "Static::Definition::Register(T): T must derive Collider::ICollider");
        }

        template <typename T>
        std::shared_ptr<T> _spRegister(T x)
        {
            auto sp = std::make_shared<T>(x);
            auto ip = std::static_pointer_cast<Collider::ICollider>(sp);
            m_interfaces.push_back(ip);
            return sp;
        }
    };
} // namespace PLSC::Static
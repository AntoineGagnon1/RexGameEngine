#pragma once

#include <type_traits>

namespace RexEngine
{
    // Use to prevent the destructor of a class from being called
    // Ex : static NoDestroy<Shader> happyOpengl; <- Opengl wont crash because the shader is destroyed after the context
    template<class T>
    class NoDestroy
    { // From : https://stackoverflow.com/questions/27671498/c-disable-destructors-for-static-variables
    public:
        using pointer = typename std::add_pointer_t<T>;
        using pointerConst = typename std::add_pointer_t<std::add_const_t<T>>;

        template<class... Args>
        NoDestroy(Args && ...args)
        {
            new (m_data) T(std::forward<Args>(args)...);
        }

        pointer operator->()
        {
            return reinterpret_cast<pointer>(m_data);
        }

        T& operator*()
        {
            return *(reinterpret_cast<pointer>(m_data));
        }

        const T& operator*() const
        {
            return *(reinterpret_cast<pointerConst>(m_data));
        }

    private:
        alignas(T) int8_t m_data[sizeof(T)];
    };
}
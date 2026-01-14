#pragma once
#include <functional>
#include <vector>

namespace fanta {

/**
 * fanta::Observable<T>
 * A lightweight reactive wrapper for values.
 */
template <typename T>
class Observable {
public:
    Observable(T value) : m_value(value) {}

    const T& get() const { return m_value; }

    void set(const T& value) {
        if (m_value != value) {
            m_value = value;
            notify();
        }
    }

    void operator=(const T& value) { set(value); }
    operator const T&() const { return m_value; }

    // Manual notification (for complex types)
    void notify() {
        for (auto& cb : m_callbacks) {
            if (cb) cb(m_value);
        }
    }

    void onChange(std::function<void(const T&)> cb) {
        m_callbacks.push_back(cb);
    }

private:
    T m_value;
    std::vector<std::function<void(const T&)>> m_callbacks;
};

} // namespace fanta

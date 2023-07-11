#pragma once
#define PLSC_PROFILE 1

// Static internal profiling without branches
#ifdef PLSC_PROFILE
    #include "PLSC/Typedefs.hpp"

    #include <chrono>
    #include <vector>

namespace PLSC::DBG::PROFILE
{
    using namespace std::chrono;

    class profile_data
    {
    public:
        mutable u64_t      t_ns       = 0;
        mutable u64_t      hits       = 0;
        mutable u64_t      complexity = 0;
        const char * const m_name;
        explicit profile_data(const char * const);
    };

    class time_recorder_t
    {
    private:
        profile_data * const                    m_pData;
        const u32_t                             m_complexity;
        const high_resolution_clock::time_point m_t;

    public:
        explicit time_recorder_t(profile_data * const);
        time_recorder_t(profile_data * const, const u32_t);
        ~time_recorder_t();
    };

    void output();

    class profile_t
    {
    public:
        mutable std::vector<profile_data *> m_pointers;
        void                                reg(profile_data * const pdata);
    };

    static profile_t g_profile;
} // namespace PLSC::DBG::PROFILE

    #define PROFILE_MACRO_PASTE(id, name)                                                                    \
        static PLSC::DBG::PROFILE::profile_data   _PROFILE_DATA##id(name);                                   \
        const PLSC::DBG::PROFILE::time_recorder_t _PROFILE_RECORDER##id                                      \
            = PLSC::DBG::PROFILE::time_recorder_t(&_PROFILE_DATA##id);
    #define PROFILE_MACRO_EVAL(id, name) PROFILE_MACRO_PASTE(id, name)

    #define PROFILE()           PROFILE_MACRO_EVAL(__COUNTER__, __FUNCTION__)
    #define PROFILE_NAMED(name) PROFILE_MACRO_EVAL(__COUNTER__, name)

    #define PROFILE_MACRO_PASTE_COMPLEXITY(id, name, complexity)                                             \
        static PLSC::DBG::PROFILE::profile_data   _PROFILE_DATA##id(name);                                   \
        const PLSC::DBG::PROFILE::time_recorder_t _PROFILE_RECORDER##id                                      \
            = PLSC::DBG::PROFILE::time_recorder_t(&_PROFILE_DATA##id, static_cast<size_t>(complexity));
    #define PROFILE_MACRO_EVAL_COMPLEXITY(id, name, complexity)                                              \
        PROFILE_MACRO_PASTE_COMPLEXITY(id, name, complexity)

    #define PROFILE_COMPLEXITY_NAMED(name, complexity)                                                       \
        PROFILE_MACRO_EVAL_COMPLEXITY(__COUNTER__, name, complexity)
    #define PROFILE_COMPLEXITY(complexity)                                                                   \
        PROFILE_MACRO_EVAL_COMPLEXITY(__COUNTER__, __FUNCTION__, complexity)
    #define PROFILE_OUTPUT() PLSC::DBG::PROFILE::output();

#else
    #define PROFILE()
    #define PROFILE_NAMED(x)
    #define PROFILE_OUTPUT()
    #define PROFILE_COMPLEXITY(x)
    #define PROFILE_COMPLEXITY_NAMED(x, y)
#endif
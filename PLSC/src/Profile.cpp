#include "PLSC/DBG/Profile.hpp"

#include <iostream>

#ifdef PLSC_PROFILE

namespace PLSC::DBG::PROFILE
{
    using namespace std;
    using namespace chrono;

    //-- profile_data
    profile_data::profile_data(const char * const name) : m_name(name) { g_profile.reg(this); }

    //-- time_recorder_t
    time_recorder_t::time_recorder_t(profile_data * const pData) :
        m_pData(pData), m_complexity(1), m_t(high_resolution_clock::now())
    {
    }

    time_recorder_t::time_recorder_t(profile_data * const pData, const u32_t complexity) :
        m_pData(pData), m_complexity(complexity), m_t(high_resolution_clock::now())
    {
    }

    time_recorder_t::~time_recorder_t()
    {
        m_pData->t_ns += duration_cast<nanoseconds>(high_resolution_clock::now() - m_t).count();
        m_pData->hits++;
        m_pData->complexity += m_complexity;
    }

    //-- profile_t
    inline void profile_t::reg(profile_data * const pData)
    {
        std::cout << "PROFILE REGISTER: " << pData->m_name << std::endl;
        m_pointers.push_back(pData);
    }

    //-- output()
    using namespace std;
    enum output_names
    {
        NAME,
        HITS,
        AVG_COMPLEXITY,
        TOTAL_MS,
        NS_PER_COMPLEXITY,
        length
    };
    static const string suffixes[output_names::length] = {" [", " i,", " c/i]", " ms/i", " ns/c"};
    static const string prefixes[output_names::length] = {"", "", "", "", ""};
    struct output_fmt_t
    {
        string fields[output_names::length];
        output_fmt_t(const char * name, profile_data data)
        {
            u64_t  t_ns                             = data.t_ns;
            u64_t  hits                             = data.hits;
            u64_t  complexity                       = data.complexity;
            u64_t  avg_complexity                   = complexity / hits;
            u64_t  ns_per_complexity                = t_ns / complexity;
            double t_ms                             = (double) t_ns / (double) 1e6;
            fields[output_names::NAME]              = string(name);
            fields[output_names::HITS]              = to_string(data.hits);
            fields[output_names::AVG_COMPLEXITY]    = to_string(avg_complexity);
            fields[output_names::TOTAL_MS]          = to_string(t_ms / (double) hits);
            fields[output_names::NS_PER_COMPLEXITY] = to_string(ns_per_complexity);
            for (size_t i(0); i < output_names::length; ++i)
            {
                fields[i] = prefixes[i] + fields[i] + suffixes[i];
            }
        }
    };
    void output()
    {
        /* Construct format objects */
        vector<output_fmt_t> outputs;
        for (profile_data * pdata : g_profile.m_pointers) { outputs.emplace_back(pdata->m_name, *pdata); }

        /* Work out paddings */
        u32 longest_fields[output_names::length] = {0};
        for (auto o : outputs)
        {
            for (size_t i(0); i < output_names::length; ++i)
            {
                if (o.fields[i].length() > longest_fields[i]) { longest_fields[i] = o.fields[i].length(); }
            }
        }

        /* Print objects w/ padding */
        for (auto o : outputs)
        {
            for (size_t i(0); i < output_names::length; ++i)
            {
                string field_padding = "";
                u32    field_length  = longest_fields[i] - o.fields[i].length() + 3;
                for (size_t j(0); j < field_length; ++j) { field_padding += ' '; }

                cout << field_padding << o.fields[i];
            }
            cout << endl;
        }
    }
} // namespace PLSC::DBG::PROFILE
#endif
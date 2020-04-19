#pragma once

#include <cstdint>

#include <typeinfo>
#include <typeindex>
#include <vector>
#include <array>
#include <optional>
#include <unordered_map>
#include <iostream>
#include <functional>

#ifdef HOT_RELOAD
    #include "HotReload/module_loader.h"
#endif

using Group = uint16_t;
using Instance = uint16_t;

static constexpr inline Group group_none = -1;
static constexpr inline Instance instance_none = -1;

struct Stage
{
    Group group = group_none;
    Instance instance = instance_none;
    friend std::ostream& operator<<(std::ostream& output, const Stage& stage) {
        // TODO magic_enum
        return output << "{" << stage.group << ", " << stage.instance << "}";
    }
};


using GroupSet = std::vector<Stage>;

/*
 * Pointer to a function in discrete time that acts on prop tuples
 * A SunLambda should be defined in a file named based on the Prop tuple types it accesses sorted alphabetically
 * This process of naming files based on props accessed should ultimately be (optionally, default opt-in) technically enforced
 * Refactoring of SunLambda file path and includes from a change in a SunLambda implied signature should be automated and blindingly fast
 * For example, changing props accessed should automatically move the SunLambda struct declaration to a new file live while programming
 * without interrupting flow
 * 
 * Jolts are SunLambdas registered with a partial lifetime function called when adding a prop that creates a novel tuple
 */
struct SunLambda // λ
{
    using Id = std::size_t;
    using Caller = void (*)(const SunLambda&);
    using JoltCaller = void(*)(const SunLambda&, std::vector<size_t>);
    using Functor = void*;

    Id id;
    Caller caller = nullptr;
    JoltCaller jolt; // PropIdRaw
    Functor functor = nullptr;
    const char* name = nullptr;

    void operator() (const Stage& stage = {}) const
    {
        Act(stage);
    }

    void Act(const Stage& stage = {}) const
    {
        (*caller)(*this);
    }
    
    void Emerge(std::vector<size_t> sunData) const
    {
//         std::cout << "Emerging with " << sunData.size() << " sun data" << std::endl;
        (*jolt)(*this, sunData);
    }
};

class SunLambdaRegistry
{
public:

    void Register(const SunLambda& lambda)
    {
        sunLambdas[lambda.id] = lambda;
    }

    template<typename T>
    void Register()
    {
        Register(T{});
    }

    SunLambda& Get(SunLambda::Id id)
    {
        return sunLambdas[id];
    }

    static SunLambdaRegistry& GetInstance()
    {
        static SunLambdaRegistry registry;
        return registry;
    }

#ifdef HOT_RELOAD
    void Unload()
    {
        if(module.IsValid())
        {
            module.Unload();
        }
    }

    void Reload()
    {
        Unload();

        module = Module::Load(HOT_RELOAD_LIB);

        for(auto& [id, lambda] : sunLambdas)
        {
            lambda.functor = module.GetFunction(lambda.name + std::string("_Act"));
        }
    }
#endif

private: 
    SunLambdaRegistry() = default;
    std::unordered_map<SunLambda::Id, SunLambda> sunLambdas;

#ifdef HOT_RELOAD
    Module module;
#endif
};

#ifndef HOT_RELOAD
    #define SUN_EXPORT
#else
    #ifdef _WIN32
        #define SUN_EXPORT extern "C" __declspec( dllexport )
    #else
        #define SUN_EXPORT extern "C"
    #endif
#endif

#define DeclareSunLambda(LAMBDA_NAME, ...)      \
\
SUN_EXPORT void LAMBDA_NAME ## _Act(__VA_ARGS__);      \
\
inline void LAMBDA_NAME ## _Caller(const SunLambda& lambda) \
{\
   BicycleMango::IterateProps(reinterpret_cast<void (*)(__VA_ARGS__)>(lambda.functor), lambda.id);\
}      \
\
inline void LAMBDA_NAME ## _TypesetCaller(const SunLambda& lambda, std::vector<size_t> sunData)\
{\
    BicycleMango::CallJolt<__VA_ARGS__>(reinterpret_cast<void (*)(__VA_ARGS__)>(lambda.functor), lambda.id, sunData);\
}\
struct LAMBDA_NAME : SunLambda               \
{                                             \
    LAMBDA_NAME()                              \
    {                                           \
        caller = &LAMBDA_NAME ## _Caller;          \
        jolt = &LAMBDA_NAME ## _TypesetCaller;\
        functor = reinterpret_cast<void*>(&LAMBDA_NAME ## _Act);          \
        name = #LAMBDA_NAME;                      \
        SunLambda::id = LAMBDA_NAME::Id();         \
        BicycleMango::ConsiderTypeset<__VA_ARGS__>(Id());\
    };                                              \
    static Id Id(){ return std::type_index(typeid(LAMBDA_NAME)).hash_code(); } \
    static const inline int ForceInit = (SunLambdaRegistry::GetInstance().Register<LAMBDA_NAME>(), 0); \
};

/*
 * A structure to facilitate computed imagining
 * Prop constraints:
 *  Should not contain Props as members
 *  Should not define functions which affect non-local data
 * 
 * Props are considered to be part of the same tuple if 
 * they share at least one stage (or consider adding a tuple check lambda when creating a sunlambda)
 */
struct Prop
{
    size_t id;
};

// User defined engine config
enum LOOP_TIMES : uint8_t
{
    FORAGE,
    NETWORK_RECEIVE,
    INPUT,
    UPDATE,
    EVENTS,
    NETWORK_SEND,
    ANIMATION,
    FX,
    RENDER,
    DISPLAY, 
};

#include "groups.h"

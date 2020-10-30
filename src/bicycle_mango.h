#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <SFML/System/Clock.hpp>

#include "specificity.h"
#include "Utils/hole_vector.h"
#include "sun_lambda.h"

using byte = uint8_t;
#define SpecificityDepth 4
using ScheduleSpecificity = Specificity<SpecificityDepth>;

/*
 * Bicycle Mango
 * A hopeful gameplay framework
 * by Wesxdz and lapinozz
 * fast and free for (0b11111111 : while(true))
 *
 *
 *      ***               o000P
 *          \               |.
 *           |_____________/_----____/
 *           // \             \\
 *          //   \             \\       ___
 *     00000/     \             000011 /  v \
 *   00\-|--00     \          00\-|--11  v  /
 *  (0---O---0)     ( λ )----(0--xOx--0)   V
 *   00/-|--00        |-------00/-|--00
 *     00000         ^^^        00000
 *
 *
 * The role of a game engine is to empower the negotiation of imagination.
 * Creating a game should be fun.
 *
 */
class BicycleMango
{
public:
    // The time between the beginning and end of the last loop
    static inline sf::Time delta;
    // Should the Bicycle Mango gameplay loop stop?
    static inline bool brake;

    
    struct SunSchedule
    {
        SunLambda::Id id;
        ScheduleSpecificity specificity;
    };
    static inline std::vector<SunSchedule> schedules;

    static void Loop()
    {
        sf::Clock clock;
        auto startTime = clock.getElapsedTime();
        for (const SunSchedule& schedule : schedules)
        {
            SunLambdaRegistry::GetInstance().Get(schedule.id)();
        }

#ifdef HOT_RELOAD
        if(ShouldReloadLambdas)
        {
            ReloadLambdas();
            ShouldReloadLambdas = false;
        }
#endif

        delta = clock.getElapsedTime() - startTime;
    }

#ifdef HOT_RELOAD
    static inline bool ShouldReloadLambdas = false;
    static void ReloadLambdas()
    {
        SunLambdaRegistry::GetInstance().Unload();

        system(HOT_RELOAD_CMAKE " --build " HOT_RELOAD_BUILD_PATH " --target " HOT_RELOAD_TARGET);

        SunLambdaRegistry::GetInstance().Reload();
    }
#endif

    using PropTypeId = size_t;
    using PropIdRaw = size_t;
    // ALL STAGES OF A PROP MUST RETURN TRUE ON THE COMPATIBILITY CONSTRAINT FOR THAT PROP TO BE CONSIDERED TO FORM PART OF A NOVEL TUPLE OF THE SunLambda::Id WHICH HAS THIS CompatibleConstraint AS PART OF ITS NovelTupleCreator
    // Can this be considered to be part of a novel tuple?
    using CompatibleConstraint = std::function<bool(PropTypeId, const Stage&)>;
    // Can this prop be reused in novel tuples
    using PartialStaticIndicators = std::unordered_map<PropTypeId, std::function<bool(std::set<Stage>&)>>;
    
    struct NovelTupleCreator
    {
        CompatibleConstraint compatible;
        PartialStaticIndicators reuseOnStages;
    };

    static inline std::set<SunLambda::Id> emerges;
    static void Emerge(SunLambda::Id id, CompatibleConstraint compatible = All)
    {
        emerges.insert(id);
        novelTupleCreators[id].compatible = compatible;
    }


    static inline std::unordered_map<void*, PropIdRaw> pidq; // Prop id query

    // Shared between emerge/plan/breakup: we assume a SunLambda cannot have multiple NovelTupleCreators for now
    static inline std::unordered_map<SunLambda::Id, NovelTupleCreator> novelTupleCreators;
    // This data structure is an augment of potential neighbors when searching for props to form novel tuples
    static inline std::unordered_map<SunLambda::Id, std::vector<std::vector<PropIdRaw>>> partialStatics;
    
    static inline CompatibleConstraint All = [](PropTypeId, const Stage&){return true;};
    static void Plan(SunLambda::Id id, const ScheduleSpecificity& specificity, CompatibleConstraint compatible = All)
    {
        SunSchedule schedule{id, specificity};

        auto it = std::upper_bound(schedules.begin(), schedules.end(), schedule,
        [](const SunSchedule& a, const SunSchedule& b) -> bool {
            return a.specificity < b.specificity;
        });

        schedules.insert(it, schedule);
        novelTupleCreators[id].compatible = compatible;
    }
    
    static inline std::set<SunLambda::Id> breakups;
    static void Breakup(SunLambda::Id id, CompatibleConstraint compatible = All)
    {
        breakups.insert(id);
        novelTupleCreators[id].compatible = compatible;
    }

    template<typename T>
    struct PropId { PropIdRaw id; };

    // I decided to place PropTypeId before Group this data structure because it is more commonly used this way in the code, even though the console works by query by group
    // TODO Will we need this later for faster console querying??
//     using GroupTypeInstance = std::unordered_map<PropTypeId, std::unordered_map<Group, std::set<std::pair<Instance, PropIdRaw>>>>;

    static inline std::unordered_map<PropTypeId, std::unordered_map<PropIdRaw, std::set<Stage>>> ptpsq; // Prop type prop stages query
    // SHOULD WE ONLY STORE THIS INFORMATION FOR PARTIAL STATICS OR EVERYTHING???
//     static inline std::unordered_map<PropTypeId, std::unordered_map<Group, std::unordered_map<Instance, std::set<PropIdRaw>>>> ptgid;
    
    struct GlobalPropId {PropTypeId typeId;PropIdRaw id;};
    friend inline bool operator< (const GlobalPropId& lhs, const GlobalPropId& rhs){ return lhs.typeId < rhs.typeId && lhs.id < rhs.id; }


    // Rather ironically, Typeset is a vector because the prop types must be delivered in a certain order to the SunLambda functor despite the conceptual set of props being acted upon
    using Typeset = std::vector<PropTypeId>;
    // All SunLambda and Jolt typeset signatures used (we should only keep track of novel tuples of typesets that are used by gameplay programmers by adding considered typesets in the DefineSunLambda constructor)
    
    static inline std::unordered_map<PropTypeId, std::set<Typeset>> mappedPropTupleTypesets;
    static inline std::set<Typeset> globalPropTupleTypesets;

    // Each typeset stores a map of types to props which have been added and have not been formed into novel tuples of this SunLambda typeset yet
    // Partial static indicators that return true
    static inline std::map<SunLambda::Id, std::unordered_map<PropTypeId, std::vector<PropIdRaw>>> stagingPropTuples;
    
    static inline std::map<SunLambda::Id, std::vector<std::vector<GlobalPropId>>> novelTuples;
    static inline std::unordered_map<SunLambda::Id, Typeset> sunLambdaTypesets;
    static inline std::map<Typeset, std::vector<SunLambda::Id>> typesetSunLambdas;

    template<typename... PTypes>
    static void ConsiderTypeset(SunLambda::Id id)
    {
        Typeset typeset = {BicycleMango::GetPropTypeId<std::decay_t<PTypes>>()...};
        sunLambdaTypesets[id] = typeset;
        typesetSunLambdas[typeset].push_back(id);
        for (PropTypeId ptid : typeset)
        {
            BicycleMango::globalPropTupleTypesets.insert(typeset);
            BicycleMango::mappedPropTupleTypesets[ptid].insert(typeset);
        }
    }
    
    template <typename... PTypes>
    static void CallJolt(void (*functor)(PTypes...), SunLambda::Id id, std::vector<PropIdRaw>& sunData)
    {
        CallJolt<PTypes...>(functor, id, sunData, std::index_sequence_for<PTypes...> {});
        
    }
    
    template <typename... PTypes, std::size_t ... Is>
    static void CallJolt(void (*functor)(PTypes...), SunLambda::Id id, std::vector<PropIdRaw>& sunData, std::index_sequence<Is...> seq)
    {
//         std::cout << "Calling jolt with " << sizeof...(PTypes) << " props and " << sunData.size() << " sun data!" << std::endl;
        functor(GetProps<std::decay_t<PTypes>>()[sunData[Is]]...);
    }

    // Prop ----------------
    
    static inline std::unordered_map<PropTypeId, std::string> propTypeNames;

    template <typename T>
    static constexpr PropTypeId GetPropTypeId()
    {
        const PropTypeId id = std::type_index(typeid(std::decay_t<T>)).hash_code();
        propTypeNames[id] = std::type_index(typeid(std::decay_t<T>)).name();
        return id;
    }
    
    static inline std::unordered_map<Group, IdPool<Instance>> instanceBuffer;
    
    static Stage Next(Group group)
    {
        return {group, instanceBuffer[group].next()};
    }

    template<typename PropType>
    static void AddPropStage(PropId<PropType> propId, const Stage& stage)
    {
        instanceBuffer[stage.group].nextId = stage.instance + 1; // This is kind of a hack, and could cause some large sections of unused ids potentially but I think it will work
        ptpsq[GetPropTypeId<PropType>()][propId.id].insert(stage);
//         ptgid[GetPropTypeId<PropType>()][stage.group][stage.instance].insert(propId.id);
    }

    template<typename PropType>
    static void AddPropStages(PropId<PropType> propId, const GroupSet& stages)
    {
        for(const Stage& stage : stages)
        {
            AddPropStage(propId, stage);
        }
    }
    
    template<typename PropType>
    static PropType* AddProp(const GroupSet& stages)
    {
        auto [id, prop] = GetProps<PropType>().next();

        pidq[(void*)&prop] = id;

        PropId<PropType> propId{id};
        AddPropStages(propId, stages);
        
        auto propTypeId = GetPropTypeId<PropType>();
        std::cout << "--- Adding prop {" << propTypeNames[propTypeId] << ", " << id << "}" << std::endl;
        auto typesetsWithAddedPropType = BicycleMango::mappedPropTupleTypesets[propTypeId];
        
        // ---
        for (auto typeset_it = typesetsWithAddedPropType.begin(); typeset_it != typesetsWithAddedPropType.end(); ++typeset_it)
        {
            for (auto sunlambda_it = typesetSunLambdas[(*typeset_it)].begin();
                 sunlambda_it != typesetSunLambdas[(*typeset_it)].end(); 
                ++sunlambda_it)
            {
        // --- 'for each SunLambda that PropType is a parameter of'
                std::cout << "Checking for novel tuple on SunLambda: " << SunLambdaRegistry::GetInstance().Get((*sunlambda_it)).name << std::endl;
                // Check if a novel tuple is formed with this prop from the staged prop neighbors of each SunLambda that propTypeId is in!
                // Partial statics are not considered as potentialNeighbors because then we'd have to copy the stagingPropTuples vector rather than using a ref
                std::unordered_map<PropTypeId, std::vector<PropIdRaw>>& potentialNeighbors = BicycleMango::stagingPropTuples[(*sunlambda_it)];
                // However, both potential neighbors AND partial statics will be added to compatibleNeighbors later in this function
                std::unordered_map<PropTypeId, std::vector<PropIdRaw>> compatibleNeighbors;
                bool addedPropFulfillsCompatabilityConstraint = false;
                bool novelTupleRuledOut = false;
                
                auto IsPropCompatibleWithSunLambda = [sunlambda_it](PropTypeId ptid, PropIdRaw rid) -> bool
                {
                    std::cout << propTypeNames[ptid] << ", " << "id: " << rid << " on SunLambda: " << SunLambdaRegistry::GetInstance().Get((*sunlambda_it)).name << std::endl;
                    
                    for (const Stage& stage : ptpsq[ptid][rid])
                    {
                        std::cout << "Checking compatability on stage: " << stage << " for {" << propTypeNames[ptid] << ", " << rid << "}" << std::endl;
                        if (novelTupleCreators[(*sunlambda_it)].compatible(ptid, stage))
                        {
                            std::cout << "COMPATIBLE!" << std::endl;
                            return true;
                        }
                    }
                    return false;
                };
                
                // The first thing we need to check is if the prop we're adding fulfills the compatability constraint for this SunLambda, if not then we can short circut everything
                if (IsPropCompatibleWithSunLambda(propTypeId, id))
                {
                    addedPropFulfillsCompatabilityConstraint = true;
                } else
                {
                    novelTupleRuledOut = true;
                    std::cout << "Prop does not fulfill compatability constraint!" << std::endl;
                }
                
                bool isAddedPropPartialStatic = false;
                if (!novelTupleRuledOut)
                {
                    if (novelTupleCreators.count((*sunlambda_it)) > 0)
                    {
                        NovelTupleCreator& creator = novelTupleCreators[(*sunlambda_it)];
                        if (creator.reuseOnStages.count(propTypeId) > 0)
                        {
                            std::cout << "Has stage constraints!" << std::endl;
                            if(creator.reuseOnStages[propTypeId](ptpsq[propTypeId][id]))
                            {
                                isAddedPropPartialStatic = true;
                                size_t i = 0;
                                for (const PropTypeId& ptid : (*typeset_it))
                                {
                                    if (ptid == propTypeId)
                                    {
                                        // I really hope this automatically resizes lol
                                        std::vector<std::vector<PropIdRaw>>& sad = partialStatics[(*sunlambda_it)];
                                        if (sad.size() == 0)
                                        {
                                            std::vector<PropIdRaw> p((*typeset_it).size());
                                            sad.push_back(p);
                                        }
                                        std::cout << sad.size() << std::endl;
                                        sad[0][i] = id; // TODO replace 0, WHICH PARTIAL STATIC TO CHOOSE?? creator.reuseOnStages
                                        
                                    }
                                    i++;
                                    break;
                                }
                            }
                        }
                    }
                }
                
                std::unordered_map<PropTypeId, PropIdRaw> partialStaticNeighbors;
                auto FindPartialStatic = [sunlambda_it, &partialStaticNeighbors, &IsPropCompatibleWithSunLambda](PropTypeId ptid)
                {
                    for (auto& partialStaticVector : partialStatics[(*sunlambda_it)])
                    {
                        for (const PropIdRaw& partialStatic : partialStaticVector)
                        {
                            if (IsPropCompatibleWithSunLambda(ptid, partialStatic))
                            {
                                // TODO: Maybe use a different data structure for efficiency, it depends how many partial statics there could be
                                std::cout << "Found partial static: " << propTypeNames[ptid] << std::endl;
                                partialStaticNeighbors[ptid] = partialStatic;
                                return true;
                            } else
                            {
                                std::cout << "Partial static not compatible!" << std::endl;
                            }
                        }
                    }
                    return false;
                };
                
                if (!novelTupleRuledOut)
                {
                    // We need to make sure that potentialNeighbors has at least one prop of each type in this SunLambda::Id other than the type of prop being added, otherwise we can rule out creating a novel tuple
                    for (PropTypeId ptid : (*typeset_it))
                    {
                        if (ptid != propTypeId && potentialNeighbors[ptid].empty())
                        {
                            if (!FindPartialStatic(ptid))
                            {
                                novelTupleRuledOut = true;
                                break;
                            }
                        }
                    }
                }
                
                if (!novelTupleRuledOut)
                {
                    for (auto neighbor_it = potentialNeighbors.begin(); neighbor_it != potentialNeighbors.end(); ++neighbor_it)
                    {
                        if ((*neighbor_it).first == propTypeId || partialStaticNeighbors.count((*neighbor_it).first)) continue;
                        if (partialStaticNeighbors.count((*neighbor_it).first)) continue; // If there is a partial static neighbor, let's just use that :)
                        for (auto& neighborId : (*neighbor_it).second)
                        {
//                                 std::cout << "Let's go " << propTypeNames[(*neighbor_it).first] << " has " << (*neighbor_it).second.size() << std::endl;
                            if (IsPropCompatibleWithSunLambda((*neighbor_it).first, neighborId))
                            {
                                std::cout << "Found it!" << std::endl;
                                compatibleNeighbors[(*neighbor_it).first].push_back(neighborId);
                                break; // Not sure what to do if there are multiple compatible neighbors, for now we just FIFO after considering compatability
                            }
                        }
                        if (compatibleNeighbors[(*neighbor_it).first].empty()) 
                        {
                            std::cout << "Compatible neighbors is empty on " << propTypeNames[(*neighbor_it).first] << " while the prop type being added is " << propTypeNames[propTypeId] << std::endl;
                            novelTupleRuledOut = true;
                            break; // No compatible neighbors of ptid to pair with
                        }
                    }
                }
                // Compatible neighbors is now populated with props that are compatible with (*sunlambda_it)
                std::vector<GlobalPropId> novelTuple((*typeset_it).size());
                
                int addedPropTypeIndex = 0;
                if (!novelTupleRuledOut)
                {
                    int i = 0;
                    for (const PropTypeId& ptid : (*typeset_it))
                    {
                        if (ptid == propTypeId) 
                        {
                            addedPropTypeIndex = i;
                            continue;
                        }
                        if (partialStaticNeighbors.count(ptid))
                        {
                            novelTuple[i] = {ptid, partialStaticNeighbors[ptid]};
                        } else
                        {
                            novelTuple[i] = {ptid, compatibleNeighbors[ptid].back()};
                            potentialNeighbors[ptid].pop_back();
                        }
                        i++;
                    }
                    
                    std::cout << "Formed novel tuple of " << SunLambdaRegistry::GetInstance().Get((*sunlambda_it)).name << " with the following props:" << std::endl;
                    
                    
                    // The prop that we're adding should be included in the novel tuple! Hooray!
                    novelTuple[addedPropTypeIndex] = {propTypeId, id};
                    // The prop that we're adding should be removed from potential neighbors
    //                 std::cout << "Inserting novel tuple of " << novelTuple.size() << " size" << std::endl;
                    novelTuples[(*sunlambda_it)].push_back(novelTuple);
                    
                    // This could be optimized with an emerge only sunLambdaTypesets data structure
                    for (auto emergesun_it = emerges.begin(); emergesun_it != emerges.end(); ++emergesun_it)
                    {
                        if (sunLambdaTypesets[(*emergesun_it)] == (*typeset_it))
                        {
                            std::vector<PropIdRaw> sunData(novelTuple.size()); // The SunLambda already knows the types in order, therefore we only need pass it the PropIdRaw values and it can imply the types
                            for (GlobalPropId& gpid : novelTuple)
                                sunData.push_back(gpid.id);
                            SunLambda& sun = SunLambdaRegistry::GetInstance().Get(*(emergesun_it));
                            sun.Emerge(sunData);
                            break;
                        }
                    }
                    
                } 
                else
                {
                    if (addedPropFulfillsCompatabilityConstraint && !isAddedPropPartialStatic)
                    {
                        std::cout << "Staging " << propTypeNames[propTypeId] << ": " << id << std::endl;
                        BicycleMango::stagingPropTuples[(*sunlambda_it)][propTypeId].push_back(id);
                    }
                }
            } // --- end sunlambda_it
        } // --- end typeset_it

        return &prop;
    }

    // Props should only be removed by stage rather than considering type
    static void RemovePropsWithStage(Stage stage)
    {
        // Find all props with the stage
        // Remove all novel tuples that contain at least one of these props
        // Restage
    }
    
    // Let's start over
    static void ResetProps()
    {
        partialStatics.clear();
        ptpsq.clear();
        stagingPropTuples.clear();
        novelTuples.clear();
        instanceBuffer.clear();
    }
    
    static void ResetSunLambdas()
    {
        // DO NOT CLEAR REGISTRY OR TYPESETS
        breakups.clear();
        emerges.clear();
        schedules.clear();
        novelTupleCreators.clear();
    }
    
    static void Reset()
    {
        ResetProps();
        ResetSunLambdas();
    }
    
    template <typename PropType>
    static void Singleton(SunLambda::Id sunLambdaId)
    {
        BicycleMango::novelTupleCreators[sunLambdaId].reuseOnStages[BicycleMango::GetPropTypeId<PropType>()] = 
                [](std::set<Stage>&){return true;};
    }

    template <typename PropType>
    static PropType& GetProp(PropId<PropType> id)
    {
        return GetProps[id.id];
    }

    template <typename PropType>
    static HoleVector<PropType>& GetProps()
    {
        static HoleVector<PropType> props;
        return props;
    }

    template <typename PropType>
    static PropType* GetProp()
    {
        for(auto& prop : GetProps<PropType>())
        {
            return &prop;
        }

        return nullptr;
    }

private:

    template <typename ... PTypes, std::size_t ... Is>
    static auto IterateProps(void (*functor)(PTypes...), const SunLambda::Id& id, std::index_sequence<Is...> seq)
    {
        for (auto& tuple : novelTuples[id])
        {
            functor(GetProps<std::decay_t<PTypes>>()[tuple[Is].id]...);
        }
    }

public:

    template <typename ... PTypes>
    static auto IterateProps(void (*functor)(PTypes...), const SunLambda::Id& id)
    {
        return IterateProps<PTypes...>(functor, id, std::index_sequence_for<PTypes...> {});
    }
};



